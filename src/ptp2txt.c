/**
 * Primo .ptp BASIC program list
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
#define VS 2
#define VB 'b'

int verbose = 0;
int utf8 = 0; // Ha 1, akkor uft-8 karakterré konvertálódnak az ékezetes karakterek

const unsigned char tokens[] = { 0xc5, 0x4e, 0x44, 0xc6, 0x4f, 0x52, 0xd2, 0x45, 0x53, 0x45, 0x54, 0xd3, 0x45, 0x54, 0xc3, 0x4c, 0x53, 0xc3, 0x4d, 0x44, 0xd2, 0x41, 0x4e, 0x44, 0x4f, 0x4d, 0xce, 
0x45, 0x58, 0x54, 0xc4, 0x41, 0x54, 0x41, 0xc9, 0x4e, 0x50, 0x55, 0x54, 0xc4, 0x49, 0x4d, 0xd2, 0x45, 0x41, 0x44, 0xcc, 0x45, 0x54, 0xc7, 0x4f, 0x54, 0x4f, 0xd2, 0x55, 0x4e, 0xc9, 0x46, 0xd2, 0x45, 
0x53, 0x54, 0x4f, 0x52, 0x45, 0xc7, 0x4f, 0x53, 0x55, 0x42, 0xd2, 0x45, 0x54, 0x55, 0x52, 0x4e, 0xd2, 0x45, 0x4d, 0xd3, 0x54, 0x4f, 0x50, 0xc5, 0x4c, 0x53, 0x45, 0xd4, 0x52, 0x4f, 0x4e, 0xd4, 0x52, 
0x4f, 0x46, 0x46, 0xc4, 0x45, 0x46, 0x53, 0x54, 0x52, 0xc4, 0x45, 0x46, 0x49, 0x4e, 0x54, 0xc4, 0x45, 0x46, 0x53, 0x4e, 0x47, 0xc4, 0x45, 0x46, 0x44, 0x42, 0x4c, 0xc2, 0x45, 0x45, 0x50, 0xc5, 0x44, 
0x49, 0x54, 0xc5, 0x52, 0x52, 0x4f, 0x52, 0xd2, 0x45, 0x53, 0x55, 0x4d, 0x45, 0xcf, 0x55, 0x54, 0xcf, 0x4e, 0xcf, 0x50, 0x45, 0x4e, 0xc6, 0x49, 0x45, 0x4c, 0x44, 0xc7, 0x45, 0x54, 0xd0, 0x55, 0x54, 
0xc3, 0x4c, 0x4f, 0x53, 0x45, 0xcc, 0x4f, 0x41, 0x44, 0xcd, 0x45, 0x52, 0x47, 0x45, 0xd4, 0x45, 0x53, 0x54, 0xcb, 0x49, 0x4c, 0x4c, 0xc3, 0x52, 0x45, 0x41, 0x54, 0x45, 0xe6, 0x6e, 0xd3, 0x41, 0x56,
0x45, 0xd3, 0x43, 0x52, 0x45, 0x45, 0x4e, 0xcc, 0x50, 0x52, 0x49, 0x4e, 0x54, 0xc4, 0x45, 0x46, 0xd0, 0x4f, 0x4b, 0x45, 0xd0, 0x52, 0x49, 0x4e, 0x54, 0xc3, 0x4f, 0x4e, 0x54, 0xcc, 0x49, 0x53,
0x54, 0xcc, 0x4c, 0x49, 0x53, 0x54, 0xc4, 0x45, 0x4c, 0x45, 0x54, 0x45, 0xc1, 0x55, 0x54, 0x4f, 0xc3, 0x4c, 0x45, 0x41, 0x52, 0xc3, 0x4c, 0x4f, 0x41, 0x44, 0xc3, 0x53, 0x41, 0x56, 0x45, 0xce,
0x45, 0x57, 0xd4, 0x41, 0x42, 0x28, 0xd4, 0x4f, 0xc6, 0x4e, 0xd5, 0x53, 0x49, 0x4e, 0x47, 0xd6, 0x41, 0x52, 0x50, 0x54, 0x52, 0xc3, 0x41, 0x4c, 0x4c, 0xc5, 0x52, 0x4c, 0xc5, 0x52, 0x52, 0xd3,
0x54, 0x52, 0x49, 0x4e, 0x47, 0x24, 0xc9, 0x4e, 0x53, 0x54, 0x52, 0xd0, 0x4f, 0x49, 0x4e, 0x54, 0xd4, 0x49, 0x4d, 0x45, 0x24, 0xd0, 0x49, 0xc9, 0x4e, 0x4b, 0x45, 0x59, 0x24, 0xd4, 0x48, 0x45,
0x4e, 0xce, 0x4f, 0x54, 0xd3, 0x54, 0x45, 0x50, 0xab, 0xad, 0xaa, 0xaf, 0x9f, 0xc1, 0x4e, 0x44, 0xcf, 0x52, 0xbe, 0xbd, 0xbc, 0xd3, 0x47, 0x4e, 0xc9, 0x4e, 0x54, 0xc1, 0x42, 0x53, 0xc6, 0x52,
0x45, 0xc9, 0x4e, 0x50, 0xd0, 0x4f, 0x53, 0xd3, 0x51, 0x52, 0xd2, 0x4e, 0x44, 0xcc, 0x4f, 0x47, 0xc5, 0x58, 0x50, 0xc3, 0x4f, 0x53, 0xd3, 0x49, 0x4e, 0xd4, 0x41, 0x4e, 0xc1, 0x54, 0x4e, 0xd0,
0x45, 0x45, 0x4b, 0xc3, 0x56, 0x49, 0xc3, 0x56, 0x53, 0xc3, 0x56, 0x44, 0xc5, 0x4f, 0x46, 0xcc, 0x4f, 0x43, 0xcc, 0x4f, 0x46, 0xcd, 0x4b, 0x49, 0x24, 0xcd, 0x4b, 0x53, 0x24, 0xcd, 0x4b, 0x44,
0x24, 0xc3, 0x49, 0x4e, 0x54, 0xc3, 0x53, 0x4e, 0x47, 0xc3, 0x44, 0x42, 0x4c, 0xc6, 0x49, 0x58, 0xcc, 0x45, 0x4e, 0xd3, 0x54, 0x52, 0x24, 0xd6, 0x41, 0x4c, 0xc1, 0x53, 0x43, 0xc3, 0x48, 0x52,
0x24, 0xcc, 0x45, 0x46, 0x54, 0x24, 0xd2, 0x49, 0x47, 0x48, 0x54, 0x24, 0xcd, 0x49, 0x44, 0x24, 0xa7 };

int primo2Utf( unsigned char character ) { // http://lzsiga.users.sourceforge.net/ekezet.html z
    if ( utf8 ) {
        switch ( character ) { 
            case 0x7D : return 0xC3A1; // | á | C3A1 |
            case 0x5D : return 0xC381; // | Á | C381 |
            case 0x60 : return 0xC3A9; // | é | C3A9 |
            case 0x40 : return 0xC389; // | É | C389 |
            case 0x7C : return 0xC3B6; // | ö | C3B6 |
            case 0x5C : return 0xC396; // | Ö | C396 |
            case 0x5B : return 0xC3B3; // | ó | C3B3 |
    //      case 0x00 : return 0xC593; // | Ó | C393 |
            case 0x7B : return 0xC591; // | ő | C591 |
    //      case 0x00 : return 0xC590; // | Ő | C590 |
            case 0x7E : return 0xC3BC; // | ü | C3BC |
            case 0x5E : return 0xC39C; // | Ü | C39C |
            case 0x1E : return 0xC3AD; // | í | C3AD |
    //      case 0x00 : return 0xC38D; // | Í | C38D |
            case 0x5F : return 0xC3BA; // | ú | C3BA |
    //      case 0x00 : return 0xC39A; // | Ú | C39A |
            case 0x7F : return 0xC5B1; // | ű | C5B1 |
            case 0xF1 : return 0xC5B0; // | Ű | C5B0 |
    //      case 0x00 : return 0xC3A4; // | ä | C3A4 |
    //      case 0x00 : return 0xC384; // | Ä | C384 |
            default : return character;
        }
    } else {
        return character;
    }
}

int line_counter = 0;               // BASIC lines counter
int max_line_counter = 0;           // BASIC lines counter
uint16_t *origi_line_numbers = 0;   // Origi line numbers array
char origi_line_numbers_loaded = 0; // If 1, origi_line_numbers array is loaded
uint16_t renumber_first = 10;
uint16_t renumber_diff = 10;

uint16_t get_new_line_number_by_index0( uint16_t line_number_index0 ) { return renumber_first + line_number_index0 * renumber_diff; }
uint16_t get_new_line_number_by_number( uint16_t line_number ) {
    if ( !origi_line_numbers || !origi_line_numbers_loaded ) {
        fprintf( stderr, "Renumber error 1\n" );
        exit(1);
    }
    int line_number_index0 = 0;
    while( line_number_index0<max_line_counter && origi_line_numbers[ line_number_index0 ] != line_number ) line_number_index0++;
    if ( line_number_index0<max_line_counter ) { // founded
        return get_new_line_number_by_index0( line_number_index0 );
    } else {
        fprintf( stderr, "Line number not found: %d\n", line_number, max_line_counter );
        return line_number;
        exit(1);
    }
}

int tapeType = 0; // 0x83:BASIC, 0x87:BASIC DATA
char programname[17] = "";
unsigned char line_prefix[4];
int line_prefix_length = 0;


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

int get_ptp_block_size( FILE *ptp ) {
    unsigned char byte = fgetc( ptp );
    if ( feof( ptp ) ) {
        if ( verbose ) fprintf( stdout, "End of file\n" );        
        return 0;
    } else {
        if ( byte == 0xFF ) { // Ok
            uint16_t size;
            fblockread( &size, 2, ptp );
            if ( verbose > 1 ) fprintf( stdout, "Ptp block size: %d\n", size );        
            return size - 3;
        } else {
            fprintf( stderr, "Invalid .ptp block format. Bad first block character: 0x%02X.\n", byte );
            exit(1);
        }
    }
}

void store_programname( FILE *ptp ) {
    unsigned char block_counter = fgetc( ptp );
    if ( block_counter != 0 ) {
        fprintf( stderr, "HIBA! Névblokk csak a 0. blokk lehet!\n" );
        exit(1);
    }
    unsigned char namelength = fgetc( ptp );
    if ( namelength > 16 ) {
        fprintf( stderr, "HIBA! A név maximum 16 karakter lehet!\n" );
        exit(1);
    }
    fblockread( &programname, namelength, ptp );
    programname[ namelength ] = 0;
    unsigned char crc = fgetc( ptp );
}

unsigned char is_BASIC_token( unsigned char byte ) { return byte > 127 && byte < 252; }

void print_token( FILE *txt, unsigned char token ) {
    int current_token = 127;
    for( int i=0; i<sizeof( tokens ); i++ ) {
        if ( tokens[ i ] > 127 ) { // Token first char
            current_token++;
            if ( token == current_token ) { // Ez a keresett token első karaktere
                if ( txt ) fprintf( txt, "%c", tokens[i]-128 );
            }
        } else if ( token == current_token ) { // Ez a keresett token egy nem első karaktere
            if ( txt ) fprintf( txt, "%c", tokens[i] );
        }
    }
}

// unsigned long ftell2( FILE* ptp ) { unsigned long pos = ftell( ptp ); return pos-line_prefix_length; }

unsigned char fgetc2( FILE *ptp ) { // Ha van puffer, onnan, ha nincs, akkor file-ból olvas
    if ( line_prefix_length ) {
        unsigned char byte = line_prefix[ 0 ];
        line_prefix_length--;
        for( int i = 1; i <= line_prefix_length; i++ ) line_prefix[ i-1 ] = line_prefix[ i ];
        return byte;
    } else {
        return fgetc( ptp );
    }
}

void print_line_prefix4( FILE *ptp, FILE *txt ) { // A tárolt BASIC sorok mindig egy 2 bájtos címmel és egy 2 bájtos sorszámmal kezdődnek.
    uint16_t next_line_addr = 0;
    uint16_t line_number = 0;
    next_line_addr = fgetc2( ptp ) + 256 * fgetc2( ptp );
    line_number = fgetc2( ptp ) + 256 * fgetc2( ptp );
    if ( origi_line_numbers && !origi_line_numbers_loaded ) { // Pass 2
        origi_line_numbers[ line_counter ] = line_number;
        if ( verbose ) fprintf( stdout, "%d. line number ( %d ) stored\n", line_counter+1, line_number );
    }
    if ( origi_line_numbers_loaded ) line_number = get_new_line_number_by_index0( line_counter ); // Pass 3
    line_counter++;
    if ( txt ) fprintf( txt, "%d ", line_number );
// printf("*** LN:%d\n", line_number );
}

void write_line_number_to( FILE *txt, uint16_t line_number_parameter ) {
    if ( origi_line_numbers_loaded ) {
        fprintf( txt, "%d", get_new_line_number_by_number( line_number_parameter ) );
    } else {
        fprintf( txt, "%d", line_number_parameter );
    }
}

void write_not_line_number_character_to( FILE *txt, unsigned char byte ) {
    uint16_t c = primo2Utf( byte );
    if ( c < 256 ) {
        if ( txt ) fprintf( txt, "%c", c );
    } else {
        if ( txt ) fprintf( txt, "%c%c", c/256, c%256 );
    }
}

#define GOTO 0x8D
#define GOSUB 0x91
#define RESTORE 0x90
#define THEN 0xCA

/**
 * 0x8D - GOTO sorszam(,sorszam)*
 * 0x91 - GOSUB sorszam(,sorszam)*
 * 0x90 - RESTORE [sorszam]
 * 0xCA - THEN sorszam
 */
