#ifndef Graph_IG
#define Graph_IG

//#include "hashtable.h"
#include "../util/idk.h"

struct node_t {
	// links
	DynArray* links;	// array of pointers to all the links
	int id;
};

typedef struct node_t Node;

typedef struct edge_t {
	Node* from;
	Node* to;

	char visited;
	unsigned char freeAttempts; // AAAAAAAAAA
} Edge;

Node* makeNode();
void connectNode(Node* what, Node* to);
void freeNode(void* what);
void removeEdge(Edge* what);

#endif
