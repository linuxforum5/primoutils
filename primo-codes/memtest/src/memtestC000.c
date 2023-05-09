#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>   /* Needed just for srand seed */
#include <arch/z80.h>

#pragma output REGISTER_SP = 0xB000    // place stack at $B000 at startup

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

    printf( "   0x4000 - 0x7FFF    0x8000 - 0xBFFF\r" );
    printf( "   1.7.2.0.5.6.4.3.   3.6.7.0.1.5.4.2.\r" );
    printf( "   - - - - - - - -    - - - - - - - -\r" );

    printf( "\r       0xC000 - 0xFFFF\r" );
    printf( "   0. 1. 2. 7. 3. 4. 5. 6.\r" );
    printf( "E: %-2X %-2X %-2X %-2X %-2X %-2X %-2X %-2X\r", err_bits[ 0 ], err_bits[ 1 ], err_bits[ 2 ], err_bits[ 7 ], err_bits[ 3 ], err_bits[ 4 ], err_bits[ 5 ], err_bits[ 6 ] );

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

unsigned int from0 = 0xC000; // test range from before screen swap
unsigned int to0  = 0xDFFF;  // test range to before screen swap
unsigned int from1 = 0xE000;
unsigned int to1   = 0xFFFF;
unsigned char port = 0x1F;

void di() {
    unsigned char *port_mirror_address = 0x403B;
    *port_mirror_address &= 0x80;
    outp( port, *port_mirror_address );
}

void ei() {
    unsigned char *port_mirror_address = 0x403B;
    *port_mirror_address |= 0x80;
    outp( port, *port_mirror_address );
}

void activeHighScreen() {
    di();
    unsigned int *screen_start_address = 0x4039;
    *screen_start_address = 0xE800;

    unsigned char *screen_start_address_high = 0x404A;
    *screen_start_address_high = 0xE8;

    unsigned char *port_mirror_address = 0x403B;
    *port_mirror_address |= 0x08;

    outp( port, *port_mirror_address );
    ei();

    cls();
    // SP ?
}

void activeLowScreen() { // @TODO: how to change display page??
    di();
    unsigned int *screen_start_address = 0x4039;
    *screen_start_address = 0xC800;

    unsigned char *screen_start_address_high = 0x404A;
    *screen_start_address_high = 0xC8;

    unsigned char *port_mirror_address = 0x403B;
    *port_mirror_address &= 0xF7;

    outp( port, *port_mirror_address );
    ei();
    cls();
    // SP ?
}

int main() {
    unsigned char quit = 0;
    while( !quit ) {
        activeHighScreen();
        status = 1;
        redrawScreen();
        for ( address = from0; ( err_cnt < 256 ) && ( address<=to0 ); address++ ) {
            checkByte( address, 0 );
            checkByte( address, 0xFF );
            checkByte( address, 0xAA );
            checkByte( address, 0x55 );
            if ( address % 1024 == 0 ) redrawScreen();
        }
        activeLowScreen();
        for ( address = from1; ( err_cnt < 256 ) && ( address<=to1 ) && (address); address++ ) {
            checkByte( address, 0 );
            checkByte( address, 0xFF );
            checkByte( address, 0xAA );
            checkByte( address, 0x55 );
            if ( address % 1024 == 0 ) redrawScreen();
        }
        address--;
        status = 2;
        activeHighScreen();
        redrawScreen();
        char key = 0;
        for ( key = toupper( getchar() ); key != 'R' && key != 'Q'; key = toupper( getchar() ) );
        quit = key == 'Q';
        status = 0;
    }
}