unsigned char listBytes( FILE *ptp, FILE *txt, uint16_t counter, unsigned char in_line ) {
    static unsigned char last_token = 0;
    static uint16_t last_line_number_parameter = 0;

    counter += line_prefix_length;
    if ( !in_line ) {
        if ( counter >= 4 ) {
            print_line_prefix4( ptp, txt );
            counter -= 4;
        } else if ( counter == 2 ) { // end zeros
            counter = 0;
        } else {
            fprintf( stderr, "Invalid BASIC block structure!\n" );
            exit;
        }
    }
    if ( !counter ) {
// printf( "!!!BAJ\n" );exit(1);
    }
// printf( "*** Begin line. Counter=%d\n", counter );
    for( int i=0; i<counter; i++ ) { // Ez már biztosan soron belül van
        unsigned char byte = fgetc2( ptp );
        if ( byte ) {
            in_line = 1;
            if ( is_BASIC_token( byte ) ) {
                if ( txt ) print_token( txt, byte );
// printf( "*** TOKEN:0x%02X\n", byte );
                last_token = byte;
            } else {
                if ( last_token == GOTO || last_token == GOSUB || last_token == RESTORE || last_token == THEN ) { // GOTO or GOSUB parameter
                    if ( byte >= '0' && byte <= '9' ) { // number
                        last_line_number_parameter = 10 * last_line_number_parameter + byte - '0';
                    } else { // not a number character
                        if ( last_line_number_parameter ) { // end of line number in command
                            if ( txt ) write_line_number_to( txt, last_line_number_parameter );
                            last_line_number_parameter = 0;
                        }
                        if ( txt ) write_not_line_number_character_to( txt, byte );
                        if ( byte != ' ' && byte != ',' ) last_token = 0;
                    }
                } else {
                    if ( txt ) write_not_line_number_character_to( txt, byte );
                }
            }
        } else { // End of basic program line
            if ( last_line_number_parameter ) { // end of line number in command
                if ( txt ) write_line_number_to( txt, last_line_number_parameter );
                last_line_number_parameter = 0;
            }
            if ( txt ) fprintf( txt, "\n" );
            in_line = 0;
            if ( i >= counter-4 ) {
                line_prefix_length = counter - i - 1; // <= 3
                for ( int j=0; j < line_prefix_length; j++ ) {
                    line_prefix[ j ] = fgetc( ptp );
                }
                i = counter;
            } else {
                print_line_prefix4( ptp, txt );
// printf( "*** NextLine. Counter=%d, i=%d\n", counter, i );
                i += 4;
                in_line = 1;
            }
        }
    }
    return in_line;
}

