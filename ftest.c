#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#define DIRPREFIX "hagenr.rooms."
#define MAXDIRLEN 128

int main() {
    int mypid = getpid();
    char cpid[16];
    memset(cpid, '\0', 16);
    sprintf(cpid, "%d", mypid);
    char dirname[MAXDIRLEN];
    memset(dirname, '\0', MAXDIRLEN);
    strcat(dirname, DIRPREFIX);
    strcat(dirname, cpid);

    int result;
    if ((result = mkdir(dirname, 0755)) != 0) {
	printf("error creating directory\n");
	exit(1);
    }

    return 0;
}
