#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "graph.h"
#include "assert.h"

#include "../util/idk.h"

#define ALLOC_LINKS 4 // default amount of links a node will preallocate memory for

void freeNode(void* ptr) {
	assert(ptr != NULL);

	Node* what = (Node*)ptr;

	for (size_t i = 0; i < daSize(what->links); i++) {
		Edge* edge = (Edge*)daGet(what->links, i);
		if (edge->from == what) {
			edge->from = NULL;
		} else {
			edge->to = NULL;
		}
	}

	daFree(what->links);
	free(what);
}

const char weightUsedTimes = 2;

struct weight {
	double* n;
	unsigned char freed;
};

static void freeWeight(void* help_me) {
	struct weight* a = (struct weight*)help_me;

	// this is terrible D:
	a->freed++;

	if (a->freed > 1) {
		if (a->freed == weightUsedTimes) {
			free(a);
		}

		return;
	}

	free(a->n);
}

static void freeEdge(void* ptr) {
	free(ptr);
}

Node* makeNode() {
	Node* ret = asmalloc(sizeof(Node));

	ret->links = daNew(ALLOC_LINKS, freeEdge);

	return ret;
}

Edge* makeEdge(Node* u, Node* v, double cap) {
	Edge* ret = asmalloc(sizeof(Edge));
	ret->from = u;
	ret->to = v;
	ret->flow = 0;
	ret->capacity = cap;

	return ret;
}

// in dinic, our graph is not oriented, meaning we add links to both nodes
void connectNode(Node* from, Node* to, double weight) {
	Edge* edge = makeEdge(from, to, weight);
	daPush(from->links, edge);

	Edge* revEdge = makeEdge(to, from, weight);
	daPush(to->links, revEdge); // reverse edge
}
