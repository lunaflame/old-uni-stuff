#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "graph.h"
#include "graphFile.h"
#include "dinic.h"

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

	char parsedOk = parseFile(fp, graphs);
	fclose(fp);

	if (!parsedOk) {
		printf("Error while parsing file.\n");
		htFree(graphs);
		return 1;
	}

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

	printf("Finding...\n");
	double maxFlow = doTheDinic(fromNode, toNode);
	printf("feenished: %lf\n", maxFlow);

	htFree(graphs);
	return 0;
}
