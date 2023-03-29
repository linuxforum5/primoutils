#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>   /* Needed just for srand seed */

/**
 * Primo MEMORY
 * 0000 - 3FFF : ROM
 * 4000 - 7FFF : RAM 16K
 * 8000 - AFFF : RAM 16K
 * B000 - FFFF : RAM 16K
 * Úgy tűnik, D000-tól betölthető
 */

#define PEEK(addr)      (*(unsigned char *)(addr))
#define POKE(addr, b)   (*(unsigned char *)(addr) = (b))

int status = 0; // 0 előtte, 1 fut, 2 vége
unsigned int address = 0;
unsigned int err_bits[ 8 ]; //  = { 0,0,0,0,0,0,0,0 };

void cls() { printf( "%c", 12 ); }

void redrawScreen() {
    cls();
    printf( "Primo Memory Test\n" );
    printf( "Current: 0x%04X \n", address );
    printf( " 0   1   2   3   4   5   6   7\n" );
    for( unsigned char bit = 0; bit < 8; bit++ ) {
        printf( "%03X ", err_bits[ bit ] );
    }
    printf( "\n" );
    if ( status == 2 ) {
        printf( "Test completted.\n" );
        printf( "R: restart, Q: quit\n" );
    }
}

void checkByte( int address, unsigned char value ) {
    POKE( address, value );
    unsigned char readed_value = PEEK( address );
    if ( value != readed_value ) { // Error
        unsigned char bitValue = 1;
        for( unsigned char bit = 0; bit < 8; bit++ ) {
            if ( bitValue & readed_value != bitValue & value ) { // Ez a bit hibás
                err_bits[ bit ]++;
            }
            bitValue *= 2;
        }
    }
}

unsigned int from = 0x4200;
unsigned int to   = 0xAFFF;

int main() {
    unsigned char quit = 0;
    while( !quit ) {
	status = 1;
        redrawScreen();
        for ( address = from; address<=to; address++ ) {
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
