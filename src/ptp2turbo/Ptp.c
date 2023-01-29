/**
 * Primo .ptp formátum kezelése
 */
#include "Ptp.h"

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

static int verbose = 0;

static unsigned char bytes[65535];
static u_int16_t byte_counter = 0;
static u_int16_t load_address = 0; // first byte load address
static u_int16_t run_address = 0; // run address after load
static char tape_name[17] = ""; // Tape name in tape_name_block

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

PTP_DATA create_ptp_data() {
    PTP_DATA payload;
    strcpy( payload.name, tape_name );
    payload.run_address = run_address;
    payload.load_address = load_address;
    payload.byte_counter = byte_counter;
    payload.bytes = malloc( byte_counter );
    for( int i=0; i<byte_counter; i++ ) {
        payload.bytes[i] = bytes[i];
    }
    return payload;
}

PTP_DATA load_payload_from_ptp( FILE *ptp, int verb ) {
    verbose = verb;
    if ( 0xFF == fgetc( ptp ) ) { // PTP first byte
        u_int16_t ptpLength = 0;
        fread( &ptpLength, 2, 1, ptp );
        unsigned char ptpBlockType = read_ptp_block( ptp );
        while( ptpBlockType != 0xAA ) {
            ptpBlockType = read_ptp_block( ptp );
        }
        return create_ptp_data();
    } else {
        fprintf( stderr, "Invalid ptp fileformat.\n" );
        exit(1);
    }    
}
