#ifndef AStar_IG
#define AStar_IG

#include "../util/idk.h"

// internal
typedef struct as_data_t AStar_Data;

typedef struct as_path_t {
	AStar_Data* workingData;

	DynArray* retPath; 		// array of nodes: calculated path will be stored here
	double pathCost;

	// defined in the constructor
	int (*heurFunc)(struct as_path_t* path, const void* node, const void* to);	// heuristic function, passed into the constructor
	void (*getLinks)(struct as_path_t* path, const void* node);
} AStar_Path;

AStar_Path* asNewPath(int (*heurFunc) (AStar_Path* path, const void* link, const void* to),
	void (*getLinks) (AStar_Path* path, const void* link));

void asAddLink(AStar_Path* path, void* what, double cost);
char asCalculatePath(AStar_Path* path, void* from, void* to);
void asFree(AStar_Path* what);

#endif