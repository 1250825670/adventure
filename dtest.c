#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

/* preprocessor constants */
#define NUM_ROOMS 7
#define DIR_PREFIX "hagenr.rooms."
#define FILE_SUFFIX "_room"
#define MAX_LEN 128
#define MAX_BUF_LEN 1024

/* globals */
struct room {
    int id;
    char* name;
    char* type;
    int num_connections;
    char* connected_rooms[NUM_ROOMS-1];
} rooms[NUM_ROOMS];

const char* con_title = "CONNECTION 1: ";
const char* name_title = "ROOM NAME: ";
const char* type_title = "ROOM TYPE: ";
const char* start_type = "START_ROOM";
const char* end_type = "END_ROOM";

int main() {
    int newest_dir_time = -1;
    char newest_dir_name[MAX_LEN];

    DIR* dp;
    struct dirent* file_in_dir;
    struct stat dir_attributes;

    dp = opendir(".");
    if (dp == NULL) {
	perror("failed to open starting dir");
	exit(1);
    }

    while ((file_in_dir = readdir(dp)) != NULL) {
	if (strstr(file_in_dir->d_name, DIR_PREFIX) != NULL) {
	    stat(file_in_dir->d_name, &dir_attributes);
	    if ((int)dir_attributes.st_mtime > newest_dir_time) {
		newest_dir_time = (int) dir_attributes.st_mtime;
		memset(newest_dir_name, '\0', MAX_LEN);
		strcpy(newest_dir_name, file_in_dir->d_name);
	    }
	}
    }
    closedir(dp);

    /* printf("newest dir: %s\n", newest_dir_name); */

    dp = opendir(newest_dir_name);
    if (dp == NULL) {
	perror("failed to open latest dir");
	exit(1);
    }

    int fd;
    char file_name[MAX_LEN];
    char data[MAX_LEN];
    char ch;
    int i, j, k, nread, num_cns;


    i = 0;
    while ((file_in_dir = readdir(dp))) {
	if (strstr(file_in_dir->d_name, ".") == NULL) {
	    memset(file_name, '\0', MAX_LEN);
	    strcat(file_name, "./");
	    strcat(file_name, newest_dir_name);
	    strcat(file_name, "/");
	    strcat(file_name, file_in_dir->d_name);
	    /* printf("%s\n", file_name); */

	    fd = open(file_name, O_RDONLY);
	    if (fd < 0) {
		perror("failed to open file");
		exit(1);
	    }

	    /* get the name */
	    memset(data, '\0', MAX_LEN);
	    lseek(fd, strlen(name_title), SEEK_SET);
	    k = 0;
	    while (1) {
		nread = read(fd, &ch, 1);
		if (ch == '\n') break;
		data[k++] = ch;
	    }
	    data[k] = '\0';
	    /* printf("name: '%s'\n", data); */
	    rooms[i].name = calloc(MAX_LEN, sizeof(char));
	    strcpy(rooms[i].name, data);

	    /* get the connections */
	    num_cns = 0;
	    j = 0;
	    while (1) {
		k = 0;
		memset(data, '\0', MAX_LEN);
		lseek(fd, strlen(con_title), SEEK_CUR);
		while (1) {
		    nread = read(fd, &ch, 1);
		    if (ch == '\n') break;
		    data[k++] = ch;
		}
		data[k] = '\0';
		num_cns++;
		rooms[i].connected_rooms[j] = calloc(MAX_LEN, sizeof(char));
		strcpy(rooms[i].connected_rooms[j++], data);
		/* printf("connection: %s\n", data); */
		/* still in connections? */
		nread = read(fd, &ch, 1);
		lseek(fd, -1, SEEK_CUR);
		if (ch != con_title[0]) break;
	    }
	    rooms[i].num_connections = num_cns;
	    /* printf("num_cns: %d\n", num_cns); */

	    /* get the type */
	    memset(data, '\0', MAX_LEN);
	    lseek(fd, strlen(type_title), SEEK_CUR);
	    k = 0;
	    while (1) {
		nread = read(fd, &ch, 1);
		if (ch == '\n') break;
		data[k++] = ch;
	    }
	    data[k] = '\0';
	    rooms[i].type = calloc(MAX_LEN, sizeof(char));
	    strcpy(rooms[i].type, data);
	    /* printf("type: '%s'\n", data); */

	    i++;
	    close(fd);
	}
    }

    closedir(dp);
    for (i = 0; i < NUM_ROOMS; i++) {
	printf("name: %s, type: %s, #conns: %d, connections: \n",
		rooms[i].name, rooms[i].type, rooms[i].num_connections);
	for (j = 0; j < rooms[i].num_connections; j++) {
	    printf("%s ", rooms[i].connected_rooms[j]);
	}
	printf("\n");
    }

    for (i = 0; i < NUM_ROOMS; i++) {
	if (strstr(start_type, rooms[i].type)) {
	    printf("CURRENT LOCATION: %s \nPOSSIBLE CONNECTIONS: ", rooms[i].name);
	    for (j = 0; j < rooms[i].num_connections; j++) {
		if (j < rooms[i].num_connections - 1) {
		    printf("%s, ", rooms[i].connected_rooms[j]);
		} else {
		    printf("%s.", rooms[i].connected_rooms[j]);
		}
	    }
	}
	printf("\n");
    }

    for (i = 0; i < NUM_ROOMS; i++) {
	free(rooms[i].name);
	free(rooms[i].type);
	for (j = 0; j < rooms[i].num_connections; j++) {
	    free(rooms[i].connected_rooms[j]);
	}
    }
    return 0;
}
