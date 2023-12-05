#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define PLEASEWHY(OHGOD) #OHGOD
#define MTOSTR(WHY) PLEASEWHY(WHY)
#define START_CAPACITY 64
#define GROW_RATIO 2
#define READ_BUFFER 16 // read N chars at a time (+1 will be allocated on stack)
#define DEFAULT_FILE "test.txt" // if filename not provided, open this
#define MAX_PRINT 64 // i really dont want to realloc n stuff for output

#define LAB6

typedef struct lt_t {
	size_t* arr; // [line_idx] = line_sz;
	size_t top;
	size_t cap;
} LineTable;


LineTable* makeLineTable() {
	LineTable* t = (LineTable*)malloc(sizeof(LineTable));
	if (t == NULL) { return NULL; } // wtf!!!

	t->top = 0;
	t->cap = START_CAPACITY;
	t->arr = (size_t*)calloc(sizeof(size_t), START_CAPACITY);

	if (t->arr == NULL) {
		free(t); // wtf!!!!!!!!
		return NULL;
	}

	return t;
}

void freeLineTable(LineTable* lt) {
	free(lt->arr);
	free(lt);
}

// returns false on reallocation failure; true otherwise
bool ensureCapFor(LineTable* lt, size_t amt) {
	if (amt >= lt->cap) {
		lt->cap *= GROW_RATIO;
		if (lt->cap < amt) {
			lt->cap = amt;
		}

		size_t* new = realloc(lt->arr, sizeof(size_t) * lt->cap);

		if (new == NULL) {
			return false;
		}

		lt->arr = new;
	}

	return true;
}

// convenience function to make sure the next line will fit
bool ensureCap(LineTable* lt) {
	return ensureCapFor(lt, lt->top + 1);
}

// insert [idx] = len & make sure we can fit it
// keep in mind this is an array, not a hashtable;
// ridiculous indices will allocate ridiculous amounts of memory
bool insertLine(LineTable* lt, size_t idx, size_t len) {
	if (!ensureCapFor(lt, idx + 1)) { return false; }

	lt->arr[idx] = len;

	if (idx + 1 > lt->top) {
		lt->top = idx + 1;
	}

	return true;
}

// convenience function to push a line at the top
bool pushLine(LineTable* lt, size_t len) {
	return insertLine(lt, lt->top, len);
}

bool fillFromFile(LineTable* lt, int fd) {
	char buf[READ_BUFFER + 1];

	size_t cursor = 0;
	size_t curSz = 0;

	while (1) {
		int readAmt = read(fd, &buf, READ_BUFFER);
		if (readAmt == 0) { // EOF instead of newline?
			int left = cursor - curSz;
			if (left > 0) {
				pushLine(lt, cursor);
			}
			break;
		}

		if (readAmt == -1) {
			perror("Error while reading from file.");
			return false;
		}

		buf[readAmt] = '\0';

		char* wut = buf;
		curSz += readAmt;

		while ((wut = strchr(wut, '\n')) != NULL) {
			size_t offset = wut - buf; // basically @ what position in the string the newline is
			bool ok = pushLine(lt, cursor + offset);
			curSz = cursor + offset;
			if (!ok) {
				perror("Error while writing line offset. Failed allocation?...");
				return false;
			}
			wut++; // stop searching here
		}

		cursor += readAmt;
	}

	return true;
}

size_t readFromTable(LineTable* lt, int fd, size_t line,
	char* into, size_t readAmt) {

	// +1 to skip the newline of the previous line
	// hack? idk
	size_t seekTo = line == 0 ? 0 : lt->arr[line - 1] + 1;

	lseek(fd, seekTo, SEEK_SET);

	int toRead = lt->arr[line] - seekTo;

	if (toRead > readAmt) { toRead = readAmt; }

	int cur = read(fd, into, toRead);
	if (cur == -1) { return -1; }

	into[cur] = 0;
	return cur;
}

