#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <X11/extensions/Xrandr.h>
#include <sys/utsname.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#define _POSIX_C_SOURCE 200809L
#define chunk 1024

char *fileparse(char *file, int reqline, char *regex) {
	FILE* fp = fopen(file, "r");
	if(fp == NULL) {
		printf("fopen failed to open the file\n");
		exit(-1);
	}
	char line[2048];
	char itemCode[50];
	char *item;
	item = malloc(50);
	int lineno;
	lineno = 0;
	while(fgets(line, sizeof(line), fp) != NULL) {
		if(sscanf(line, (regex), item) != 0) {
			exit;
		}
		if(lineno == reqline) {
			sscanf(line, regex, item);
			return item;
		}
		lineno++;
	}
	fclose(fp);
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
	free(result);
}

int os() {
        struct utsname utbuffer;
        errno = 0;


        if (uname(&utbuffer) != 0) {
                perror("uname");
                exit(EXIT_FAILURE);
        }

        char *result =  fileparse("/usr/lib/os-release", 1, "%*[^\"]\"%127[^\"]");
        char *architecture = (utbuffer.machine);
        printf("\e[36;1m OS\e[m:  %.20s\n", strcat((strcat((void *) result, " ")), architecture));
}

int model() {
        char *name = fileopen("/sys/devices/virtual/dmi/id/product_name");
	char *temp;
	temp = strchr(name,'\n');
	*temp = '\0';
	char *version = fileopen("/sys/devices/virtual/dmi/id/product_version");
        char *vtemp;
        vtemp = strchr(version,'\n');
        *vtemp = '\0';
	printf("\e[36;1m Host\e[m: %s\n", strcat(strcat((void *) name, " "), (void *) version));
}

int Kernel() {
        struct utsname utbuffer;
        errno = 0;


        if (uname(&utbuffer) != 0) {
                perror("uname");
                exit(EXIT_FAILURE);
        }

	printf("\e[36;1m Kernel\e[m: %s\n", utbuffer.release);
}

int uptime() {
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if(error != 0) {
		printf("code error = %d\n", error);
	}
	int time = (s_info.uptime / 60);
        char result[256];
	if(time >= 60) {
		int hourcount = 0;
		while(time >=60) {
			time = time-60;
			hourcount++;
		}
		char hours[50];
		char mins[50];
		sprintf(hours, "%d hours, ", hourcount);
		sprintf(mins, "%d mins", time);
		sprintf(result, strcat(hours, mins));
	}
	else {
		sprintf(result, "%d MIN", time);
	}
	printf("\e[36;1m Uptime\e[m: %s\n", result);
}

int packages() {
	DIR *d;
	struct dirent *dir;
	d = opendir("/var/cache/pacman/pkg");
	int files = 0;
	if (d) {
		while ((dir = readdir(d)) != NULL) {
			files++;
		}
		closedir(d);
	}
	int packnum = (files - 2);
	printf("\e[36;1m Packages\e[m: %d %s\n", packnum, "(pacman)");
}


int cpu() {
	char* file = fileparse("/proc/cpuinfo", 4, "%*[^:]:%[^\n]");
	printf("\e[36;1m CPU\e[m:%s\n", file);
	return(0);
}

int resolution() {
}


int main(void) {
	os();
	model();
	Kernel();
	uptime();
	packages();
	cpu();
	return EXIT_SUCCESS;
}
