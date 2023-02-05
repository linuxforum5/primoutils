/**
 * Create Primo .ptp file
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

/*
 * Tape blokktípusok:
 * Név blokkok
 *   0x83 - Programnév BASIC vagy gépikódú
 *   0x87 - Adatállomány neve
 * Adat blokkok
 *   0xF1 - BASIC program
 *   0xF5 - Képernyő
 *   0xF7 - BASIC adat
 *   0xF9 - Gépikódú program
 * Záró blokk
 *   0xB1 - BASIC vagy gépikódú program vége
 *   0xB5 - Képernyőkép vége
 *   0xB7 - BASIC adatállomány vége
 *   0xB9 - Autostart gépikódú program vége
 */

/********************************************************************************************************************
 * Create functions
 ********************************************************************************************************************/
typedef struct {
    unsigned char type;     // Source block type: Basic, Name, System, screen
    unsigned char bytes[ 65536 ]; // 
    uint16_t load_address;  // A betöltés kezőcíme
    uint16_t byte_counter;  // A betöltendő bájtok száma összesen a kezdőcímtől folytonosan
    uint16_t run_address;   // A betöltés után erre a címre kell ugrani, vagy 0
    uint16_t __inverz;      // If need inverz conversion. Only for screen block
    uint16_t __utf8;        // If need utf8->primo preconverzion. Only for BASIC text block
} SourceBlock;

SourceBlock newSourceBlock( unsigned char type ) {
    SourceBlock sb;
    sb.type = type;
    sb.bytes[0] = 0;
    sb.load_address = 0;
    sb.byte_counter = 0;
    sb.run_address = 0;
    sb.__inverz = 0;
    sb.__utf8 = 0;
    return sb;
}

/********************************************************************************************************************
 * BASIC encoder
 ********************************************************************************************************************/
// #define BASIC_START 0x4B0B
#define BASIC_START 0x43EA
/*
c54e44c64f52d245534554d34554c34c53c34d44d2414e444f4dce
455854c4415441c94e505554c4494dd2454144cc4554c74f544fd2554ec946d245
53544f5245c74f535542d2455455524ed2454dd3544f50c54c5345d4524f4ed452
4f4646c44546535452c44546494e54c44546534e47c4454644424cc2454550c544
4954c552524f52d24553554d45cf5554cf4ecf50454ec649454c44c74554d05554
c34c4f5345cc4f4144cd45524745d4455354cb494c4cc35245415445e66ed34156
45d3435245454ecc5052494e54c44546d04f4b45d052494e54c34f4e54cc4953
54cc4c495354c4454c455445c155544fc34c454152c34c4f4144c353415645ce
4557d4414228d44fc64ed553494e47d64152505452c3414c4cc5524cc55252d3
5452494e4724c94e535452d04f494e54d4494d4524d049c94e4b455924d44845
4ece4f54d3544550abadaaaf9fc14e44cf52bebdbcd3474ec94e54c14253c652
45c94e50d04f53d35152d24e44cc4f47c55850c34f53d3494ed4414ec1544ed0
45454bc35649c35653c35644c54f46cc4f43cc4f46cd4b4924cd4b5324cd4b44
24c3494e54c3534e47c344424cc64958cc454ed3545224d6414cc15343c34852
24cc45465424d24947485424cd494424a7
 */

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

/*
int primo2Utf( unsigned char character ) { // http://lzsiga.users.sourceforge.net/ekezet.html z
    if ( utf8 ) {
        switch ( character ) {.
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
    } else {
        return character;
    }
}
*/

