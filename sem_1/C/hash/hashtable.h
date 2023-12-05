#ifndef htDefined
#define htDefined

#include <stddef.h>
#include <stdint.h>

// structs:
struct htElem {
	void* data;

	char* key;
	size_t keySize;

	uint32_t hash1;
	uint32_t hash2;
	char taken;
};

typedef struct htElem htElem_t;

struct htTable {
	size_t curCap;
	size_t curStored;
	htElem_t** table;
};

typedef struct htTable htTable_t;


// functions:
htTable_t* newHashtable(size_t);
void htInsert(htTable_t*, void*, size_t, void*);
void* htGet(htTable_t*, void*, size_t);
#endif