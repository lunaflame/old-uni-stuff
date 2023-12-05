#include <stdio.h>
#include <string.h>

#define FIELD_W 16
#define FIELD_H 24

#define START_X 8
#define START_Y 8

#define FIELD_TYPE short int

static const char Xs[] = {2, 1,  2,  1, -2, -1, -2, -1};
static const char Ys[] = {1, 2, -1, -2, 1,  2,  -1,  -2};

typedef struct horse {
	int x;
	int y;

	int turns;
} Horse;

void insertionSort(char turns[8][2], int n)
{
	char turnsCnt;
	char key;

	for (int i = 1; i < n; i++) {
		int j = i - 1;

		turnsCnt = turns[i][0];
		key = turns[i][1];

		while (j >= 0 && turns[j][0] > turnsCnt) {
			memcpy(turns[j + 1], turns[j], sizeof(turns[1]));
			j = j - 1;
		}

		turns[j + 1][0] = turnsCnt;
		turns[j + 1][1] = key;
	}

}

unsigned char XRange(int x) {
	return x >= 0 && x < FIELD_W;
}

unsigned char YRange(int y) {
	return y >= 0 && y < FIELD_H;
}

unsigned char getTurns(FIELD_TYPE field[FIELD_W][FIELD_H], int x, int y) {
	char turns = 0;

	for (int i = 0; i < 8; i++) {
		int tryX = x + Xs[i];
		int tryY = y + Ys[i];

		if (XRange(tryX) && YRange(tryY) && !field[tryX][tryY]) {
			turns++;
		}
	}

	return turns;
}

char recur(FIELD_TYPE field[FIELD_W][FIELD_H], Horse horse) {
	// this assumes you set the horse's XY to new XY and
	// you already checked field[x][y]

	int curX = horse.x;
	int curY = horse.y;

	horse.turns++;
	field[curX][curY] = horse.turns;

	if (horse.turns == FIELD_W * FIELD_H) {
		return 1;
	}

	// fetch next XY to try

	char turnCounts[8][2] = {
		{9, 0},		// if 9 turns are detected then it
		{9, 0},		// won't attempt to go to that cell
		{9, 0},
		{9, 0},
		{9, 0},
		{9, 0},
		{9, 0},
		{9, 0}
	};

	for (int i = 0; i < 8; i++) {
		int tryX = curX + Xs[i];
		int tryY = curY + Ys[i];

		if (XRange(tryX) && YRange(tryY) && !field[tryX][tryY]) {
			unsigned char turns = getTurns(field, tryX, tryY);
			turnCounts[i][0] = turns;
			turnCounts[i][1] = i;
		}
	}

	insertionSort(turnCounts, 8);

	for (int i = 0; i < 8; i++) {
		if (turnCounts[i][0] != 9) {
			unsigned char key = turnCounts[i][1];

			// set new horse properties
			horse.x = curX + Xs[key];
			horse.y = curY + Ys[key];

			if (recur(field, horse)) {
				return 1;
			}
		}
	}

	return 0;
}

int main() {
	FIELD_TYPE field[FIELD_W][FIELD_H] = {{0}};

	Horse horse = {START_X, START_Y, 0};
	field[horse.x][horse.y] = 1;
	char hasPath = recur(field, horse);

	if (hasPath) {
		for (int y = 0; y < FIELD_H; y++) {
			for (int x = 0; x < FIELD_W; x++) {
				printf("%4d ", field[x][y]);
			}
			printf("\n");
		}
	} else {
		printf("no path\n");
	}
}
