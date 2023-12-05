
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"
//#include "hashtable.h"
#include "graphFile.h"

Node* getNode(htTable* graphs, int id, DynArray* arr) {
	Node* node = htGet(graphs, &id, sizeof(id));

	if (node == NULL) {
		node = makeNode();
		node->id = id;
		htInsert(graphs, &id, sizeof(id), node);
		daPush(arr, node);
	}

	return node;
}

char parseFile(FILE* what, htTable* graphs, DynArray* arr) {
	bool gotEOF = false;

	while (!gotEOF) {
		int g1 = 0; int g2 = 0;

		int readAmt = fscanf(what, "%d %d", &g1, &g2);

		if (readAmt != 2 && readAmt != EOF) {
			return 0; // not supposed to happen
		}

		if (readAmt == EOF) {
			break;
		}

		Node* curNode = getNode(graphs, g1, arr);
		Node* toNode = getNode(graphs, g2, arr);	// we don't need this node but
											// we need to make sure it exists
		connectNode(curNode, toNode);
	}

	printf("done!\n");

	return 1;
}
