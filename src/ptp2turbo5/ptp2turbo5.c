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

const unsigned int defaultBaud = 62000; // 54000 // 2*25000;
// const unsigned int tick = 2; // 1 byte ~ 48us
const unsigned char bottom = POS_PEAK;
// const unsigned char bottom = SILENCE;
// const unsigned char bottom = NEG_PEAK;

// const unsigned char top  = POS_PEAK;
// const unsigned char top  = SILENCE;
const unsigned char top  = NEG_PEAK;
int verbose = 0;
int autoBasicRun = 0;
unsigned long payload_start_position = 0; // A wav fájlban a payload kezdőpozíciója

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
    uint32_t full_size = ftell( wav );
    fseek( wav, 4, SEEK_SET );
    fwrite( &full_size, sizeof( full_size ), 1, wav ); // Wave header 2. field : filesize with header. First the lowerest byte
    uint32_t data_size = full_size - sizeof( waveHeader );
    fseek( wav, sizeof( waveHeader ) - 4 ,SEEK_SET ); // data chunk size position: 40
    fwrite( &data_size, sizeof( data_size ), 1, wav );
    fclose( wav );
    printf( "Full size is %dKB\n", payload.full_size/1024 );
    printf( "Full time is %d seconds\n", data_size / defaultBaud );
    unsigned long speed = (long)payload.full_size * (long)8 * (long)defaultBaud / (long)data_size;
    unsigned long payload_wav_size = full_size - payload_start_position;
    unsigned long turbo_speed = (long)payload.full_size * (long)8 * (long)defaultBaud / payload_wav_size;
    printf( "Speed is %lu baud (turbo speed - without original slow loader - is %lu baud)\n", speed, turbo_speed );
}

/****************************************************************************************************************************************
 * Standard wav modules for ROM BASIC LOAD command
 * http://primo.homeserver.hu/html/konvertfajlok.html
 ****************************************************************************************************************************************/
const unsigned char bit1_peak_slow_counter = 13; // 11
const unsigned char bit0_peak_slow_counter = 39; // 32

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
    write_loader_name_block( wav, "Turbo loader 2.5" );
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

void check_payload_block_addresses( uint16_t loader_first_address, uint16_t loader_last_address ) {
    for( unsigned int block_index0 = 0; block_index0<payload.block_counter; block_index0++ ) {
        uint16_t first = payload.blocks[ block_index0 ].load_address;
        uint16_t last = first + payload.blocks[ block_index0 ].byte_counter - 1;
        if ( first >= loader_first_address && first <= loader_last_address ) {
            fprintf( stderr, "Turbo loader error: payload content in loader memory (1)\n" ); exit(1);
        } else if ( last >= loader_first_address && last <= loader_last_address ) {
            fprintf( stderr, "Turbo loader error: payload content in loader memory (2)\n" ); exit(1);
        } else if ( first <= loader_first_address && last >= loader_last_address ) {
            fprintf( stderr, "Turbo loader error: payload content in loader memory (3)\n" ); exit(1);
        }
    }
}

void shift_loader5( uint16_t first_free_top_address ) {
    uint16_t last_loader_address = first_free_top_address + turbo_loader.byte_counter;
    if ( last_loader_address <= 0x67A0 ) { // A32 fut
        printf( "Run on A32\n" );
    } else if ( last_loader_address <= 0xA7A0 ) { // A48 fut
        printf( "Run on A48\n" );
    } else if ( last_loader_address <= 0xE7A0 ) { // A64 fut
        printf( "Run on A64\n" );
    } else {
        fprintf( stderr, "Turbo loading not possible. (Not enough room on top of payload.):(\n" );
        exit(1);
    }
    if ( first_free_top_address > turbo_loader.load_address ) { // Csak felfelé lehet másolni
        if ( first_free_top_address - turbo_loader.load_address < turbo_loader.byte_counter ) first_free_top_address = turbo_loader.load_address + turbo_loader.byte_counter;
        // uint16_t dest_address = first_free_top_address;
        uint16_t shift = turbo_loader.load_address - first_free_top_address;
        check_payload_block_addresses( first_free_top_address, last_loader_address );
        printf( "Move loader from 0x%04X to 0x%04X with 0x%04X\n", turbo_loader.load_address, first_free_top_address, shift );
        turbo_loader.load_address -= shift;
        turbo_loader.run_address  -= shift;                        //                                           Abs addr. Rel addr.
        turbo_loader.bytes[ 0x43 ] = first_free_top_address % 256; // COPY_GET_BYTE0_TO_GET_BYTE: + 1 :         4442+1 -> 0x0043
        turbo_loader.bytes[ 0x44 ] = first_free_top_address / 256;
        uint16_t loading_msg = first_free_top_address + 0x00ED;    // A LOADING_MSG relatív címe:               44ED   -> 0x00ED
        turbo_loader.bytes[ 0x29 ] = loading_msg % 256; // LOADING_MSG-re való hivatkozás az ENTRY_ADDRESS: + 1 4428+1 -> 0x0029 
        turbo_loader.bytes[ 0x2A ] = loading_msg / 256;
        uint16_t error_msg = first_free_top_address + 0x001B; // ERROR_MSG relatív címe:                        441B   -> 0x001B
        turbo_loader.bytes[ 0xBC ] = error_msg % 256; // ERROR_MSG-re való hivatkozás az ERROR: + 1 címen:      44BB+1 -> 0x00BC
        turbo_loader.bytes[ 0xBD ] = error_msg / 256;
    } else { // No move
        printf( "Loader stay on 0x%04X address\n", turbo_loader.load_address );
    }
}
/****************************************************************************************************************************************
 * Turbo functions
 ****************************************************************************************************************************************/
