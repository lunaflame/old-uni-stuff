#define MAX 16

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

typedef struct Node_s {
	int id;
	struct Node_s* siblings[MAX];
	size_t siblSz;
} Node;

Node* ndNew(int id) {
	Node* ret = malloc(sizeof(Node)); {
		ret->id = id;
		memset(ret->siblings, 0, sizeof(ret->siblings));
		ret->siblSz = 0;
	}

	return ret;
}

void ndAddSibl(Node* to, Node* sibl) {
	to->siblings[to->siblSz] = sibl;
	to->siblSz++;
}

void ndFree(Node* what) {
	free(what);
}

// helper function, won't work in "real" situations
Node* getNode(Node* graph[], int id) {
	return graph[id - 1];
}

void ndAddSiblID(Node* graph[], int toID, int siblID) {
	Node* to = getNode(graph, toID);
	Node* sibl = getNode(graph, siblID);

	// check if we have connected the same nodes multiple times by accident
	for (size_t i=0; i<to->siblSz; i++) {
		assert(to->siblings[i] != sibl);
	}

	to->siblings[to->siblSz] = sibl;
	to->siblSz++;

	for (size_t i=0; i<sibl->siblSz; i++) {
		assert(sibl->siblings[i] != to);
	}

	sibl->siblings[sibl->siblSz] = to;
	sibl->siblSz++;
}

size_t init(Node* graph[]) {

	for (size_t i=0; i < MAX; i++) {
		graph[i] = ndNew(i + 1);
	}

	ndAddSiblID(graph, 1, 2);

	ndAddSiblID(graph, 2, 3);		ndAddSiblID(graph, 6, 7);
	ndAddSiblID(graph, 2, 4);		ndAddSiblID(graph, 6, 8);
	ndAddSiblID(graph, 2, 14);		ndAddSiblID(graph, 6, 12);
	ndAddSiblID(graph, 2, 15);		ndAddSiblID(graph, 6, 13);

	ndAddSiblID(graph, 3, 4);		ndAddSiblID(graph, 7, 8);

	ndAddSiblID(graph, 4, 5);		ndAddSiblID(graph, 8, 9);
	ndAddSiblID(graph, 4, 6);		ndAddSiblID(graph, 8, 10);
	ndAddSiblID(graph, 4, 13);		ndAddSiblID(graph, 8, 11);
	ndAddSiblID(graph, 4, 14);		ndAddSiblID(graph, 8, 12);

	ndAddSiblID(graph, 5, 6);		ndAddSiblID(graph, 9, 10);


	ndAddSiblID(graph, 10, 1);
	ndAddSiblID(graph, 10, 2);
	ndAddSiblID(graph, 10, 11);
	ndAddSiblID(graph, 10, 15);

	// inner-est circle of hell
	ndAddSiblID(graph, 11, 15);
	ndAddSiblID(graph, 15, 14);
	ndAddSiblID(graph, 14, 13);
	ndAddSiblID(graph, 13, 12);
	ndAddSiblID(graph, 12, 11);

	size_t retSize = 0;

	for (size_t i=0; i < MAX; i++) {
		if (graph[i]->siblSz == 0) {
			ndFree(graph[i]);
		} else {
			retSize++;
		}
	}

	return retSize;
}

char checkOdd(Node* graph[], size_t sz) {
	assert(sz != 0);

	for (size_t i=0; i < sz; i++) {
		if (graph[i]->siblSz % 2 != 0) {
			printf("%zu(%d) is odd %zu\n", i, graph[i]->id, graph[i]->siblSz);
			return 0;
		}
	}

	return 1;
}

int main() {
	printf("starting\n");

	Node* graph[MAX];
	size_t sz = init(graph);
	printf("created %zu nodes\n", sz);
	char ok = checkOdd(graph, sz);

	if (!ok) {
		printf("Halting; odd-check failed.");
		return 0;
	}

	printf("Odd check passed...\n");

	for (size_t i = 0; i < sz; i++) {
		//ndFree(graph[i]);
	}

	printf("freed all\n");

	return 0;
}



