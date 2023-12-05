#ifndef Cache_IG
#define Cache_IG
	
#include "hashtable.h" // needed for cache_t definition

typedef struct dll_t {	// doubly linked list
	void* data;
	int key;

	struct dll_t* prev;
	struct dll_t* next;
} DLL;

typedef struct cache_t {
	htTable_t* ht;  // [key] = ptrToDLLContainingData

	size_t sz;

	DLL* firstNode;
	DLL* lastNode;

	void (*dtor)(void*);
} Cache;

Cache* newCache( size_t sz, void dtor(void*) );
void freeCache(Cache* cache);
void setCache(Cache* cache, int key, void* value);
void* getCache(Cache* cache, int key);

#endif