unsigned char utf2primo( unsigned char c1, unsigned char c2 ) { // http://lzsiga.users.sourceforge.net/ekezet.html
    switch ( c1 ) {
        case 0xC3 :
            switch( c2 ) {
                case 0xA1 : return 0x7D; break; // 0xC3A1; // | á | C3A1 |
                case 0x81 : return 0x5D; break; // 0xC381; // | Á | C381 |
                case 0xA9 : return 0x60; break; // 0xC3A9; // | é | C3A9 |
                case 0x89 : return 0x40; break; // 0xC389; // | É | C389 |
                case 0xB6 : return 0x7C; break; // 0xC3B6; // | ö | C3B6 |
                case 0x96 : return 0x5C; break; // 0xC396; // | Ö | C396 |
                case 0xB3 : return 0x5B; break; // 0xC3B3; // | ó | C3B3 |
                case 0xBC : return 0x7E; break; // 0xC3BC; // | ü | C3BC |
                case 0x9C : return 0x5E; break; // 0xC39C; // | Ü | C39C |
                case 0xAD : return 0x1E; break; // 0xC3AD; // | í | C3AD |
                //      case 0x00 : return 0xC38D; // | Í | C38D |
                case 0xBA : return 0x5F; break; //  : return 0xC3BA; // | ú | C3BA |
                //      case 0x00 : return 0xC39A; // | Ú | C39A |
                //      case 0x00 : return 0xC3A4; // | ä | C3A4 |
                //      case 0x00 : return 0xC384; // | Ä | C384 |
                default: return c1;
            }
            break;
        case 0xC5 : 
            switch( c2 ) {
                //      case 0x00 : return 0xC593; // | Ó | C393 |
                case 0x91 : return 0x7B; // 0xC591; // | ő | C591 |
                //      case 0x00 : return 0xC590; // | Ő | C590 |
                case 0xB1 : return 0x7F; // 0xC5B1; // | ű | C5B1 |
                case 0xB0 : return 0xF1; // 0xC5B0; // | Ű | C5B0 |
                default : return c1;
            }
            break;
        default:
            return c1;
    }
}

// unsigned char is_BASIC_token( unsigned char byte ) { return byte > 127 && byte < 252; }

unsigned char get_token( unsigned char *string, int *string_length ) { // Visszaadja a szóhoz tartozó toke kódot, vagy 0 kódot, ha nem tokenizálható.
    int current_token = 127; // A tokenszavak első karakterének 7. bitje 1. A token azt mondja meg, hogy hányadik(+127) tokenszó a keresett. Az első szó az "END" Ennek kódja 128 
    unsigned char match_length = 0;
    for( int i=0; i<sizeof( tokens ); i++ ) {
        if ( tokens[ i ] > 127 ) current_token++; // Token first char. Increment token counter
        unsigned char c = match_length ? string[ match_length ] : string[ match_length ] + 128;
        if ( c>='a' && c<='z' ) c -= 32; // c & 0b11011111; // Uppercase
        if ( tokens[ i ] == c ) { // A keresett karakter jó
            match_length++; // Eggyel több illeszkedik
            if ( i == sizeof( tokens ) || tokens[ i+1 ] > 127 ) { // Tokenszó vége
                *string_length = match_length; // Az illesztett szó hossza
                return current_token;
            } else if ( match_length >= *string_length ) { // Nincs több illeszthető keresett karakter, ez nem token
                return 0;
            } // else { // Különben folytatjuk 
        } else { // Nem illeszkedő karakter
            match_length = 0; // Kezdjük előről
        }
    }
    return 0;
}

/*
void print_token( FILE *txt, unsigned char token ) {
    int current_token = 127; // A tokenszavak első karakterének 7. bitje 1. A token azt mondja meg, hogy hányadik(+127) tokenszó a keresett. Az első szó az "END" Ennek kódja 128 
    for( int i=0; i<sizeof( tokens ); i++ ) {
        if ( tokens[ i ] > 127 ) { // Token first char
            current_token++;
            if ( token == current_token ) { // Ez a keresett token első karaktere
                if ( txt ) fprintf( txt, "%c", tokens[i]-128 );
            }
        } else if ( token == current_token ) { // Ez a keresett token egy nem első karaktere
            if ( txt ) fprintf( txt, "%c", tokens[i] );
        }
    }
}
*/

#define MAX_LINE_LENGTH 1024

