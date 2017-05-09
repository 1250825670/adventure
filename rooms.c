#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct room {
    int id;
    char* name;
    int numOutboundConnections;
    struct room* outboundConnections[6];
};

void PrintRoomOutboundConnections(struct room* input) {
    printf("the rooms connected to (%s/%d) are:\n",
	    input->name, input->id);

    int i;
    for (i = 0; i < input->numOutboundConnections; i++)
	printf("  (%s/%d)\n", input->outboundConnections[i]->name,
		input->outboundConnections[i]->id);
    return;
}

int main(int argc, char* argv[]) {
    struct room startRoom;
    startRoom.id = 0;
    startRoom.name = calloc(16, sizeof(char));
    strcpy(startRoom.name, "living room");

    struct room midRoom1;
    midRoom1.id = 1;
    midRoom1.name = calloc(16, sizeof(char));
    strcpy(midRoom1.name, "kitchen");

    struct room midRoom2;
    midRoom2.id = 2;
    midRoom2.name = calloc(16, sizeof(char));
    strcpy(midRoom2.name, "dining room");

    struct room endRoom;
    endRoom.id = 3;
    endRoom.name = calloc(16, sizeof(char));
    strcpy(endRoom.name, "garage");

    startRoom.numOutboundConnections = 2;
    startRoom.outboundConnections[0] = &midRoom1;
    startRoom.outboundConnections[1] = &midRoom2;

    midRoom1.numOutboundConnections = 3;
    midRoom1.outboundConnections[0] = &startRoom;
    midRoom1.outboundConnections[1] = &midRoom2;
    midRoom1.outboundConnections[2] = &endRoom;

    midRoom2.numOutboundConnections = 3;
    midRoom2.outboundConnections[0] = &startRoom;
    midRoom2.outboundConnections[1] = &midRoom1;
    midRoom2.outboundConnections[2] = &endRoom;

    endRoom.numOutboundConnections = 2;
    endRoom.outboundConnections[0] = &midRoom1;
    endRoom.outboundConnections[1] = &midRoom2;

    PrintRoomOutboundConnections(&startRoom);
    PrintRoomOutboundConnections(&midRoom1);
    PrintRoomOutboundConnections(&midRoom2);
    PrintRoomOutboundConnections(&endRoom);

    free(startRoom.name);
    free(midRoom1.name);
    free(midRoom2.name);
    free(endRoom.name);
    return 0;
}
