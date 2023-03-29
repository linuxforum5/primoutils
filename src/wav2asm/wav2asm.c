/**
 * From wav to z80asm source data
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
    8000,            //     Sample Rate - 4 byte hosszú, mintavételezési frekvenciát tartalmazza, esetünkben 0x00007D00 (decimálisan 32000)
    8000,            //     Byte Rate - 4 byte hosszú, értéke 0x0000FA00 (decmálisan 64000)
    1,               //     Block Align - 2 byte hosszú, az 1 mintában található bájtok számát tartalmazza - 0x0002
    8,               //     Bits Per Sample - 2 byte hosszú, felbontást tartalmazza bitekben, értéke 0x0008
    'd','a','t','a', //     Sub Chunk2 ID - konstans, 4 byte hosszú, értéke 0x64617461, ASCII kódban "data"
    0                //     Sub Chunk2 Size - 4 byte hosszú, az adatblokk méretét tartalmazza bájtokban, értéke 0x01D61A1E
};
#pragma pack()

struct wav_header read_header( FILE *wavFile ) {
    struct wav_header wh;
    fread( &wh, sizeof( wh ), 1, wavFile );
    return wh;
}

void asm_write_line( FILE *asmFile, const char* line ) {
    for( int i=0; line[i]; i++ ) {
        fputc( line[i], asmFile );
    }
}

char line[ 1024 ];

void convert_to_format_3( FILE *wavFile, FILE* asmFile, int divider, unsigned long max_sample_counter ) {
    int ds = waveHeader.data_size;
    if ( max_sample_counter && ds > max_sample_counter ) ds = max_sample_counter;
    sprintf( line, "DATA: ; %d\n", ds );
    asm_write_line( asmFile, line );
    int lineSize = 16;
    for( int i=0; i<ds; i+=lineSize ) {
        sprintf( line, "    DB " );
        asm_write_line( asmFile, line );
        for( int j=0; j<lineSize; j++ ) {
            unsigned char sample = fgetc( wavFile );
            unsigned char sample1 = sample / 32;
            switch( sample1 ) {
                case 0 : sample1 = 0b10000000; break;
                case 1 : sample1 = 0b01000000; break;
                case 2 : sample1 = 0b00100000; break;
                case 3 : sample1 = 0b00010000; break;
                case 4 : sample1 = 0b00001000; break;
                case 5 : sample1 = 0b00000100; break;
                case 6 : sample1 = 0b00000010; break;
                case 7 : sample1 = 0b00000001; break;
                default: sample1 = 0b00000001; break;
//            unsigned char sample2 = ( 255 - sample ) / divider + 1;
            }
            if ( j>0 ) {
                fputc( ',', asmFile );
            }
//            sprintf( line, "$%02X", sample1 );
            sprintf( line, "$%02X", sample1 );
            asm_write_line( asmFile, line );
        }
        sprintf( line, "\n" );
        asm_write_line( asmFile, line );
    }
    asm_write_line( asmFile, "    DB 0\n" );
}

void convert_to_format_4( FILE *wavFile, FILE* asmFile, int divider, unsigned long max_sample_counter ) {
    int ds = waveHeader.data_size;
    if ( max_sample_counter && ds > max_sample_counter ) ds = max_sample_counter;
    ds = ds / 2; // 1 byte 2 samples
    sprintf( line, "DATA:\n    DB $%02X,$%02X ; %d\n", ds%256, ds/256, ds );
    asm_write_line( asmFile, line );
    int lineSize = 16;
    for( int i=0; i<ds; i+=lineSize ) {
        sprintf( line, "    DB " );
        asm_write_line( asmFile, line );
        for( int j=0; j<lineSize && (i+j<ds); j++ ) {
            unsigned char sample1 = fgetc( wavFile ) / 16;
            unsigned char sample2 = fgetc( wavFile ) / 16;
if ( feof( wavFile ) ) {
    printf( "Size error!\n" );
    exit(1);
}
            unsigned char sample = sample1 + sample2 * 16;
            if ( j>0 ) {
                fputc( ',', asmFile );
            }
            sprintf( line, "$%02X", sample );
            asm_write_line( asmFile, line );
        }
        sprintf( line, "\n" );
        asm_write_line( asmFile, line );
    }

fgetc( wavFile );
if ( feof( wavFile ) ) {
    printf( "Size2 error!\n" );
    exit(1);
}

}

void convert_to_format_2( FILE *wavFile, FILE* asmFile, int divider, unsigned long max_sample_counter ) {
    int ds = waveHeader.data_size;
    ds /=divider;
    if ( max_sample_counter && ds > max_sample_counter ) ds = max_sample_counter;
    sprintf( line, "DATA: ; %d\n", ds/4 );
    asm_write_line( asmFile, line );
    int lineSize = 16;
    for( int i=0; i<ds; i+=lineSize*4 ) {
        sprintf( line, "    DB " );
        asm_write_line( asmFile, line );
        for( int j=0; j<lineSize; j++ ) {
            unsigned char sample1 = fgetc( wavFile );
            unsigned char sample2 = fgetc( wavFile );
            unsigned char sample3 = fgetc( wavFile );
            unsigned char sample4 = fgetc( wavFile );
            for( int k=1; k<divider; k++ ) {
                sample1 = fgetc( wavFile );
                sample2 = fgetc( wavFile );
                sample3 = fgetc( wavFile );
                sample4 = fgetc( wavFile );
            }

            unsigned char sample = sample1 / 4 + sample2 / 4 * 4 + sample3 / 4 * 16 + sample4 / 4 * 64;
            if ( sample < 255 ) sample++;
            if ( j>0 ) {
                fputc( ',', asmFile );
            }
            sprintf( line, "$%02X", sample );
            asm_write_line( asmFile, line );
        }
        sprintf( line, "\n" );
        asm_write_line( asmFile, line );
    }
    asm_write_line( asmFile, "    DB 0\n" );
}

void convert_to_format_envelope( FILE *wavFile, FILE* asmFile, unsigned long max_sample_counter ) {
    int ds = waveHeader.data_size;
    sprintf( line, "DATA: ; %d\n", ds );
    asm_write_line( asmFile, line );
    unsigned char zero = 0x80;
    unsigned char d = 4;
    unsigned char last = zero;
    unsigned int length = 0;
    unsigned int sum_length = 0;
    unsigned int last_pulse = 0;
    unsigned int half_length_divider = 6;
    unsigned int half_pulse_divider = 2;
    int counter = 0;
    for ( int i=0; i<ds; i++ ) {
        unsigned char c = fgetc( wavFile );
        if ( c < zero - d ) {
            if ( last > zero ) { // Elhagytuk a maximum periódus csúcsát
                unsigned int pulse_length = ( length + half_length_divider ) / ( 2 * half_length_divider ); //  / 12 + 1;
                if ( pulse_length > 255 ) {
                    pulse_length = 255;
                    printf( "Chunk %d. length!\n", counter );
                }
                if ( pulse_length ) { // A 0 hosszúságú impulzust kihagyjuk
                    unsigned char pulse = ( last - zero + half_pulse_divider ) / ( 2 * half_pulse_divider ) + 1; // Minden értéket megnövelünk, így az 1-es érték jelenti a szünetet, azaz ha egyáltalán nem kell bekapcsolni a hangot ebben az impulzusban
                    if ( pulse == 255 ) {
                        printf( "Pulse too big!\n" );
                        pulse--;
                    }
                    if ( pulse != last_pulse ) {
                        counter++;
                        sprintf( line, "  DB $%02X,$%02X\n", pulse_length, pulse );
                        asm_write_line( asmFile, line );
                        length = 0;
                        last_pulse = pulse;
                    }
                    last = c;
                }
            }
            if ( c < last ) last = c;
        } else if ( c > zero + d ) {
            if ( last < zero ) { // Elhagytuk a maximum periódus csúcsát
//                counter++;
//                int puse_length = length / 12 + 1;
//                if ( puse_length > 255 ) {
//                    puse_length = 255;
//                    printf( "Chunk %d. length!\n", counter );
//                }
//                sprintf( line, "  DB $%02X,$%02X\n", puse_length, ( zero - last ) / 8 );
//                asm_write_line( asmFile, line );
                last = c;
//                length = 0;
            }
            if ( c > last ) last = c;
        }
        length++;
        sum_length++;
    }
    asm_write_line( asmFile, "    DB 0\n" );
    printf( "%d. sample writted (%d secs)\n", counter, sum_length/512 );
}

void print_usage() {
    printf( "wav2asm v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Wav to z80asm source data converter.\n");
    printf( "Copyright 2023 by László Princz\n");
    printf( "Usage:\n");
    printf( "wav2asm [options] -i <input_wav_filename> -o <output_asm_filename>\n");
    printf( "Command line option:\n");
    printf( "-c counter   : Maximum converted sample counter\n");
    printf( "-f format_id : Format id: 3=3 bit / byte, 4=4 bit, 2=2 bit 5=envelope\n");
    printf( "-h           : prints this text\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    int finished = 0;
    int arg1;
    FILE *wavFile = 0, *asmFile = 0;
    int divider = 1;
    unsigned long max_sample_counter = 0; // All
    int format = 3;

    while (!finished) {
        switch (getopt (argc, argv, "?h:i:o:d:c:f:")) {
            case -1:
            case ':':
                finished = 1;
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'c': // counter
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-c'.\n");
                    exit(2);
                } else {
                    max_sample_counter = arg1;
                }
                break;
            case 'f': // format
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-f'.\n");
                    exit(2);
                } else {
                    format = arg1;
                }
                break;
            case 'd': // divider
                if ( !sscanf( optarg, "%i", &arg1 ) ) {
                    fprintf( stderr, "Error parsing argument for '-d'.\n");
                    exit(2);
                } else {
                    divider = arg1;
                }
                break;
            case 'i':
                if ( !(wavFile = fopen( optarg, "rb")) ) {
                    fprintf( stderr, "Error opening %s.\n", optarg);
                    exit(4);
                }
                break;
            case 'o':
                if ( !(asmFile = fopen( optarg, "wb")) ) {
                    fprintf( stderr, "Error creating %s.\n", optarg);
                    exit(4);
                }
                break;
            default:
                break;
        }
    }

    if ( !wavFile ) {
        print_usage();
    } else if ( !wavFile ) {
        print_usage();
    } else {
        waveHeader = read_header( wavFile );
        if ( waveHeader.nBitsPerSample == 8 && waveHeader.nChannels == 1 && waveHeader.wFormatTag == 1 ) {
            switch( format ) {
                case 2 : convert_to_format_2( wavFile, asmFile, divider, max_sample_counter ); break;
                case 3 : convert_to_format_3( wavFile, asmFile, divider, max_sample_counter ); break;
                case 4 : convert_to_format_4( wavFile, asmFile, divider, max_sample_counter ); break;
                case 5 : convert_to_format_envelope( wavFile, asmFile, max_sample_counter ); break;
                default : fprintf( stderr, "Infalid format: %d\n", format );
            }
        } else {
            printf( "Csak 8 bites mono PCM wav fájl használható!\n" );
        }
        fclose( wavFile );
        fclose( asmFile );
    }

    return 0;
}
