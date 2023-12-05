#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "introsort.h"

#define MAX_BUF_SIZE (size_t)( 90 * 1024 * 1024 )

#define MIN_LINE_SIZE 1 	// if you can guarantee a minimal line size, set it to that, otherwise leave it at 1
#define MAX_LINE_SIZE 121

char isSpace(int in) {
	return in == ' ' || in == '	';
}

struct charLine {
	size_t size;
	char* ptr;
};


static char compLineFunc(const void* a, const void* b) {
	struct charLine** lp1 = (struct charLine**)a;
	struct charLine** lp2 = (struct charLine**)b;

	struct charLine* l1 = *lp1;
	struct charLine* l2 = *lp2;

	if (l1->size != l2->size) {
		return l1->size > l2->size;
	}

	return strncmp(l1->ptr, l2->ptr, l1->size) <= 0; //a->size < b->size;
}

static struct charLine* makeNewLine(size_t mallocSize) {
	struct charLine* ret = malloc(sizeof(struct charLine));
		ret->size = 0;
		if (mallocSize != 0) {
			ret->ptr = calloc(mallocSize, 1);
		}

	return ret;
}

static struct charLine* readCharLine(FILE* file, char* isLastPtr) {	// if isLastPtr is provided, it'll be overwritten with 0 meaning it's not the last line,
	struct charLine* ret = calloc(sizeof(struct charLine), 1);		// or 1, meaning it's the last line
	char buf[MAX_LINE_SIZE];

	for (size_t i = 0; i < MAX_BUF_SIZE; i++) {
		int curChar = fgetc(file);

		if (curChar == EOF || isSpace(curChar)) {
			ret->size = i;
			ret->ptr = calloc(i, 1);
			memcpy(ret->ptr, &buf, i);

			if (NULL != isLastPtr) {
				*isLastPtr = (char)(curChar == EOF);
			}

			return ret;
		}

		buf[i] = curChar;
	}

	return NULL; // shut up gcc
}

static void delLine(struct charLine* line) {
	free(line->ptr);
	free(line);
}


static void numToTemp(char** bufPtr, int num) {
	char* buf = calloc(32, sizeof(char));
	char fnStart[] = "temp_";

	char numChar[11] = {0}; // 2^32 is 10 digits + \0
	sprintf(numChar, "%d", num);

	char fnEnd[] = ".txt";

	strcat(buf, fnStart);
	strcat(buf, numChar);
	strcat(buf, fnEnd);
	buf[31] = '\0';

	*bufPtr = buf; // just don't forget to free it
}

/*
static void printLine(struct charLine line) {
	for (size_t i=0; i < line.size; i++) {
		printf("%c", line.ptr[i]);
	}
	printf("\n");
}
*/

static char flushLines(FILE* out, struct charLine** lines, int lineCounter) {

	for (int lineNum = 0; lineNum < lineCounter; lineNum++) {
		struct charLine* line = lines[lineNum];

		char* charBuf = line->ptr;
		for (size_t charNum = 0; charNum < line->size; charNum++) {
			fputc(charBuf[charNum], out);
		}

		if (lineNum != lineCounter - 1) { // avoid a trailing space
			fputc(' ', out);
		}

		delLine(line);
	}

	return 1;
}

// int division with rounding up
int mydiv(int a, int b) {
	return (a + (b + 1)) / b;
}

