/**
 * ptp2pri konverzió
 * .pri vagy .prg fájlok specifikációja:
 * http://primo.homeserver.hu/html/konvertfajlok.html
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

#ifdef _WIN32
#include <stdint.h>
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
#endif

#define VM 0
#define VS 1
#define VB 'b'

u_int16_t full_bytes_size = 0; // A kiírt hasznos bájtok száma

unsigned char verbose = 0;

void pri_close_block( FILE *pri, u_int16_t address ) {
    if ( address ) {
        fputc( 0xC3, pri );
        fputc( address % 256, pri );
        fputc( address / 256, pri );
    } else {
        fputc( 0xC9, pri );
    }
}

void copy_bytes_into_pri( FILE *ptp, u_int16_t cnt, FILE* pri ) { // if counter1 == 0, then means 256. Counter2==0 means 0.
    while ( cnt-- != 0 ) fputc( fgetc( ptp ), pri );
}

void skip_tape_block( FILE *ptp, int is_data, int is_load_address ) {
    if ( verbose ) printf( "\tSKIP this block\n" );
    if ( is_load_address ) {
        fgetc( ptp ); // skip low
        fgetc( ptp ); // skip high
    }
    if ( is_data ) {
        unsigned char counter = fgetc( ptp );
        while( counter--) fgetc( ptp );
    }
    fgetc( ptp ); // skip crc
}

void copy_tape_data_block( unsigned char priBlockType, FILE *ptp, FILE* pri, unsigned char blockIndex ) { // blockType and blockIndex already readed from ptp
    if ( verbose ) printf( "\t%02X. tape block type: Data\n", blockIndex );
    unsigned char loadAddressL = fgetc( ptp );
    unsigned char loadAddressH = fgetc( ptp );
    unsigned char byteCounter = fgetc( ptp ); // If 0, then 256 bytes
    u_int16_t size = byteCounter ? byteCounter : 256;
    fputc( priBlockType, pri );
    fputc( loadAddressL, pri );
    fputc( loadAddressH, pri );
    fputc( size % 256, pri );
    fputc( size / 256, pri );
    full_bytes_size += size;
    copy_bytes_into_pri( ptp, size, pri );
    unsigned char crc = fgetc( ptp );
}

int copy_tape_block( FILE *ptp, FILE* pri, u_int16_t *autostartAddress ) { // Egy tape blokk kovertálása
    unsigned char tapeBlockType = fgetc( ptp );
    unsigned char blockIndex = fgetc( ptp );
    if ( verbose ) printf( "\t%02X. tape block type: %02X\n", blockIndex, tapeBlockType );
    switch( tapeBlockType ) {
        case 0x83 : skip_tape_block( ptp, 1, 0 ); break; // Basic programnév blokk : nem kerül konvertálásra
        case 0x87 : skip_tape_block( ptp, 1, 0 ); break; // Adatállománynév blokk : nem kerül konvertálásra
        case 0xF1 : copy_tape_data_block( 0xD1, ptp, pri, blockIndex ); break;  // Basic programkód blokk
        case 0xF5 : copy_tape_data_block( 0xD5, ptp, pri, blockIndex ); break;  // Képernyő tartalom blokk
        case 0xF7 : skip_tape_block( ptp, 1, 1 ); break; // Basic adat blokk : nem kerül konvertálásra
        case 0xF9 : copy_tape_data_block( 0xD9, ptp, pri, blockIndex ); break; // Gépikódú blokk
        case 0xB1 : skip_tape_block( ptp, 0, 0 ); break; // Basic vagy nem automatikusan induló gépikódú blokk vége : nem kerül konvertálásra
        case 0xB5 : skip_tape_block( ptp, 0, 0 ); break; // Képernyőtartalom vége : nem kerül konvertálásra
        case 0xB7 : skip_tape_block( ptp, 0, 0 ); break; // Basic adatállomény vége : nem kerül konvertálásra
        case 0xB9 : fread( autostartAddress, 2, 1, ptp ); fgetc( ptp ); break; // Automatikusan idnuló gépikód indítási címe
        default:
            fprintf( stderr, "Invalid tape block type: 0x%02X at position %u\n", tapeBlockType, ftell( ptp )-1 );
            exit(1);
            break;
    }
    return feof( ptp );
}

unsigned char copy_ptp_block( FILE *ptp, FILE* pri, u_int16_t *autostartAddress ) {
    unsigned char ptpBlockType = fgetc( ptp );
    if ( ptpBlockType == 0x55 || ptpBlockType == 0xAA ) { // közbenső vagy lezáró ptp blokk
        if ( verbose ) printf( "PTP Block type: 0x%02X\n", ptpBlockType );
        u_int16_t ptpBlockSize = 0;
        fread( &ptpBlockSize, 2, 1, ptp );  // Beolvassuk a ptp blokk méretét
        u_int16_t last_pos = ftell( ptp ) + ptpBlockSize;
        while( last_pos > ftell( ptp ) ) {
            copy_tape_block( ptp, pri, autostartAddress );
        }
        // unsigned char tapeBlockIndex = fgetc( ptp );
    } else {
        fprintf( stderr, "Invalid ptp block type: 0x%02X at position %u\n", ptpBlockType, ftell( ptp )-1 );
        exit(1);
    }
    return ptpBlockType;
}

void convert( FILE *ptp, FILE* pri ) {
    if ( 0xFF == fgetc( ptp ) ) { // PTP first byte
        u_int16_t ptpLength = 0;
        fread( &ptpLength, 2, 1, ptp );
        u_int16_t autostartAddress = 0;
        unsigned char ptpBlockType = copy_ptp_block( ptp, pri, &autostartAddress );
        while( ptpBlockType != 0xAA ) {
            ptpBlockType = copy_ptp_block( ptp, pri, &autostartAddress );
        }
        pri_close_block( pri, autostartAddress );
    } else {
        fprintf( stderr, "Invalid ptp fileformat.\n" );
        exit(1);
    }
}

void print_usage() {
    printf( "ptp2pri v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Microkey Primo ptp to .PRI (.PRG) file converter.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptp2pri [options] -i <input_ptp_filename> -o <output_pri_filename>\n");
    printf( "Command line option:\n");
    printf( "-v           : verbose mode\n");
    printf( "-h           : prints this text\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int finished = 0;
    int arg1;
    FILE *ptpFile = 0, *pri = 0;

    while (!finished) {
        switch (getopt (argc, argv, "v?h:i:o:")) {
            case -1:
            case ':':
                finished = 1;
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'v':
                verbose = 1;
                break;
            case 'i':
                if ( !(ptpFile = fopen( optarg, "rb")) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
            case 'o':
                if ( !(pri = fopen( optarg, "wb")) ) {
                    fprintf( stderr, "Error creating %s.\n", optarg);
                    exit(4);
                }
                break;
            default:
                break;
        }
    }

    if ( !ptpFile ) {
        print_usage();
    } else if ( !pri ) {
        print_usage();
    } else {
        convert( ptpFile, pri );
        fclose( pri );
        fclose( ptpFile );
    }

    return 0;
}
