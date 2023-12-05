#include <assert.h>
#include <stdio.h>
#include <math.h>

#include "../util/idk.h"
#include "dinic.h"
#include "graph.h"

#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a, b) ( ((a) > (b)) ? (a) : (b) )

typedef struct graph_t {
	size_t sz;
	int to;

	int flow;       // current flow
	int capacity;   // max flow
} Graph;


// adds relevant nodes to queue
static char bfs(Queue* queue, htTable* visited, Node* curNode, Node* toNode) {

	char gotTo = 0;

	for (size_t i = 0; i < daSize(curNode->links); i++) {
		Edge* edge = daGet(curNode->links, i);

		Node* other = edge->to;

		void* already_visited = htGet(visited, &other->id, sizeof(other->id));
		if (already_visited != NULL) {
			continue;
		}

		char can_fit = edge->flow < edge->capacity;
		if (!can_fit) {
			continue;
		}

		htInsert(visited, &other->id, sizeof(other->id), (void*)1);
		qAdd(queue, other);

		if (other == toNode) {
			gotTo = 1;
		}

	}

	return gotTo;
}

static char canReachSink(Node* from, Node* to) {
	assert(from != NULL);
	assert(to != NULL);

	Queue* queue = newQueue(32);
	qAdd(queue, from);

	htTable* visited = newHashtable(32, NULL);
	htInsert(visited, &from->id, sizeof(from->id), (void*)1);

	size_t depth = 0;

	char sinkFound = 0;

	while (!qEmpty(queue)) {
		size_t depthSize = qSize(queue);

		for (size_t i = 0; i < depthSize; i++) {
			Node* qNode = qPop(queue);

			qNode->depth = depth;
			char gotTo = bfs(queue, visited, qNode, to);
			if (gotTo) {
				sinkFound = 1;
			}
		}

		depth++;
	}

	qFree(queue);
	htFree(visited);

	return sinkFound;
}

static double sendFlow(Node* from, Node* to, double curFlow) {
	if (from == to) {
		return curFlow; // ALL OF IT
	}

	for (size_t i = 0; i < daSize(from->links); i++) {
		Edge* edge = daGet(from->links, i);
		Node* conTo = edge->to;
		int levelDiff = conTo->depth - from->depth;

		// if we connect to 1 level deeper and we can fit more flow in there
		if (levelDiff == 1 && edge->flow < edge->capacity) {
			// fit as much flow as we can
			double addFlow = MIN(curFlow, edge->capacity - edge->flow);
			double limitedFlow = sendFlow(conTo, to, addFlow);

			if (limitedFlow > 0) { // we managed to fit some
				edge->flow += limitedFlow;
				// take flow from the reversed edge
				// because dinic? idk?
				Edge* revEdge = NULL;
				for (size_t i = 0; i < daSize(conTo->links); i++) { // have to find it tho
					Edge* edge2 = daGet(conTo->links, i);
					if (edge2->to == from) {
						revEdge = edge2;
						break;
					}
				}

				assert(revEdge != NULL);
				revEdge->flow -= limitedFlow;

				return limitedFlow;
			}
		}
	}

	return 0; // didnt fit anything ig
}

double doTheDinic(Node* from, Node* to) {

	double ret = 0;

	while (canReachSink(from, to)) {
		double flow = sendFlow(from, to, INFINITY);
		while (flow > 0) {
			ret += flow;
			flow = sendFlow(from, to, INFINITY);
		}
	}

	return ret;
}
