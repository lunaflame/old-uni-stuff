#define MAX 16

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "../util/idk.h"
#include "graph.h"
#include "graphFile.h"

Node* checkOdd(DynArray* nodes) {
	size_t odds = 0;
	Node* startNode = NULL;

	for (size_t i = 0; i < daSize(nodes); i++) {
		Node* node = (Node*)daGet(nodes, i);

		size_t links = daSize(node->links);
		if (links % 2 == 1) {
			odds++;
			startNode = node;
		}

		if (startNode == NULL) {
			startNode = node;
		}
	}

	if (odds != 0 && odds != 2) {
		return (Node*)NULL;
	} else {
		return startNode;
	}
}

char isEuler(DynArray* nodes) {
	Node* start = checkOdd(nodes);

	if (start == NULL) {
		printf("Odd check failed.\n");
		return 0;
	}

	DynArray* cpath = daNew(32, NULL);
	DynArray* epath = daNew(32, NULL);

	daPush(cpath, start);

	Node* cur = daGet(cpath, daSize(cpath) - 1);

	while (cur != NULL) {

		if (daSize(cur->links) > 0) {
			for (size_t i = daSize(cur->links); i > 0; i--) {
				Edge* edge = daGet(cur->links, i-1);

				Node* other = edge->to == cur ? edge->from : edge->to;
				daPush(cpath, other);
				removeEdge(edge);
			}
		} else {
			daPush(epath, cur);
			cur = daPop(cpath);
		}
	}

	for (size_t i = 0; i < daSize(epath); i++) {
		Node* nd = daGet(epath, i);
		printf("%d", nd->id);
		if (i != daSize(epath) - 1) {
			printf(" -> ");
		}
	}
	printf("\n");

	daFree(cpath);
	daFree(epath);

	return 1;
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("No file supplied.\n");
		return 1;
	}

	printf("Opening file: %s\n", argv[1]);

	FILE* fp = fopen(argv[1], "r");

	if (fp == NULL) {
		printf("Failed to open file.\n");
		return 1;
	}

	htTable* graphs = newHashtable(64, freeNode);
	assert(graphs != NULL);

	DynArray* allNodes = daNew(64, NULL);

	char parsedOk = parseFile(fp, graphs, allNodes);
	fclose(fp);

	if (!parsedOk) {
		printf("Error while parsing file.\n");
		htFree(graphs);
		return 1;
	}

	/*
	int from = -1;
	int to = -1;

	printf("Enter nodes to find max flow between: ");
	int amt = scanf("%d %d", &from, &to);
	if (amt != 2) {
		printf("Failed to read one of the nodes.\n");
		htFree(graphs);
		return 0;
	}

	Node* fromNode = htGet(graphs, &from, sizeof(from));
	Node* toNode = htGet(graphs, &to, sizeof(to));

	if (!fromNode) {
		printf("Node #%d [from] does not exist.\n", from);
		htFree(graphs);
		return 0;
	}

	if (!toNode) {
		printf("Node #%d [to] does not exist.\n", to);
		htFree(graphs);
		return 0;
	}
	*/

	char path = isEuler(allNodes);

	htFree(graphs);
	daFree(allNodes);

	if (!path) {
		printf("No path.");
	}

	return 0;
}
