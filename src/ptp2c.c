/**
 * Primo .ptp c bináris tartalmának c forráskódbakonvertálása
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

#ifdef _WIN32
#include <stdint.h>
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
#endif

#define VM 0
#define VS 2
#define VB 'b'

unsigned char bytes[65535];
u_int16_t byte_counter = 0;
u_int16_t load_address = 0; // first byte load address
u_int16_t run_address = 0; // run address after load
char tape_name[17] = ""; // Tape name in tape_name_block
char var_name[100] = "ptp_content"; // A C változó neve

int verbose = 0;
int hexData = 1; // if 0 then bytes stored in decimal format
int constg = 1; // if 0 no const generated

void create_c_code( FILE *cCode ) {
    fprintf( cCode, "/* ptp2c generated c source code from '%s' ptp file.*/\n\n", tape_name );
    fprintf( cCode, "static %sstruct {\n", constg?"const ":"" );
    fprintf( cCode, "    char name[ 17 ];\n" );
    fprintf( cCode, "    uint16_t run_address;\n" );
    fprintf( cCode, "    uint16_t load_address;\n" );
    fprintf( cCode, "    uint16_t byte_counter;\n" );
    fprintf( cCode, "    unsigned char bytes[ %d ];\n", byte_counter );
    if ( hexData ) {
        fprintf( cCode, "} %s = { \"%s\", 0x%04X, 0x%04X, 0x%04X // First free address: 0x%04X\n", var_name, tape_name, run_address, load_address, byte_counter, load_address + byte_counter );
    } else {
        fprintf( cCode, "} %s = { %d, %d, %d\n", var_name, run_address, load_address, byte_counter );
    }
    int line = 16;
    int indent = strlen( var_name ) + 5;
    for( int i=0; i<byte_counter; i+=line ) {
        for( int j=0; j<indent; j++ ) fprintf( cCode, " " );
        for( int j=0; j<line && i+j < byte_counter; j++ ) {
            if ( hexData ) {
                fprintf( cCode, ", 0x%02X", bytes[ i + j ] );
            } else {
                fprintf( cCode, ", %d", bytes[ i + j ] );
            }
        }
        fprintf( cCode, "\n" );
    }
    for( int j=0; j<indent; j++ ) fprintf( cCode, " " );
    fprintf( cCode, "};\n" );
}

u_int16_t read_tape_close_block( FILE *ptp, unsigned char blockIndex, int is_autostart ) { // blockType and blockIndex already readed from ptp
    if ( verbose ) printf( "\t%02X. tape block type: Close\n", blockIndex );
    if ( is_autostart ) {
        unsigned char runAddressL = fgetc( ptp );
        unsigned char runAddressH = fgetc( ptp );
        unsigned char crc = fgetc( ptp );
        run_address = runAddressL + runAddressH * 256;
        return 5;
    } else {
        unsigned char crc = fgetc( ptp );
        return 3;
    }
}

u_int16_t read_tape_name_block( FILE *ptp, unsigned char blockIndex ) { // blockType and blockIndex already readed from ptp
    if ( verbose ) printf( "\t%02X. tape block type: Name\n", blockIndex );
    unsigned char namesize = fgetc( ptp ); // < 17
    if ( namesize > 16 ) {
        printf( "Error: tape name too long! (%d>16)\n", namesize );
        exit(1);
    }
    for( int i=0; i<namesize; i++ ) {
        tape_name[i]=fgetc( ptp );
    }
    tape_name[ namesize ] = 0;
    unsigned char crc = fgetc( ptp );
    return namesize + 4;
}

u_int16_t read_tape_data_block( FILE *ptp, unsigned char blockIndex ) { // blockType and blockIndex already readed from ptp
    unsigned char loadAddressL = fgetc( ptp );
    unsigned char loadAddressH = fgetc( ptp );
    uint16_t loadAddress = loadAddressL + loadAddressH * 256;
    unsigned char byteCounter = fgetc( ptp ); // If 0, then 256 bytes
    if ( verbose ) printf( "\t%02X. tape block type: Data. Load: 0x%04X, Count1: 0x%02X\n", blockIndex, loadAddress, byteCounter );
    u_int16_t counter = byteCounter ? byteCounter : 256;
    if ( byte_counter == 0 ) { // First block
        load_address = loadAddress;
    } else {
        if ( load_address + byte_counter == loadAddress ) { // Continues
        } else {
            printf( "ERROR: Space in block chain! Last addr: 0x%04X, First addr: 0x%04X\n", load_address + counter, loadAddress );
            exit(1);
        }
    }
    for( int i = 0; i < counter; i++ ) {
        bytes[ byte_counter++ ] = fgetc( ptp );
    }
    unsigned char crc = fgetc( ptp );
    return counter+6;
}

