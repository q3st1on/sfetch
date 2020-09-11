#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/utsname.h>
#define _POSIX_C_SOURCE 200809L
#define chunk 1024

char *substring(char *string, int position, int length) {
	char *pointer;
	int c;

	pointer = malloc(length+1);

	if (pointer == NULL) {
		printf("Unable to allocate memory.\n");
		exit(1);
	}

	for (c = 0 ; c < length ; c++) {
		*(pointer+c) = *(string+position-1);
		string++;
	}

	*(pointer+c) = '\0';

	return pointer;
}

char *fileopen(char *file) {
        FILE *filePointer ;
        char dataToBeRead[5002];
        filePointer = fopen( file, "r") ;
        fseek(filePointer, 0L, SEEK_END);
        long int res = ftell(filePointer);
        char *result = calloc(5000, 5000);

        if ( filePointer == NULL ) {
                printf( "%s file failed to open.", file ) ;
        }
        else {
                fseek(filePointer, 0, SEEK_SET);
                while( fgets ( dataToBeRead, 50, filePointer ) != NULL ) {
                strcat(result, dataToBeRead );
                }

                fclose(filePointer);
        }
	return result;
}

int os() {
        struct utsname utbuffer;
        errno = 0;


        if (uname(&utbuffer) != 0) {
                perror("uname");
                exit(EXIT_FAILURE);
        }

        char *result =  fileopen("/usr/lib/os-release");
        char *out = (result+ strlen(result)- 251);
        char *os[50];
        strncpy((void *) os, out, 11);
        char *architecture = (utbuffer.machine);
        printf("\e[36;1m OS\e[m:  %.20s\n", strcat((strcat((void *)os, " ")), architecture));
}

int model() {
        char *name = fileopen("/sys/devices/virtual/dmi/id/product_name");
	char *version = fileopen("/sys/devices/virtual/dmi/id/product_version");
        printf("\e[36;1m Host\e[m: %s\n", strcat((void *) name, (void *) version));
}
int main(void) {
/*	printf("system name = %s\n", utbuffer.sysname);
	printf("node name= %s\n", utbuffer.nodename);
	printf("release = %s\n", utbuffer.release);
	printf("version = %s\n", utbuffer.version);
	printf("\n\n\n\n\n\n\n");
*/
	os();
	model();
	return EXIT_SUCCESS;

}

