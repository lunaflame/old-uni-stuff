#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>

#include "astar.h"
//#include "hashtable.h"
#include "../util/idk.h"

#ifndef M_LOG2E // micro$oft
#define M_LOG2E (double)1.44269504088896340736
#endif

typedef struct as_data_t {
	// this struct is only used during the calculation
	DynArray* openSet;
	DynArray* closedSet;

	DynArray* neighbors;
	htTable* g_score;
	htTable* f_score;
	htTable* weights;

	void* linkingTo;		// node ptr  | used for asAddLink to keep track of
	htTable* connections;	// hashtable | what node can connect to what
} AStar_Data;

static AStar_Data asInitData() {
	// 2 DA's and 4 HT's
	AStar_Data ret;

	ret.openSet = daNew(8, NULL);
	ret.closedSet = daNew(8, NULL);
	ret.neighbors = daNew(32, NULL);

	htTable* connections = newHashtable(32, htFree); // { [node] = { [node] = 1 or 0 } }
	ret.connections = connections;

	htTable* weights = newHashtable(32, free);
	ret.weights = weights;

	htTable* g_score = newHashtable(32, free);
	ret.g_score = g_score;

	htTable* f_score = newHashtable(32, free);
	ret.f_score = f_score;

	return ret;
}

static void asFreeData(AStar_Data* what) {
	htFree(what->connections);
	htFree(what->weights);
	htFree(what->g_score);
	htFree(what->f_score);

	daFree(what->neighbors);
	daFree(what->openSet);
	daFree(what->closedSet);
}

AStar_Path* asNewPath(int (*heurFunc) (AStar_Path* path, const void* link, const void* to),
	void (*getLinks) (AStar_Path* path, const void* link)) {
	assert(heurFunc != NULL);
	assert(getLinks != NULL);

	AStar_Path* ret = malloc(sizeof(AStar_Path));
	assert(ret != NULL);

	ret->heurFunc = heurFunc;
	ret->getLinks = getLinks;

	ret->retPath = NULL;
	ret->pathCost = -INFINITY;

	return ret;
}

static void asChangeToClosed(AStar_Path* path, void* what) {
	// todo: O(n)

	assert(path != NULL);
	// what can be NULL

	char removed = 0;

	DynArray* os = path->workingData->openSet;

	for (size_t i = 0; i < daSize(os); i++) {
		void** vtx = os->ptr[i];
		if (vtx == what) {
			daSwapPop(os, i);
			removed = 1;
			break;
		}
	}

	assert(removed);

	daPush(path->workingData->closedSet, what);
}

static void* asGetOpenNode(const AStar_Path* path, const htTable* f_score) {
	// todo: O(n)
	assert(path != NULL);
	assert(f_score != NULL);

	// both path and f_score are guaranteed to not be NULL
	void* ret = NULL;
	double minscore = INFINITY;

	for (size_t i = 0; i < path->workingData->openSet->sz; i++) {
		void* vtx = path->workingData->openSet->ptr[i];

		double* score = htGet(f_score, &vtx, sizeof(&vtx));
		assert(score != NULL);

		if (*score < minscore || minscore == INFINITY) {
			ret = vtx;
			minscore = *score;
		}
	}

	assert(ret != NULL);
	return ret;
}

void asAddLink(AStar_Path* path, void* what, double cost) {
	assert(path != NULL);
	// `what` can be NULL, it doesnt care lol
	assert(path->workingData->linkingTo != NULL);

	DynArray* ngh = (DynArray*)path->workingData->neighbors;
	daPush(ngh, what);

	void* cur = path->workingData->linkingTo;
	htTable* subcons = htGet(path->workingData->connections, &cur, sizeof(&cur));
	if (subcons == NULL) {
		subcons = newHashtable(32, NULL);
		htInsert(path->workingData->connections, &cur, sizeof(&cur), subcons);
	}

	// we don't need to put a pointer there, any non-NULL value will do
	htInsert(subcons, &what, sizeof(&what), (void*)1);

	// store scores in path
	double* heapcost = malloc(sizeof(cost));
	*heapcost = cost;

	htInsert(path->workingData->weights, &what, sizeof(&what), heapcost);
}

