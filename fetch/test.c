#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#define _POSIX_C_SOURCE 200809L
#define chunk 1024

int main(void) {
	struct utsname utbuffer;
	errno = 0;


	if (uname(&utbuffer) != 0) {
		perror("uname");
		exit(EXIT_FAILURE);
	}
	printf("system name = %s\n", utbuffer.sysname);
	printf("node name= %s\n", utbuffer.nodename);
	printf("release = %s\n", utbuffer.release);
	printf("version = %s\n", utbuffer.version);
	printf("machine = %s\n", utbuffer.machine);
	printf("\n\n\n\n\n\n\n");

        FILE *filePointer ;
        char dataToBeRead[50];
        filePointer = fopen("/usr/lib/os-release", "r") ;
	fseek(filePointer, 0L, SEEK_END);
	long int res = ftell(filePointer);
	char result[res+1];

        if ( filePointer == NULL ) {
                printf( "os-release file failed to open." ) ;
        }
        else {
		fseek(filePointer, 0, SEEK_SET);
                while( fgets ( dataToBeRead, 50, filePointer ) != NULL ) {
		strcat(result, dataToBeRead );
                }

                fclose(filePointer);
        }
	char *ptr = strstr(result, "NAME=\"");
        char *ptr2 = strstr(result, "Artix Linux\"");
	printf("\n\nLENGTH1 = %p\n", (void *) &ptr);
	printf("\n\nLENGTH2 = %p\n", (void *) &ptr2);
	return EXIT_SUCCESS;

}


