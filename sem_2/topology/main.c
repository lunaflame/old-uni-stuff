#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define NODE_NUM 9
#define MAKE_CYCLIC 0	// set to 1 to make the graph cyclic (for testing purposes)


struct dfsState {
	char mtrx[NODE_NUM][NODE_NUM];	// node connections matrix: ` [node1][node2] = (char)has_connection `

	char* excluded;	// "persistent" markers	, used for not re-visiting nodes visited by previous DFS'es
	char* chain;	// "temporary" markers	, resets on every new depth visit; used for finding loops

	int* out;		// output array, starts from deepest node
	int* outLen;	// current output array len
};

// yay dfs

/*
	return:
		0: OK
		1: graph is cyclic
*/

char dfs(struct dfsState* state, int col) {
	if (state->excluded[col]) { // don't revisit node if previous DFS'es already did
		return 0;
	}

	if (state->chain[col]) {	// if we already visited this node in _this_ instance of DFS, then it's a cyclic loop
		return 1;
	}

	state->chain[col] = 1;

	for (int i = 0; i < NODE_NUM; i++) {
		// if this node isn't already added and it connects to node 'i',

		if (!state->excluded[i] && state->mtrx[col][i]) {
			// check that one out first
			char pleaseStopHesAlreadyDead = dfs(state, i);
			if (pleaseStopHesAlreadyDead) {
				return 1;
			}
		}
		
	}

	state->chain[col] = 0;
	state->excluded[col] = 1;
	// and then add ourselves to 'out'
	int* lenPtr = state->outLen;
	state->out[*lenPtr] = col + 1;
	(*lenPtr)++;

	return 0;
	// the "last" nodes (so the ones to which everything connects to) will be the first in out
	// the "first" nodes (from which there are no connections) will be the last
}

// check once so we don't drain resources on recursion
char dfs_check(struct dfsState* state, int col) {
	assert(state->mtrx 		!= NULL);
	assert(state->excluded 	!= NULL);
	assert(state->chain 	!= NULL);
	assert(state->out 		!= NULL);
	assert(state->outLen 	!= NULL);

	return dfs(state, col);
}

int main() {
	const char mtrx[NODE_NUM][NODE_NUM] = {
		{ 0, 1, 0, 0, 0, 1, 0, 0, MAKE_CYCLIC},	// [0]
		{ 0, 0, 0, 0, 1, 0, 0, 0, 0},	// [1]
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0},	// ...
		{ 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 1, 0, 0, 0, 0, 0},
		{ 0, 0, 0, 1, 0, 0, 0, 0, 1},
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 1, 0, 0, 0, 0, 0, 0, 0, 0},
		{ 0, 0, 1, 0, 0, 0, 0, 0, 0},	// [8]
	};



	struct dfsState state;
	{
		char excluded[NODE_NUM] = {0};
		char chain[NODE_NUM] = {0};

		memcpy(&state.mtrx, &mtrx, sizeof(mtrx));
		state.excluded = excluded;
		state.chain = chain;
		state.out = out;
		state.outLen = &outLen;
	}

	int out[NODE_NUM] = {0};
	int outLen = 0;

	for (int col = 0; col < NODE_NUM; col++) {
		if (!excluded[col]) {
			if (dfs_check(&state, col)) {
				printf("Node is cyclic; halting and exiting.\n");

				printf("(loop with ");

				for (int i=0; i < NODE_NUM; i++) {
					if (state.chain[i]) {
						printf("%d", i+1);
						if (i < NODE_NUM - 1) {
							printf(", ");
						}
					}
				}

				printf(")\n");
				return 0;
			}
		}
	}

	// reverse iterate
	for (int i=outLen - 1; i >= 0; i--) {
		printf("%d", out[i]);
		if (i > 0) {
			printf(" -> ");
		}
	}
}
