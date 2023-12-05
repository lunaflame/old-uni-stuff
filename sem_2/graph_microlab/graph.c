#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "graph.h"
//#include "hashtable.h"
#include "assert.h"

#include "../util/idk.h"

#define ALLOC_LINKS 4 // default amount of links a node will preallocate memory for

void freeEdge(void* ptr) {
	Edge* edge = (Edge*)ptr;

	if (edge->freeAttempts == 1) {
		free(ptr);
	} else {
		edge->freeAttempts++;
	}
}

void freeNode(void* ptr) {
	assert(ptr != NULL);

	Node* what = (Node*)ptr;

	for (size_t i = daSize(what->links); i > 0; i--) {
		freeEdge(daGet(what->links, i-1));
	}

	daFree(what->links);
	free(what);
}

Node* makeNode() {
	Node* ret = malloc( sizeof(Node) );
	assert(ret != NULL);

	ret->links = daNew(ALLOC_LINKS, freeEdge);

	return ret;
}



Edge* makeEdge(Node* u, Node* v) {
	Edge* ret = asmalloc(sizeof(Edge));
	ret->from = u;
	ret->to = v;
	ret->visited = 0;
	ret->freeAttempts = 0;

	return ret;
}

void removeEdge(Edge* what) {
	for (size_t i = 0; i < daSize(what->from->links); i++) {
		Edge* edge = daGet(what->from->links, i);
		if (edge == what) {
			daSwapPop(what->from->links, i);
		}
	}

	for (size_t i = 0; i < daSize(what->to->links); i++) {
		Edge* edge = daGet(what->to->links, i);
		if (edge == what) {
			daSwapPop(what->to->links, i);
		}
	}

	free(what);
}

void connectNode(Node* from, Node* to) {
	Edge* edge = makeEdge(from, to);
	daPush(from->links, edge);
	daPush(to->links, edge);
}
