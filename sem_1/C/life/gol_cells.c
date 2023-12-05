#include "gol_cells.h"
#include "gol_scream.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define GEN_ALIVE_CHANCE 20		// in percent (0-100)
#define STRING_GROUP 1000

char DEAD_CELL = ' ';	// if you find a way to print unicode tell me ty
char ALIVE_CELL = 'O';

struct cellPos {
	int x;
	int y;
};

char initField(struct gameField** where, int w, int h) {
	struct gameField* g_Field = malloc(sizeof(struct gameField));

	g_Field->field = calloc(w * h, sizeof(g_Field->field));
	g_Field->fieldCopy = calloc(w * h, sizeof(g_Field->field));

	memset(g_Field->neighborsLive, false, sizeof(g_Field->neighborsLive));
		g_Field->neighborsLive[2] = 1;
		g_Field->neighborsLive[3] = 1;

	memset(g_Field->neighborsReproduce, false, sizeof(g_Field->neighborsLive));
		g_Field->neighborsReproduce[3] = 1;

	if (NULL == g_Field->field) {
		GoL_Error(	"Failed to allocate memory for the field. Now is a good time to die,\n"
				"but we're not even allowed to die without permission.\n");
		return 1;
	} else if (NULL == g_Field->fieldCopy) {
		GoL_Error(	"Failed to allocate memory for the field copy. Now is a good time to die,\n"
				"but we're not even allowed to die without permission.\n");
		return 1;
	}

	g_Field->width = w;
	g_Field->height = h;

	g_Field->totalSize = w * h;

	*where = g_Field;

	return 0;
}

int mod(int a, int b) {
	int r = a % b;
	return r < 0 ? r + abs(b) : r;
}

int PostoIndex(struct gameField g_Field, struct cellPos pos) {
	return (pos.y * g_Field.width) + pos.x;
}

void DestroyField(struct gameField* g_Field) {
	free(g_Field->field);
	free(g_Field->fieldCopy);

	free(g_Field); // bye bye monkey
}

void WrapPos(struct gameField g_Field, struct cellPos* pos) {
	pos->x = mod(pos->x, g_Field.width);
	pos->y = mod(pos->y, g_Field.height);
}

struct cellPos IndexToPos(struct gameField g_Field, int ind) {
	struct cellPos ret;
	ret.x = ind % g_Field.width;
	ret.y = ind / g_Field.width;

	return ret;
}

void generateRandomField(struct gameField* fieldptr) {
	struct gameField g_Field = *fieldptr;
	int chance = GEN_ALIVE_CHANCE * (RAND_MAX / 100);

	for (unsigned int i = 0; i < g_Field.width * g_Field.height; i++) {
		int rnd = rand();
		g_Field.field[i] = rnd < chance;
		g_Field.fieldCopy[i] = g_Field.field[i];
	}
}



void printField(struct gameField* fieldptr) {
	struct gameField g_Field = *fieldptr;

	char strings[1001];
	int lastY = 0;
	int shifts = 0;

	for (unsigned int coord = 0; coord < g_Field.totalSize; coord++) {
		bool alive = g_Field.field[coord];

		struct cellPos position = IndexToPos(g_Field, coord);

		int y = position.y;
		int charPos = (coord % STRING_GROUP) + shifts; 	// + 1 due to \n

		if (y > lastY) {
			strings[charPos] = '\n';
			lastY = y;
			charPos++;
			shifts++;
		}

		if (alive) {
			strings[charPos] = ALIVE_CELL;
		} else {
			strings[charPos] = DEAD_CELL;
		}

		if (coord % STRING_GROUP == (STRING_GROUP - 1)) {
			strings[STRING_GROUP] = '\0';
			shifts = 0;
			printf("%s", strings);
		}
	}

	strings[g_Field.totalSize % STRING_GROUP + shifts] = '\0';
	printf("%s\n", strings);
}


// https://i.imgur.com/7KxsUxt.png

void cellLogic(struct gameField* fieldptr, int ind) {
	struct gameField g_Field = *fieldptr;

	bool isCellAlive = g_Field.field[ind];

	struct cellPos curCellPos = IndexToPos(g_Field, ind);

	struct cellPos oneUp;
		oneUp.x = curCellPos.x - 1;
		oneUp.y = curCellPos.y - 1;
		WrapPos(g_Field, &oneUp);

	struct cellPos oneDown;
		oneDown.x = curCellPos.x - 1;
		oneDown.y = curCellPos.y + 1;
		WrapPos(g_Field, &oneDown);

	struct cellPos curRow;
		curRow.x = curCellPos.x - 1;
		curRow.y = curCellPos.y;
		WrapPos(g_Field, &curRow);

	int alive = 0;
	int i;

	// top row
	for (i = 0; i < 3; i++) {
		int oneUpInd = PostoIndex(g_Field, oneUp);

		if (g_Field.field[oneUpInd]) {
			alive++;
		}

		oneUp.x++;
		WrapPos(g_Field, &oneUp);
	}

	// current row (jump over the current cell)
	for (i = 0; i < 3; i += 2) {
		int curInd = PostoIndex(g_Field, curRow);

		if (g_Field.field[curInd]) {
			alive++;
		}

		curRow.x += 2; // 2 steps
		WrapPos(g_Field, &curRow);
	}

	// bottom row
	for (i = 0; i < 3; i++) {
		int oneDownInd = PostoIndex(g_Field, oneDown);

		if (g_Field.field[oneDownInd]) {
			alive++;
		}

		oneDown.x++;
		WrapPos(g_Field, &oneDown);
	}

	if (!isCellAlive) {
		g_Field.fieldCopy[ind] = !!g_Field.neighborsReproduce[alive];
	} else {
		g_Field.fieldCopy[ind] = !!g_Field.neighborsLive[alive];
	}

}


void simulateField(struct gameField* fieldptr) {
	struct gameField g_Field = *fieldptr;
	for (unsigned int i = 0; i < g_Field.width * g_Field.height; i++) {
		cellLogic(fieldptr, i);
	}

	/*
		// this does not work
		void* temp = g_Field.field;
		g_Field.field = g_Field.fieldCopy;
		g_Field.fieldCopy = temp;
	*/

	memcpy(g_Field.field, g_Field.fieldCopy, g_Field.width * g_Field.height);
}