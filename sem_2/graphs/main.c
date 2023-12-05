#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "graphFile.h"
#include "graph.h"
//#include "hashtable.h"
#include "astar.h"
#include "../util/idk.h"


// used in the heuristic function, assuming the graph is a grid
#define GRID_W 10
#define GRID_H 10
// distance between 2 nodes on the same coordinate (not diagonal)
#define EDGE_DIST 10

void getLinks(AStar_Path* path, const void* ptrnode) {
	assert(path != NULL);
	assert(ptrnode != NULL);

	const Node* node = (Node*)ptrnode;
	DynArray* links = node->links;

	for (size_t i = 0; i < daSize(links); i++) {

		Node* toNode = node->links->ptr[i];

		double* weight = htGet(node->weights, &(toNode->id), sizeof(toNode->id));
		assert(weight != NULL);

		asAddLink(path, (void*)toNode, *weight);
	}
}

int heurFunc(AStar_Path* path, const void* fromptr, const void* toptr) {
	assert(path != NULL);
	assert(fromptr != NULL);
	assert(toptr != NULL);

	(void)path;

	const Node* from = (Node*)fromptr;
	const Node* to = (Node*)toptr;

	int from_id = from->id;
	int from_x = from_id / GRID_H;
	int from_y = (from_id - 1) % GRID_W;

	int to_id = to->id;
	int to_x = to_id / GRID_H;
	int to_y = (to_id - 1) % GRID_W;

	double dist = sqrt( pow(to_x - from_x, 2) + pow( (to_y - from_y) * EDGE_DIST, 2) );
	// printf("(%d, %d) -> (%d, %d) = %f\n", from_x, from_y, to_x, to_y, dist);
	return dist;
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		printf("No file supplied.\n");
		return 0;
	}

	printf("Opening file: %s\n", argv[1]);

	FILE* fp = fopen(argv[1], "r");

	if (fp == NULL) {
		printf("Failed to open file.\n");
		return 0;
	}

	htTable* graphs = newHashtable(64, freeNode);
	assert(graphs != NULL);

	char parsedOk = parseFile(fp, graphs);

	if (!parsedOk) {
		printf("Error while parsing file.\n");
		htFree(graphs);
		return 0;
	}

	int from = -1;
	int to = -1;

	printf("Enter path to find: ");
	int amt = scanf("%d %d", &from, &to);
	if (amt != 2) {
		printf("Failed to read one of the nodes.\n");
		htFree(graphs);
		return 0;
	}

	Node* fromNode = htGet(graphs, &from, sizeof(from));
	Node* toNode = htGet(graphs, &to, sizeof(to));

	if (!fromNode) {
		printf("Node #%d [from] does not exist.\n", from);
		htFree(graphs);
		return 0;
	}

	if (!toNode) {
		printf("Node #%d [to] does not exist.\n", to);
		htFree(graphs);
		return 0;
	}

	printf("Finding %d -> %d...\n", from, to);

	AStar_Path* path = asNewPath(heurFunc, getLinks);
	char ok = asCalculatePath(path, fromNode, toNode);

	printf("Path calculated...\n");

	if (ok) {

		for (size_t i = path->retPath->sz; i > 0; i--) {
			Node* node = (Node*)path->retPath->ptr[i - 1];
			printf("%d", node->id);

			if (i - 1 != 0) {
				printf(" -> ");
			}
		}

		printf("\nTotal cost: %.1lf\n", path->pathCost);
	} else {
		printf("A* reported no path!\n");
	}

	asFree(path);
	htFree(graphs);


	return 0;
}