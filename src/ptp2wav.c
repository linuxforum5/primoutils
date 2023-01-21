/**
 * Based on Varga Viktor's primotools : https://github.com/vargaviktor/primotools
 *
 * program ptp2wav;
 *
 * PRIMO "0" bit, 936 usec:  (2*) 8 minta, itt 1000 usec
 * PRIMO "1" bit, 312 usec:  (2*) 3 minta, itt  375 usec
 *
 * PRIMO "csend", 0.25 masodpercig (250000 usec)
 *
 *  [200 minta]
 *
 *
 * PRIMO allomanyszinkron mezo:
 *
 *  512 db $AA (10101010) byte, tehat 512*4*(16+6) = [45056 minta]
 *
 * PRIMO blokkszinkron mezo:
 *
 *  96 db $FF byte, [468 minta] + 3 db $D3 (11010011) byte [234 minta]
 *
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

// unsigned long wrtcnt = 0; // For wav size

const unsigned char  SILENCE = 0x80;
const unsigned char POS_PEAK = 0xf8;
const unsigned char NEG_PEAK = 0x08;

u_int8_t bit1_peak_counter = 3; // const char bt1[ 6 ]  = { POS_PEAK, POS_PEAK, POS_PEAK, NEG_PEAK, NEG_PEAK, NEG_PEAK };
u_int8_t bit0_peak_counter = 8; // const char bt0[ 16 ] = { POS_PEAK, POS_PEAK, POS_PEAK, POS_PEAK, POS_PEAK, POS_PEAK, POS_PEAK, POS_PEAK, NEG_PEAK, NEG_PEAK, NEG_PEAK, NEG_PEAK, NEG_PEAK, NEG_PEAK, NEG_PEAK, NEG_PEAK };

const unsigned int defaultBaud = 8000; // max 13000

unsigned char verbose = 0;

/* WAV file header structure */
/* should be 1-byte aligned */
#pragma pack(1)
struct wav_header { // 44 bytes
    char           riff[ 4 ];       // 4 bytes
    unsigned int   rLen;            // 4 bytes
    char           WAVE[ 4 ];       // 4 bytes
    char           fmt[ 4 ];        // 4 bytes
    unsigned int   fLen;            /* 0x1020 */
    unsigned short wFormatTag;      /* 0x0001 */
    unsigned short nChannels;       /* 0x0001 */
    unsigned int   nSamplesPerSec;
    unsigned int   nAvgBytesPerSec; // nSamplesPerSec*nChannels*(nBitsPerSample/8)
    unsigned short nBlockAlign;     /* 0x0001 */
    unsigned short nBitsPerSample;  /* 0x0008 */
    char           datastr[ 4 ];    // 4 bytes
    unsigned int   data_size;       // 4 bytes
} waveHeader = {
    'R','I','F','F', //     Chunk ID - konstans, 4 byte hosszú, értéke 0x52494646, ASCII kódban "RIFF"
    0,               //     Chunk Size - 4 byte hosszú, a fájlméretet tartalmazza bájtokban a fejléccel együtt, értéke 0x01D61A72 (decimálisan 30808690, vagyis a fájl mérete ~30,8 MB)
    'W','A','V','E', //     Format - konstans, 4 byte hosszú,értéke 0x57415645, ASCII kódban "WAVE"
    'f','m','t',' ', //     SubChunk1 ID - konstans, 4 byte hosszú, értéke 0x666D7420, ASCII kódban "fmt "
    16,              //     SubChunk1 Size - 4 byte hosszú, a fejléc méretét tartalmazza, esetünkben 0x00000010
    1,               //     Audio Format - 2 byte hosszú, PCM esetében 0x0001
    1,               //     Num Channels - 2 byte hosszú, csatornák számát tartalmazza, esetünkben 0x0002
    defaultBaud,     //     Sample Rate - 4 byte hosszú, mintavételezési frekvenciát tartalmazza, esetünkben 0x00007D00 (decimálisan 32000)
    defaultBaud,     //     Byte Rate - 4 byte hosszú, értéke 0x0000FA00 (decmálisan 64000)
    1,               //     Block Align - 2 byte hosszú, az 1 mintában található bájtok számát tartalmazza - 0x0002
    8,               //     Bits Per Sample - 2 byte hosszú, felbontást tartalmazza bitekben, értéke 0x0008
    'd','a','t','a', //     Sub Chunk2 ID - konstans, 4 byte hosszú, értéke 0x64617461, ASCII kódban "data"
    0                //     Sub Chunk2 Size - 4 byte hosszú, az adatblokk méretét tartalmazza bájtokban, értéke 0x01D61A1E
};
#pragma pack()

