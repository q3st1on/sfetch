#include <pwd.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <X11/Xlib.h>
/*#include <pci/pci.h>*/
#include <sys/utsname.h>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <sys/sysinfo.h>
#define XLIB_ILLEGAL_ACCESS
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

/*int gpu() {
	struct pci_access *pacc;
	struct pci_dev *dev;
	char namebuf[1024], *name;
	counter = 0;
	pacc = pci_alloc();
	pci_init(pacc);
	pci_scan_bus(pacc);
	for (dev=pacc->devices; dev; dev=dev->next) {
		if(counter == 4) {
			pci_fill_info(dev, PCI_FILL_IDENT | PCI_FILL_BASES | PCI_FILL_CLASS);
			name = pci_lookup_name(pacc, namebuf, sizeof(namebuf), PCI_LOOKUP_DEVICE, dev->vendor_id, dev->device_id);
			printf("\e[36;1m GPU\e[m: %s\n", name);
		}
		counter++;
	}
	pci_cleanup(pacc);
	return 0;
} */

int memory() {
	struct sysinfo s_info;
	int error = sysinfo(&s_info);
	if(error != 0) {
		printf("code error = %d\n", error);
	}
	printf("\e[36;1m Memory\e[m: %d/%d\n", ((s_info.totalram)-(s_info.freeram)),(s_info.totalram));
}

int de() {
	char* de = getenv("XDG_CURRENT_DESKTOP");
	if (de == NULL) {
		printf("\n");
		return(0);
	} else {
		printf("\e[36;1m Desktop Environment\e[m:   %s\n", de);
		return(0);
	}
}

int header() {
	struct passwd *pw;

	char *lgn = getlogin();
	char *header = fileopen("/etc/hostname");

	char underscore[64];
	while(strlen(underscore)<= strlen(header)) {
		strcat(underscore, "-");
	}
	printf("                                          \e[36;1m %s\e[m@\e[36;1m%s\e[m", lgn, header);
	printf("\e[36;1m                   '                       \e[m%s\n", underscore);
}

int resolution() {
	Display* pdsp = XOpenDisplay(NULL);
	Window wid = DefaultRootWindow(pdsp);

	Screen* pwnd = DefaultScreenOfDisplay(pdsp);
	int sid = DefaultScreen(pdsp);

	XWindowAttributes xwAttr;
	XGetWindowAttributes(pdsp,wid,&xwAttr);

	printf ("\e[36;1m Resolution\e[m: %dx%d\n", xwAttr.width, xwAttr.height);

	XCloseDisplay( pdsp );

	return 1;
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
	int time = (s_info.uptime)/60;
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
	else if(time >=1) {
		sprintf(result, "%d mins", time);
	}
	else {
		sprintf(result, "%d secs", (s_info.uptime));
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
	int cores = sysconf(_SC_NPROCESSORS_ONLN);
	printf("\e[36;1m CPU\e[m:%s (%d)\n", file, cores);
	return(0);
}

int term() {
	printf("\e[36;1m Terminal\e[m: %s\n", getenv("TERM"));
	return(0);
}

int shell() {
	char shell[256];
	sprintf(shell, "%s", getpwuid(geteuid())->pw_shell);
	printf("\e[36;1m Shell\e[m: %s\n", shell+strlen(shell) -(strlen(shell)-4));
	return(0);
}

int ascii() {
	printf("\e[36;1m                  'o'                  \e[m   ", header());
	printf("\e[36;1m                 'ooo'                 \e[m   ", os());
	printf("\e[36;1m                'ooxoo'                \e[m   ", model());
	printf("\e[36;1m               'ooxxxoo'               \e[m   ", Kernel());
	printf("\e[36;1m              'oookkxxoo'              \e[m   ", uptime());
	printf("\e[36;1m             'oiioxkkxxoo'             \e[m   ", packages());
	printf("\e[36;1m            ':;:iiiioxxxoo'            \e[m   ", resolution());
	printf("\e[36;1m               `'.;::ioxxoo'           \e[m   ", cpu());
	printf("\e[36;1m          '-.      `':;jiooo'          \e[m   ", term()); 
	printf("\e[36;1m         'oooio-..     `'i:io'         \e[m   ", memory());
	printf("\e[36;1m        'ooooxxxxoio:,.   `'-;'        \e[m   ", shell());
	printf("\e[36;1m       'ooooxxxxxkkxoooIi:-.  `'       \e[m   ", de());
	printf("\e[36;1m      'ooooxxxxxkkkkxoiiiiiji'         \e[m   \n");
	printf("\e[36;1m     'ooooxxxxxkxxoiiii:'`     .i'     \e[m   \n");
	printf("\e[36;1m    'ooooxxxxxoi:::'`       .;ioxo'    \e[m   \n");
	printf("\e[36;1m   'ooooxooi::'`         .:iiixkxxo'   \e[m   \n");
	printf("\e[36;1m  'ooooi:'`                `'';ioxxo'  \e[m   \n");
	printf("\e[36;1m 'i:'`                          '':io' \e[m   \n");
	printf("\e[36;1m'`                                   `'\e[m   \n");
}

int main(void) {
	ascii();
	return EXIT_SUCCESS;
}
