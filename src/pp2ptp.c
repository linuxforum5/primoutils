/**
 * Primo .PP file convert to .PTP file.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include <libgen.h>

#define VM 0
#define VS 1
#define VB 'b'

int verbose = 0;
char ptp_name[ 17 ] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; // Name for load

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

void write_ptp_name_block_record( FILE *ptp, char *name ) {
    unsigned char crc = 0;
    unsigned char length = 0;
    for( ; length<17 && name[length]; length++ ) crc += name[length];

    unsigned char ptp_block_type = 0x55; // Ptp block
    uint16_t ptp_block_size = length + 4; // Name block size
    fwrite( &ptp_block_type, 1, 1, ptp );
    fwrite( &ptp_block_size, 2, 1, ptp );
    if ( verbose ) fprintf( stdout, "Create ptp block ( 0x%02X )\n", ptp_block_type );

    unsigned char tape_record_type = 0x83;
    unsigned char blockIndex0 = 0;
    if ( verbose ) fprintf( stdout, "Create %2d. tape name record ( 0x%02X ). Name for load is '%s'\n", blockIndex0, tape_record_type, name );
    fwrite( &tape_record_type, 1, 1, ptp );
    fwrite( &blockIndex0, 1, 1, ptp ); // Blokk sorszáma BCD formátumban. De itt kötelezően 0
    crc += length;
    fwrite( &length, 1, 1, ptp );
    fwrite( name, length, 1, ptp );
    fwrite( &crc, 1, 1, ptp );
}

unsigned char BCD( unsigned char binaryInput ) {
    unsigned char bcdResult = 0;
    unsigned char shift = 0;
    while ( binaryInput > 0) {
        bcdResult |= ( binaryInput % 10 ) << (shift++ << 2);
        binaryInput /= 10;
    }
    return bcdResult;
}

void write_ptp_data_record( FILE *pp, FILE *ptp, uint16_t loadAddress, unsigned char blockIndex0, unsigned char bytesCounter ) {
    uint16_t counter16 = bytesCounter ? bytesCounter : 256;

    unsigned char ptp_block_type = 0x55; // Ptp block
    uint16_t ptp_block_size = counter16 + 6; // Name block size
    fwrite( &ptp_block_type, 1, 1, ptp );
    fwrite( &ptp_block_size, 2, 1, ptp );
    if ( verbose ) fprintf( stdout, "Create ptp block ( 0x%02X )\n", ptp_block_type );

    unsigned char tape_record_type = 0xF9; // Gépi kódú blokk
    unsigned char crc = blockIndex0;
    if ( verbose ) fprintf( stdout, "Create %2d. tape data record ( 0x%02X ) from %d bytes. Load address is 0x%04X\n", blockIndex0, tape_record_type, counter16, loadAddress );
    fwrite( &tape_record_type, 1, 1, ptp );
    unsigned char BCDindex = BCD( blockIndex0 );
    fwrite( &BCDindex, 1, 1, ptp ); // Blokk sorszáma BCD formátumban
    fwrite( &loadAddress, 2, 1, ptp ); // Betöltési cím
    crc += loadAddress/256 + loadAddress%256;
    fwrite( &bytesCounter, 1, 1, ptp );
    crc += bytesCounter;
    while ( counter16-- ) {
        unsigned char byte = fgetc( pp );
        fwrite( &byte, 1, 1, ptp );
        crc += byte;
    }
    fwrite( &crc, 1, 1, ptp );
}

// Write ptp and tape close blocks in one
uint16_t write_ptp_last_block_record( FILE *pp, FILE *ptp, uint16_t startAddress, unsigned char blockIndex0 ) {
    unsigned char ptp_block_type = 0xAA; // Last ptp block
    uint16_t ptp_block_size = 8; // Last block size
    fwrite( &ptp_block_type, 1, 1, ptp );
    fwrite( &ptp_block_size, 2, 1, ptp );
    unsigned char tape_record_type = 0xB9; // System last tape record, autostart
    unsigned char crc = blockIndex0;
    if ( verbose ) fprintf( stdout, "Create last ptp block ( 0x%02X ). Start address is 0x%04X\n", ptp_block_type, startAddress );
    if ( verbose ) fprintf( stdout, "Create %2d. - last - tape record ( 0x%02X ).\n", blockIndex0, tape_record_type );
    fwrite( &tape_record_type, 1, 1, ptp );
    unsigned char BCDindex = BCD( blockIndex0 );
    fwrite( &BCDindex, 1, 1, ptp ); // Blokk sorszáma BCD formátumban
    fwrite( &startAddress, 2, 1, ptp ); // Betöltési cím
    crc += startAddress/256 + startAddress%256;
    fwrite( &crc, 1, 1, ptp );
    return ptp_block_size;
}

void write_ptp( FILE *pp, FILE *ptp, uint16_t loadAddress, uint16_t startAddress, uint16_t dataSize ) {
    unsigned char ptp_first_byte = 0xFF;
    fwrite( &ptp_first_byte, 1, 1, ptp );
    uint16_t fileSize = 0;
    int posFullSize = ftell( ptp );
    fwrite( &fileSize, 2, 1, ptp ); // Utólag kerül feltöltése

    write_ptp_name_block_record( ptp, ptp_name ); // 0x83
    uint16_t blockIndex0 = 1;
    while( dataSize ) {
        uint16_t chunkSize16 = ( dataSize > 255 ) ? 256 : dataSize;
        unsigned char chunkSize8 = chunkSize16; // 0 if 256
        write_ptp_data_record( pp, ptp, loadAddress, blockIndex0, chunkSize8 ); // 0xF9
        dataSize -= chunkSize16;
        loadAddress += chunkSize16;
        blockIndex0++;
    }
    write_ptp_last_block_record( pp, ptp, startAddress, blockIndex0 ); // 0xB9

    uint16_t fullFileSize = ftell( ptp );
    fseek( ptp, posFullSize, SEEK_SET );
    fwrite( &fullFileSize, 2, 1, ptp ); // Write the correst size
}

void conv_pp_file( FILE *pp, FILE *ptp ) {
    unsigned char byte = 0;
    uint16_t loadAddress;
    uint16_t startAddress;
    uint16_t dataSize;
    fseek( pp, 0, SEEK_SET );
    fblockread( &loadAddress, 2, pp );
    fblockread( &startAddress, 2, pp );
    for( ;!feof( pp );fgetc( pp ) ) dataSize++;
    fseek( pp, 4, SEEK_SET );
    if ( ptp ) {
        write_ptp( pp, ptp, loadAddress, startAddress, dataSize );
        fclose( ptp );
    }
    fclose( pp );
}

void copy_to_name( char* basename ) {
    int i = 0;
    for( i = 0; i<16 && basename[ i ]; i++ ) ptp_name[ i ] = basename[ i ];
    for( int j = i; j < 17; j++ ) ptp_name[ j ] = 0;
}

char* copyStr( char *str, int chunkPos ) {
    int size = 0;
    while( str[size++] );
    char *newStr = malloc( size );
    for( int i=0; i<size; i++ ) newStr[i] = str[ i ];
    if ( chunkPos>0 && chunkPos<size ) { // Chunk extension
        if ( newStr[ size - chunkPos ] == '.' ) newStr[ size - chunkPos ] = 0;
    }
    return newStr;
}

char* copyStr3( char *str1, char *str2, char *str3 ) {
    int size1 = 0; while( str1[size1++] );
    int size2 = 0; while( str2[size2++] );
    size2--;
    int size3 = 0; while( str3[size3++] );
    char *newStr = malloc( size1 + size2 + size3 );
    for( int i=0; i<size1; i++ ) newStr[ i ] = str1[ i ];
    if ( size1 ) newStr[ size1 - 1 ] = '/';
    for( int i=0; i<size2; i++ ) newStr[ size1 + i ] = str2[ i ];
    for( int i=0; i<size3; i++ ) newStr[ size1 + size2 + i ] = str3[ i ];
    return newStr;
}

int is_dir( const char *path ) {
    struct stat path_stat;
    stat( path, &path_stat );
    return S_ISDIR( path_stat.st_mode );
}

void print_usage() {
    printf( "pp2ptp v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Convert Primo .pp file to .ptp format.\n");
    printf( "Copyright 2022 by László Princz\n");
    printf( "Usage:\n");
    printf( "pp2ptp [options] -i <pp_filename> [ -o <ptp_filename> ]\n");
    printf( "Command line option:\n");
    printf( "-o <ptp_file> : Output filename or directory. Default, the input filename with ptp extension.\n");
    printf( "-n <name>     : Name for the load. Default, tha basename of the input file.\n");
    printf( "-v            : Verbose output.\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int opt = 0;
    char *srcBasename = 0;
    char *destDir = 0;
    FILE *ppFile = 0;
    FILE *ptpFile = 0;

    while ( ( opt = getopt (argc, argv, "v?h:i:n:o:") ) != -1 ) {
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
            case 'n': // define ptp name
                copy_to_name( optarg );
                break;

            case 'i': // open pp file
                if ( !( ppFile = fopen( optarg, "rb" ) ) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                srcBasename = copyStr( basename( optarg ), 4 );
                destDir = copyStr( dirname( optarg ), 0 );
                if ( !ptp_name[ 0 ] ) copy_to_name( srcBasename );
            break;
            case 'o': // create ptp file
                if ( is_dir( optarg ) ) { // Az output egy mappa
                    destDir = copyStr( optarg, 0 );
                } else {
                    if ( !( ptpFile = fopen( optarg, "wb" ) ) ) {
                        fprintf( stderr, "Error creating %s.\n", optarg);
                        exit(4);
                    }
                }
                break;
            default:
                break;
        }
    }

    if ( ppFile ) {
        if ( !ptpFile ) { // Nincs megadott tap fájl. destDir biztosan nem null
            char *ptpName = copyStr3( destDir, srcBasename, ".ptp" );
            if ( !( ptpFile = fopen( ptpName, "wb" ) ) ) {
                fprintf( stderr, "Error creating %s.\n", ptpName );
                exit(4);
            }
            fprintf( stdout, "Create file %s\n", ptpName );
        }
        conv_pp_file( ppFile, ptpFile );
        fprintf( stdout, "Ok\n" );
    } else {
        print_usage();
    }
    return 0;
}
