#include <stdint.h>

#define MAX_LINE_LENGTH 2048

#define BASIC_START 0x43EA

typedef struct {
    int bin_line_full_length; // strlen(bin_line)+2*2+1 Full readed binari line length with address, line number and encloser zero
    uint16_t next_address;
    uint16_t line_number;
    unsigned char bin_line[ MAX_LINE_LENGTH ];
    char text_line[ MAX_LINE_LENGTH ];
    char charset; // P:primo U:utf8
} BASIC_LINE;

BASIC_LINE decode_basic_line( const unsigned char *line, int size, int utf8 );
int check_load_addresses( unsigned char *bin_basic, int length, uint16_t load_address );