int get_line_from( FILE *f, unsigned char* line, uint16_t *line_number ) {
    *line_number = 0;
    int read_number = 1;
    int line_length = 0;
    for ( unsigned char c = fgetc( f ); !feof( f ) && c!=10 && c!=13; c = fgetc( f ) ) {
        if ( read_number ) {
            if ( c>='0' && c<='9' ) {
                *line_number = *line_number * 10 + c - '0';
            } else if ( !*line_number && c <=' ' ) { // SKIP prefix
            } else if ( c==' ' || c==9 ) { // SKIP line prefix spaces
            } else {
                read_number = 0;
            }
        }
        if ( !read_number ) {
            line[ line_length++ ] = c;
            if ( line_length == MAX_LINE_LENGTH ) {
                printf( "Line to length!\n" ); exit(1);
            }
        }
    }
    return line_length;
}

#define REM 0x93
#define DATA 0x88

char* encode_line( char* line, int line_length, unsigned char *encoded_line, int utf8 ) {
    int encoded_length = 0;
    unsigned char token = 0;
    int apostrofe_mode = 0;
    int REM_mode = 0; // token = 0x93
    int DATA_mode = 0; // token = 0x88
    for( int i=0; i<line_length; i++ ) { // i = next_char start position
        char next_char = line[ i ];
        int next_char_start_position = i;
        if ( utf8 && ( i < line_length-1 ) ) {
            next_char = utf2primo( line[ i ], line[ i+1 ] );
            if ( next_char != line[ i ] ) { // was utf8 conversion
                i++;
            }
        }
        int length = line_length-i; // Hátralévő hossz
        if ( REM_mode ) {
            encoded_line[ encoded_length++ ] = next_char;
        } else if ( next_char == '"' ) { // Idézőjel üzemmód
            apostrofe_mode = 1 - apostrofe_mode;
            encoded_line[ encoded_length++ ] = next_char;
        } else if ( apostrofe_mode ) {
            encoded_line[ encoded_length++ ] = next_char;
            if ( next_char == ':' ) DATA_mode = 0;
        } else if ( DATA_mode ) {
            encoded_line[ encoded_length++ ] = next_char;
        } else if ( token = get_token( line+next_char_start_position, &length ) ) { // Az i. indextől kezdve van egy token
            encoded_line[ encoded_length++ ] = token;
            i += length - 1;
            REM_mode = token == REM;
            DATA_mode = token == DATA;
        } else { // i. index egy az egyben másolandó, mivel nem token
            encoded_line[ encoded_length++ ] = next_char;
        }
    }
    encoded_line[ encoded_length ] = 0;
}

// Line: NextRowAddrL NextRowAddrH NumL NumH tokenized 0
// Program end: 0 0
SourceBlock encodeBasic( FILE *txt, int utf8 ) { // Encode text into src.bytes
    SourceBlock encoded = newSourceBlock( 'B' ); // Basic source code
    uint16_t next_line_addr = BASIC_START;
    uint16_t line_number = 0;
    unsigned char line[ MAX_LINE_LENGTH ];
    unsigned char encoded_line[ MAX_LINE_LENGTH ];
    int line_length = 0;
    while( line_length  = get_line_from( txt, line, &line_number ) ) {
        encode_line( line, line_length, encoded_line, utf8 );
        int encoded_line_length = strlen( encoded_line );
        next_line_addr += encoded_line_length + 5;
        encoded.bytes[ encoded.byte_counter++ ] = next_line_addr % 256;
        encoded.bytes[ encoded.byte_counter++ ] = next_line_addr / 256;
        encoded.bytes[ encoded.byte_counter++ ] = line_number % 256;
        encoded.bytes[ encoded.byte_counter++ ] = line_number / 256;
        for( int j=0; j<encoded_line_length; j++ ) encoded.bytes[ encoded.byte_counter++ ] = encoded_line[ j ];
        encoded.bytes[ encoded.byte_counter++ ] = 0; // End of line
    }
    encoded.bytes[ encoded.byte_counter++ ] = 0; // End Of Basic Program
    encoded.bytes[ encoded.byte_counter++ ] = 0; // End Of Basic Program
    return encoded;
}
/********************************************************************************************************************
 * Ptp functions
 ********************************************************************************************************************/
