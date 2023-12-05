//gcc 7.4.0

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

#define FILT_SZ 10000 	// bloom filter size in bytes
#define MURMUR_SEED 69 	// random seed for murmurhash

typedef struct bloom_t {
	char* cells;
	size_t len; // length is in bits
} Bloom;

// todo: add more hashes
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

static uint32_t ElfHash(const uint8_t* key, size_t length)
{
	size_t i = 0;

	unsigned long h = 0;
	unsigned long high;

	while (i != length) {
		h = (h << 4) + key[i];
		high = (h & 0xF0000000);
		if (high)
			h ^= high >> 24;
		h &= ~high;
		i++;
	}
	return h;
}

static inline uint32_t murmur_32_scramble(uint32_t k) {
	k *= 0xcc9e2d51;
	k = (k << 15) | (k >> 17);
	k *= 0x1b873593;
	return k;
}

// murmurhash

static uint32_t hash2(const uint8_t* key, size_t len) {
	uint32_t h = MURMUR_SEED;
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


uint32_t (*const Hashes[])() = {
		&hash1,
		&ElfHash,
		&hash2,
};

Bloom* newBloom(size_t len) {
	Bloom* ret = malloc(sizeof(Bloom));
		ret->cells = calloc( len, 1 ) ;
		ret->len = len * 8;

	return ret;
}

void blAdd(Bloom* filt, const void* what, size_t len) {
	assert(filt != NULL);

	size_t amtHashFuncs = sizeof(Hashes) / sizeof( Hashes[0] );
	for (size_t i = 0; i < amtHashFuncs; i++) {
		uint32_t hash = Hashes[i] (what, len);
		uint32_t total_bit = hash % filt->len;

		uint32_t byte = total_bit / 8; // floored div
		char bit = total_bit % 8;

		filt->cells[byte] |= (1 << bit);
	}
}

char blCheck(Bloom* filt, const void* what, size_t len) {
	assert(filt != NULL);

	size_t amtHashFuncs = sizeof(Hashes) / sizeof( Hashes[0] );

	for (size_t i = 0; i < amtHashFuncs; i++) {
		uint32_t hash = Hashes[i] (what, len);
		uint32_t total_bit = hash % filt->len;

		uint32_t byte = total_bit / 8; // floored div
		char bit = total_bit % 8;

		if ((filt->cells[byte] & (1 << bit)) == 0) {
			return 0;
		}
	}

	return 1;
}

void freeBloom(Bloom* filt) {
	free(filt->cells);
	free(filt);
}

void flushInput() {
	// c is cringe
	int chr = getchar();
	while (chr != '\n' && chr != EOF) { chr = getchar(); }
}

void showOptions() {
	printf("1: Put a value into the filter\n"
		   "2: Check if a value was used\n"
		   "3: Exit\n");
}

void readV(Bloom* filt) {
	char val[65536] = {0};
	printf("Enter value (65535chars max.):	> ");
	scanf("%65535s", val);

	blAdd(filt, val, strlen(val));
}

void readFilter(Bloom* filt) {
	char val[65536] = {0};
	printf("Enter value (65535chars max.):	> ");
	scanf("%65535s", val);

	if ( blCheck(filt, val, strlen(val)) ) {
		printf("Existed.\n");
	} else {
		printf("Did not exist.\n");
	}
}

void readOption(Bloom* filt) {
	char validOption = 0;
	char option;

	option = 0;

	while (!validOption) {
		printf("\n");

		showOptions();
		printf("Your choice: > ");
		scanf("%c", &option);

		if ( (option > '0' && option < '4') ) {
			switch (option - '0') {
				case 1:
					readV(filt);
					flushInput();
					break;

				case 2:
					readFilter(filt);
					flushInput();
					break;

				case 3:
					return;

				default:
					printf("unrecognized option? not supposed to happen!\n");
			}

		} else {
			printf("Unrecognized option: '%d'\n", option);
		}

	}

}

int main(void) {
	Bloom* b = newBloom(FILT_SZ);
	readOption(b);
	freeBloom(b);
	return 0;
}