void wav_init( FILE *wav ) { fwrite( &waveHeader, sizeof( waveHeader ), 1, wav ); }

void wav_close( FILE *wav ) {
    int full_size = ftell( wav );
    fseek( wav, 4, SEEK_SET );
    fwrite( &full_size, sizeof( full_size ), 1, wav ); // Wave header 2. field : filesize with header. First the lowerest byte

    int data_size = full_size - sizeof( waveHeader );
    fseek( wav, sizeof( waveHeader ) - 4 ,SEEK_SET ); // data chunk size position: 40
    fwrite( &data_size, sizeof( data_size ), 1, wav );
    fclose( wav );
}

void write_primo_silence( FILE *wav ) { for( int i=0; i<2000; i++ ) fputc( SILENCE, wav ); }

void write_peaks( FILE *wav, unsigned char cnt ) {
    for( unsigned char i = 0; i < cnt; i++ ) fputc( POS_PEAK, wav );
    for( unsigned char i = 0; i < cnt; i++ ) fputc( NEG_PEAK, wav );
}

void write_bit_into_wav( FILE *wav, unsigned char bit ) { write_peaks( wav, bit ? bit1_peak_counter : bit0_peak_counter ); }

void write_byte_into_wav( FILE* wav, unsigned char c ) {
    for( int i = 0; i < 8; i++ ) {
        unsigned char bit = ( c << i ) & 128; // 7. bit a léptetés után. 1. lépés után az eredeti 6. bitje.
        write_bit_into_wav( wav, bit );
    }
}

void write_block_into_wav( FILE *wav, int counter, unsigned char byte ) {
    for( int i=0; i<counter; i++ ) {
        write_byte_into_wav( wav, byte );
    }
}

void copy_bytes_into_wav( FILE *ptp, unsigned char counter1, unsigned char counter2, FILE* wav ) { // if counter1 == 0, then means 256. Counter2==0 means 0.
    u_int16_t cnt = counter1;
    if ( !cnt ) cnt=256;
    cnt+=counter2;
    while ( cnt-- != 0 ) {
        write_byte_into_wav( wav, fgetc( ptp ) );
    }
}

void copy_tape_close_block( FILE *ptp, FILE* wav, unsigned char blockIndex, int is_autostart ) { // blockType and blockIndex already readed from ptp
    if ( verbose ) printf( "\t%02X. tape block type: Close\n", blockIndex );
    if ( is_autostart ) {
        copy_bytes_into_wav( ptp, 3, 0, wav ); // 0 == 256
    } else {
        copy_bytes_into_wav( ptp, 1, 0, wav ); // 0 == 256
    }
}

void copy_tape_data_block( FILE *ptp, FILE* wav, unsigned char blockIndex ) { // blockType and blockIndex already readed from ptp
    if ( verbose ) printf( "\t%02X. tape block type: Data\n", blockIndex );
    unsigned char loadAddressL = fgetc( ptp );
    unsigned char loadAddressH = fgetc( ptp );
    unsigned char byteCounter = fgetc( ptp ); // If 0, then 256 bytes
    write_byte_into_wav( wav, loadAddressL );
    write_byte_into_wav( wav, loadAddressH );
    write_byte_into_wav( wav, byteCounter );
    copy_bytes_into_wav( ptp, byteCounter, 1, wav ); // 0 == 256
}

void copy_tape_name_block( FILE *ptp, FILE* wav, unsigned char blockIndex ) { // blockType and blockIndex already readed from ptp
    if ( verbose ) printf( "\t%02X. tape block type: Name\n", blockIndex );
    unsigned char namesize = fgetc( ptp );
    write_byte_into_wav( wav, namesize );
    copy_bytes_into_wav( ptp, namesize, 1, wav );
}