int tape_block_index = 0;
int crc = 0;

unsigned char BCD( unsigned char byte ) { return ( byte / 10 ) * 16 + ( byte % 10 ); }
unsigned char CRC( unsigned char byte ) { crc += byte; return byte; }

uint16_t write_programname_ptptape_block( FILE *ptp, SourceBlock src ) {
    if ( verbose ) printf( "Write ProgramName block\n" );
    if ( src.byte_counter > 16 ) {
        printf( "Invalid tapename block length: %d\n", src.byte_counter );
        exit( 1 );
    }
    uint16_t ptpBlockSize = src.byte_counter + 4;
    fputc( 0x55, ptp ); // inner ptp block
    fputc( ptpBlockSize % 256, ptp );
    fputc( ptpBlockSize / 256, ptp );
    fputc( 0x83, ptp ); // BASIC os SYSTEM progamname block
    crc = 0;
    fputc( CRC( BCD( tape_block_index++ ) ), ptp );
    fputc( CRC(  src.byte_counter ), ptp );
    for( int i=0; i<src.byte_counter; i++ ) fputc( CRC( src.bytes[ i ] ), ptp );
    fputc( crc, ptp );
    return src.byte_counter + 7; // + 4 + 3
}

uint16_t write_ptptape_data_block( FILE *ptp, unsigned char tape_data_type_code, SourceBlock src, int from, int length ) {
    if ( verbose ) printf( "Write data block type 0x%02X\n", tape_data_type_code );
    uint16_t ptpBlockSize = length + 6;
    fputc( 0x55, ptp ); // inner ptp block
    fputc( ptpBlockSize % 256, ptp );
    fputc( ptpBlockSize / 256, ptp );
    fputc( tape_data_type_code, ptp ); // 
    crc = 0;
    fputc( CRC( BCD( tape_block_index++ ) ), ptp );
    fputc( CRC( from % 256 ), ptp );
    fputc( CRC( from / 256 ), ptp );
    fputc( CRC( length % 256 ), ptp );
    for( int i=0; i<length; i++ ) fputc( CRC( src.bytes[ from+i ] ), ptp );
    fputc( crc, ptp );
    return length + 9; // + 6 + 3
}

uint16_t write_basic_last_ptptape_block( FILE *ptp ) {
    if ( verbose ) printf( "Write last basic block (0x%02X)\n", 0xB1 );
    uint16_t ptpBlockSize = 3;
    fputc( 0xAA, ptp ); // inner ptp block
    fputc( ptpBlockSize % 256, ptp );
    fputc( ptpBlockSize / 256, ptp );
    fputc( 0xB1, ptp ); // BASIC vége
    crc = 0;
    fputc( CRC( BCD( tape_block_index++ ) ), ptp );
    fputc( crc, ptp );
    return 6; // 3 + 3
}

uint16_t write_basic_source_block( FILE *ptp, SourceBlock src ) {
    uint16_t ptp_block_length = 0;
    for( int i=0; i<src.byte_counter; i+=256 ) {
        ptp_block_length += write_ptptape_data_block( ptp, 0xF1, src, i, ( src.byte_counter - i > 255 ) ? 256 : src.byte_counter - i );
    }
    return ptp_block_length;
}

uint16_t write_screen_source_block( FILE *ptp, SourceBlock src ) {
    uint16_t ptp_block_length = 0;
    for( int i=0; i<src.byte_counter; i+=256 ) {
        ptp_block_length += write_ptptape_data_block( ptp, 0xF5, src, i, ( src.byte_counter - i > 255 ) ? 256 : src.byte_counter - i );
    }
    return ptp_block_length;
}