static char sortIntoTemp(FILE* in, FILE* out, size_t fileSize, FILE** tempFiles, int* tempFilesNumPtr) {

	size_t allocSize = fileSize;

	if (allocSize > MAX_BUF_SIZE) {
		allocSize = MAX_BUF_SIZE;
	}

									// size_t + char* + char[MAX_LINE_SIZE]
	size_t lineMaximumMemorySize = sizeof(struct charLine) + MAX_LINE_SIZE;

	// amount of lines we can hold before we overflow the given memory
	size_t maxHoldLines = allocSize / (lineMaximumMemorySize + sizeof(struct charLine*));

	// buffer of pointers to line structs
	struct charLine** lineBuf = calloc(maxHoldLines, sizeof(struct charLine*));

	int totalLineCounter = 0;

	if (NULL == lineBuf) {
		printf("Failed to allocate memory for the line buffer...??? (%d bytes)\n", allocSize);
		return -1;
	}

	int loops = mydiv(fileSize, MAX_BUF_SIZE / lineMaximumMemorySize);

	int tempFilesNum = 0;

	// if there's a new loop while we were reading the file,
	// back up the line so we can continue filling it after
	// the loop ends

	struct charLine* curLine = makeNewLine(MAX_LINE_SIZE);
	char ranOut = 0; // when we reach the input EOF, this becomes true
	char charBuffer[MAX_LINE_SIZE] = {0};

	int i = 0;

	while (!ranOut) {
		size_t readChars;
		size_t lineCounter = 0;

		// read new chars from the file into the buf
		// cant really use readCharLine because we need to differentiate between EOF and a space separator

		for (readChars = 0; readChars < MAX_BUF_SIZE; readChars++) {
			int fchar = fgetc(in);

			if (fchar == EOF) {
				// create the next line object for the next loop

				if (curLine->size > 0) {
					struct charLine* newLine = makeNewLine(0);

					lineBuf[lineCounter] = curLine;
					curLine->ptr = malloc(curLine->size);
					memcpy(curLine->ptr, &charBuffer, curLine->size);

					curLine = newLine;

					lineCounter++;
					totalLineCounter++;
				}
				ranOut = 1;
				break;
			}

			if (isSpace(fchar)) {
				if (curLine->size == 0) {
					continue;
				} // oh goddamnit

				// create the next line object
				struct charLine* newLine = makeNewLine(0);

				// put the current one in the buffer
				lineBuf[lineCounter] = curLine;
				curLine->ptr = malloc(curLine->size);
				memcpy(curLine->ptr, &charBuffer, curLine->size);
				// replace the currently active line with this new one
				curLine = newLine;
				lineCounter++;
				totalLineCounter++;

				if (lineCounter > maxHoldLines) {
					break;
				}
			} else {

				if (curLine->size > MAX_LINE_SIZE + 1) {
					// something went wrong; line size is overflowing
					// stop the loop, free all memory and return an error
					printf("Line size is %lld (bigger than set max. line size: %lld), halting.\n",
							curLine->size, MAX_LINE_SIZE);
					for (size_t i2=0; i2 < lineCounter; i++) {
						delLine(lineBuf[i2]);
					}
					delLine(curLine);
					free(lineBuf);
					return 1;
				}

				charBuffer[curLine->size] = (char)fchar;
				//curLine->ptr[curLine->size] = (char)fchar;
				curLine->size++;
			}

		}

		introsort((char*)lineBuf, (long long int)lineCounter, sizeof(struct charLine*), compLineFunc);

		if (loops == 1) {
			// flush all lines
			// (flushLines also frees the lines themselves)
			flushLines(out, lineBuf, lineCounter);

			// cleanup
			free(lineBuf);
			delLine(curLine);
			fclose(out);

			// ret. code 1 == "don't proceed"
			return 1;	// the file was small enough that we didn't have to bother with temp files;
						// just write to output and bail
		} else {
			// string handling in C is awful

			char* fn;
			numToTemp(&fn, i);

			FILE* tempOut = fopen(fn, "w+");
			if (NULL == tempOut) {
				printf("Failed to create file '%s'!", fn);
				return -1;
			}

			free(fn);	// that one's malloced

			tempFiles[i] = tempOut;
			tempFilesNum++;

			flushLines(tempOut, lineBuf, lineCounter);

			rewind(tempOut);	// reset ptr back to 0 so we start reading from the bottom
		}
		i++;
	}

	*tempFilesNumPtr = tempFilesNum;

	free(lineBuf);
	delLine(curLine);

	return 0;
}

static char readInOutArgs(int argc, char** argv, FILE** inPtr, FILE** outPtr) {
	if (argc < 2) {
		printf("Supply a file to open as an argument\n");
		return -1;
	}

	FILE* out = NULL;

	if (argc < 3) {
		printf("No output filename provided; using 'output.txt'.\n");
		out = fopen("output.txt", "w+");
	} else {
		printf("Using '%s' as the output file.\n", argv[2]);
		out = fopen(argv[2], "w+");
	}

	FILE* in = fopen(argv[1], "r");

	if (NULL == in) {
		printf("Failed to open input file.\n");
		return -1;
	}

	if (NULL == out) {
		printf("Failed to open output file.\n");
		return -1;
	}

	*inPtr = in;
	*outPtr = out;

	return 0;
}