u_int16_t read_tape_block( FILE *ptp ) {
    unsigned char tapeBlockType = fgetc( ptp );
    if ( feof( ptp ) ) {
        return 0;
    } else {
        unsigned char blockIndex = fgetc( ptp );
        switch( tapeBlockType ) {
            case 0x83 :
            case 0x87 : return read_tape_name_block( ptp, blockIndex ); break;
//        case 0xF1 : // Basic program
//        case 0xF5 : // Képernyő
//        case 0xF7 : // Basic adat
            case 0xF9 : return read_tape_data_block( ptp, blockIndex ); break; // Gépi program
//        case 0xB1 : // Állomány vége
//        case 0xB5 : // Képernyő vége
//        case 0xB7 : copy_tape_close_block( ptp, wav, blockIndex, 0 ); break; // BASIC adat vége
            case 0xB9 : return read_tape_close_block( ptp, blockIndex, 1 ); break; // Gépi kódú vége, autostart
            default:
                fprintf( stderr, "Invalid tape block type: 0x%02X\n", tapeBlockType );
                exit(1);
                break;
        }
    }
}

unsigned char read_ptp_block( FILE *ptp ) {
    unsigned char ptpBlockType = fgetc( ptp );
    if ( ptpBlockType == 0x55 || ptpBlockType == 0xAA ) {
        if ( verbose ) printf( "PTP Block type: 0x%02X\n", ptpBlockType );
        u_int16_t ptpBlockSize = 0;
        fread( &ptpBlockSize, 2, 1, ptp );
        if ( verbose ) fprintf( stdout, "\tRead first tape block in ptp block.\n" );
        u_int16_t tapeBlockSizeSum = read_tape_block( ptp );
        while( tapeBlockSizeSum < ptpBlockSize ) { // && ptpBlockType != 0xAA ) { // Hibás a ptp fájlban az utolsó blokkméret? Az AA blokk mérete 8, míg a benne lévő blokk csak 5 bájtos. TODO
            if ( verbose ) fprintf( stdout, "\tRead next tape block in ptp block, because tapes sum size (%d) smaller than ptp block size (%d).\n", tapeBlockSizeSum, ptpBlockSize );
            tapeBlockSizeSum += read_tape_block( ptp );
        }
        if ( tapeBlockSizeSum > ptpBlockSize ) {
            fprintf( stderr, "Invalid ptp block size: %d\n", ptpBlockSize );
            exit(1);
        }
        // unsigned char tapeBlockIndex = fgetc( ptp );....
    } else {
        fprintf( stderr, "Invalid ptp block type: 0x%02X\n", ptpBlockType );
        exit(1);
    }
    return ptpBlockType;
}

void ptp_conv( FILE *ptp, FILE *cCode ) {
    if ( 0xFF == fgetc( ptp ) ) { // PTP first byte
        u_int16_t ptpLength = 0;
        fread( &ptpLength, 2, 1, ptp );
        unsigned char ptpBlockType = read_ptp_block( ptp );
        while( ptpBlockType != 0xAA ) {
            ptpBlockType = read_ptp_block( ptp );
        }
        create_c_code( cCode );
    } else {
        fprintf( stderr, "Invalid ptp fileformat.\n" );
        exit(1);
    }
}

void print_usage() {
    printf( "ptp2c v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Convert System .ptp file contents to .c source file.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptp2c -i <ptp_filename> [ -o <c_filename> ]\n");
    printf( "-n name : Set the name of created c variable. Default value is: %s.\n", var_name );
    printf( "-N      : No const! The generated variable is no const. Default is const.\n" );
    printf( "-v      : Verbose mode.\n");
    exit( 1 );
}

int main(int argc, char *argv[]) {
    int opt = 0;
    FILE *ptpFile = 0;
    FILE *cFile = stdout;
    int renumber = 0;
    while ( ( opt = getopt (argc, argv, "v?hNn:i:o:") ) != -1 ) {
        switch ( opt ) {
            case -1:
            case ':':
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'v': verbose = 1; break;
            case 'n': // c variable name
                strcpy( var_name, optarg );
                break;
            case 'N': constg = 0; break;
            case 'i': // open ptp file
                ptpFile = fopen( optarg, "rb" );
                if ( !ptpFile ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
            case 'o': // open txt file
                cFile = fopen( optarg, "wb" );
                if ( !cFile ) {
                    fprintf( stderr, "Error creating %s.\n", optarg);
                    exit(4);
                }
                break;
        }
    }
    if ( ptpFile && cFile ) {
        ptp_conv( ptpFile, cFile );
        fclose( ptpFile );
        if ( cFile != stdout ) fclose( cFile );
    } else {
        print_usage();
    }
    return 0;
}
