/**
 * Primo basic functions
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basic.h"

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
}

unsigned char is_BASIC_token( unsigned char byte ) { return byte > 127 && byte < 252; }

int get_token_length( unsigned char token ) {
    int current_token = 127;
    int token_length = 0;
    for( int i=0; i<sizeof( tokens ); i++ ) {
        if ( tokens[ i ] > 127 ) { // Token first char
            current_token++;
            if ( token == current_token ) { // Ez a keresett token első karaktere
                token_length++;
            }
        } else if ( token == current_token ) { // Ez a keresett token egy nem első karaktere
            token_length++;
        }
    }
    return token_length++;
}

int decode_token( unsigned char token, unsigned char* buffer ) {
    int current_token = 127;
    int token_length = 0;
    for( int i=0; i<sizeof( tokens ); i++ ) {
        if ( tokens[ i ] > 127 ) { // Token first char
            current_token++;
            if ( token == current_token ) { // Ez a keresett token első karaktere
                buffer[ token_length++ ] = tokens[i]-128;
            }
        } else if ( token == current_token ) { // Ez a keresett token egy nem első karaktere
            buffer[ token_length++ ] = tokens[i];
        }
    }
    return token_length;
}

/**
 * 
 * @params *line Egy teljes BASIC sor a lezáró 0-ával együtt
 */
BASIC_LINE decode_basic_line( const unsigned char *line, int max_size, int utf8 ) {
//    unsigned char *line = src_line;
    BASIC_LINE bl;
    bl.next_address = line[0] + 256*line[1];
    bl.line_number = line[2] + 256*line[3];
    line += 4;
    int size = strlen( line );
    size++; // For 0
    bl.bin_line_full_length = size + 4;
    if ( bl.bin_line_full_length > max_size ) {
        printf( "Size error! Invalid basic block splice?\n" );exit(1);
    }
    bl.bin_line[0] = 0;
    strcpy( bl.bin_line, line );
    bl.text_line[0] =0;
    int char_pos = 0;
    for( int i=0; i<size; i++ ) {
        if ( is_BASIC_token( line[ i ] ) ) {
            char_pos += decode_token( line[ i ], bl.text_line + char_pos );
        } else {
            if ( utf8 ) {
                unsigned int utf = primo2Utf( line[ i ] );
                if ( utf > 255 ) {
                    bl.text_line[ char_pos++ ] = utf/256;
                    bl.text_line[ char_pos++ ] = utf%256;
                } else {
                    bl.text_line[ char_pos++ ] = line[ i ];
                }
            } else {
                bl.text_line[ char_pos++ ] = line[ i ];
            }
        }
    }
    bl.text_line[ char_pos++ ] = 0;
    bl.charset = 'P';
    return bl;
}

int check_load_addresses( unsigned char *bin_basic, int length, uint16_t load_address ) {
    int chng_counter = 0;
    int pos = 0;
    while( pos < length-4 ) {
        uint16_t next_addr = bin_basic[ pos++ ] + 256*bin_basic[ pos++ ];
        uint16_t line_number = bin_basic[ pos++ ] + 256*bin_basic[ pos++ ];
        int line_length = 5; // Lezáró 0 és bevezető 4 bájt
        while( (pos<length) && bin_basic[ pos ] ) {
            line_length++;
            pos++;
        }
        pos++;
        uint16_t real_next_addr = load_address + line_length;
// printf( "Origi next line address: 0x%04X. Real next line address: 0x%04X\n", next_addr, real_next_addr );
        if ( next_addr != real_next_addr ) {
            bin_basic[ pos - line_length ] = real_next_addr%256;
            bin_basic[ pos - line_length + 1 ] = real_next_addr/256;
            chng_counter++;
        }
        load_address += line_length;
    }
    return chng_counter;
}