int copy_tape_block( FILE *ptp, FILE* wav ) {
    unsigned char tapeBlockType = fgetc( ptp );
    unsigned char blockIndex = fgetc( ptp );
    write_byte_into_wav( wav, tapeBlockType );
    write_byte_into_wav( wav, blockIndex );
    switch( tapeBlockType ) {
        case 0x83 :
        case 0x87 : copy_tape_name_block( ptp, wav, blockIndex ); break;
        case 0xF1 :
        case 0xF5 :
        case 0xF7 :
        case 0xF9 : copy_tape_data_block( ptp, wav, blockIndex ); break;
        case 0xB1 :
        case 0xB5 :
        case 0xB7 : copy_tape_close_block( ptp, wav, blockIndex, 0 ); break;
        case 0xB9 : copy_tape_close_block( ptp, wav, blockIndex, 1 ); break;
        default:
            fprintf( stderr, "Invalid tape block type: 0x%02X\n", tapeBlockType );
            exit(1);
            break;
    }
    return feof( ptp );
}

unsigned char copy_ptp_block( FILE *ptp, FILE* wav ) {
    unsigned char ptpBlockType = fgetc( ptp );
    if ( ptpBlockType == 0x55 || ptpBlockType == 0xAA ) {
        if ( verbose ) printf( "PTP Block type: 0x%02X\n", ptpBlockType );
        write_block_into_wav( wav, 96, 0xFF );
        write_block_into_wav( wav, 3, 0xD3 );
        u_int16_t ptpBlockSize = 0;
        fread( &ptpBlockSize, 2, 1, ptp );
        while( copy_tape_block( ptp, wav ) );
        // unsigned char tapeBlockIndex = fgetc( ptp );    
    } else {
        fprintf( stderr, "Invalid ptp block type: 0x%02X\n", ptpBlockType );
        exit(1);
    }
    return ptpBlockType;
}

void convert( FILE *ptp, FILE* wav ) {
    if ( 0xFF == fgetc( ptp ) ) { // PTP first byte
        u_int16_t ptpLength = 0;
        fread( &ptpLength, 2, 1, ptp );
        write_primo_silence( wav );
        write_block_into_wav( wav, 512, 0xAA ); // File szinkron blokk: 20msec szünet, 512*AA
        unsigned char ptpBlockType = copy_ptp_block( ptp, wav );
        while( ptpBlockType != 0xAA ) {
            ptpBlockType = copy_ptp_block( ptp, wav );
        }
    } else {
        fprintf( stderr, "Invalid ptp fileformat.\n" );
        exit(1);
    }
}

void print_usage() {
    printf( "ptp2wav v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Microkey Primo ptp to PCM wave file converter.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptp2wav [options] -i <input_ptp_filename> -o <output_wav_filename>\n");
    printf( "If you want to best fastest load, use the '-f 13000 -0 7' options.\n");
    printf( "Command line option:\n");
    printf( "-0 <samples> : Set the umber of samples for bit 0. Default value is %d\n", bit0_peak_counter );
    printf( "-1 <samples> : Set the umber of samples for bit 1. Default value is %d\n", bit1_peak_counter );
    printf( "-f <baud>    : Fake baud for load instead of %d. (default %d). 13000 is a good fast value.\n", defaultBaud, defaultBaud );
    printf( "-h           : prints this text\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int finished = 0;
    int arg1;
    FILE *ptpFile = 0, *wav = 0;

    while (!finished) {
        switch (getopt (argc, argv, "?h:i:o:0:1:f:")) {
            case -1:
            case ':':
                finished = 1;
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'f': // fake baud
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-f'.\n");
                    exit(2);
                } else {
                    waveHeader.nSamplesPerSec = arg1;
                    waveHeader.nAvgBytesPerSec = waveHeader.nSamplesPerSec * waveHeader.nChannels * ( waveHeader.nBitsPerSample / 8 );
                }
                break;
            case '0':
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-g'.\n");
                    exit(2);
                } else {
                    if ( arg1<=0 ) {
                        fprintf( stderr, "Illegal samples counter value for bit 0: %i.\n", arg1);
                    }
                    bit0_peak_counter = arg1;
                }
                break;
            case '1':
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-g'.\n");
                    exit(2);
                } else {
                    if ( arg1<=0 ) {
                        fprintf( stderr, "Illegal samples counter value for bit 1: %i.\n", arg1);
                    }
                    bit1_peak_counter = arg1;
                }
                break;
            case 'i':
                if ( !(ptpFile = fopen( optarg, "rb")) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
            case 'o':
                if ( !(wav = fopen( optarg, "wb")) ) {
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
    } else if ( !wav ) {
        print_usage();
    } else {
        wav_init( wav );
        convert( ptpFile, wav );
        wav_close( wav );
        fclose( ptpFile );
    }

    return 0;
}
