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
#define MIN_CONNS 3
#define MAX_CONNS 6
#define NUM_NAMES 10
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
    struct room* connected_rooms[NUM_ROOMS-1];
} rooms[NUM_ROOMS];

const char* names[] = { "limbo", "lust", "gluttony", "greed", "wrath", 
    "heresy", "violence", "fraud", "treachery", "tyrone" };
const char* types[] = { "START_ROOM", "END_ROOM", "MID_ROOM" };
const char* con_title[]  = { "CONNECTION 1: ", "CONNECTION 2: ",
    "CONNECTION 3: ", "CONNECTION 4: ", "CONNECTION 5: ", "CONNECTION 6: " };
const char* name_title = "ROOM NAME: ";
const char* type_title = "ROOM TYPE: ";

/* function prototypes */
int randi(int lo, int hi);
void init_rooms();
int enough_conns(int connections[NUM_ROOMS][NUM_ROOMS], int row);
int check_connections(int connections[NUM_ROOMS][NUM_ROOMS]);
void create_connections(int connections[NUM_ROOMS][NUM_ROOMS]);
void cleanup();
void create_dir(char* dirname);
void create_files(char* dirname);


int main() {
    srand(time(NULL));
    init_rooms();

    char dirname[MAX_LEN];
    create_dir(dirname);
    create_files(dirname);

    return 0;
}

void init_rooms() {
    int i, j, k, name_num;

    int taken[NUM_NAMES];
    for (i = 0; i < NUM_NAMES; i++)
	taken[i] = 0;

    int connections[NUM_ROOMS][NUM_ROOMS];

    /* assign name, default type */
    for (i = 0; i < NUM_ROOMS; i++) {
	rooms[i].id = i;
	rooms[i].name = calloc(16, sizeof(char));
	rooms[i].type = calloc(16, sizeof(char));

	while (1) {
	    name_num = randi(0, NUM_NAMES-1);
	    if (taken[name_num] == 0) {
		taken[name_num] = 1;
		strcpy(rooms[i].name, names[name_num]);
		break;
	    }
	}

	strcpy(rooms[i].type, types[2]);
    }

    /* assign start & end room type */
    int start_num, end_num;
    start_num = randi(0, NUM_ROOMS-1);
    strcpy(rooms[start_num].type, types[0]);
    while (1) {
	if ((end_num = randi(0, NUM_ROOMS-1)) != start_num) {
	    strcpy(rooms[end_num].type, types[1]);
	    break;
	}
    }

    /* get random connections */
    while (!check_connections(connections)) 
	create_connections(connections);

    /* assign connections */
    for (i = 0; i < NUM_ROOMS; i++) {
	k = 0;
	for (j = 0; j < NUM_ROOMS; j++) {
	    if (connections[i][j]) {
		rooms[i].connected_rooms[k++] = &rooms[j];
	    }
	}
	rooms[i].num_connections = k;
    }
}

/* 
 * return a value in range [lo, hi]
 * from cs 475 
 */
int randi(int lo, int hi) {
    float r = (float)rand();
    float low = (float)lo;
    float high = (float)hi + 0.9999f;
    return (int)(low + r * (high - low) / (float)RAND_MAX);
}

/* are there enough connections */
int enough_conns(int connections[NUM_ROOMS][NUM_ROOMS], int row) {
    int j, conns;
    conns = 0;

    for (j = 0; j < NUM_ROOMS; j++)
	if (connections[row][j]) conns++;

    return (conns >= MIN_CONNS && conns <= MAX_CONNS);
}

void create_connections(int connections[NUM_ROOMS][NUM_ROOMS]) {
    int i, j, tries;

    /* initialize adj matrix to 0, no connections */
    for (i = 0; i < NUM_ROOMS; i++)
	for (j = 0; j < NUM_ROOMS; j++)
	    connections[i][j] = 0;

    tries = 0;
    for (i = 0; i < NUM_ROOMS; i++) {
	while (!enough_conns(connections, i) && tries < 10) {
	    tries++;
	    for (j = i; j < NUM_ROOMS; j++) {
		if (randi(0, 1) && i != j) {
		    connections[i][j] = 1;
		    connections[j][i] = 1;
		}
	    }
	}
    }
}

int check_connections(int connections[NUM_ROOMS][NUM_ROOMS]) {
    int i, j, retval, total_cns;

    retval = 1;
    for (i = 0; i < NUM_ROOMS; i++) {
	total_cns = 0;
	for (j = 0; j < NUM_ROOMS; j++) {
	    if (connections[i][j]) total_cns++;
	}
	if (total_cns < MIN_CONNS) {
	    retval = 0;
	    break;
	}
    }
    return retval;
}

void cleanup() {
    int i;
    /* free calloc'd strings */
    for (i = 0; i < NUM_ROOMS; i++) {
	free(rooms[i].name);
	free(rooms[i].type);
    }
}

void create_dir(char* dirname) {
    int result;

    /* get pid and convert to string */
    int mypid = getpid();
    char cpid[MAX_LEN];
    memset(cpid, '\0', MAX_LEN);
    sprintf(cpid, "%d", mypid);

    /* create dir, exit if error */
    memset(dirname, '\0', MAX_LEN);
    strcat(dirname, DIR_PREFIX);
    strcat(dirname, cpid);

    if ((result = mkdir(dirname, 0755)) != 0) {
	printf("error creating directory\n");
	exit(1);
    }
}

void create_files(char* dirname) {
    int i, j, fd;
    size_t n, len;
    char file_name[MAX_LEN];
    char file_path[MAX_LEN];
    char tmp[MAX_BUF_LEN];

    for (i = 0; i < NUM_ROOMS; i++) {
	/* clean slate */
	memset(file_path, '\0', MAX_LEN);
	memset(file_name, '\0', MAX_LEN);
	memset(tmp, '\0', MAX_BUF_LEN);

	/* create and open file, exit if error */
	strcpy(file_name, rooms[i].name);
	strcat(file_path, dirname);
	strcat(file_path, "/");
	strcat(file_path, file_name);
	strcat(file_path, FILE_SUFFIX);

	fd = open(file_path, O_RDWR | O_CREAT, 0644);
	if (fd == -1) {
	    printf("error creating file\n");
	    exit(1);
	}

	/* room name */
	strcat(tmp, name_title);
	strcat(tmp, rooms[i].name);
	strcat(tmp, "\n");

	/* room connections */
	for (j = 0; j < rooms[i].num_connections; j++) {
	    strcat(tmp, con_title[j]);
	    strcat(tmp, rooms[i].connected_rooms[j]->name);
	    strcat(tmp, "\n");
	}

	/* room type */
	strcat(tmp, type_title);
	strcat(tmp, rooms[i].type);
	strcat(tmp, "\n");

	/* write data to file, exit if not all bytes written */
	len = strlen(tmp) * sizeof(char);
	n = write(fd, tmp, len);

	if (n != len) {
	    printf ("error writing to file\n");
	    exit(0);
	}

	close(fd);
    }
}

