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
#define MAX_LEN 128
#define CON_TITLE "CONNECTION 1: "
#define NAME_TITLE "ROOM NAME: "
#define TYPE_TITLE "ROOM TYPE: "
#define START_TYPE "START_ROOM"
#define END_TYPE "END_ROOM"
#define PROMPT1 "CURRENT LOCATION: "
#define PROMPT2 "POSSIBLE CONNECTIONS: "

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
int get_room_details(char* input, char* room_name, char* connectors);

int main() {
    int i, j;
    load_struct();
    /* for (i = 0; i < NUM_ROOMS; i++) { */
	/* printf("name: %s, type: %s, #conns: %d, connections: \n", */
		/* rooms[i].name, rooms[i].type, rooms[i].num_connections); */
	/* for (j = 0; j < rooms[i].num_connections; j++) { */
	    /* printf("%s ", rooms[i].connected_rooms[j]); */
	/* } */
	/* printf("\n"); */
    /* } */
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
	    rooms[i].name = calloc(k, sizeof(char));
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
		rooms[i].connected_rooms[j] = calloc(k, sizeof(char));
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
	    rooms[i].type = calloc(k, sizeof(char));
	    strcpy(rooms[i].type, data);

	    i++;
	    close(fd);
	}
    }
    closedir(dp);
}

int get_room_details(char* input, char* room_name, char* connectors) {
    int i, j, retval;

    memset(room_name, '\0', MAX_LEN);
    memset(connectors, '\0', MAX_LEN);
    retval = 0;

    if (!input) {
	for (i = 0; i < NUM_ROOMS; i++) {
	    if (strstr(START_TYPE, rooms[i].type)) {
		strcpy(room_name, rooms[i].name);
		for (j = 0; j < rooms[i].num_connections; j++) {
		    strcat(connectors, rooms[i].connected_rooms[j]);
		    strcat(connectors, ", ");
		}
		break;
	    }
	}
    } else {
	for (i = 0; i < NUM_ROOMS; i++) {
	    if (strstr(input, rooms[i].name)) {
		strcpy(room_name, rooms[i].name);
		if (strstr(END_TYPE, rooms[i].type)) retval = 1;
		for (j = 0; j < rooms[i].num_connections; j++) {
		    strcat(connectors, rooms[i].connected_rooms[j]);
		    strcat(connectors, ", ");
		}
		break;
	    }
	}
    }
    i = strlen(connectors) - 2;
    connectors[i] = '.';
    return retval;
}

/*
   get starting room details
   while(game_not_over)
   display room and connectors
   prompt
   get user input
   if input == time
   show time
   else if input == connecting room
   get room details
   if room is end
   show winning message, #steps, path
   end game
   else
   steps++, append room to path
   else
   show huh msg
   */
void play_game() {
    int num_chars, steps, game_over, room_type;
    char room_name[MAX_LEN];
    char connectors[MAX_LEN];
    char output[MAX_LEN];
    char path[MAX_LEN];
    char* input = NULL;
    size_t buf_size = 0;

    steps = 0;
    game_over = 0; 
    memset(path, '\0', MAX_LEN);

    room_type = get_room_details(0, room_name, connectors);
    while (!game_over) {

	memset(output, '\0', MAX_LEN);
	strcat(output, PROMPT1);
	strcat(output, room_name);
	printf("%s\n", output);
	memset(output, '\0', MAX_LEN);
	strcat(output, PROMPT2);
	strcat(output, connectors);
	printf("%s\n", output);

	printf("WHERE TO? >");
	num_chars = getline(&input, &buf_size, stdin);
	input[num_chars-1] = '\0';
	/* printf("you entered: %s\n", input); */
	printf("\n");

	if (strstr(connectors, input)) {
	    room_type = get_room_details(input, room_name, connectors);
	    steps++;
	    strcat(path, room_name);
	    strcat(path, "\n");
	    if (room_type == 1) {
		printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
		printf("YOU TOOK %d %s. YOUR PATH TO VICTORY WAS:\n", steps, steps==1?"STEP":"STEPS");
		printf("%s", path);
		game_over = 1;
	    }
	} else if (strcmp(input, "time") == 0) {
	    printf("getting the time\n\n");
	} else {
	    printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	}
    }

    free(input);
    input = NULL;
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