uint16_t write_ptp_block( FILE *ptp, SourceBlock src ) {
    switch( src.type ) {
        case 'B' : return write_basic_source_block( ptp, src ); break;
        case 'S' : return write_screen_source_block( ptp, src ); break;
        case 'N' : return write_programname_ptptape_block( ptp, src ); break;
        default: printf( "Invalid source type: 0x%02X\n", src.type ); exit(1); break;
    }
}

void create_ptp_from( FILE *ptp, SourceBlock srcs[], int sourceBlockCounter ) {
    fputc( 0xFF, ptp );
    uint16_t ptp_content_length = 3;
    fputc( 0, ptp ); // placegholder for ptp size
    fputc( 0, ptp ); // 
printf( "Block counter = %d\n", sourceBlockCounter );
    for( int i=0; i<sourceBlockCounter; i++ ) {
        ptp_content_length += write_ptp_block( ptp, srcs[ i ] );
    }
    ptp_content_length += write_basic_last_ptptape_block( ptp );
    fseek( ptp, 1, SEEK_SET );
    fputc( ptp_content_length % 256, ptp );
    fputc( ptp_content_length / 256, ptp );
}
/********************************************************************************************************************
 * FS functions
 ********************************************************************************************************************/
int ext_is( const char* filename, const char* ext ) {
    int ext_length = strlen( ext );
    int fn_length = strlen( filename );
    if ( fn_length > ext_length ) {
        int separator_pos = fn_length - ext_length - 1;
        if ( filename[ separator_pos ] == '.' ) { // ext separator
            int match = 1;
            for( int i=0; match && i<ext_length; i++ ) match = ext[i]==filename[ separator_pos + 1 + i ];
            return match;
        } else {
            return 0;
        }
    } else {
        return 0;
    }
} 

/********************************************************************************************************************
 * SourceBlock functions
 ********************************************************************************************************************/

SourceBlock create_source_mirror_block_from_file( unsigned char type, const char* filename, unsigned char xor ) {
    SourceBlock src = newSourceBlock( type ); // Screen data
    FILE *f = fopen( filename, "rb" ); // 0 if not source
    if ( !f ) {
        fprintf( stderr, "Error opening %s.\n", filename );
        exit(4);
    }
    for( src.bytes[ src.byte_counter ] = fgetc( f ); !feof( f ); src.bytes[ src.byte_counter ] = xor ^ fgetc( f ) ) src.byte_counter++;
    fclose( f );
    return src;
}

SourceBlock create_source_block_from_BASIC_text_file( const char* filename, int utf8 ) {
    SourceBlock src = newSourceBlock( 'B' ); // Basic source code
    FILE *f = fopen( filename, "rb" ); // 0 if not source
    if ( !f ) {
        fprintf( stderr, "Error opening %s.\n", filename );
        exit(4);
    }
    // for( src.bytes[ src.byte_counter ] = fgetc( f ); !feof( f ); src.bytes[ src.byte_counter ] = fgetc( f ) ) src.byte_counter++;
    src = encodeBasic( f, utf8 );
    fclose( f );
    return src;
}

SourceBlock create_source_block_from_file( const char* filename, int need_inversion ) {
    if ( ext_is( filename, "utf8.txt" ) ) return create_source_block_from_BASIC_text_file( filename, 1 );
    if ( ext_is( filename, "txt" ) ) return create_source_block_from_BASIC_text_file( filename, 0 );
    if ( ext_is( filename, "bas" ) ) return create_source_mirror_block_from_file( 'B', filename, 0 );

    if ( ext_is( filename, "scr" ) ) return create_source_mirror_block_from_file( 'S', filename, need_inversion ? 255 : 0 );

    if ( ext_is( filename, "pnm" ) ) return create_source_mirror_block_from_file( 'N', filename, 0 );
    printf( "Invalid file type (extension)!\n" );
    exit( 3 );
}

SourceBlock create_name_block() {
    SourceBlock src = newSourceBlock( 'N' ); // Name source code
    return src;
}

