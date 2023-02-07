/**
 * Primo .ptp blokinformációkat jelenít meg
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include <libgen.h>
#include "lib/fs.h"
#include "lib/basic.h"
#include "tlib/gifenc.h"

#define VM 0
#define VS 1
#define VB 'b'

int verbose = 0;
int dump = 0; // Ha 0, akkor nincs dump, különben 1-től a dumpéok indexe
int decode_basic = 0; // If 1, the basic block decoded to txt
int create_gif = 0; // If 1 create a gif
int utf8 = 0;
char *dump_name_prefix = "block";

/*****************************************************************************************************************************
 *** GIF
 *****************************************************************************************************************************/
void create_gif_file( const char *fn, unsigned char* pixels, uint16_t size ) {
    int w = 256, h = size*8/w;
    /* create a GIF */
    ge_GIF *gif = ge_new_gif(
        fn,  /* file name */
        w, h,       /* canvas size */
        ( uint8_t [] ) {  /* palette */
            0x00, 0x00, 0x00, /* 0 -> black */
            0xFF, 0xFF, 0xFF, /* 1 -> white */
        },
        1,              /* palette depth == log2(# of colors) */
        -1,             /* no transparency */
        0               /* infinite loop */
    );
    /* draw some frames */
    for( int pixel=0; pixel<w*h; pixel+=8 ) {
        int bitV = 128;
        for( int bit=0; bit<8; bit++ ) {
            gif->frame[ pixel+bit ] = ( pixels[ pixel/8 ] & bitV ) ? 1 : 0;
            bitV /= 2;
        }
    }
    ge_add_frame( gif, 10 );
    /* remember to close the GIF */
    ge_close_gif(gif);
}
/*****************************************************************************************************************************
 *** BASIC
 *****************************************************************************************************************************/
void create_basic_list( const char *fn, const unsigned char* bytes, uint16_t size ) {
    FILE *f = fopen( fn, "wb" );
    while( ( bytes[0] || bytes[1] ) && size ) {
        BASIC_LINE bl = decode_basic_line( bytes, size, utf8 );
        fprintf( f, "%d %s\n", bl.line_number, bl.text_line );
        bytes += bl.bin_line_full_length;
        size -= bl.bin_line_full_length;
    }
    fclose( f );
}

/*****************************************************************************************************************************
 *** TAPE
 *****************************************************************************************************************************/
/*
 * Tape blokktípusok:
 * Név blokkok
 *   0x83 - Programnév BASIC vagy gépikódú
 *   0x87 - Adatállomány neve
 * Adat blokkok
 *   0xF1 - BASIC program
 *   0xF5 - Képernyő
 *   0xF7 - BASIC adat
 *   0xF9 - Gépikódú program
 * Záró blokk
 *   0xB1 - BASIC vagy gépikódú program vége
 *   0xB5 - Képernyőkép vége
 *   0xB7 - BASIC adatállomány vége
 *   0xB9 - Autostart gépikódú program vége
 */

typedef struct {
    unsigned char type;     // Tape block type
    int tape_block_counter; // Hány adatblokkban tárolódik a szallagon
    int close_counter;      // Hány lezáróblokk van benne
    uint16_t load_address;  // A betöltés kezőcíme
    uint16_t byte_counter;  // A betöltendő bájtok száma összesen a kezdőcímtől folytonosan
    uint16_t run_address;   // A betöltés után erre a címre kell ugrani, vagy 0
    unsigned char bytes[ 65536 ];   // A memóriába betöltendő adatok vagy 0
} TapeBlockType;


void fblockread( void *bytes, size_t size, FILE *src ) {
    int pos = ftell( src );
    int ret = fread( bytes, size, 1, src );
    if ( ret != 1 ) {
        fprintf( stderr, "File block read error 2 at pos 0x%04X\n", pos );
        exit;
    }
    if ( feof( src ) ) {
        fprintf( stderr, "File block read error at pos 0x%04X\n", pos );
        exit(1);
    }
}

