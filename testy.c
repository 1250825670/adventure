#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* preprocessor constants */
#define NUM_ROOMS 7
#define MIN_CONNS 3
#define MAX_CONNS 6
#define NUM_NAMES 10

/* globals */
struct room {
    int id;
    char* name;
    char* type;
    int num_connections;
    struct room* connected_rooms[NUM_ROOMS-1];
} rooms[NUM_ROOMS];

const char* names[NUM_NAMES] = { "limbo", "lust", "gluttony", "greed", "wrath", 
    "heresy", "violence", "fraud", "treachery", "tyrone" };
const char* types[] = { "START_ROOM", "END_ROOM", "MID_ROOM" };

/* function prototypes */
int randi(int lo, int hi);
int enough_conns(int connections[NUM_ROOMS][NUM_ROOMS], int row);
int check_connections(int connections[NUM_ROOMS][NUM_ROOMS]);
void create_connections(int connections[NUM_ROOMS][NUM_ROOMS]);

void print_conns(struct room* room) {
    printf("rooms connnected to (%s/%d) are:\n", room->name, room->id);
    int i;

    for (i = 0; i < room->num_connections; i++) {
	if (room->connected_rooms[i]) {
	    printf(  "(%s/%d)\n", 
		    room->connected_rooms[i]->name, 
		    room->connected_rooms[i]->id);
	}
    }
    return;
}

int main() {
    srand(time(NULL));
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

    /* print results */
    for (i = 0; i < NUM_ROOMS; i++) {
	printf("id: %d, name: %s, type: %s, num cns: %d\n",
		rooms[i].id, rooms[i].name, rooms[i].type, rooms[i].num_connections);
	print_conns(&rooms[i]);
    }

    /* free calloc'd strings */
    for (i = 0; i < NUM_ROOMS; i++) {
	free(rooms[i].name);
	free(rooms[i].type);
    }

    return 0;
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
        	    /* printf("setting connection to [%d][%d]\n", i, j); */
        	    connections[i][j] = 1;
        	    connections[j][i] = 1;
        	}
            }
        }
    }

    for (i = 0; i < NUM_ROOMS; i++) {
	printf("room %d connections: ", i);
	for (j = 0; j < NUM_ROOMS; j++) {
	    printf("%d ", connections[i][j]);
	}
	printf("\n");
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
