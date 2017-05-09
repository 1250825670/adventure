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
#define DIRPREFIX "hagenr.rooms."
#define FILESUFFIX "_room"
#define MAXLEN 128

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
void init_rooms();
int enough_conns(int connections[NUM_ROOMS][NUM_ROOMS], int row);
int check_connections(int connections[NUM_ROOMS][NUM_ROOMS]);
void create_connections(int connections[NUM_ROOMS][NUM_ROOMS]);
void cleanup();
void print_conns(struct room* room);
void create_dir(char* dirname);
void create_files(char* dirname);
