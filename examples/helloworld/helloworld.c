#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int main() {
	printf("Hello world! Perss any key");
	if ( toupper( getchar() ) == 'Y') {
		printf("\nYess!!!\n");
	} else {
		printf("\nNooo\n");
        }

	do {
		printf("\n");
		printf("Again? Y/N\n");
	} while ( toupper( getchar() )!='N' );
}


