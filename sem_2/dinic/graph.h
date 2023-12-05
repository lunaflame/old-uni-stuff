#ifndef Graph_IG
#define Graph_IG

//#include "hashtable.h"
#include "../util/idk.h"

struct node_t {
	// links
	DynArray* links;	// array of pointers to all the links

	int id;
	int depth;	// for bfs
};

typedef struct node_t Node;

typedef struct edge_t {
	Node* from;
	Node* to;

	int flow;       // current flow
	int capacity;   // max flow
} Edge;

Node* makeNode();
void connectNode(Node* what, Node* to, double weight);
void freeNode(void* what);

#endif