int get_ptp_file_size( FILE *ptp, int file_index ) {
    unsigned char byte = fgetc( ptp );
    if ( feof( ptp ) ) {
        return -1;
    } else {
        if ( byte == 0xFF ) { // Ok
            uint16_t size;
            fblockread( &size, 2, ptp );
            if ( verbose ) fprintf( stdout, "%d. ptp file size: %d\n", file_index, size );
            return size;
        } else {
            fprintf( stderr, "Invalid .ptp file format. Bad first block character: 0x%02X.\n", byte );
            exit(1);
        }
    }
}

int get_programname_tape_block( FILE *ptp, unsigned char block_counter, TapeBlockType *currTapeBlock ) {
    if ( block_counter != 0 ) {
        fprintf( stderr, "HIBA! Névblokk csak a 0. blokk lehet!\n" );
        exit(1);
    }
    unsigned char namelength = fgetc( ptp );
    if ( namelength > 16 ) {
        fprintf( stderr, "HIBA! A név maximum 16 karakter lehet!\n" );
        exit(1);
    }
    currTapeBlock->byte_counter = namelength;
    for( int i=0; i<namelength; i++ ) currTapeBlock->bytes[ i ] = fgetc( ptp );
    currTapeBlock->bytes[ namelength ] = 0;
    if ( verbose ) fprintf( stdout, "    Tape stored name: '%s'\n", currTapeBlock->bytes ); // Itt helyes a ptp_block_size? Nem!!!
    return namelength + 4;
}

int get_next_tape_block( FILE *ptp, TapeBlockType *currTapeBlock ) {
    unsigned char tapeBlockType = fgetc( ptp );
    unsigned char blockIndex = fgetc( ptp );
    int tape_block_size = 0;
    currTapeBlock->type = tapeBlockType;
    currTapeBlock->tape_block_counter = 1;
    currTapeBlock->load_address = 0;
    currTapeBlock->byte_counter = 0;
    currTapeBlock->run_address = 0;
    currTapeBlock->close_counter = 0;
    if ( verbose ) fprintf( stdout, "  Tape block type: 0x%02X, index=%02x\n", tapeBlockType, blockIndex ); // Itt helyes a ptp_block_size? Nem!!!
    if ( tapeBlockType == 0x83 || tapeBlockType == 0x87 ) {
        tape_block_size = get_programname_tape_block( ptp, blockIndex, currTapeBlock );
    } else if ( tapeBlockType == 0xF1 || tapeBlockType == 0xF7 || tapeBlockType == 0xF5 || tapeBlockType == 0xF9 ) {
        unsigned char loadAddressL = fgetc( ptp ); // Absolute if 0xF9 else relative
        unsigned char loadAddressH = fgetc( ptp ); // Absolute if 0xF9 else relative
        unsigned char dataCounter = fgetc( ptp );
        currTapeBlock->load_address = loadAddressL + 256*loadAddressH;
        currTapeBlock->byte_counter = dataCounter ? dataCounter : 256;
        currTapeBlock->run_address = 0;
        for( int i=0; i<currTapeBlock->byte_counter; i++ ) currTapeBlock->bytes[i] = fgetc( ptp );
        if ( verbose ) fprintf( stdout, "    Load address: 0x%04X\n", loadAddressL + 256 * loadAddressH ); // Itt helyes a ptp_block_size? Nem!!!
        tape_block_size = currTapeBlock->byte_counter + 6;
    } else if ( tapeBlockType == 0xB1 || tapeBlockType == 0xB5 || tapeBlockType == 0xB7 ) { // BASIC program, adat vagy képernyőtartalom vége
        currTapeBlock->close_counter = 1;
        tape_block_size = 3;
    } else if ( tapeBlockType == 0xB9 ) { // Auto start address block
        currTapeBlock->close_counter = 1;
        fblockread( &currTapeBlock->run_address, 2, ptp );
        tape_block_size = 5;
    } else {
        fprintf( stderr, "Invalid tape block type: 0x%02X.\n", tapeBlockType );
        exit(2);
    }
    unsigned char crc = fgetc( ptp );
    return tape_block_size;
}

