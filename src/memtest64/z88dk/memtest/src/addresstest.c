#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

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
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

int status = 0; // 0 előtte, 1 fut, 2 vége
unsigned int address = 0;
unsigned int err_bits[ 8 ]; //  = { 0,0,0,0,0,0,0,0 };
int err_addr_counter = 0;
const int max_err_addr = 10;
char mainChoice = 0;

unsigned int from = 0x4400;
unsigned int to   = 0xCFFF;
int maxStartAddr = 0;
int maxLength = 0;

void cls() { printf( "%c", 12 ); }

char selectCharFrom( char *chars ) {
    char a = 0;
    while( !a ) {
        char answer = toupper( getchar() );
        int i;
        for( i=0; chars[i] && chars[i]!=answer; i++ );
        a = chars[i] ? chars[i] : 0;
    }
    return a;
}

char mainMenuChoice() {
    cls();
    printf( "Primo Memory Test - select type\n" );
    printf( "1 - Search first %d error\n", max_err_addr );
    printf( "B - Bit sum test\n" );
    printf( "S - Select Memory Bank\n" );
    printf( "R - Rom test\n" );
    printf( "Q - quit\n" );
    return selectCharFrom( "1BQSR" );
}

void showFirstNStatus() {
    cls();
    printf( "First %d errors\n", max_err_addr );
    printf( "Errors:\n" );
}

void showBitSumStatus() {
    cls();
    printf( "Primo Memory Test\n" );
    printf( "Current: 0x%04X \n", address );
    printf( " 0   1   2   3   4   5   6   7\n" );
    for( unsigned char bit = 0; bit < 8; bit++ ) {
        printf( "%03X ", err_bits[ bit ] );
    }
    printf( "\n" );
    printf( "Max ok from %04X length %04X\n", maxStartAddr, maxLength );
}

/**
 * FF -> 7F
 * FF -> DF
 * 00 -> 20
 * 55 -> 75
 * AA -> 2A
 * AA -> 8A
 */
char checkByte( int address, unsigned char value ) { // return 1 if ok (no errors)
    POKE( address, value );
    unsigned char readed_value = PEEK( address );
    if ( value != readed_value ) { // Error
        if ( mainChoice == '1' ) { // First n address test
            printf( "E: w:%02X -> %04X -> %02X\n", value, address, readed_value );
            err_addr_counter++;
        } else { // sum bit test
            for( unsigned char bit = 0; bit < 8; bit++ ) {
                if ( CHECK_BIT( readed_value, bit ) != CHECK_BIT( value, bit ) ) { // Ez a bit hibás
                    err_bits[ bit ]++;
                }
            }
        }
        return 0;
    } else {
        return 1;
    }
}


void firstNErrorTest() {
    status = 1;
    char next = 'F';
    address = from;
    while( next == 'F' ) {
        err_addr_counter = 0;
        showFirstNStatus();
        for ( ; address<=to && err_addr_counter<max_err_addr; address++ ) {
            checkByte( address, 0 );
            checkByte( address, 0xFF );
            checkByte( address, 0xAA );
            checkByte( address, 0x55 );
        }
        printf( "F - Forward, E - End\n" );
        next = selectCharFrom( "FE" );
    }
    status = 2;
}

void selectMemoryBank() {
    cls();
    printf( "Currently selected: %04X-%04X\n", from, to );
    printf( "Select new memory area\n" );
    printf( "1 - 4100-7FFF\n" );
    printf( "2 - 8000-AFFF\n" );
    printf( "3 - B000-CFFF\n" );
    printf( "4 - D000-E7FF\n" );
    printf( "Q - quit\n" );
    char s = selectCharFrom( "1234Q" );
    if ( s == '1' ) {
        from = 0x4100;
        to = 0x7FFF;
    } else if ( s == '2' ) {
        from = 0x8000;
        to = 0xAFFF;
    } else if ( s == '3' ) {
        from = 0xB000;
        to = 0xCFFF;
    } else if ( s == '4' ) {
        from = 0xD000;
        to = 0xE7FF;
    }
}

void romTest() {
    cls();
    if ( PEEK( 0x05 ) == 0xC3 ) {
        printf( "ROM ok\n" );
    } else {
        printf( "ROM READ ERROR\n" );
    }
}

void bitSumTest() {
    status = 1;
    for( int i=0; i<8; i++ ) err_bits[ i ] = 0;
    showBitSumStatus();
    int currentLength = 0;
    int currentStartAddr = 0;
    maxLength = 0;
    maxStartAddr = 0;
    for ( address = from; address<=to; address++ ) {
        char l1 = checkByte( address, 0 );
        char l2 = checkByte( address, 0xFF );
        char l3 = checkByte( address, 0xAA );
        char l4 = checkByte( address, 0x55 );
        if ( l1 && l2 && l3 && l4 ) { // ok
            if ( !currentLength ) {
                currentLength = 1;
                currentStartAddr = address;
            } else {
                currentLength++;
            }
            if ( currentLength > maxLength ) {
                maxStartAddr = currentStartAddr;
                maxLength = currentLength;
            }
        } else { // Error
            currentLength = 0;
        }
        if ( address % 1024 == 0 ) showBitSumStatus();
    }
    address--;
    status = 2;
    showBitSumStatus();
}

int main() {
    mainChoice = mainMenuChoice(); // S,B,1,Q
    while( mainChoice != 'Q' ) {
        if ( mainChoice == 'B' ) bitSumTest();
        else if ( mainChoice == '1' ) firstNErrorTest();
        else if ( mainChoice == 'S' ) selectMemoryBank();
        else if ( mainChoice == 'R' ) romTest();
        printf( "\nTest completted.\n" );
        printf( "M - Main menu\n" );
        selectCharFrom( "M" );
        mainChoice = mainMenuChoice();
    }
}