void listBasicBlock( FILE *ptp, FILE *txt ) {
    static unsigned char in_line = 0; // If true, we are in a basic line
    unsigned char blockIndex = fgetc( ptp ); // tape_block_index
    uint16_t relStartAddr = 0;
    fblockread( &relStartAddr, 2, ptp );
    uint16_t size = fgetc( ptp ); // tape block size
    if ( !size ) size = 256;
    // if ( verbose ) fprintf( stdout, "\tBASIC tape block size: %d\n", size );
    in_line = listBytes( ptp, txt, size, in_line );
}

unsigned char get_next_block( FILE *ptp, FILE *txt, int *readedBlocksSize ) {
    unsigned char blockType = fgetc( ptp );
    if ( blockType == 0x55 || blockType == 0xAA ) {
        uint16_t size;
        fblockread( &size, 2, ptp );
        *readedBlocksSize += size + 3;
        if ( verbose > 1 ) fprintf( stdout, "Block size: %d (", size );
        int pos = ftell( ptp );

        unsigned char tapeBlockType = fgetc( ptp );
        if ( verbose > 1 ) {
            switch( tapeBlockType ) {
                case 0x83 : fprintf( stdout, "BASIC programname" ); break;
                case 0x87 : fprintf( stdout, "BASIC DATA filename" ); break;
                case 0xF1 : fprintf( stdout, "BASIC progam" ); break;
                case 0xF5 : fprintf( stdout, "Screen" ); break;
                case 0xF7 : fprintf( stdout, "BASIC DATA" ); break;
                case 0xF9 : fprintf( stdout, "SYSTEM code" ); break;
                case 0xB1 : fprintf( stdout, "BASIC program end" ); break;
                case 0xB5 : fprintf( stdout, "Screen end" ); break;
                case 0xB7 : fprintf( stdout, "BASIC DATA end" ); break;
                case 0xB9 : fprintf( stdout, "SYSTEM code end" ); break;
                default : fprintf( stderr, "\nInvalid block type: 0x%02X\n", tapeBlockType ); exit;
            }
            fprintf( stdout, ")\n" );
        }
        if ( tapeBlockType == 0x83 || tapeBlockType == 0x87 ) { // BASIC PROGRAM NAME OR DATA NAME
            tapeType = tapeBlockType;
            store_programname( ptp );
        } else if ( tapeBlockType == 0xF1 || tapeBlockType == 0xF7 || tapeBlockType == 0xF9 ) { // F1 : BASIC PROGRAM BLOCK
            if ( tapeBlockType == 0xF1 ) {
                listBasicBlock( ptp, txt );
            } else {
                fprintf( stderr, "Nem BASIC programblock! (Block type = 0x%02X)\n", tapeBlockType );
            }
        }

        fseek( ptp, pos+size, SEEK_SET );
    } else {
        fprintf( stderr, "Invalid .ptp block type: 0x%02X.\n", blockType );
        exit(2);
    }
    return blockType;
}

