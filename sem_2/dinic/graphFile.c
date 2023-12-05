
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "graph.h"
#include "graphFile.h"

Node* getNode(htTable* graphs, int id) {
	Node* node = htGet(graphs, &id, sizeof(id));

	if (node == NULL) {
		node = makeNode();
		node->id = id;
		htInsert(graphs, &id, sizeof(id), node);
	}

	return node;
}

char parseFile(FILE* what, htTable* graphs) {
	bool gotEOF = false;

	while (!gotEOF) {
		int g1 = 0; int g2 = 0;
		double weight = 0;

		int readAmt = fscanf(what, "%d %d %lf", &g1, &g2, &weight);

		if (readAmt != 3 && readAmt != EOF) {
			return 0; // not supposed to happen
		}

		if (readAmt == EOF) {
			break;
		}

		Node* curNode = getNode(graphs, g1);
		Node* toNode = getNode(graphs, g2);	// we don't need this node but
									// we need to make sure it exists
		connectNode(curNode, toNode, weight);
	}

	return 1;
}