void printTrimmed(char* buf) {
	printf("`%." MTOSTR(MAX_PRINT) "s", buf);
	if (strlen(buf) == MAX_PRINT) {
		printf("...");
	}
	printf("`\n");
}

void timeoutDeath(int curFd) {
	if (curFd == -1) { return; }
	printf("Timeout; printing out file.\n");

	int cur;
	char into[READ_BUFFER + 1];

	lseek(curFd, 0, SEEK_SET);

	while ((cur = read(curFd, into, READ_BUFFER)) > 0) {
		into[cur] = 0;
		printf("%s", into);
	}
}

int main(int argc, char** argv) {
	char* fpath;
	if (argc < 2) {
		printf("No file provided, using default: `%s`\n", DEFAULT_FILE);
		fpath = DEFAULT_FILE;
	} else {
		fpath = argv[1];
		printf("Opening `%s`...\n", fpath);
	}

	int fd = open(fpath, 0);
	if (fd == -1) {
		perror("Error while opening file for reading, exiting");
		return -1;
	}

	LineTable* lt = makeLineTable();
	if (lt == NULL) {
		perror("Error while allocating line table, exiting");
		freeLineTable(lt);
		close(fd);
		return -1;
	}

	if (!fillFromFile(lt, fd)) {
		perror("Error while filling the line table, exiting");
		freeLineTable(lt);
		close(fd);
		return -1;
	}

	char buf[MAX_PRINT + 1];

	/*
	// Debug: print out the whole table

	lseek(fd, 0, SEEK_SET);

	for (int i = 0; i < lt->top; i++) {
		printf("%d: ", i);

		size_t ok = readFromTable(lt, fd, i, buf, MAX_PRINT);

		if (ok == -1) {
			printf("im a doubter sadge\n");
			return -1;
		}

		printTrimmed(buf);
	}
	*/

	// Start reading input

	char readBuf[120];
	char* endPtr; // strtol sets it to where it stopped trying


	while (1) {
		printf("Enter a number or `q` to quit.\n> ");

#ifdef LAB6
		fd_set rfds;
	    struct timeval tv;
	    int retval;

	    /* Watch stdin (fd 0) to see when it has input. */
	    FD_ZERO(&rfds);
	    FD_SET(0, &rfds);
	    /* Wait up to five seconds. */
	    tv.tv_sec = 5;
	    tv.tv_usec = 0;
	    retval = select(1, &rfds, NULL, NULL, &tv);

		if (retval == -1) // select gave us an error; handle it
	        perror("select()");
	    else if (!retval) { // timeout called
	    	timeoutDeath(fd);
	    	break;
	    }
	    else {
#endif
			char* ok = fgets(readBuf, 120, stdin);
			if (ok == NULL) {
				printf("Nothing was read from stdin; exiting.\n");
				break;
			}

			if (readBuf[0] == 'q') {
				printf("Exiting.\n");
				break;
			}

			long ln = strtol(readBuf, &endPtr, 10);

			if (endPtr == readBuf) {
				// cursor didn't move at all meaning we failed to convert to a number
				printf("That ain't a number, try again\n");
				continue;
			}

			if (ln <= 0) {
				printf("Yes, give me the %ldth line - statements dreamed up by the utterly deranged\n", ln);
				continue;
			}


			if (lt->top <= ln - 1) {
				printf("We don't have that many lines (we only have %ld)\n", lt->top);
			} else {
				size_t ok = readFromTable(lt, fd, ln - 1, buf, MAX_PRINT);

				if (ok == -1) {
					printf("i failed sadeg\n");
					return -1;
				}

				printf("Line #%ld: ", ln);
				printTrimmed(buf);
			}
#ifdef LAB6
		}
#endif
	}

	if (close(fd) != 0) {
		perror("how,,,,,,,, how he,,,,,,,,,,,,,,,,,");
		return -1;
	}

	freeLineTable(lt);
}