static char sortIntoOutput(FILE* out, FILE** tempFileArray, int tempFilesNum, size_t fileSize) {
	struct charLine** charPile = calloc(tempFilesNum, sizeof(struct charLine*));

	if (NULL == charPile) {
		printf("Failed to create char-pile buffer...?????\n\n\nHOW\n");
		return -1;
	}

	for (int i=0; i < tempFilesNum; i++) {
		FILE* temp = tempFileArray[i];

		char isLastPtr = 0;
		struct charLine* fileLine = readCharLine(temp, &isLastPtr);

		charPile[i] = fileLine;

		if (isLastPtr) {
			// the file only had 1 line; NULL it so we don't read from it anymore
			fclose(tempFileArray[i]);
			tempFileArray[i] = (FILE*)NULL;
		}

	}

	for (size_t charNum = 0; charNum < fileSize; charNum++) {

		struct charLine* maxLine = NULL;
		int whatFile = -1;

		// find max. char from all temp files
		for (int i=0; i <= tempFilesNum; i++) {
			if (NULL == tempFileArray[i]) { // don't compare chars whose files already ended
				continue;
			}

			struct charLine* piled = charPile[i];

			// if we don't have a 'max' line then we set one automatically
			// otherwise, we compare
			if ( whatFile == -1 || compLineFunc(&piled, &maxLine) ) {
				maxLine = piled;
				whatFile = i;
			}
		}

		// there are no more files, we're done!
		if (whatFile == -1) {
			break;
		}

		// push the next line onto the comparing buffer
		if (NULL != tempFileArray[whatFile]) {
			char isLastPtr = 0;
			charPile[whatFile] = readCharLine(tempFileArray[whatFile], &isLastPtr);

			if (isLastPtr) {	// the file ended; release it and NULL it so we don't use it anymore
				fclose(tempFileArray[whatFile]);
				tempFileArray[whatFile] = (FILE*)NULL;
			}

		}

		if (charNum != 0) {
			fputc(' ', out);
		}

		// write the max. line contents into output
		for (size_t lineChar = 0; lineChar < maxLine->size; lineChar++) {
			fputc(maxLine->ptr[lineChar], out);
		}

		delLine(maxLine);
	}

	free(charPile);

	return 0;
}

static char sortInputToOutput(FILE* in, FILE* out) {
	// Read input file size
	size_t fileSize = 0;

	fseek(in, 0, SEEK_END);
	fileSize = ftell(in);

	rewind(in);


	// Write temporary files, according to the input file size

	FILE* tempFiles[1000] = {NULL}; 	// max. 1000 temp files, i dont think the kernel allows more
										// working around that limit is out of this lab's scope
	int tempFilesNum = 0;
	char status = sortIntoTemp(in, out, fileSize, tempFiles, &tempFilesNum);

	if (status != 0) {
		if (status < 0) {
			return status;
		} else {
			// positive status = we're done, time to bail
			return 0;
		}
	}

	// Read every temp file one by one, sort them between each other
	// and put them into output
	char outputstatus = sortIntoOutput(out, tempFiles, tempFilesNum, fileSize);

	for (int i=0; i < tempFilesNum; i++) {
		char* buf;
		numToTemp(&buf, i);

		if (NULL != tempFiles[i]) {
			fclose(tempFiles[i]);
		}

		int ok = remove(buf);
		free(buf);
		if (ok != 0) {
			printf("Something went wrong while deleting a temp file? Err. code: %d, "
					"filename: %s", ok, buf);
		}
	}

	if (outputstatus != 0) {
		return -0xF - outputstatus;
	}

	return 0;
}


int main(int argc, char** argv) {

	FILE* in = NULL;
	FILE* out = NULL;
	if (readInOutArgs(argc, argv, &in, &out) != 0) {
		printf("Failed to get I/O file handles, exiting.\n");
		return -1;
	}

	char status = sortInputToOutput(in, out);

	if (status != 0) {	// TODO: sensible error codes
		if (status > -0xF) {
			printf("Error while sorting data into temporary files.\n");
			return -1;
		} else {
			printf("Error while writing sorted data into output.\n");
			return -1;
		}
	} else {
		printf("Finished!\n");
	}

	return 0;
}