/********************************************************************************************************************
 * Main functions
 ********************************************************************************************************************/
void print_usage() {
    printf( "ptpcreate v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Create a new Primo .ptp file from input source blocks.\n");
    printf( "The -b option define a block source file. For many block use many -b options in necessery order.\n");
    printf( "The source block file extension define the source block type, for ptp creating.\n");
    printf( "The useable extensions are the next:\n");
    printf( "- .pnm         : programname for load (0x83).\n");
    printf( "- .bas         : define binary BASIC program code (0xF1).\n");
    printf( "- .txt         : define BASIC source program list int text format, primo encoded (0xF1).\n");
    printf( "- .utf8.txt    : define BASIC source program list int text format, utf8 encoded (0xF1).\n");
    printf( "- .scr         : define screen data (0xF5).\n");
    printf( "- .sys or .bin : Binary machine language program code (0xF9). It is necessary, the filename contains the absolute load address int the next format: -ORGxxxxH-.\n");
    printf( "- .run         : Run block for machine code (0xB9). 2 bytes only.\n");
    printf( "For absolute load address, and au- .sys or .bin : Binary machine language program code.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptpblocks -[b|i] <source_file> -o <created_ptp_filename>\n");
    printf( "-b source_block : A source block define for ptp.\n");
    printf( "-i screen_block : Use for inverz screen block.\n");
    printf( "-n name_on_tape : The tape name. Default name created from ptp filename.\n");
    printf( "-a address      : If you want, to sart the program after load automatically on the define address.\n");
    printf( "-v              : Verbose output.\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    FILE *ptp = 0;
    FILE *txt = 0;
    SourceBlock srcs[10];
    int need_default_name = 1;
    int sourceBlockCounter = 0;
    srcs[ sourceBlockCounter++ ] = create_name_block();
    while ( ( opt = getopt (argc, argv, "v?h:b:i:o:") ) != -1 ) {
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
            case 'n': // define load name
                for( srcs[ 0 ].byte_counter=0; srcs[ 0 ].byte_counter<16 && optarg[ srcs[ 0 ].byte_counter ]; srcs[ 0 ].byte_counter++ ) {
                    srcs[ 0 ].bytes[ srcs[0].byte_counter ] = optarg[ srcs[ 0 ].byte_counter ];
                }
                srcs[ 0 ].bytes[ srcs[ 0 ].byte_counter ] = 0;
                need_default_name = 0;
                break;
            case 'i': // open screen block and invert it
            case 'b': // open binari source block
                srcs[ sourceBlockCounter++ ] = create_source_block_from_file( optarg, opt == 'i' );
                if ( srcs[ sourceBlockCounter-1 ].type == 'N' ) { // Defined name block
                    sourceBlockCounter--;
                    srcs[ 0 ] = srcs[ sourceBlockCounter ];
                    if ( verbose ) printf( "New name block: '%s'\n", srcs[ 0 ].bytes );
                    need_default_name = 0;
                }
                break;
            case 'o': // Create ptp file
                ptp = fopen( optarg, "wb" );
                if ( !ptp ) {
                    fprintf( stderr, "Error creating %s.\n", optarg );
                    exit(4);
                }
                if ( need_default_name ) {
                    for( srcs[0].byte_counter=0; srcs[0].byte_counter<16 && optarg[ srcs[0].byte_counter ] && optarg[ srcs[0].byte_counter ] != '.'; srcs[0].byte_counter++ ) {
                        srcs[ 0 ].bytes[ srcs[0].byte_counter ] = optarg[ srcs[0].byte_counter ];
                    }
                    srcs[ 0 ].bytes[ srcs[0].byte_counter ] = 0;
                }
                break;
        }
    }
    if ( ptp && sourceBlockCounter ) {
        create_ptp_from( ptp, srcs, sourceBlockCounter );
        fclose( ptp );
    } else {
        print_usage();
    }
    return 0;
}
