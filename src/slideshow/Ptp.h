/**
 * Primo .ptp formátum kezelése
 */
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <unistd.h>
#include <stdint.h>
#include <stdio.h>
// #include <dirent.h>
// #include <stdlib.h>
// #include <string.h>
// #include "getopt.h"
// #include <libgen.h>
// #include "lib/fs.h"
// 
#ifdef _WIN32
#include <stdint.h>
typedef uint8_t u_int8_t;
typedef uint16_t u_int16_t;
typedef uint32_t u_int32_t;
#endif

typedef struct {
    unsigned char type;
    uint16_t load_address;
    uint16_t byte_counter;
    unsigned char* bytes;
} PTP_BLOCK_DATA;

typedef struct {
    char name[ 17 ];
    uint16_t min_address;
    uint16_t run_address;
    uint16_t block_counter;
    uint16_t max_address;
    uint16_t full_size;
    unsigned char basic_block_counter;
    PTP_BLOCK_DATA *blocks;
} PTP_DATA;

PTP_DATA load_payload_from_ptp( FILE *ptp, int verbose, uint16_t basic_run_address );
