#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#define LINE_SIZE 1 << 4

typedef struct node {
	struct node* next;
	char* value;
} Node;

Node* newNode() {
	Node* ret = (Node*)malloc(sizeof(Node));
	if (ret == NULL) {
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
int sanitizeString(char* str) {
	int len = strlen(str);
	if (str[len - 1] == '\n') {
		// SCREW YOU
		len--;
		str[len] = '\0';
	}

	return len;
}

Node* addString(char* str) {
	Node* new = newNode();
	if (new == NULL) { return new; }

	int len = sanitizeString(str);
	new->value = (char*)malloc( sizeof(char) * (len + 1) ); // +1 for NULL at the end
	strcpy(new->value, str);

	return new;
}

void attach(Node* to, Node* what) {
	assert(to != NULL);
	assert(to->next == NULL);

	to->next = what;
}

int main() {
	char line[LINE_SIZE];

	Node* root = NULL; // the first node (to start printing from)
	Node* head = NULL; // the last node (to attach new ones to)

	while (1) {
		// fgets returns a NULL if nothing was read or an error was encountered
		char* ret = fgets(line, LINE_SIZE, stdin);

		if (ret == NULL) { break; }

		if (ret[0] == '.') {
			break;
		}

		if (root == NULL) {
			// no node yet; make one and slap a string to it
			root = addString(ret);
			head = root;

			if (root == NULL) {
				perror("Failed to allocate root. Somehow. Exiting...");
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
		printf("	%d: %-17s [%lu chars]\n", i, cursor->value, strlen(cursor->value));
		cursor = cursor->next;
	}

	printf("-- End.\n");
	freeChain(root);
}