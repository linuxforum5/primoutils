/**
 * Filesystem
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "getopt.h"
#include <libgen.h>

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

char is_ext( const char *filename, const char *ext ) {
    int fileNameSize = 0;
    while( filename[ fileNameSize ] ) fileNameSize++;
    int extSize = 0;
    while( ext[ extSize ] ) extSize++;
    while( extSize && fileNameSize && ext[ extSize ] == filename[ fileNameSize ] ) {
        extSize--;
        fileNameSize--;
    }
    return !extSize;
}
