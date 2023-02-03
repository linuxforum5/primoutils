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

#include "Ptp.h"
#include "wavloader.c"

#define VM 0
#define VS 1
#define VB 'b'

PTP_DATA payload;

/****************************************************************************************************************************************
 * Wav functions
 ****************************************************************************************************************************************/
const unsigned char  SILENCE = 0x80;
const unsigned char POS_PEAK = 0xff; // Origi = f8
const unsigned char NEG_PEAK = 0x00; // Origi = 08

const unsigned int defaultBaud = 54000; // 54000 // 2*25000;
const unsigned int tick = 2; // 1 byte ~ 48us
const unsigned char bottom = POS_PEAK;
// const unsigned char bottom = SILENCE;
// const unsigned char bottom = NEG_PEAK;

// const unsigned char top  = POS_PEAK;
// const unsigned char top  = SILENCE;
const unsigned char top  = NEG_PEAK;
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

void wav_close( FILE *wav ) {
    int full_size = ftell( wav );
    fseek( wav, 4, SEEK_SET );
    fwrite( &full_size, sizeof( full_size ), 1, wav ); // Wave header 2. field : filesize with header. First the lowerest byte
    int data_size = full_size - sizeof( waveHeader );
    fseek( wav, sizeof( waveHeader ) - 4 ,SEEK_SET ); // data chunk size position: 40
    fwrite( &data_size, sizeof( data_size ), 1, wav );
    fclose( wav );
}

/****************************************************************************************************************************************
 * Standard wav modules for ROM BASIC LOAD command
 * http://primo.homeserver.hu/html/konvertfajlok.html
 ****************************************************************************************************************************************/
const unsigned char bit1_peak_slow_counter = 12; // 11
const unsigned char bit0_peak_slow_counter = 36; // 32

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
    write_loader_name_block( wav, "Turbo loader" );
    unsigned char block_index = 1;

    unsigned int name_index_from = turbo_loader.byte_counter - 36;
    unsigned int name_length = strlen( payload.name );
    for( unsigned int i=0; i<name_length; i++ ) {
        turbo_loader.bytes[ name_index_from + i ] = payload.name[ i ];
    }

    for( uint16_t writed = 0; writed < turbo_loader.byte_counter; ) {
        uint16_t block_size = turbo_loader.byte_counter - writed;
        if ( block_size > 256 ) {
            block_size = 256;
            write_loader_f9_block( wav, block_index++, turbo_loader.load_address + writed, turbo_loader.bytes + writed, block_size );
        } else { // Last block
            write_loader_f9_block( wav, block_index++, turbo_loader.load_address + writed, turbo_loader.bytes + writed, block_size );
            write_loader_b9_block( wav, block_index++, turbo_loader.run_address );
        }
        writed += block_size;
    }
}
/****************************************************************************************************************************************
 * Turbo functions
 ****************************************************************************************************************************************/
/*
 * 1 sample = 1/48000 = 20us
 * 1 tick = 80us
 * 1 bit = 240us+240us
 */
void save_turbo_bit( unsigned char bit, FILE *wav ) { // TTTTTTTTTTTTTTBBTBTTBBTBTTBBTBTTBBTBTTBB
    wav_write_sample( wav, top, 2*tick );         // Ez ment 1 tick-kel is. Miért okoz hibát 3 tick?
    if ( bit ) { // Ha a bit 1
        wav_write_sample( wav, bottom, tick );    // ~ 48us
        wav_write_sample( wav, top, 2*tick );     // ~ 95us
    } else { // Ha a bit 0
        wav_write_sample( wav, bottom, 2*tick );  // ~ 95us
        wav_write_sample( wav, top, tick );       // ~ 48us
    }
    wav_write_sample( wav, bottom, tick );      // ~ 48us
}

void save_turbo_byte( unsigned char byte, FILE *wav ) {
    for( int bit=1; bit<256; bit=bit*2 ) {
        save_turbo_bit( byte & bit, wav );
    }
    wav_write_sample( wav, top, 2*tick ); // Postfix HIGH ~ 95us
}

void save_turbo_addr( unsigned int addr, FILE *wav ) {
    save_turbo_byte( addr % 0x100, wav );
    save_turbo_byte( addr / 0x100, wav );
}

void save_turbo_header( FILE *wav ) {
//    wav_write_sample( wav, top, tick );    // ~ 48us
    wav_write_sample( wav, bottom, tick ); // ~ 48us synchron
}

