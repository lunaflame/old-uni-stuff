#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

#define MAX_NAME_LEN 512
#define MAX_NUM_LEN 128

struct studentInfo {
	char* name;
	unsigned int weight;
	unsigned int height;
};

// returns -1 with EOF
char readTillSpace(FILE* in, char* buf, size_t len, size_t* lenOut) {
	size_t i = 0;

	for (; i < len; i++) {
		int curChar = fgetc(in);
		if (EOF == curChar) {
			return -1;
		}

		if (' ' == curChar || '\n' == curChar) {
			break;
		}

		buf[i] = curChar;
	}

	if (lenOut != NULL) {
		*lenOut = i;
	}

	return 0;
}

/*char popTrailingSymbols(FILE* in) {
	// get that GARBAGE off my file
	while (1) {
		int curChar = fgetc(in);
		printf("popTrailingSymbols: read: %c (%d)\n", curChar, curChar);
		if (curChar == EOF) {
			return 1;
		}

		if (curChar == '\n') {
			return 0;
		}
	}
}*/

// -1 - unexpected EOF
// 0 - read a student and there's more
// 1 - read a student and there's no more

char readStudent(FILE* fp, struct studentInfo** where) {
	char bufName[MAX_NAME_LEN + 1] = {0};
	char bufNum[MAX_NUM_LEN + 1] = {0};

	char status = 0;

	size_t nameLen = 0;
	status = readTillSpace(fp, bufName, MAX_NAME_LEN, &nameLen);

	if (status != 0) {
		return -1; // reee
	}

	size_t numLen = 0;
	status = readTillSpace(fp, bufNum, MAX_NUM_LEN, &numLen);

	if (status != 0) {
		return -1; // reee
	}

	int height = atoi(bufNum);

	for (size_t i = 0; i < numLen; i++) {
		bufNum[i] = 0;
	}

	status = readTillSpace(fp, bufNum, MAX_NUM_LEN, NULL);
	int weight = atoi(bufNum);

	struct studentInfo* newStudent = malloc(sizeof(struct studentInfo));
	{
		newStudent->name = malloc(nameLen + 1);
		memcpy(newStudent->name, bufName, nameLen);
		newStudent->name[nameLen] = '\0';

		newStudent->height = height;
		newStudent->weight = weight;
	}

	*where = newStudent;
	if (status != 0) {
		// EOF
		return 1;
	} else {
		return 0;
	}
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Supply a file to open as an argument.\n");
		return -1;
	}

	FILE* in = fopen(argv[1], "r");

	if (NULL == in) {
		printf("Failed to open input file.\n");
		return -1;
	}

	htTable_t* ht = newHashtable(4);
	if (NULL == ht) {
		printf("Failed to initialize hashtable\n");
		return 1;
	}

	while (1) {
		struct studentInfo* studptr;
		char status = readStudent(in, &studptr);

		if (status < 0) {
			printf("Encountered an error while reading a student; stopping read.\n");
			break;
		}

		printf("Inserted student: %s\n", studptr->name);
		htInsert(ht, studptr->name, strlen(studptr->name), studptr);
		if (status != 0) {
			break;
		}
	}


	char input[MAX_NAME_LEN + 1] = {0};
	char timeToYeet = 0;

	while (!timeToYeet) {
		printf("== Input a student name or 'q' to quit... ==\n");

		for (int i = 0; i < MAX_NAME_LEN; i++) {
			int curChar = fgetc(stdin);
			if (curChar == EOF || curChar == '\n') {
				if (input[0] == 'q' && i == 1) {
					printf("Quitting!\n");
					timeToYeet = 1;
					break;
				}
				input[i] = 0;
				printf("Searching for %s...\n", input);
				struct studentInfo* studptr = (struct studentInfo*)htGet(ht, input, i);
				if (NULL == studptr) {
					printf("Student not found!\n");
				} else {
					printf("Student found!\n	Name: %s\n	Height: %d\n	Weight: %d\n",
						studptr->name, studptr->height, studptr->weight);
				}
				break;
			}

			input[i] = curChar;
		}
	}

	htFree(ht);
	printf("Done\n");
}