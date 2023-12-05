#include "cache.h"
#include "hashtable.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

DLL* newDLL() {
	DLL* ret = (DLL*)malloc(sizeof(DLL)); {
		ret->key = -1;
		ret->data = NULL;

		ret->next = NULL;
		ret->prev = NULL;
	}

	return ret;
}

DLL* attachNextDLL(DLL* to) {
	DLL* new = newDLL(); {
		to->next = new;
		new->prev = to;
	}

	return new;
}


Cache* newCache( size_t sz, void dtor(void*) ) {
	assert(sz != 0);
	assert(dtor != NULL);

	Cache* ret = malloc(sizeof(Cache));
		ret->ht = newHashtable(sz, NULL);
		ret->firstNode = newDLL();
		ret->sz = sz;
		ret->dtor = dtor;

	DLL* lastDLL = ret->firstNode;

	for (size_t i=1; i < sz; i++) { // starting from one because we already made the first node
		lastDLL = attachNextDLL(lastDLL);
	}

	ret->lastNode = lastDLL;

	return ret;
}

void freeCache(Cache* cache) {
	DLL* cur = cache->firstNode;
	for (size_t i=0; i < cache->sz; i++) {
		DLL* cur2 = cur;
		cur = cur->next;


		// now the hashtable handles deleting data
		if (cur2->data != NULL) {
			cache->dtor(cur2->data);
		}


		free(cur2);
		if (cur == NULL) {
			break;
		}
	}

	htFree(cache->ht);
	free(cache);
}

// sets a value in cache
void setCache(Cache* cache, int key, void* value) {

	DLL* newFirst = htGet(cache->ht, &key, sizeof(key));

	if (newFirst == NULL) {
		newFirst = cache->lastNode;
	}

	assert(newFirst != NULL);

	// connect the adjacent 2 nodes

	if (cache->firstNode != newFirst) {
		if (newFirst->prev != NULL) {
			newFirst->prev->next = newFirst->next;
		}

		if (newFirst->next != NULL) {
			newFirst->next->prev = newFirst->prev;
		}

		// if we were the last node, make cache's last node the prelast one
		if (newFirst == cache->lastNode) {
			cache->lastNode = newFirst->prev;
		}

		// make the current node be the first node
		cache->firstNode->prev = newFirst;

		newFirst->next = cache->firstNode;
		newFirst->prev = NULL;
	}

	cache->firstNode = newFirst;

	// now override the data
	if (newFirst->data != NULL) {
		if (newFirst->data != value) {
			cache->dtor(newFirst->data);
		}

		htInsert(cache->ht, &newFirst->key, sizeof(int), NULL);
	}

	newFirst->data = value;
	newFirst->key = key;

	htInsert(cache->ht, &key, sizeof(int), newFirst);
}

// returns a value from cache
void* getCache(Cache* cache, int key) {
	DLL* node = htGet(cache->ht, &key, sizeof(int));

	if (node == NULL || node->data == NULL) {
		return (void*)NULL;
	}

	DLL* curFirst = cache->firstNode;

	if (node == curFirst) {
		return node->data; // no actions necessary
	}

	// prepare to move ourselves; connect adjacent nodes

	assert(node->prev != NULL);
	node->prev->next = node->next;
	if (node->next != NULL) {
		node->next->prev = node->prev;
	}

	// moving ourselves:
	// remove our previous and make next = current first node
	node->prev = NULL;
	node->next = curFirst;

	// current first node's previous is ourselves
	curFirst->prev = node;
	// I CAME TO TAKE YOUR PLACE
	cache->firstNode = node;

	return node->data;
}


