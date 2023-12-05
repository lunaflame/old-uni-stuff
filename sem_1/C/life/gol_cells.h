#ifndef GoL_Cells
#define GoL_Cells

#include <stdbool.h>

struct gameField {
	unsigned int width;
	unsigned int height;

	unsigned int totalSize;

	bool* field;
	bool* fieldCopy;

	bool initialized;

	bool neighborsLive[10];			// [numOfCells] = should_live
	bool neighborsReproduce[10];	// [numOfCells] = should_reproduce
};

char initField(struct gameField** where, int w, int h);
void simulateField(struct gameField*);
void printField(struct gameField*);
void generateRandomField(struct gameField*);
#endif