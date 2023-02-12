/**
 * Egy ptp fájlból turbo wav fájlt generál
 * A ptp2c segédprogrammal kell konvertálni mind a loader mind a payload fájlokat.
 * Ez a program összefűzi a kettőt 1 wav fájllá, először betölti a loader-t, majd a loader betölti a paylod adatot.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"

#include <stdint.h>
#define guint unsigned int
#define guint8 unsigned char

#include <stdint.h>

// #include "Ptp.h"
#include "wavloader.c"

#define VM 0
#define VS 1
#define VB 'b'

// PTP_DATA payload;

/****************************************************************************************************************************************
 * Wav functions
 ****************************************************************************************************************************************/
const unsigned char  SILENCE = 0x80;
const unsigned char POS_PEAK = 0xf8; // Origi = f8
const unsigned char NEG_PEAK = 0x08; // Origi = 08

const unsigned int defaultBaud = 62000; // 55555; // 55447; // 26042; // 44000; // 54000; // 54000 // 2*25000;
const unsigned char bottom = POS_PEAK;
// const unsigned char bottom = SILENCE;
// const unsigned char bottom = NEG_PEAK;

// const unsigned char top  = POS_PEAK;
// const unsigned char top  = SILENCE;
const unsigned char top  = NEG_PEAK;
const unsigned char middle  = SILENCE;
int verbose = 0;
int autoBasicRun = 0;

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

void wav_write_sample( FILE *wav, unsigned char sample, unsigned int counter ) { for( int i=0; i<counter; i++ ) fputc( sample, wav ); }

void wav_write_silence( FILE *wav, int pulse_counter ) { for( int i=0; i<pulse_counter; i++ ) fputc( SILENCE, wav ); }

unsigned long payload_start_position = 0;

void wav_close( FILE *wav, unsigned long payload_size ) {
    int full_size = ftell( wav );
    fseek( wav, 4, SEEK_SET );
    fwrite( &full_size, sizeof( full_size ), 1, wav ); // Wave header 2. field : filesize with header. First the lowerest byte
    int data_size = full_size - sizeof( waveHeader );
    fseek( wav, sizeof( waveHeader ) - 4 ,SEEK_SET ); // data chunk size position: 40
    fwrite( &data_size, sizeof( data_size ), 1, wav );
    fclose( wav );
    printf( "Full size is %dKB\n", payload_size/1024 );
    printf( "Full time is %d seconds\n", data_size / defaultBaud );
    unsigned long speed = (long)payload_size * (long)8 * (long)defaultBaud / (long)data_size;
    unsigned long payload_wav_size = full_size - payload_start_position;
    unsigned long turbo_speed = (long)payload_size * (long)8 * (long)defaultBaud / payload_wav_size;
    printf( "Speed is %lu baud (turbo speed - without original slow loader - is %lu baud)\n", speed, turbo_speed );
}

/****************************************************************************************************************************************
 * Standard wav modules for ROM BASIC LOAD command
 * http://primo.homeserver.hu/html/konvertfajlok.html
 ****************************************************************************************************************************************/
const unsigned char bit1_peak_slow_counter = 13; //b12; // 11
const unsigned char bit0_peak_slow_counter = 39; // 36; // 32

void write_slow_peaks( FILE *wav, unsigned char cnt ) {
    for( unsigned char i = 0; i < cnt; i++ ) fputc( POS_PEAK, wav );
    for( unsigned char i = 0; i < cnt; i++ ) fputc( NEG_PEAK, wav );
}

void write_slow_bit_into_wav( FILE *wav, unsigned char bit ) { write_slow_peaks( wav, bit ? bit1_peak_slow_counter : bit0_peak_slow_counter ); }

unsigned char write_slow_byte_into_wav( FILE* wav, unsigned char c, unsigned char crc ) {
    for( int i = 0; i < 8; i++ ) {
        unsigned char bit = ( c << i ) & 128; // 7. bit a léptetés után. 1. lépés után az eredeti 6. bitje.
        write_slow_bit_into_wav( wav, bit );
    }
    return crc+c;
}

unsigned char write_slow_bytes_into_wav( FILE* wav, const unsigned char bytes[], unsigned int counter, unsigned char crc ) {
    for( int i = 0; i < counter; i++ ) {
        crc = write_slow_byte_into_wav( wav, bytes[i], crc );
    }
    return crc;
}

void write_slow_block_into_wav( FILE *wav, int counter, unsigned char byte ) {
    for( int i=0; i<counter; i++ ) {
        write_slow_byte_into_wav( wav, byte, 0 );
    }
}

unsigned char write_loader_block_header( FILE *wav ) {
    write_slow_block_into_wav( wav, 96, 0xFF ); // Block szinkron
    write_slow_block_into_wav( wav, 3, 0xD3 );
    return 0;
}