unsigned char get_next_ptp_block( FILE *ptp, int ptp_file_counter, int ptp_block_counter, int *memorySize, TapeBlockType *currTapeBlock, int *ptp_block_sum_size ) { // Egy ptp blok felolvasása
    unsigned char ptpBlockType = fgetc( ptp );
    if ( ptpBlockType == 0x55 || ptpBlockType == 0xAA ) {
        uint16_t ptp_block_size;
        fblockread( &ptp_block_size, 2, ptp );
        if ( verbose ) fprintf( stdout, " %d. ptp file %d. ptp block found. Type: 0x%02X, block size: %d\n", ptp_file_counter, ptp_block_counter, ptpBlockType, ptp_block_size );
        int pos = ftell( ptp );
        int readed_tape_block_sum_size = 0;
        while( readed_tape_block_sum_size < ptp_block_size ) { // Egy ptpt blokkon belül több tape blokk is lehet
            readed_tape_block_sum_size += get_next_tape_block( ptp, currTapeBlock );
        }
        *ptp_block_sum_size += readed_tape_block_sum_size + 3;
    } else {
        fprintf( stderr, "Invalid .ptp block type: 0x%02X.\n", ptpBlockType );
        exit(2);
    }
    return ptpBlockType;
}

int is_header = 0;

char *curfile;
int curfile_printed = 0; // Ha 1, akkor már kiírtuk

void show_block_info( int file_index, TapeBlockType bigTapeBlock ) {
    if ( !is_header ) {
        printf( "File\tTape\tType\tName\tCnt\tClose\t>LoadH\t>SizeH\t>RunH\t>TopH\t>A32Free\t>A48Free\t>A64Free\n" );
        is_header = 1;
    }
    char type_name[20] = "";
    switch( bigTapeBlock.type ) {
        case 0x83 : strcpy( type_name, "PrgName" ); break;
        case 0xF1 : strcpy( type_name, "BASIC" ); break;
        case 0xF5 : strcpy( type_name, "Screen" ); break;
        case 0xF7 : strcpy( type_name, "DATA" ); break;
        case 0xF9 : strcpy( type_name, "SYSTEM" ); break;
        case 0xB9 : strcpy( type_name, "SysStart" ); break; // Lehet, hogy a system indító blokk a legvégén van, külön a system blokktól, mivel csak a teljes betöltés után akarja indítani
        default: strcpy( type_name, "???" ); break;
    }
    uint16_t run_address = ( bigTapeBlock.type == 0xF1 ) ? BASIC_START : bigTapeBlock.run_address;
    uint16_t load_address = ( bigTapeBlock.type == 0xF1 ) ? bigTapeBlock.load_address + BASIC_START : bigTapeBlock.load_address;
    uint16_t top_address = 0;
    if ( run_address && load_address ) top_address = load_address + bigTapeBlock.byte_counter;
    printf( "%s\t%d.\t0x%02X\t%s\t%d\t%d\t%4X\t%X\t%X\t%X\t%X\t%X\t%X\n", curfile_printed ? "" : curfile, file_index, bigTapeBlock.type, type_name, bigTapeBlock.tape_block_counter, bigTapeBlock.close_counter, load_address, bigTapeBlock.byte_counter, run_address, top_address, top_address?0x6800-top_address:0, top_address?0xA800-top_address:0, top_address?0xE800-top_address:0 );
    curfile_printed = 1;
    if ( dump ) {
        char dumpname[100];
        switch( bigTapeBlock.type ) {
            case 0X83 : sprintf( dumpname, "%s.block.%d.%d.pnm", dump_name_prefix, file_index, dump++, bigTapeBlock.load_address ); break;
            case 0XF1 : sprintf( dumpname, "%s.block.%d.%d.L%04XH.bas", dump_name_prefix, file_index, dump++, bigTapeBlock.load_address ); break;
            case 0XF5 : sprintf( dumpname, "%s.block.%d.%d.L%04XH.scr", dump_name_prefix, file_index, dump++, bigTapeBlock.load_address ); break;
            case 0XF7 : sprintf( dumpname, "%s.block.%d.%d.L%04XH.dat", dump_name_prefix, file_index, dump++, bigTapeBlock.load_address ); break;
            case 0XF9 : sprintf( dumpname, "%s.block.%d.%d.L%04XH.sys", dump_name_prefix, file_index, dump++, bigTapeBlock.load_address ); break;
            case 0XB9 : sprintf( dumpname, "%s.block.%d.%d.L%04XH.run", dump_name_prefix, file_index, dump++, bigTapeBlock.load_address ); break;
            default: sprintf( dumpname, "%s.block.%d.%d.0x%02X.bin", dump_name_prefix, file_index, dump++, bigTapeBlock.type );
        }
        if ( bigTapeBlock.type == 0xF5 && create_gif ) {
            int ei = strlen( dumpname )-3;
            dumpname[ ei++ ] = 'g';
            dumpname[ ei++ ] = 'i';
            dumpname[ ei++ ] = 'f';
            create_gif_file( dumpname, bigTapeBlock.bytes, bigTapeBlock.byte_counter );
        } else if ( bigTapeBlock.type == 0xF1 && decode_basic ) {
            int ei = strlen( dumpname )-3;
            dumpname[ ei++ ] = 't';
            dumpname[ ei++ ] = 'x';
            dumpname[ ei++ ] = 't';
            create_basic_list( dumpname, bigTapeBlock.bytes, bigTapeBlock.byte_counter );
        } else {
            FILE * d = fopen( dumpname, "wb" );
            for( int i=0; i<bigTapeBlock.byte_counter; i++) fputc( bigTapeBlock.bytes[i], d );
            fclose( d );
        }
    }
}

