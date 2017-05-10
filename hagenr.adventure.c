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
#define CON_TITLE "CONNECTION 1: "
#define NAME_TITLE "ROOM NAME: "
#define TYPE_TITLE "ROOM TYPE: "
#define START_TYPE "START_ROOM"
#define END_TYPE "END_ROOM"

/* globals */
struct room {
    int id;
    char* name;
    char* type;
    int num_connections;
    char* connected_rooms[NUM_ROOMS-1];
} rooms[NUM_ROOMS];

/* function prototypes */
void load_struct();
void get_newest_dir(char* newest_dir_name);
void read_files(char* newest_dir_name);
void play_game();
void cleanup();
int get_start_room();
int get_room();
void reprint_room(int idx);

int main() {
    load_struct();
    play_game();
    cleanup();
    return 0;
}

void load_struct () {
    char newest_dir[MAX_LEN];
    get_newest_dir(newest_dir);
    read_files(newest_dir);
}

void get_newest_dir(char* newest_dir_name) {
    int newest_dir_time = -1;

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
}

void read_files(char* newest_dir_name) {
    DIR* dp;

    dp = opendir(newest_dir_name);
    if (dp == NULL) {
	perror("failed to open latest dir");
	exit(1);
    }

    struct dirent* file_in_dir;
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

	    fd = open(file_name, O_RDONLY);
	    if (fd < 0) {
		perror("failed to open file");
		exit(1);
	    }

	    /* get the name */
	    memset(data, '\0', MAX_LEN);
	    lseek(fd, strlen(NAME_TITLE), SEEK_SET);
	    k = 0;
	    while (1) {
		nread = read(fd, &ch, 1);
		if (ch == '\n') break;
		data[k++] = ch;
	    }
	    data[k] = '\0';
	    rooms[i].name = calloc(MAX_LEN, sizeof(char));
	    strcpy(rooms[i].name, data);

	    /* get the connections */
	    num_cns = 0;
	    j = 0;
	    while (1) {
		k = 0;
		memset(data, '\0', MAX_LEN);
		lseek(fd, strlen(CON_TITLE), SEEK_CUR);
		while (1) {
		    nread = read(fd, &ch, 1);
		    if (ch == '\n') break;
		    data[k++] = ch;
		}
		data[k] = '\0';
		num_cns++;
		rooms[i].connected_rooms[j] = calloc(MAX_LEN, sizeof(char));
		strcpy(rooms[i].connected_rooms[j++], data);
		/* still in connections? */
		nread = read(fd, &ch, 1);
		lseek(fd, -1, SEEK_CUR);
		if (ch != CON_TITLE[0]) break;
	    }
	    rooms[i].num_connections = num_cns;

	    /* get the type */
	    memset(data, '\0', MAX_LEN);
	    lseek(fd, strlen(TYPE_TITLE), SEEK_CUR);
	    k = 0;
	    while (1) {
		nread = read(fd, &ch, 1);
		if (ch == '\n') break;
		data[k++] = ch;
	    }
	    data[k] = '\0';
	    rooms[i].type = calloc(MAX_LEN, sizeof(char));
	    strcpy(rooms[i].type, data);

	    i++;
	    close(fd);
	}
    }
    closedir(dp);
}

int get_room(char* room_name) {
    int i, j;

    for (i = 0; i < NUM_ROOMS; i++) {
	if (strstr(room_name, rooms[i].name)) {
	    /* if (rooms[i */
	    printf("\nCURRENT LOCATION: %s \nPOSSIBLE CONNECTIONS: ", rooms[i].name);
	    for (j = 0; j < rooms[i].num_connections; j++) {
		if (j < rooms[i].num_connections - 1) {
		    printf("%s, ", rooms[i].connected_rooms[j]);
		} else {
		    printf("%s.", rooms[i].connected_rooms[j]);
		}
	    }
	    break;
	}
    }
    printf("\n\n");
    return i;
}

int get_start_room() {
    int i, j;

    for (i = 0; i < NUM_ROOMS; i++) {
	if (strstr(START_TYPE, rooms[i].type)) {
	    printf("\nCURRENT LOCATION: %s \nPOSSIBLE CONNECTIONS: ", rooms[i].name);
	    for (j = 0; j < rooms[i].num_connections; j++) {
		if (j < rooms[i].num_connections - 1) {
		    printf("%s, ", rooms[i].connected_rooms[j]);
		} else {
		    printf("%s.", rooms[i].connected_rooms[j]);
		}
	    }
	    break;
	}
    }
    printf("\n\n");
    return i;
}

void reprint_room(int idx) {
    int j;

    printf("\nCURRENT LOCATION: %s \nPOSSIBLE CONNECTIONS: ", rooms[idx].name);
    for (j = 0; j < rooms[idx].num_connections; j++) {
	if (j < rooms[idx].num_connections - 1) {
	    printf("%s, ", rooms[idx].connected_rooms[j]);
	} else {
	    printf("%s.", rooms[idx].connected_rooms[j]);
	}
    }
    printf("\n\n");
}

void play_game() {
    int i, j, num_chars, steps, idx, match;
    char* path[NUM_ROOMS];
    char* input;
    size_t buf_size = 0;

    steps = 0;
    while (1) {
	match = 0;
	if (steps == 0) idx = get_start_room();
	printf("WHERE TO? >");
	num_chars = getline(&input, &buf_size, stdin);
	input[num_chars-1] = '\0';
	/* printf("you entered: %s\n", input); */

	for (i = 0; i < rooms[idx].num_connections; i++) {
	    if (strstr(input, rooms[idx].connected_rooms[i])) {
		idx = get_room(input);
		steps++;
		match = 1;
		break;
	    }
	}
	if (!match) {
	    printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n");
	}
    }

    free(input);
    input = NULL;

    // for (i = 0; i < NUM_ROOMS; i++) {
    //     printf("name: %s, type: %s, #conns: %d, connections: \n",
    //     	rooms[i].name, rooms[i].type, rooms[i].num_connections);
    //     for (j = 0; j < rooms[i].num_connections; j++) {
    //         printf("%s ", rooms[i].connected_rooms[j]);
    //     }
    //     printf("\n");
    // }
}

void cleanup() {
    int i, j;
    for (i = 0; i < NUM_ROOMS; i++) {
	free(rooms[i].name);
	free(rooms[i].type);
	for (j = 0; j < rooms[i].num_connections; j++) {
	    free(rooms[i].connected_rooms[j]);
	}
    }
}
