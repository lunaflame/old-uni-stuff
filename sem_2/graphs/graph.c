#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "graph.h"
//#include "hashtable.h"
#include "assert.h"

#include "../util/idk.h"

#define ALLOC_LINKS 4 // default amount of links a node will preallocate memory for

void freeNode(void* ptr) {
	assert(ptr != NULL);

	Node* what = (Node*)ptr;
	htFree(what->weights);
	daFree(what->links);
	free(what);
}

Node* makeNode() {
	Node* ret = malloc( sizeof(Node) );
	assert(ret != NULL);

	ret->links = daNew(ALLOC_LINKS, NULL);
	ret->weights = newHashtable(ALLOC_LINKS, free);

	return ret;
}

void connectNode(Node* from, Node* to, double weight) {
	daPush(from->links, to);

	// the hashtable dtor should free this by itself
	double* weightHeap = malloc(sizeof(weight));
	assert(weightHeap != NULL);

	memcpy(weightHeap, &weight, sizeof(weight));

	htInsert(from->weights, &to->id, sizeof(to->id), weightHeap);
}