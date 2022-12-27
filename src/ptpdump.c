/**
 * Primo .ptp fájlban tárolt memóriatartalom kinyomtatása
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
int binary_dump = 0;
char programname[17] = "";
int dumpRowSize = 16;
uint16_t origin = 0; // A kezdőcím
uint16_t first_free_addr = 0; // Az első szabad bájt címe betöltés közben. Az összefüggő blokkok felismeréséhez

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
            fblockread( &size, sizeof( size ), ptp );
            if ( verbose ) fprintf( stdout, "Ptp block size: %d\n", size );
            return size - 3;
        } else {
            fprintf( stderr, "Invalid .ptp block format. Bad first block character: 0x%02X.\n", byte );
            exit(1);
        }
    }
}

/**
 *
 */
unsigned char dumpBytes( FILE *ptp, FILE *txt, uint16_t counter ) {
    for( int i = 0; i < counter; i+=dumpRowSize ) { // Ez már biztosan soron belül van
        for ( int j = 0; j < dumpRowSize; j++ ) {
            unsigned char byte = fgetc( ptp );
            if ( binary_dump ) {
                fprintf( txt, "%c", byte );
            } else { // Hex dump
                fprintf( txt, " %02X", byte );
            }
        }
        if ( !binary_dump ) fprintf( txt, "\n" );
    }
}

void dumpSystemBlock( FILE *ptp, FILE *txt ) {
    unsigned char blockIndex = fgetc( ptp ); // 2. bájt a blokkban
    uint16_t absAddr = 0;
    fblockread( &absAddr, sizeof( absAddr ), ptp );
    uint16_t size = fgetc( ptp );
    if ( !size ) size = 256;
    // if ( verbose ) fprintf( stdout, "\tBASIC tape block size: %d\n", size );
    if ( absAddr != first_free_addr ) {
        origin = absAddr;
        if ( binary_dump ) {
            fprintf( stdout, "Origin: 0x%04X\n", absAddr );
        } else {
            fprintf( txt, "Origin: 0x%04X\n", absAddr );
        }
    }
    first_free_addr = absAddr + size;
    dumpBytes( ptp, txt, size );
}

unsigned char get_next_block( FILE *ptp, FILE *txt, int *readedBlocksSize, int blockCounter ) {
    unsigned char blockType = fgetc( ptp );
    if ( blockType == 0x55 || blockType == 0xAA ) {
        uint16_t size;
        fblockread( &size, sizeof( size ), ptp );
        *readedBlocksSize += size + 3;
        if ( verbose ) fprintf( stdout, "%d. block size: %d (", blockCounter, size );
        int pos = ftell( ptp );

        unsigned char tapeBlockType = fgetc( ptp );
        if ( verbose ) {
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
            // store_programname( ptp );
        } else if ( tapeBlockType == 0xF1 || tapeBlockType == 0xF7 || tapeBlockType == 0xF9 ) { // F9 : SYSTEM PROGRAM BLOCK
            if ( tapeBlockType == 0xF9 ) {
                dumpSystemBlock( ptp, txt );
            } else {
                fprintf( stderr, "Nem SYSTEM program!\n" );
            }
        }

        fseek( ptp, pos+size, SEEK_SET );
    } else {
        fprintf( stderr, "Invalid .ptp block type: 0x%02X.\n", blockType );
        exit(2);
    }
    return blockType;
}

void ptp_dump( FILE *ptp, FILE *txt ) {
    fseek( ptp, 0, SEEK_SET );
    int ptpBlockSumSize = get_ptp_block_size( ptp );
    int readedBlocksSize = 0;
    int blockCounter = 1;
    unsigned char blockType = get_next_block( ptp, txt, &readedBlocksSize, blockCounter );
    while( ( blockType != 0xAA ) && ptpBlockSumSize ) {
        blockCounter++;
        if ( ptpBlockSumSize == readedBlocksSize ) {
            ptpBlockSumSize = get_ptp_block_size( ptp );
            readedBlocksSize = 0;
        }
        if ( ptpBlockSumSize ) get_next_block( ptp, txt, &readedBlocksSize, blockCounter );
    }
}

void print_usage() {
    printf( "ptpdump v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Memory dump from .ptp file.\n");
    printf( "Copyright 2022 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptpdump -i <ptp_filename> [ -o <dump_filename> ]\n");
    printf( "-c n : Col counter. Default = 16.\n");
    printf( "-b   : Binary dump. Default hex dump.\n");
    printf( "-v   : Verbose mode.\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    FILE *ptpFile = 0;
    FILE *dumpFile = stdout;
    int renumber = 0;
    while ( ( opt = getopt ( argc, argv, "v?bh:i:o:c:" ) ) != -1 ) {
        switch ( opt ) {
            case -1:
            case ':':
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'v': verbose = 1; break;
            case 'b': binary_dump = 1; break;
            case 'c': dumpRowSize = atoi( optarg ); break;
            case 'i': // open ptp file
                ptpFile = fopen( optarg, "rb" );
                if ( !ptpFile ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
            case 'o': // open txt file
                dumpFile = fopen( optarg, "wb" );
                if ( !dumpFile ) {
                    fprintf( stderr, "Error creating %s.\n", optarg);
                    exit(4);
                }
                break;
        }
    }
    if ( ptpFile && dumpFile ) {
        ptp_dump( ptpFile, dumpFile );
        fclose( ptpFile );
        if ( dumpFile != stdout ) fclose( dumpFile );
    } else {
        print_usage();
    }
    return 0;
}