int is_close_type_for( unsigned char main, unsigned char next ) {
    switch( main ) {
        case 0xF1 : return next == 0xB1; break;
        case 0xF5 : return next == 0xB5; break;
        case 0xF7 : return next == 0xB7; break;
        case 0xF9 : return next == 0xB1 || next == 0xB9; break;        
        default: return 0; break;
    }
}

TapeBlockType add_new_tape_block( int file_index, TapeBlockType lastBigTapeBlock, TapeBlockType currTapeBlock ) {
    if ( currTapeBlock.type == lastBigTapeBlock.type || is_close_type_for( lastBigTapeBlock.type, currTapeBlock.type ) ) {
        int is_last_block = is_close_type_for( lastBigTapeBlock.type, currTapeBlock.type  );
        if ( is_last_block || lastBigTapeBlock.load_address + lastBigTapeBlock.byte_counter == currTapeBlock.load_address ) {
            for( int i=0; i<currTapeBlock.byte_counter; i++ ) lastBigTapeBlock.bytes[ lastBigTapeBlock.byte_counter++ ] = currTapeBlock.bytes[ i ];
            lastBigTapeBlock.tape_block_counter++;
            if ( lastBigTapeBlock.run_address ) printf( "Error: Run address in block????" );
            lastBigTapeBlock.run_address = currTapeBlock.run_address;
            lastBigTapeBlock.close_counter += currTapeBlock.close_counter;
        } else {
            show_block_info( file_index, lastBigTapeBlock );
            lastBigTapeBlock = currTapeBlock;
        }
    } else {
        show_block_info( file_index, lastBigTapeBlock );
        lastBigTapeBlock = currTapeBlock;
    }
    return lastBigTapeBlock;
}

