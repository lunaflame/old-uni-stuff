#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define START_LINE_SIZE 1 << 4
#define GROW_RATIO 2 // pls >1

typedef struct node {
	struct node* next;
	char* value;
} Node;

typedef struct string {
	char* ptr;
	size_t sz;
} String;

void freeString(String* s) {
	free(s->ptr);
	free(s);
}

Node* newNode() {
	Node* ret = (Node*)malloc(sizeof(Node));
	if (ret == NULL) {
		perror("Failed to allocate a new node!");
		return NULL;
	}

	ret->next = NULL;
	ret->value = NULL;

	return ret;
}

void freeNode(Node* nd) {
	if (nd->value != NULL) {
		free(nd->value);
	}

	free(nd);
}

void freeChain(Node* nd) {
	Node* cur = nd;

	while (cur != NULL) {
		// step 1. grab current node's next node
		Node* next = cur->next;

		// step 2. free current node's resources
		freeNode(cur);

		// step 3. do it again for the next node
		cur = next;
	}
}

// remove an \n from the end of the string if one is present
// returns the size of the string for reuse
void sanitizeString(String* ss) {
	int len = ss->sz;
	char* str = ss->ptr;

	if (str[len - 1] == '\n') {
		// SCREW YOU
		len--;
		str[len] = '\0';
	}
}

Node* addString(String* str) {
	Node* new = newNode();
	if (new == NULL) { return new; }

	sanitizeString(str);
	new->value = (char*)malloc( sizeof(char) * (str->sz + 1) ); // +1 for NULL at the end
	strcpy(new->value, str->ptr);

	return new;
}

void attach(Node* to, Node* what) {
	assert(to != NULL);
	assert(to->next == NULL);

	to->next = what;
}

String* readString(char** buf, size_t* sz, size_t off) {
	char* ret = fgets(*buf[off], *sz, stdin);
	if (ret == NULL) { break; }

	if (ret[0] == '.') {
		break;
	}

	int len = strlen(*buf);
	String* out;

	if (*buf[len - 1] != '\n') {
		printf("unfinished read? %s\n", *buf);

		char* newBuf = (char*)realloc(*buf, *sz * GROW_RATIO);

		if (newBuf == NULL) {
			perror("Failed to reallocate buffer to read longer string, exiting.");
			return NULL;
		}

		*sz *= GROW_RATIO;
		*buf = newBuf;
		// RECURSION! your stack is probably safe though
		return readString(buf, sz, off + len);
	}

	out = malloc(sizeof(String));
	out->sz = off + len;
	out->ptr = strdup(*buf);

	return out;
}

int main() {
	char* curLine = (char*)malloc(sizeof(char) * START_LINE_SIZE);
	size_t curLineSize = START_LINE_SIZE;

	Node* root = NULL; // the first node (to start printing from)
	Node* head = NULL; // the last node (to attach new ones to)

	while (1) {
		// fgets returns a NULL if nothing was read or an error was encountered
		String* str = readString(&curLine, &curLineSize, 0)

		if (str == NULL) {
			break;
		}

		printf("xdeee\n");

		// ? man says fgets can return NULL if something went wrong
		// so let's handle it i guess????

		if (root == NULL) {
			// no node yet; make one and slap a string to it
			root = addString(ret);
			head = root;

			if (root == NULL) {
				perror("Failed to allocate root. Somehow. Exiting...");
				free(curLine);
				return -1;
			}
		} else {
			// we have a head already; make a new node and attach a new head to it
			Node* newHead = addString(ret);

			if (newHead == NULL) {
				perror("Failed to allocate new string. Finishing input...");
				break;
			}
			attach(head, newHead);
			head = newHead;
		}
	}

	printf("-- Finished input. Reading them out...\n");
	Node* cursor = root;
	int i = 0;
	while (cursor != NULL) {
		assert(cursor->value != NULL); // not supposed to happen

		i++;
		printf("	%d: %-20s [%lu chars]\n", i, cursor->value, strlen(cursor->value));
		cursor = cursor->next;
	}

	printf("-- End.\n");
	freeChain(root);
}