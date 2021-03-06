/*
 * author: ron hagen, cs 344-400
 * program2: hagenr.adventure.c
 * description: a maze game where the user starts in a room and must navigate
 * through connecting rooms to the end. creates the maze by reading files with
 * low-level c functions and loading the data into an array of room structs.
 * game play begins with the display of the starting room. the loop starts by 
 * prompting the user to enter the connecting room to move to, and ends when
 * the user enters the 'end room'. the number of rooms and the path taken are
 * then displayed . the user can also get the time of day (from a diff thread).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <assert.h>
#include <ctype.h>

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
#define TIME_FILE "currentTime.txt"

/* globals */
struct room {
    int id;
    char* name;
    char* type;
    int num_connections;
    char* connected_rooms[NUM_ROOMS-1];
} rooms[NUM_ROOMS];

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/* function prototypes */
void load_struct();
void get_newest_dir(char* newest_dir_name);
void read_files(char* newest_dir_name);
void play_game();
void cleanup();
int get_room_details(char* input, char* room_name, char* connectors);
void *set_time();
void get_time();

/* calls functions as described in the opening comment */
int main() {
    load_struct();
    play_game();
    cleanup();
    return 0;
}

/*
 * void load_struct ()
 * wrapper for get_newest_dir and read_files
 */
void load_struct () {
    char newest_dir[MAX_LEN];
    get_newest_dir(newest_dir);
    read_files(newest_dir);
}

/*
 * void get_newest_dir(char* newest_dir_name) 
 * params: pointer to a character array for the newest directory with the
 * specified prefix. based on code presented in lecture, opens the current 
 * dir and loops thru all subdirectories to find the one with the most recent
 * modification date-time.
 */
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

    /* loop thru all subdirectories */
    while ((file_in_dir = readdir(dp)) != NULL) {
	/* we found a dir that matches */
	if (strstr(file_in_dir->d_name, DIR_PREFIX) != NULL) {
	    stat(file_in_dir->d_name, &dir_attributes);
	    if ((int)dir_attributes.st_mtime > newest_dir_time) {
		/* if it's the most recently mod'd, copy the name */
		newest_dir_time = (int) dir_attributes.st_mtime;
		memset(newest_dir_name, '\0', MAX_LEN);
		strcpy(newest_dir_name, file_in_dir->d_name);
	    }
	}
    }
    closedir(dp);
}

/*
 * void read_files(char* newest_dir_name)
 * params: pointer to the char array with the newest dir
 * opens each file in the directory (except the '.' & '..') while looping thru
 * the array of structs reads the data by character into a temp array which is 
 * then copied into the struct data member. uses low-level c functions
 */
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

	    /* open the file for reading */
	    fd = open(file_name, O_RDONLY);
	    if (fd < 0) {
		perror("failed to open file");
		exit(1);
	    }

	    /* move past the data header & copy the name */
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

	    /* while on lines that start with 'CONNECTION...' */
	    /* move past the data header and copy the connections */
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
		/* get the # of connections while we're here */
		num_cns++;
		rooms[i].connected_rooms[j] = calloc(k, sizeof(char));
		strcpy(rooms[i].connected_rooms[j++], data);
		/* still in connections? */
		nread = read(fd, &ch, 1);
		lseek(fd, -1, SEEK_CUR);
		if (ch != CON_TITLE[0]) break;
	    }
	    rooms[i].num_connections = num_cns;

	    /* on the last line of file, move past the hdr and copy type */
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

	    /* move to next struct and close this file */
	    i++;
	    close(fd);
	}
    }
    closedir(dp);
}

/*
 * int get_room_details(char* input, char* room_name, char* connectors)
 * params: name of room entered by user, pointers to array for room name (only
 * needed b/c start room input is null), array for connectors.
 * returns 1 if room is the end room, else 0
 * 1st time this is called, the input is null and we search for the start room.
 * once it's found load the name and connectors into respective arrays. all
 * other times, its called with the user input and the search is for a matching
 * room name.
 */
