/**
 * Primo .ptp file információinak megjelenítése
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
int basicList = 0;

int tapeType = 0; // 0x83:BASIC, 0x87:BASIC DATA
char programname[17] = "";
int is_autostart = 0; // is 0xB9 block
uint16_t autostart_address = 0; // If tapeType == 0xF9 && blockType = 0xB9

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
            if ( verbose ) fprintf( stdout, "Ptp block size: %d\n", size );
            return size - 3;
        } else {
            fprintf( stderr, "Invalid .ptp block format. Bad first block character: 0x%02X.\n", byte );
            exit(1);
        }
    }
}

void store_programname( FILE *ptp, unsigned char block_counter ) {
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

unsigned char get_next_ptp_block( FILE *ptp, int *readedBlocksSize, int *memorySize ) {
    unsigned char ptpBlockType = fgetc( ptp );
    if ( ptpBlockType == 0x55 || ptpBlockType == 0xAA ) {
        uint16_t ptp_block_size;
        fblockread( &ptp_block_size, 2, ptp );
        if ( verbose ) fprintf( stdout, " Ptp block found. Type: 0x%02X, block size: %d\n", ptpBlockType, ptp_block_size );
        // if ( !size ) size = 256; // Ez itt sosem fordulhat elő! Töröld!
        *readedBlocksSize += ptp_block_size + 3;
        int pos = ftell( ptp );

        unsigned char tapeBlockType = fgetc( ptp );
        unsigned char blockIndex = fgetc( ptp );
        if ( verbose ) fprintf( stdout, "  Tape block type: 0x%02X, index=%02x, memory size: %d\n", tapeBlockType, blockIndex, ptp_block_size ); // Itt helyes a ptp_block_size? Nem!!!
        if ( tapeBlockType == 0x83 || tapeBlockType == 0x87 ) {
            store_programname( ptp, blockIndex );
        } else if ( tapeBlockType == 0xF1 || tapeBlockType == 0xF7 || tapeBlockType == 0xF9 ) {
            if ( !tapeType) tapeType = tapeBlockType; // Az első adtablokk típusa
            unsigned char loadAddressL = fgetc( ptp );
            unsigned char loadAddressH = fgetc( ptp );
            unsigned char tapeBlockSize = fgetc( ptp );
            unsigned int tapeBockMemorySize = tapeBlockSize ? tapeBlockSize : 256;
            *memorySize += tapeBlockSize;
            if ( tapeBockMemorySize + 6 != ptp_block_size ) {
                fprintf( stderr, "Invalid ptp (%d) and tape (%d) block size!\n", ptp_block_size, tapeBlockSize );
                exit(1);
            }
        } else if ( tapeBlockType == 0xB9 ) { // Auto start address block
            is_autostart = 1;
            fblockread( &autostart_address, 2, ptp );
//            *memorySize += 2;
        }

        fseek( ptp, pos + ptp_block_size, SEEK_SET );
    } else {
        fprintf( stderr, "Invalid .ptp block type: 0x%02X.\n", ptpBlockType );
        exit(2);
    }
    return ptpBlockType;
}

void ptp_info( char *ptpFilenameWithPath ) {
    FILE *ptp = 0;
    if ( ptp = fopen( ptpFilenameWithPath, "rb" ) ) {
        int ptpBlockSumSize = get_ptp_block_size( ptp );
        int readedBlocksSize = 0;
        int memorySize = 0;
        int blockCounter = 1;
        unsigned char ptpBlockType = get_next_ptp_block( ptp, &readedBlocksSize, &memorySize );
        while( ( ptpBlockType != 0xAA ) && ptpBlockSumSize ) {
            blockCounter++;
            if ( ptpBlockSumSize == readedBlocksSize ) {
                ptpBlockSumSize = get_ptp_block_size( ptp );
                readedBlocksSize = 0;
            }
            if ( ptpBlockSumSize ) ptpBlockType = get_next_ptp_block( ptp, &readedBlocksSize, &memorySize );
        }
        fclose( ptp );
        if ( !verbose ) {
            if ( showFilename ) fprintf( stdout, "%s\t", ptpFilenameWithPath );
            if ( showName )     fprintf( stdout, "%s\t", programname );
            if ( showType )     fprintf( stdout, "0x%02X\t", tapeType );
            if ( showBlocks )   fprintf( stdout, "%d\t", blockCounter );
            if ( showRam )      fprintf( stdout, "%d\t", memorySize );
            if ( showMachineType ) {
                if ( memorySize > 28000 ) {
                    fprintf ( stdout, "A64\t" );
                } else if ( memorySize > 10600 ) {
                    fprintf ( stdout, "A48\t" );
                } else {
                    fprintf ( stdout, "A32\t" );
                }
            }
            fprintf( stdout, "\n" );
        } else {
            if ( showFilename ) fprintf( stdout, "Filename:\t%s\n", ptpFilenameWithPath );
            if ( showName )     fprintf( stdout, "Programname:\t%s\n", programname );
            if ( showType ) {
                fprintf( stdout, "Tape type:\t0x%02X\t", tapeType );
                switch ( tapeType ) {
                    case 0xF1 : fprintf( stdout, "BASIC PROGRAM" ); break;
                    case 0xF7 : fprintf( stdout, "BASIC DATA FILE" ); break;
                    case 0xF9 : fprintf( stdout, "SYSTEM PROGRAM" ); break;
                    default : fprintf( stdout, "Unknown" );
                }
                fprintf( stdout, "\n" );
            }
            if ( showAutostart ) {
                if ( is_autostart ) {
                    fprintf( stdout, "Autostart address is 0x%04X\n", autostart_address );
                } else {
                    fprintf( stdout, "No autostart address defined.\n" );
                }
            }
            if ( showBlocks )   fprintf( stdout, "Block counter:\t%d\n", blockCounter );
            if ( showRam )      fprintf( stdout, "Memory size:\t%d\n", memorySize );
            if ( showMachineType ) {
                fprintf ( stdout, "Machine type:\t" );
                if ( memorySize > 28000 ) {
                    fprintf ( stdout, "A64" );
                } else if ( memorySize > 10600 ) {
                    fprintf ( stdout, "A48" );
                } else {
                    fprintf ( stdout, "A32" );
                }
                fprintf( stdout, "\n" );
            }
        }
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
            char* ffn = copyStr3( ptpDir, "/", pDirent->d_name );
            ptp_info( ffn );
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
    while ( ( opt = getopt (argc, argv, "lbRtnfTv?h:i:") ) != -1 ) {
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
    }

    if ( ptpDir ) {
        ptp_dir_info( ptpDir );
    } else if ( ptpFilename ) {
        ptp_info( ptpFilename );
    } else {
        print_usage();
    }
    return 0;
}