void write_loader_b9_block( FILE *wav, unsigned char block_index, uint16_t run_address ) {
    unsigned char crc = write_loader_block_header( wav );
    write_slow_byte_into_wav( wav, 0xb9, 0 );
    crc = write_slow_byte_into_wav( wav, block_index, crc );    // Block counter
    crc = write_slow_byte_into_wav( wav, run_address % 256, crc );
    crc = write_slow_byte_into_wav( wav, run_address / 256, crc );
    write_slow_byte_into_wav( wav, crc, 0 );
}

void write_loader_f9_block( FILE *wav, unsigned char block_index, uint16_t load_address, const unsigned char* bytes, uint16_t block_size ) {
    unsigned char crc = write_loader_block_header( wav );
    write_slow_byte_into_wav( wav, 0xf9, 0 );
    crc = write_slow_byte_into_wav( wav, block_index, crc );    // Block counter
    crc = write_slow_byte_into_wav( wav, load_address % 256, crc );
    crc = write_slow_byte_into_wav( wav, load_address / 256, crc );
    crc = write_slow_byte_into_wav( wav, ( block_size == 256 ) ? 0 : block_size, crc );
    crc = write_slow_bytes_into_wav( wav, bytes, block_size, crc );
    write_slow_byte_into_wav( wav, crc, 0 );
}

void write_loader_name_block( FILE *wav, const char* name ) {
    unsigned char crc = write_loader_block_header( wav );
    write_slow_byte_into_wav( wav, 0x83, 0 ); // Program name block
    crc = write_slow_byte_into_wav( wav, 0, crc );    // Block counter
    unsigned char name_length = strlen( name );
    crc = write_slow_byte_into_wav( wav, name_length, crc );    // Name length
    crc = write_slow_bytes_into_wav( wav, name, name_length, crc );    // Name
    write_slow_byte_into_wav( wav, crc, 0 );    // Name length    
}

void save_loader( FILE *wav ) {
    wav_write_silence( wav, 2000 );               // File szinkron blokk: 20msec szünet, 512*AA
    write_slow_block_into_wav( wav, 512, 0xAA );
    write_loader_name_block( wav, "Slideshow Loader" );
    unsigned char block_index = 1;

//    char *payload_name = "screen loader";
//    unsigned int name_index_from = screen_loader.byte_counter - 36;
//    unsigned int name_length = strlen( payload_name );
//    for( unsigned int i=0; i<name_length; i++ ) {
//        screen_loader.bytes[ name_index_from + i ] = payload_name[ i ];
//    }

    for( uint16_t writed = 0; writed < screen_loader.byte_counter; ) {
        uint16_t block_size = screen_loader.byte_counter - writed;
        if ( block_size > 256 ) {
            block_size = 256;
            write_loader_f9_block( wav, block_index++, screen_loader.load_address + writed, screen_loader.bytes + writed, block_size );
        } else { // Last block
            write_loader_f9_block( wav, block_index++, screen_loader.load_address + writed, screen_loader.bytes + writed, block_size );
            write_loader_b9_block( wav, block_index++, screen_loader.run_address );
        }
        writed += block_size;
    }
}
/****************************************************************************************************************************************
 * Turbo functions
 ****************************************************************************************************************************************/
/*
 * 4Mhz-es érték számítása(V)=V*2.5/4
 * 55556 : 1 sample = 1/44000 ~ 18us | 4Mhz: ~ 11.25us
 * 54kHz : 18,518518519 ~ 11,574074074
 * 44kHz : 1 sample = 1/44000 = 22,727272727us | 4Mhz:14,204545455 ~ 14.2us
 * 40kHz : 1 sample = 1/40000 = 25us | 4Mhz:15,626us
 * 26042Hz: ~ 24us
 * 1 tick = 80us
 * 1 bit = 240us+240us
 */
const unsigned int tick = 1; // ~ 37us
// tick=2 : 10, 7, 5, 2, 3
#define bit0 6
#define bit1 1 * bit0

void save_turbo_bit( unsigned char bit, FILE *wav ) { // TTTTTTTTTTTTTTBBTBTTBBTBTTBBTBTTBBTBTTBB
//    wav_write_sample( wav, top, 2 );    //
//    wav_write_sample( wav, bottom, 1 );    //
    // wav_write_sample( wav, top, 2*tick );         // Ez ment 1 tick-kel is. Miért okoz hibát 3 tick?
//    wav_write_sample( wav, bottom, 1 );      // ~ 48us
    if ( bit ) { // Ha a bit 1
        wav_write_sample( wav, bottom, 4 );    //
//        wav_write_sample( wav, top, 4 );    //
// printf( "0x%02X %d\n", next_bit, bit1 );
    } else { // Ha a bit 0
//        wav_write_sample( wav, bottom, 4 );    //
        wav_write_sample( wav, bottom, 2 );    //
// printf( "0x%02X %d\n", next_bit, bit0 );
    }
//    wav_write_sample( wav, bottom, 1 );      // ~ 48us
    wav_write_sample( wav, top, 3 );      // ~ 48us
}

