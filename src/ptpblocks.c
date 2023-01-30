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

#define VM 0
#define VS 1
#define VB 'b'

int verbose = 0;

int showBlocks = 1;
int showRam = 1;
int showType = 1;
int showName = 1;
int showFilename = 1;
int showMachineType = 1;
int showAutostart = 1;
int showFirstLoadAddr = 1;
int basicList = 0;

int tapeType = 0; // 0x83:BASIC, 0x87:BASIC DATA
char programname[17] = "";
int is_autostart = 0; // is 0xB9 block
uint16_t first_load_address = 0; // Az első betöltött 0xF9 blokk kezdőcíme
uint16_t autostart_address = 0; // If tapeType == 0xF9 && blockType = 0xB9

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
            return size - 3;
        } else {
            fprintf( stderr, "Invalid .ptp file format. Bad first block character: 0x%02X.\n", byte );
            exit(1);
        }
    }
}

int get_programname( FILE *ptp, unsigned char block_counter, TapeBlockType *currTapeBlock ) {
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
        tape_block_size = get_programname( ptp, blockIndex, currTapeBlock );
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

unsigned char get_next_ptp_block( FILE *ptp, int ptp_file_counter, int ptp_block_counter, int *memorySize, TapeBlockType *currTapeBlock ) { // Egy ptp blok felolvasása
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
        printf( "File\tTape\tType\tName\tCnt\tClose\t>LoadH\t>SizeH\t>RunH\n" );
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
    printf( "%s\t%d.\t0x%02X\t%s\t%d\t%d\t%4X\t%X\t%X\n", curfile_printed ? "" : curfile, file_index, bigTapeBlock.type, type_name, bigTapeBlock.tape_block_counter, bigTapeBlock.close_counter, bigTapeBlock.load_address, bigTapeBlock.byte_counter, bigTapeBlock.run_address );
    curfile_printed = 1;
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
            for( int i=0; i<currTapeBlock.byte_counter; i++ ) lastBigTapeBlock.bytes[ ++lastBigTapeBlock.byte_counter ] = currTapeBlock.bytes[ i ];
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
    int ptpBlockSumSize = get_ptp_file_size( ptp, file_index );
    if ( ptpBlockSumSize <= 0 ) { // EOF
        return 0;
    } else {
        int memorySize = 0;
        int ptpBlockCounter = 1;
        TapeBlockType currTapeBlock;
        unsigned char ptpBlockType = get_next_ptp_block( ptp, file_index, ptpBlockCounter, &memorySize, &currTapeBlock );
        TapeBlockType lastBigTapeBlock = currTapeBlock;
        while( ptpBlockType != 0xAA ) {
            ptpBlockCounter++;
            ptpBlockType = get_next_ptp_block( ptp, file_index, ptpBlockCounter, &memorySize, &currTapeBlock );
            lastBigTapeBlock = add_new_tape_block( file_index, lastBigTapeBlock, currTapeBlock );
        }
        show_block_info( file_index, lastBigTapeBlock );
        return ptpBlockCounter;
    }
}

void ptp_fs_file_info( char *ptpFilenameWithPath ) {
    FILE *ptp = 0;
    if ( ptp = fopen( ptpFilenameWithPath, "rb" ) ) {
        curfile = ptpFilenameWithPath;
        curfile_printed = 0;
        int memorySize = 0;
        int ptpBlockCounter = 0; // Összesen ennyi ptp block van
        int currPtpBlockCounter = 0; // Az aktuális fájlben ennyi ptp block van
        for( int file_counter = 1; currPtpBlockCounter = read_ptp_file( ptp, file_counter, &memorySize ); file_counter++ ) {
            ptpBlockCounter += currPtpBlockCounter;
        }
        fclose( ptp );
    } else {
        fprintf( stderr, "Error opening %s.\n", ptpFilenameWithPath );
        exit(4);
    }
}

void print_usage() {
    printf( "ptpinfo v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "View Primo .ptp file informations.\n");
    printf( "Copyright 2022 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptpinfo -i <ptp_filename>\n");
    printf( "-R            : Show RAM size only.\n");
    printf( "-b            : Show blocks counter only.\n");
    printf( "-t            : Show type only (BASIC/SYSTEM).\n");
    printf( "-T            : Show machine type only (A32/A48/A64).\n");
    printf( "-f            : Show filename with path only.\n");
    printf( "-F            : Show First load address only.\n");
//    printf( "-l            : List BASIC program.\n");
    printf( "-n            : Show program name only.\n");
    printf( "-a            : Show autostart address only.\n");
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
    int all = 1;
    showBlocks = 0; // b
    showRam = 0; // R
    showType = 0; // t
    showName = 0; // n
    showFilename = 0; // f
    showMachineType = 0; // T
    showAutostart = 0;
    showFirstLoadAddr = 0;
    while ( ( opt = getopt (argc, argv, "lbRtnafFTv?h:i:") ) != -1 ) {
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
            case 'b': showBlocks = 1;      all=0; break; // b
            case 'l': basicList = 1;       all=0; break; // b
            case 'R': showRam = 1;         all=0; break; // R
            case 't': showType = 1;        all=0; break; // t
            case 'n': showName = 1;        all=0; break; // n
            case 'f': showFilename = 1;    all=0; break; // f
            case 'T': showMachineType = 1; all=0; break; // T
            case 'a': showAutostart = 1;   all=0; break; // a
            case 'F': showFirstLoadAddr = 1;   all=0; break; // a
            case 'i': // open ptp file
                if ( is_dir( optarg ) ) { // Input is a direcory
                    ptpDir = copyStr( optarg, 0 );
                } else {
                    ptpFilename = copyStr( optarg, 0 );    
                }
                break;
        }
    }
    if ( all ) {
        showBlocks = 1; // b
        showRam = 1; // R
        showType = 1; // t
        showName = 1; // n
        showFilename = 1; // f
        showMachineType = 1; // T
        showAutostart = 1; // a
        showFirstLoadAddr = 1; // F
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