static char asIsOpen(const AStar_Path* path, const void* what) {
	assert(path != NULL);
	assert(what != NULL);

	for (size_t i = 0; i < path->workingData->openSet->sz; i++) {
		if (path->workingData->openSet->ptr[i] == what) {
			return 1;
		}
	}

	return 0;
}

static char asIsClosed(const AStar_Path* path, const void* what) {
	assert(path != NULL);
	assert(what != NULL);

	for (size_t i = 0; i < path->workingData->closedSet->sz; i++) {
		if (path->workingData->closedSet->ptr[i] == what) {
			return 1;
		}
	}

	return 0;
}

char asCalculatePath(AStar_Path* path, void* from, void* to) {
	assert(path != NULL);
	assert(from != NULL);
	assert(to != NULL);

	AStar_Data dat = asInitData();
	path->workingData = &dat;

	daPush(dat.openSet, from);

	// not very elegant
	double* g_scoreHeap = malloc(sizeof(double));
	*g_scoreHeap = 0;

	double* f_scoreHeap = malloc(sizeof(double));
	*f_scoreHeap = path->heurFunc(path, from, to);

	htInsert(dat.g_score, &from, sizeof(&from), g_scoreHeap);
	htInsert(dat.f_score, &from, sizeof(&from), f_scoreHeap);

	htTable* htPath = newHashtable(32, NULL);


	while (dat.openSet->sz > 0) {
		void* check = asGetOpenNode(path, dat.f_score);

		if (check == to) {
			break;
		}

		asChangeToClosed(path, check);

		dat.linkingTo = check;
		path->getLinks(path, check);

		htTable* subconnections = htGet(dat.connections, &check, sizeof(&check));

		if (subconnections == NULL) {
			// if the subconnections hashtable didnt even exist, that means
			// nothing linking to it was added in the first place, so we bail
			continue;
		}

		for (size_t i = 0; i < daSize(dat.neighbors); i++) {
			void* neighbor = dat.neighbors->ptr[i];
			assert(neighbor != NULL);

			if (htGet(subconnections, &neighbor, sizeof(&neighbor)) == NULL) {
				// subconnections[neighbor] was NULL, meaning we can't go there
				// so we bail
				continue;
			}

			if (!asIsClosed(path, neighbor)) {
				double* curScore = htGet(dat.g_score, &check, sizeof(&check));
				assert(curScore != NULL);

				double* checkWeight = htGet(dat.weights, &neighbor, sizeof(&neighbor));
				assert(checkWeight != NULL);

				char nbrIsOpen = asIsOpen(path, neighbor);

				double* ptrNbrScore = htGet(dat.g_score, &neighbor, sizeof(&neighbor));

				double nbrScore = INFINITY;
				if (ptrNbrScore != NULL) {
					nbrScore = *ptrNbrScore;
				}

				if ((*curScore + *checkWeight) < nbrScore || nbrScore == INFINITY) {
					double* gscore = malloc(sizeof(double));
					*gscore = *curScore + *checkWeight;
					htInsert(dat.g_score, &neighbor, sizeof(&neighbor), gscore);

					double* fscore = malloc(sizeof(double));
					*fscore = *curScore + *checkWeight + path->heurFunc(path, neighbor, to);
					htInsert(dat.f_score, &neighbor, sizeof(&neighbor), fscore);

					htInsert(htPath, &neighbor, sizeof(&neighbor), check);

					if (!nbrIsOpen) {
						daPush(dat.openSet, neighbor);
					}
				}
			}

		}
	}

	double* finalScore = htGet(dat.g_score, &to, sizeof(&to));
	if (finalScore != NULL) {
		path->pathCost = *finalScore;
	}

	// we're done with the data and we won't need it anymore
	// the only thing we need is `htPath`

	asFreeData(&dat);

	void* lastNode = to;
	DynArray* retPath = daNew(32, NULL);

	while (lastNode) {
		daPush(retPath, lastNode);

		void* newNode = htGet(htPath, &lastNode, sizeof(&lastNode));

		if (newNode == NULL) {
			if (lastNode != from) {
				// failed to find the path, free & bail
				htFree(htPath);
				daFree(retPath);
				return 0;
			}

			break;
		}

		lastNode = newNode;
	}

	path->retPath = retPath;

	htFree(htPath);

	return 1;
}


void asFree(AStar_Path* what) {
	if (what->retPath != NULL) {
		daFree(what->retPath);
	}
	free(what);
}