int get_room_details(char* input, char* room_name, char* connectors) {
    int i, j, retval;

    memset(room_name, '\0', MAX_LEN);
    memset(connectors, '\0', MAX_LEN);
    retval = 0;

    /* look for start room */
    if (!input) {
	for (i = 0; i < NUM_ROOMS; i++) {
	    if (strstr(START_TYPE, rooms[i].type)) {
		strcpy(room_name, rooms[i].name);
		/* load connectors */
		for (j = 0; j < rooms[i].num_connections; j++) {
		    strcat(connectors, rooms[i].connected_rooms[j]);
		    strcat(connectors, ", ");
		}
		/* no need to keep looking */
		break;
	    }
	}
    } else {
	/* same basic loop as above just diff search criterion */
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
    /* make sure punctuation is correct */
    i = strlen(connectors) - 2;
    connectors[i] = '.';
    return retval;
}

/*
 * void play_game()
 * game play loop: get starting room details and:
   while the user hasn't found the end room
     display room and connectors
     prompt and get user input
     if input == time
       wake up the thread and show time
     else if input == connecting room
       get room details
       if this room is end
         show winning message, #steps, path
         end game
       else
         steps++, append room to path
     else
       show huh msg
     back to while
   */
void play_game() {
    int num_chars, steps, game_over, room_type, tnum;
    char room_name[MAX_LEN];
    char connectors[MAX_LEN];
    char output[MAX_LEN];
    char path[MAX_LEN];
    char* input = NULL;
    size_t buf_size = 0;
    pthread_t mythread;

    steps = 0;
    game_over = 0; 
    memset(path, '\0', MAX_LEN);

    /* enable lock and create the thread */
    pthread_mutex_lock(&lock);
    tnum = pthread_create(&mythread, NULL, set_time, NULL);
    assert(tnum == 0);

    /* start room details */
    room_type = get_room_details(0, room_name, connectors);
    while (!game_over) {
	/* construct the 1st 2 lines */
	memset(output, '\0', MAX_LEN);
	strcat(output, PROMPT1);
	strcat(output, room_name);
	printf("%s\n", output);
	memset(output, '\0', MAX_LEN);
	strcat(output, PROMPT2);
	strcat(output, connectors);
	printf("%s\n", output);

	/* get user input */
	printf("WHERE TO? >");
	num_chars = getline(&input, &buf_size, stdin);
	input[num_chars-1] = '\0';
	printf("\n");

	/* did the user enter a valid connector? */
	if (strstr(connectors, input)) {
	    room_type = get_room_details(input, room_name, connectors);
	    steps++;
	    strcat(path, room_name);
	    strcat(path, "\n");
	    if (room_type == 1) {
		printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
		/* kinda anal, but it's a bad habit from rails... */
		printf("YOU TOOK %d %s. YOUR PATH TO VICTORY WAS:\n", steps, steps==1?"STEP":"STEPS");
		printf("%s", path);
		game_over = 1;
	    }
	} else if (strcmp(input, "time") == 0) {
	    /* unlock the mutex, release the thread to set the time */
	    /* whilst pausing main thread game play */
	    pthread_mutex_unlock(&lock);
	    pthread_join(mythread, NULL);
	    pthread_mutex_lock(&lock);
	    tnum = pthread_create(&mythread, NULL, set_time, NULL);
	    assert(tnum == 0);
	    /* read the file created by mythread */
	    get_time();
	} else {
	    printf("HUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	}
	/* cleanup, ready for reuse */
	free(input);
	input = NULL;
    }
}

/* 
 * void *set_time()
 * allow a thread exclusive use of resources to write time, in the proper format,
 * to a file. uses the higher level c file functions (a lot more convenient)
 */
void *set_time() {
    pthread_mutex_lock(&lock);
    int i;
    time_t rawtime;
    struct tm * timeinfo;
    char buf[80];
    memset(buf, '\0', 80);

    /* get the datetime */
    time (&rawtime);
    timeinfo = localtime (&rawtime);

    /* format the info in the tm struct */
    strftime (buf, 80, "%l:%M%p, %A, %B %e, %G", timeinfo);

    /* get rid of the leading space if hour is 1 digit from the %l flag */
    i = 0;
    if (buf[0] == ' ') {
	while (buf[i]) {
	    buf[i] = buf[i+1];
	    i++;
	}
    }
    buf[i] = '\0';

    /* lower case the AM/PM from the %p flag */
    i = 0;
    while (buf[i]) {
	if (i < 8 && (buf[i] == 'A' || buf[i] == 'P' || buf[i] == 'M'))
	    buf[i] = tolower(buf[i]);
	i++;
    }

    /* write the datetime to a file */
    FILE *fout = fopen(TIME_FILE, "w");
    fprintf(fout, "%s\n", buf);
    pthread_mutex_unlock(&lock);
    fclose(fout);

    return NULL;
}

/*
 * void cleanup()
 * free all memory allocated on the heap
 */
void cleanup() {
    int i, j;
    for (i = 0; i < NUM_ROOMS; i++) {
	free(rooms[i].name);
	free(rooms[i].type);
	for (j = 0; j < rooms[i].num_connections; j++) {
	    free(rooms[i].connected_rooms[j]);
	}
    }
    pthread_mutex_destroy(&lock);
}

/*
 * void get_time()
 * main thread reads file and copies contents into a buffer that is read to
 * stdout. it breaks the metaphor by printing output outside of the game play
 * loop but seems cleaner to do this way. uses the high-level c file functions
 */
void get_time() {
    FILE *fin;
    char timestr[MAX_LEN];
    fin = fopen(TIME_FILE, "r");
    fgets(timestr, MAX_LEN, fin);
    printf("%s\n", timestr);
    fclose(fin);
}
