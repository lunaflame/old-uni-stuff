#include <stdio.h>
#include "cache.h"
#include <stdlib.h>
#include <string.h>


#define CONSOLE_BUSTED 1

void printCache(Cache* cache) {
	DLL* cur = cache->firstNode;
	for (size_t i=0; i < cache->sz; i++) {

		printf("    Node #%zu: ", i+1);
		if (cur->data == NULL) {
			printf("[empty]");
		} else {
			printf("[%d] = *(%p)", cur->key, cur->data);
		}
		printf("\n");

		cur = cur->next;
		if (cur == NULL) {
			break;
		}
	}
}


// SO ftw
unsigned numDigits(const unsigned n) {
	if (n < 10) return 1;
	return 1 + numDigits(n / 10);
}

void showOptions() {
	printf("1: Put a key/value into the cache\n"
		   "2: Get a value using the key\n"
		   "3: Print out the cache\n"
		   "9: Exit\n");
}

void readKV(Cache* cache) {
	int key = 0;


	if (!CONSOLE_BUSTED) {
		printf(" -> [");
		int read = scanf("%d", &key);
		if (read == 0) {
			printf("failed to read key!\n");
			return;
		}

		printf("\033[A\33[2K\r"); // erases the current line; seems to work on linux?

		printf(" -> [%d] ", key);
		fflush(stdin);
	} else {
		printf(" key: ");
		int read = scanf("%d", &key);
		if (read == 0) {
			printf("failed to read key!\n");
			return;
		}
	}

	char* val = calloc(65536, sizeof(char));

	if (!CONSOLE_BUSTED) {
		printf("= ");
	} else {
		printf(" value: ");
	}

	scanf("%65535s", val);

	size_t len = strlen(val);

	char* newVal = realloc(val, len + 1);
	newVal[len] = '\0';

	printf(" [%d] = \"%s\"\n", key, newVal);
	setCache(cache, key, newVal);
}

void readCacheK(Cache* cache) {
	int key = 0;

	if (!CONSOLE_BUSTED) {
		printf(" -> [");
		int read = scanf("%d", &key);
		if (read == 0) {
			printf("failed to read key!\n");
			return;
		}

		printf("\033[A\33[2K\r"); // erases the current line; seems to work on linux?

		printf(" -> [%d] = ", key);
		fflush(stdin);
	} else {
		printf(" key: ");
		int read = scanf("%d", &key);
		if (read == 0) {
			printf("failed to read key!\n");
			return;
		}
	}

	char* val = getCache(cache, key);

	if (val == NULL) {

		if (!CONSOLE_BUSTED) {
			printf("NULL\n");
		} else {
			printf("No value was associated with key %d.\n", key);
		}

	} else {

		if (!CONSOLE_BUSTED) {
			printf("%s\n", val);
		} else {
			printf("`%d` = `%s`\n", key, val);
		}

	}

}

void flushInput() {
	// c is cringe
	char chr = getchar();
	while (chr != '\n' && chr != EOF) { chr = getchar(); }
}

char readOption(Cache* cache) {
	char validOption = 0;
	char option;

	readAgain:
	printf("__________________\n");
	option = 0;

	while (!validOption) {
		showOptions();
		printf("\nYour choice: ");
		scanf("%c", &option);

		if ( (option > '0' && option < '4') || option == '9' ) {
			printf("\n");
			flushInput();
			break;
		} else {
			printf("Unrecognized option: '%c'\n", option);
			flushInput();
		}
	}

	switch (option - '0') {
		case 1:
			readKV(cache);
			flushInput();
			goto readAgain;

		case 2:
			readCacheK(cache);
			flushInput();
			goto readAgain;

		case 3:
			printCache(cache);
			//flushInput();
			goto readAgain;

		case 9:
			return 1;

		default:
			printf("unrecognized option? not supposed to happen!\n");
	}

	return 1;
}


int main() {
	size_t cacheSize = 0;
	printf("Input cache size: ");

	int amt = scanf("%lu", &cacheSize);
	flushInput();

	if (amt == 0) {
		printf("\nFailed to read cache size; defaulting to 5.\n");
		cacheSize = 5;
	}

	if (cacheSize < 2) {
		printf("\na bit small, don't you think? defualting to 2.\n");
		cacheSize = 2;
	}

	Cache* cache = newCache(cacheSize, free);
	readOption(cache);
	freeCache(cache);
}