int read_ptp_file( FILE *ptp, int file_index, int *memorySize ) { // Egy ptp szallagállomány beolvasása. Nem feltétlen egy fizikai ptp fájl! Annak lehet egy része is!
    int ptpFileSize = get_ptp_file_size( ptp, file_index );
    if ( ptpFileSize <= 0 ) { // EOF
        return 0;
    } else {
        int memorySize = 0;
        int ptpBlockCounter = 1;
        int ptp_block_sum_size = 3;
        TapeBlockType currTapeBlock;
        unsigned char ptpBlockType = get_next_ptp_block( ptp, file_index, ptpBlockCounter, &memorySize, &currTapeBlock, &ptp_block_sum_size );
        TapeBlockType lastBigTapeBlock = currTapeBlock;
        while( ptpBlockType != 0xAA ) {
            ptpBlockCounter++;
            ptpBlockType = get_next_ptp_block( ptp, file_index, ptpBlockCounter, &memorySize, &currTapeBlock, &ptp_block_sum_size );
            lastBigTapeBlock = add_new_tape_block( file_index, lastBigTapeBlock, currTapeBlock );
        }
        show_block_info( file_index, lastBigTapeBlock );
        if ( ptpFileSize != ptp_block_sum_size ) {
            printf( "Invalid ptp file size! (%d != %d)\n", ptpFileSize, ptp_block_sum_size );
            exit(1);
        }
        return ptpBlockCounter;
    }
}

void ptp_fs_file_info( char *ptpFilenameWithPath ) {
    FILE *ptp = 0;
    if ( ptp = fopen( ptpFilenameWithPath, "rb" ) ) {
        dump_name_prefix = ptpFilenameWithPath;
        curfile = ptpFilenameWithPath;
        curfile_printed = 0;
        int memorySize = 0;
        int ptpBlockCounter = 0; // Összesen ennyi ptp block van
        int currPtpBlockCounter = 0; // Az aktuális fájlben ennyi ptp block van
        for( int file_counter = 1; currPtpBlockCounter = read_ptp_file( ptp, file_counter, &memorySize ); file_counter++ ) {
            if ( dump ) dump = 1; // inicialize dump counter for nes file block
            ptpBlockCounter += currPtpBlockCounter;
        }
        fclose( ptp );
    } else {
        fprintf( stderr, "Error opening %s.\n", ptpFilenameWithPath );
        exit(4);
    }
}

void print_usage() {
    printf( "ptpblocks v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "View Primo .ptp filesystem file block informations.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptpblocks -i <ptp_filename>\n");
    printf( "-g            : GIF screen block dumps. If use this option, the dump create GIF image instead of binary screen memory dump.\n");
    printf( "-t            : TXT BASIC block dumps. If use this option, the dump print BASIC source code instead of binary BASIC memory dump.\n");
    printf( "-u            : Use utf8 for BASIC decoding.\n");
    printf( "-d            : Binary dump tape blocks content.\n");
    printf( "-v            : Verbose output.\n");
    exit(1);
}

void ptp_dir_info( char *ptpDir ) {
    // FILE *ptpFile = 0;
    struct dirent *pDirent;
    DIR *pDir = opendir ( ptpDir );
    if ( pDir == NULL ) {
        printf ("Cannot open directory '%s'\n", ptpDir );
        exit(5);
    }
    while ( ( pDirent = readdir(pDir) ) != NULL ) {
        if ( is_ext( pDirent->d_name, ".ptp" ) ) {
            char* ffn = copyStr3( ptpDir, "", pDirent->d_name );
            ptp_fs_file_info( ffn );
        }
    }
    closedir (pDir);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    char *ptpDir = 0;
    char *ptpFilename = 0;
    while ( ( opt = getopt (argc, argv, "utgdv?h:i:") ) != -1 ) {
        switch ( opt ) {
            case -1:
            case ':':
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'v':
                verbose = 1;
                break;
            case 'd':
                dump = 1;
                break;
            case 't':
                decode_basic = 1;
                break;
            case 'u':
                utf8 = 1;
                break;
            case 'g':
                create_gif = 1;
                break;
            case 'i': // open ptp file
                if ( is_dir( optarg ) ) { // Input is a direcory
                    ptpDir = copyStr( optarg, 0 );
                } else {
                    ptpFilename = copyStr( optarg, 0 );    
                }
                break;
        }
    }
    if ( ptpDir ) {
        ptp_dir_info( ptpDir );
    } else if ( ptpFilename ) {
        ptp_fs_file_info( ptpFilename );
    } else {
        print_usage();
    }
    return 0;
}
