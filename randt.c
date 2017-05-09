#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUM_ROOMS 7
#define MIN_CONNS 3
#define MAX_CONNS 6

int randi(int lo, int hi);
int enough_conns(int connections[NUM_ROOMS][NUM_ROOMS], int row);
int check_connections(int connections[NUM_ROOMS][NUM_ROOMS]);
void create_connections(int connections[NUM_ROOMS][NUM_ROOMS]);

int main() {
    srand(time(NULL));
    int connections[NUM_ROOMS][NUM_ROOMS];
    while (!check_connections(connections))
    	create_connections(connections);

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