void save_turbo_byte( unsigned char byte, FILE *wav ) {
//    wav_write_sample( wav, top, 2*tick ); // Postfix HIGH ~ 95us
//    wav_write_sample( wav, top, 2 ); // Postfix HIGH ~ 95us
//    wav_write_sample( wav, bottom, 1 ); // Postfix HIGH ~ 95us
    for( int bit=1; bit<256; bit=bit*2 ) {
        save_turbo_bit( byte & bit, wav );
    }
    wav_write_sample( wav, top, 3 ); // Time for store byte
}

unsigned char reverse(unsigned char b) {
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

unsigned int save_screen_block( FILE *wav, unsigned char v, int last ) {
    save_turbo_byte( 0, wav ); // 1 == Read next block
    save_turbo_byte( 0, wav ); // 1 == Read next block

    save_turbo_byte( 0, wav ); // 1 == Read next block
    save_turbo_byte( 0x18, wav ); // 1 == Read next block

    for( int i = 0; i < 32 * 192; i++ ) {
        save_turbo_byte( v, wav ); // 1 == Read next block
        v = 255 - v;
    }
    save_turbo_byte( last ? 0 : 1, wav ); // 1 == Read next block
    return 6144;
}

void save_scr( FILE *wav, FILE *scr, int last ) {
    save_turbo_byte( 0, wav ); // 1 == Read next block
    save_turbo_byte( 0, wav ); // 1 == Read next block

    save_turbo_byte( 0, wav ); // 1 == Read next block
    save_turbo_byte( 0x18, wav ); // 1 == Read next block

    for( int i = 0; i < 6144; i++ ) {
        save_turbo_byte( fgetc( scr ), wav ); // 1 == Read next block
    }

    save_turbo_byte( last ? 0 : 1, wav ); // 1 == Read next block
}

#define MAX_FILENAME_LENGTH 1000

unsigned int save_payload( FILE *wav, char* filename, int length, int last ) {
    char fn[ MAX_FILENAME_LENGTH ] = "";
    for( int i=0; i<length; i++ ) fn[i]=filename[i];
    fn[length]=0;
    printf( "*** '%s'\n", fn );
    FILE *scr;
    if ( scr = fopen( fn, "rb" ) ) {
        save_scr( wav, scr, last );
        fclose( scr );
        return 6144;
    }
    return 0;
//    wav_write_sample( wav, top, 8*tick ); // ~ 48us synchron
//    wav_write_sample( wav, bottom, ( bit1 + bit0 ) * tick / 2 - 4*tick ); // ~ 48us synchron
//     wav_write_sample( wav, top, 1 );
//    wav_write_sample( wav, top, 8*tick ); // ~ 48us synchron

//    save_turbo_byte( 255, wav ); // 1 == Read next block
//    save_turbo_byte( 255-0xE8, wav ); // 1 == Read next block
/*    save_screen_block( wav,   0, 1 );
    save_screen_block( wav, 255, 1 );
    save_screen_block( wav,   2, 1 );
    save_screen_block( wav,  10, 1 );
    save_screen_block( wav,  42, 1 );
    save_screen_block( wav, 170, 0 );*/
}

/****************************************************************************************************************************************
 * Main section
 ****************************************************************************************************************************************/
void print_usage() {
    printf( "Slideshow v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Create slideshow as turbo wav file.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "slideshow -i <scr_file> -o <slideshow_wav_filename>\n");
    printf( "-i      : The showed scr file. More than one is useable.\n");
    printf( "-v      : Verbose mode.\n");
    exit( 1 );
}

#define MAX_FILENAME_PUFFEL_LENGTH 65000
#define MAX_FILENAME 1000

int main(int argc, char *argv[]) {
    int opt = 0;
    FILE *wav = 0;
    FILE *scr = 0;
    char filenames[ MAX_FILENAME_PUFFEL_LENGTH ] = "";
    int filename_lengths[ MAX_FILENAME ];
    int filename_counter = 0;
    while ( ( opt = getopt (argc, argv, "va?h:i:o:") ) != -1 ) {
        switch ( opt ) {
            case -1:
            case ':': break;
            case '?':
            case 'h': print_usage(); break;
            case 'v': verbose = 1; break;
            case 'i': 
                scr = fopen( optarg, "rb" );
                if ( !scr ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                } else {
                    strcat( filenames, optarg );
                    fclose( scr );
                    filename_lengths[ filename_counter++ ] = strlen( optarg );
                }
                break;
            case 'o': // create wav file
                wav = fopen( optarg, "wb" );
                if ( !wav ) {
                    fprintf( stderr, "Error creating %s.\n", optarg);
                    exit(4);
                }
                break;
        }
    }
    if ( wav ) {
        wav_init( wav );
        save_loader( wav );
        wav_write_silence( wav, 20000 );

        payload_start_position = ftell( wav );

        unsigned long payload_size = 0;
        payload_size += save_screen_block( wav, 255, 0 );
        int start_pos = 0;
        for( int i=0; i<filename_counter; i++ ) {
            payload_size += save_payload( wav, filenames + start_pos, filename_lengths[ i ], i == filename_counter-1 );
            start_pos += filename_lengths[ i ];
        }
        wav_close( wav, payload_size );
    } else {
        print_usage();
    }
    return 0;
}
