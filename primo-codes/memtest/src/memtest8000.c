#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>   /* Needed just for srand seed */

#pragma output REGISTER_SP           = 0xD000    // place stack at $d000 at startup

/**
 * Primo MEMORY
 * 0000 - 3FFF : ROM
 * 4000 - 7FFF : RAM 16K
 * 8000 - BFFF : RAM 16K
 * C000 - FFFF : RAM 16K
 * Úgy tűnik, D000-tól betölthető
 */

#define PEEK(addr)      (*(unsigned char *)(addr))
#define POKE(addr, b)   (*(unsigned char *)(addr) = (b))

int status = 0; // 0 előtte, 1 fut, 2 vége
unsigned int address = 0;
unsigned int err_bits[ 8 ]; //  = { 0,0,0,0,0,0,0,0 };
unsigned int err_cnt = 0;

void cls() { printf( "%c", 12 ); }

void redrawScreen() {
    cls();
    printf( "Primo A64 Memory Test: 0x%04X \r\r", address );

    printf( "0x4000 - 0x7FFF        0x8000 - 0xBFFF\r" );
    printf( "1.7.2.0.5.6.4.3.   3. 6. 7. 0. 1. 5. 4. 2." );
    printf( "- - - - - - - - E: %-2X %-2X %-2X %-2X %-2X %-2X %-2X %-2X", err_bits[ 3 ], err_bits[ 6 ], err_bits[ 7 ], err_bits[ 0 ], err_bits[ 1 ], err_bits[ 5 ], err_bits[ 4 ], err_bits[ 2 ] );

    printf( "\r0xC000 - 0xFFFF\r" );
    printf( "0.1.2.7.3.4.5.6.\r" );
    printf( "- - - - - - - -\r" );

    printf( "\r" );
    if ( status == 2 ) {
        if ( !err_cnt ) {
            printf( "%c Test completted. 0 error found.\r", 135 );
        } else if ( err_cnt>255 ) {
            printf( "- Test cancelled. %d errors found.\r", err_cnt );
        } else {
            printf( "- Test completted. %d errors found.\r", err_cnt );
        }
        printf( "  R: restart, Q: quit\n" );
    }
}

void checkByte( int address, unsigned char value ) {
    POKE( address, value );
    unsigned char readed_value = PEEK( address );
    if ( value != readed_value ) { // Error
        unsigned char bitValue = 1;
        for( unsigned char bit = 0; ( err_cnt < 256 ) && ( bit < 8 ); bit++ ) {
             if ( ( bitValue & readed_value ) != ( bitValue & value ) ) { // Ez a bit hibás
                err_bits[ bit ]++;
                err_cnt++;
            }
            bitValue *= 2;
        }
    }
}

// unsigned int from = 0x5000;
// unsigned int from = 0x4200;
unsigned int from = 0x8000;
unsigned int to   = 0xBFFF;

int main() {
    unsigned char quit = 0;
    while( !quit ) {
        status = 1;
        redrawScreen();
        for ( address = from; ( err_cnt < 256 ) && ( address<=to ); address++ ) {
            checkByte( address, 0 );
            checkByte( address, 0xFF );
            checkByte( address, 0xAA );
            checkByte( address, 0x55 );
            if ( address % 1024 == 0 ) redrawScreen();
        }
        address--;
        status = 2;
        redrawScreen();
        char key = 0;
        for ( key = toupper( getchar() ); key != 'R' && key != 'Q'; key = toupper( getchar() ) );
        quit = key == 'Q';
        status = 0;
    }
}
