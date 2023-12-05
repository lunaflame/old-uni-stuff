#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// a holder can only hold this many buckets (pointers) before getting a new one
#define BUCKETS_PER_HOLDER 4

#define BYTES_PER_BUCKET 32768
#define INITIAL_BUCKETS 5 // initialize allocates this many buckets for each

#define BUCKETSIZE_TYPE size_t

const BUCKETSIZE_TYPE bucketSizes[] = {
	64,
	256,
	1024,
	4096
};


const size_t BucketEnumSize = sizeof(bucketSizes) / sizeof(BUCKETSIZE_TYPE);

typedef struct bucket_t {
	// how many free slots we have in here
	size_t freeSlots;
	// the master memory will be probed for vacant buckets before sbrk'ing
	char vacant;
	// how much memory is 1 cell
	BUCKETSIZE_TYPE cellSize;
	// buckets are linked lists
	void* nextBucket;

	// the main memory which this bucket holds
	char mem[BYTES_PER_BUCKET];
} Bucket;

typedef struct bucket_holder_t {
	// how much memory does one cell in every bucket in
	// this holder hold
	BUCKETSIZE_TYPE cellSize;

	size_t freeBuckets;

	// next holder to use if we run out of space; O(n) y'all
	struct bucket_holder_t* next;
	Bucket* buckets[BUCKETS_PER_HOLDER];
} BucketHolder;


void* MasterMemory;

BucketHolder* BucketHolderMemory;
Bucket* BucketMemory;

char Initialized = 0;

BucketHolder* newHolder(BUCKETSIZE_TYPE cellSize) {
	// todo: is using malloc here legal??
	BucketHolder* holder = malloc(sizeof(BucketHolder));
	holder->cellSize = cellSize;
	holder->freeBuckets = 0;
}

Bucket* newBucket(BucketHolder* reqHolder) {
	BucketHolder* holder = reqHolder;

	// find either a holder with an available bucket slot
	// or the last holder
	while ( (holder->freeBuckets == 0) && (holder->next != NULL) ) {
		holder = holder->next;
	}

	// still no buckets available = ran out of holders, get a new one
	if (holder->freeBuckets == 0) {
		BucketHolder* prev = holder;
		holder = newHolder(holder->cellSize);
		prev->next = holder;
	}

	// todo: init & put a bucket into the holder
}

void initialize() {
	// how much memory we need to hold the initial buckets
	size_t bucketMemory = sizeof(BucketEnumSize) * BYTES_PER_BUCKET * INITIAL_BUCKETS;

	// how much memory we need to hold pointers to each bucket
	size_t holderMemory = sizeof(BucketHolder) * sizeof(BucketEnumSize);

	size_t memToAllocate = bucketMemory + holderMemory;
	MasterMemory = malloc(memToAllocate);
	BucketHolderMemory = MasterMemory;
	BucketMemory = (Bucket*)(BucketHolderMemory + holderMemory);

	for (size_t i = 0; i < BucketEnumSize; i++) {
		BucketHolder holder;
		holder.cellSize = bucketSizes[i];
		printf("initialized holder #%d [%d bytes]\n", i, holder.cellSize);
		void* holderPtr = (void*)(BucketHolderMemory + i);
		memcpy(holderPtr, &holder, sizeof(holder));
	}

	for (size_t i = 0; i < INITIAL_BUCKETS; i++) {
		BucketHolder holder;
		holder.cellSize = bucketSizes[i];
		printf("initialized holder #%d [%d bytes]\n", i, holder.cellSize);
		void* holderPtr = (void*)(BucketHolderMemory + i);
		memcpy(holderPtr, &holder, sizeof(holder));
	}
}

void* mylloc(size_t size) {
	if (MasterMemory == NULL) {
		initialize();
	}

	(void)size;
	return NULL;
}
