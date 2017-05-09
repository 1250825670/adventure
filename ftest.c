#include "mydefs.h"

int main() {
    int mypid = getpid();
    char cpid[16];
    memset(cpid, '\0', 16);
    sprintf(cpid, "%d", mypid);
    char dirname[MAXLEN];
    memset(dirname, '\0', MAXLEN);
    strcat(dirname, DIRPREFIX);
    strcat(dirname, cpid);

    int result;
    if ((result = mkdir(dirname, 0755)) != 0) {
	printf("error creating directory\n");
	exit(1);
    }

    char* myfile = "test.txt";
    char file_path[MAXLEN];
    memset(file_path, '\0', MAXLEN);
    strcat(file_path, dirname);
    strcat(file_path, "/");
    strcat(file_path, myfile);
    printf("file path: %s\n", file_path);
    int fd;

    fd = open(file_path, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
	printf("error creating file\n");
	exit(1);
    }

    size_t n, len;
    char* roomname = "purgatory";
    char* con[] = { "lust", "misery" };
    int i, num_conns;
    num_conns = 2;
    char* roomtype = "END_ROOM";
    char* con_title[]  = { "CONNECTION 1: ", "CONNECTION 2: ", "CONNECTION 3: ", "CONNECTION 4: ", "CONNECTION 5: ", "CONNECTION 6: " };
    char* name_title = "ROOM NAME: ";
    char* type_title = "ROOM TYPE: ";
    char tmp[MAXLEN];
    memset(tmp, '\0', MAXLEN);

    strcat(tmp, name_title);
    strcat(tmp, roomname);
    strcat(tmp, "\n");

    for (i = 0; i < num_conns; i++) {
	strcat(tmp, con_title[i]);
	strcat(tmp, con[i]);
	strcat(tmp, "\n");
    }

    strcat(tmp, type_title);
    strcat(tmp, roomtype);
    strcat(tmp, "\n");

    len = strlen(tmp) * sizeof(char);
    n = write(fd, tmp, len);

    if (n != len) {
	printf ("error writing to file\n");
	exit(0);
    }

    close(fd);

    return 0;
}