/*
 * 4Mhz-es érték számítása(V)=V*2.5/4
 * Egy sample hossza 62000kHz mintavételezés esetén: 1 sample = 1/62000 = 16,129032258us | 4Mhz: ~ 10us
 */
void save_turbo5_bit( unsigned char bit, FILE *wav ) {
    if ( bit ) { // Ha a bit 1
        wav_write_sample( wav, bottom, 4 );
    } else { // Ha a bit 0
        wav_write_sample( wav, bottom, 2 );
    }
    wav_write_sample( wav, top, 3 );
}

void save_turbo5_byte( unsigned char byte, FILE *wav ) {
    for( int bit=1; bit<256; bit=bit*2 ) {
        save_turbo5_bit( byte & bit, wav );
    }
    wav_write_sample( wav, top, 3 ); // Time for store byte
}

void save_turbo5_addr( unsigned int addr, FILE *wav ) {
    save_turbo5_byte( addr % 0x100, wav );
    save_turbo5_byte( addr / 0x100, wav );
}

void save_turbo5_header( FILE *wav ) {
//    wav_write_sample( wav, top, 10 );    // ~ 48us
//    wav_write_sample( wav, bottom, 1 ); // 
    uint16_t sum = 0;
    for( unsigned char i=0; i<14; i++ ) {
        unsigned char c = i*3;
        sum += c * 0x101;
        save_turbo5_byte( c, wav );
    }
    save_turbo5_byte( sum % 0x100, wav );
    save_turbo5_byte( sum / 0x100, wav );
}

void save_turbo5_block_header( PTP_BLOCK_DATA block, FILE *wav ) {
    // Bevezető byte a képernyőképhez
    save_turbo5_byte( ( block.type == 0xF5 ) ? 1 : 0, wav ); // Screen esetén 1 különben 1
//    wav_write_sample( wav, top, 4*tick );
    // 
    save_turbo5_addr( block.load_address, wav );              // load address
    save_turbo5_addr( block.byte_counter, wav );              // byte counter
printf( "Create turbo 2.5 block (type=0x%02X, size=0x%04X, load address=0x%04X)\n", block.type, block.byte_counter, block.load_address );
    // printf( "Create Big Turbo Block with size 0x%04X\n", block.byte_counter );
    wav_write_sample( wav, top, block.byte_counter/400 ); // For line writing 80f0(33008) esetén 1A(26)*tick // 1200 volt eddig jó
}

void save_payload5_block_selector( int last, FILE *wav  ) {
    if ( last ) { // Last block
        if ( autoBasicRun && payload.basic_block_counter ) {
            save_turbo5_byte( 3, wav ); // 0 == RUN next ADDRESS or BASIC
        } else if ( payload.run_address ) { // Ha nem nulla az indulási cím
            save_turbo5_byte( 0, wav ); // 0 == RUN next ADDRESS or BASIC
            // printf( "Save run address: 0x%04X\n", payload.run_address );
            save_turbo5_addr( payload.run_address, wav );    // run address
        } else {
            save_turbo5_byte( 2, wav ); // 2 == RETURN TO BASIC
        }
    } else {
        save_turbo5_byte( 1, wav ); // 1 == Read next block
    }
}

void save_payload5_block( PTP_BLOCK_DATA block, FILE *wav, int last ) {
    save_turbo5_block_header( block, wav );
    for( unsigned int i = 0; i < block.byte_counter; i++ ) save_turbo5_byte( block.bytes[ i ], wav ); // save data content
    save_payload5_block_selector( last, wav );
}

void save_payload5( FILE *wav ) {
    save_turbo5_header( wav ); // Szinkorn hullm kiírása
    for( unsigned int block_index0 = 0; block_index0<payload.block_counter; block_index0++ ) {
        save_payload5_block( payload.blocks[ block_index0 ], wav, block_index0 == payload.block_counter-1 );
    }
}

/****************************************************************************************************************************************
 * Main section
 ****************************************************************************************************************************************/
void print_usage() {
    printf( "ptp2turbo5 v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Convert .ptp file to turbo wav file.\n");
    printf( "Only for Primo with 2.5Mhz!\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "ptp2turbo5 -i <ptp_filename> -o <wav_filename>\n");
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
        wav_init( wav );
        shift_loader5( payload.max_address+1 ); // Shift if need!
        save_loader( wav );
        wav_write_silence( wav, 20000 );
        payload_start_position = ftell( wav );
        save_payload5( wav );
        wav_close( wav );
        fclose( ptp );
    } else {
        print_usage();
    }
    return 0;
}