void save_turbo_block_header( PTP_BLOCK_DATA block, FILE *wav ) {
    save_turbo_addr( block.load_address, wav );              // load address
    save_turbo_addr( block.byte_counter, wav );              // byte counter
printf( "Create turbo block (size=0x%04X)\n", block.byte_counter );
    // printf( "Create Big Turbo Block with size 0x%04X\n", block.byte_counter );
    wav_write_sample( wav, top, block.byte_counter/1200*tick ); // For line writeing 80f0(33008) esetén 1A(26)*tick
}

void save_payload_block_selector( int last, FILE *wav  ) {
    if ( last ) { // Last block
        if ( autoBasicRun ) {
            save_turbo_byte( 3, wav ); // 0 == RUN next ADDRESS or BASIC
        } else if ( payload.run_address ) { // Ha nem nulla az indulási cím
            save_turbo_byte( 0, wav ); // 0 == RUN next ADDRESS or BASIC
            save_turbo_addr( payload.run_address, wav );    // run address
        } else {
            save_turbo_byte( 2, wav ); // 2 == RETURN TO BASIC
        }
    } else {
        save_turbo_byte( 1, wav ); // 1 == Read next block
    }

}

void save_payload_block( PTP_BLOCK_DATA block, FILE *wav, int last ) {
    save_turbo_block_header( block, wav );
    for( unsigned int i = 0; i < block.byte_counter; i++ ) save_turbo_byte( block.bytes[ i ], wav ); // save data content
    save_payload_block_selector( last, wav );
}

void save_payload( FILE *wav ) {
    save_turbo_header( wav ); // Szinkorn hullm kiírása
    for( unsigned int block_index0 = 0; block_index0<payload.block_counter; block_index0++ ) {
        save_payload_block( payload.blocks[ block_index0 ], wav, block_index0 == payload.block_counter-1 );
    }
}

/****************************************************************************************************************************************
 * Main section
 ****************************************************************************************************************************************/
void print_usage() {
    printf( "ptp2turbo v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Convert .ptp file to turbo wav file.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptp2turbo -i <ptp_filename> -o <c_filename>\n");
    printf( "-a      : BASIC auto RUN command after load.\n");
    printf( "-v      : Verbose mode.\n");
    exit( 1 );
}

int main(int argc, char *argv[]) {
    int opt = 0;
    FILE *wav = 0;
    FILE *ptp = 0;
    while ( ( opt = getopt (argc, argv, "va?h:i:o:") ) != -1 ) {
        switch ( opt ) {
            case -1:
            case ':': break;
            case '?':
            case 'h': print_usage(); break;
            case 'v': verbose = 1; break;
            case 'a': autoBasicRun = 1; break;
            case 'i': // open ptp file
                ptp = fopen( optarg, "rb" );
                if ( !ptp ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
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
    if ( ptp && wav ) {
        payload = load_payload_from_ptp( ptp, verbose, 0 ); // turbo_loader.byte_counter - 46 ); // https://www.trs-80.com/wordpress/zaps-patches-pokes-tips/rom-addresses-general/
        // payload = load_payload_from_ptp( ptp, verbose, 0 ); // turbo_loader.byte_counter - 46 ); // https://www.trs-80.com/wordpress/zaps-patches-pokes-tips/rom-addresses-general/
//??        payload = load_payload_from_ptp( ptp, verbose, 0x1B5D ); // http://www.trs-80.com/wordpress/zaps-patches-pokes-tips/internal/
        // payload = load_payload_from_ptp( ptp, verbose, 0x4433 ); // http://www.trs-80.com/wordpress/zaps-patches-pokes-tips/internal/
        // payload = load_payload_from_ptp( ptp, verbose, 0x1D1E ); // http://www.trs-80.com/wordpress/zaps-patches-pokes-tips/internal/
        // payload = load_payload_from_ptp( ptp, verbose, 0x1A33 ); // GOTO BASIC COMMAND MODE. turbo_loader.byte_counter - 46 ); // https://www.trs-80.com/wordpress/zaps-patches-pokes-tips/rom-addresses-general/
        // payload = load_payload_from_ptp( ptp, verbose, turbo_loader.byte_counter - 46 ); // https://www.trs-80.com/wordpress/zaps-patches-pokes-tips/rom-addresses-general/
        wav_init( wav );
        save_loader( wav );
        wav_write_silence( wav, 20000 );
        save_payload( wav );
        wav_close( wav );
        fclose( ptp );
    } else {
        print_usage();
    }
    return 0;
}