void ptp_list( FILE *ptp, FILE *txt ) {
    fseek( ptp, 0, SEEK_SET );
    line_prefix_length = 0;
    line_counter = 0;
    int ptpBlockSumSize = get_ptp_block_size( ptp );
    int readedBlocksSize = 0;
    int blockCounter = 1;
    unsigned char blockType = get_next_block( ptp, txt, &readedBlocksSize );
    while( ( blockType != 0xAA ) && ptpBlockSumSize ) {
        blockCounter++;
        if ( ptpBlockSumSize == readedBlocksSize ) {
            ptpBlockSumSize = get_ptp_block_size( ptp );
            readedBlocksSize = 0;
        }
        if ( ptpBlockSumSize ) get_next_block( ptp, txt, &readedBlocksSize );
    }
}

void print_usage() {
    printf( "ptp2txt v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "List BASIC program from .ptp file.\n");
    printf( "Copyright 2022 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptp2txt -i <ptp_filename> [ -o <txt_filename> ]\n");
    printf( "-v            : Verbose mode.\n");
    printf( "-u            : Convert characters to utf8.\n");
    printf( "-r            : Renumber the basic program.\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    FILE *ptpFile = 0;
    FILE *txtFile = stdout;
    int renumber = 0;
    while ( ( opt = getopt (argc, argv, "ruv?h:i:o:") ) != -1 ) {
        switch ( opt ) {
            case -1:
            case ':':
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'v': verbose = 1; break;
            case 'u': utf8 = 1; break;
            case 'r': renumber = 10; break;
            case 'i': // open ptp file
                ptpFile = fopen( optarg, "rb" );
                if ( !ptpFile ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
            case 'o': // open txt file
                txtFile = fopen( optarg, "wb" );
                if ( !txtFile ) {
                    fprintf( stderr, "Error creating %s.\n", optarg);
                    exit(4);
                }
                break;
        }
    }
    if ( ptpFile && txtFile ) {
        if ( renumber ) {
            if ( verbose ) fprintf( stdout, "Pass 1 for renumber (counting lines) ...\n" );
            ptp_list( ptpFile, 0 );
            if ( verbose ) fprintf( stdout, "Pass 1 found %d lines\n", line_counter );
            max_line_counter = line_counter;

            if ( verbose ) fprintf( stdout, "Pass 2 for renumber (storing line numbers) ...\n" );
            int origi_line_numbers_array_size = max_line_counter * sizeof( uint16_t );
            origi_line_numbers = malloc( origi_line_numbers_array_size );
            ptp_list( ptpFile, 0 );
            origi_line_numbers_loaded = 1;

            if ( verbose ) fprintf( stdout, "Pass 3 for renumber (renumbering) ...\n" );
            ptp_list( ptpFile, txtFile );
        } else {
            ptp_list( ptpFile, txtFile );
        }
        if ( verbose ) fprintf( stdout, "%d BASIC lines\n", line_counter );
        fclose( ptpFile );
        if ( txtFile != stdout ) fclose( txtFile );
    } else {
        print_usage();
    }
    return 0;
}
