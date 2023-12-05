#ifndef Graph_IG
#define Graph_IG

//#include "hashtable.h"
#include "../util/idk.h"

struct node_t {
	// links
	DynArray* links;	// array of pointers to all the links

	int id;
	htTable* weights;
};

typedef struct node_t Node;

Node* makeNode();
void connectNode(Node* what, Node* to, double weight);
void freeNode(void* what);

#endif