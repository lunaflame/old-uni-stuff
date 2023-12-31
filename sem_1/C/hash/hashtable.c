

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#include "hashtable.h"

#define HASH2_SEED 69	// the   S E E D
#define RESIZE_CAP 0.75	// if the amount of taken slots exceeds this (0-1), we resize to...
#define RESIZE_SIZE 2   // x(this) the previous size

// this can return NULL, so you should NULL-check it
htTable_t* newHashtable(size_t cap) {
	if (cap == 0) { // default
		cap = 32;
	}

	htTable_t* ret = malloc(sizeof(htTable_t));
		if (NULL == ret) {
			return ret;
		}

		ret->curCap = cap;
		ret->curStored = 0;
		ret->table = calloc(cap, sizeof(htElem_t));

	return ret;
}

// jenkins' one_at_a_time hash
static uint32_t hash1(const uint8_t* key, size_t length) {
  size_t i = 0;
  uint32_t hash = 0;
  while (i != length) {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

static inline uint32_t murmur_32_scramble(uint32_t k) {
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

// murmurhash

static uint32_t hash2(const uint8_t* key, size_t len, uint32_t seed) {
	uint32_t h = seed;
    uint32_t k;

    for (size_t i = len >> 2; i; i--) {
        memcpy(&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19);
        h = h * 5 + 0xe6546b64;
    }

    k = 0;
    for (size_t i = len & 3; i; i--) {
        k <<= 8;
        k |= key[i - 1];
    }

    h ^= murmur_32_scramble(k);
	h ^= len;
	h ^= h >> 16;
	h *= 0x85ebca6b;
	h ^= h >> 13;
	h *= 0xc2b2ae35;
	h ^= h >> 16;

	h |= 1;
	return h;
}

static char equalKeys(htElem_t* key1, htElem_t* key2) {
	return (key1->keySize == key2->keySize) && (memcmp(key1->key, key2->key, key2->keySize) == 0);
}

static void htResize(htTable_t* htbl, size_t newSize) {
	if (newSize < htbl->curStored) {
		printf("Can't resize to %d! (want: %d, stored elements: %d)\n", newSize, newSize, htbl->curStored);
		return;
	}

	htElem_t** newTable = calloc(newSize, sizeof(htElem_t));

	for (size_t i=0; i < htbl->curCap; i++) {
		if (htbl->table[i] == NULL) {
			continue;
		}

		htElem_t* elem = htbl->table[i];

		int try = -1;
		while (try < 10000) {
			try++;
			size_t newIdx = (elem->hash1 + try * elem->hash2) % newSize;
			if (newTable[newIdx] != NULL && newTable[newIdx]->taken &&
				!equalKeys(elem, newTable[newIdx])) {
				continue; // oi we're busy
			}
			newTable[newIdx] = elem;
			break;
		}
	}

	free(htbl->table);
	htbl->table = newTable;

	htbl->curCap = newSize;
}

void htFree(htTable_t* what) {
	for (size_t idx = 0; idx < what->curCap; idx++) {
		if (what->table[idx] != NULL) {
			free(what->table[idx]->key);
			free(what->table[idx]);
		}
	}

	free(what->table);
	free(what);
}

void htInsert(htTable_t* into, void* key, size_t keyLen, void* what) {
	htElem_t* elem = malloc(sizeof(htElem_t));
	{
		elem->data = what;
		elem->key = malloc(keyLen);
		memcpy(elem->key, key, keyLen);

		elem->keySize = keyLen;
		elem->hash1 = hash1(key, keyLen);
		elem->hash2 = hash2(key, keyLen, HASH2_SEED);
		elem->taken = 1;
	}

	int i = -1; // turns to 0 on the first loop
	while (i < 10000) { // failsafe, will remove
		i++;
		size_t idx = (elem->hash1 + i * elem->hash2) % into->curCap;

		if (into->table[idx] != NULL && into->table[idx]->taken &&
				!equalKeys(elem, into->table[idx])) {
			// the entry is taken (deleted or busy), increase i and try again
			continue;
		}

		// ey we got it
		into->table[idx] = elem;
		into->curStored++;
		break;
	}

	if (into->curStored >= (into->curCap * RESIZE_CAP)) {
		htResize(into, into->curCap * RESIZE_SIZE);
	}
}

void* htGet(htTable_t* htbl, void* key, size_t keyLen) {
	uint32_t h1 = hash1(key, keyLen);
	uint32_t h2 = hash2(key, keyLen, HASH2_SEED);

	int i = -1;
	while (i < 10000) { // failsafe, will remove
		i++;
		size_t idx = (h1 + i * h2) % htbl->curCap;
		if (htbl->table[idx] == NULL || !htbl->table[idx]->taken) {
			// it's impossible that there's an entry with this key if we encounter an entry
			// that isn't and was never taken
			return NULL;
		}

		htElem_t* entry = htbl->table[idx];

		if (entry->keySize == keyLen && memcmp(entry->key, key, keyLen) == 0) {
			return entry->data;
		}

	}

	return NULL;
}
