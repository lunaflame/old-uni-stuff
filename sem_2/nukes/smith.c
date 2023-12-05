#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "smith.h"
#include "../util/idk.h"

#define MIN(a, b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a, b) ( ((a) > (b)) ? (a) : (b) )

typedef struct cell_t {
	double score;

	size_t fromX;
	size_t fromY;

	char dir;
	size_t jumpDist;
} Cell;

typedef struct subdat_t {
	size_t w;
	size_t h;

	char* seq1;
	char* seq2;

	double gOpen;
	double gExt;

	Cell* field;

	double (*scoreFunc) (char c1, char c2);
} SubData;

const char ADD_SIDE = 0x1;
const char ADD_UP = 0x2;

// XY are 0-indexed
static Cell* mtrxXY(SubData* mtrx, size_t x, size_t y) {
	// dont need to check < since size_t is positive
	assert(mtrx != NULL);
	assert(x < mtrx->w);
	assert(y < mtrx->h);

	return &(mtrx->field[y * mtrx->w + x]);
}

static SubData newMtrx(char* seq1, char* seq2,
	size_t w, size_t h,
	double (*scoreFunc) (char c1, char c2),
	double gapOpen, double gapExt) {

	assert(seq1 != NULL);
	assert(seq2 != NULL);
	assert(scoreFunc != NULL);

	w++;
	h++;

	SubData ret;
	ret.w = w;
	ret.h = h;
	ret.seq1 = seq1;
	ret.seq2 = seq2;
	ret.gOpen = gapOpen;
	ret.gExt = gapExt;
	ret.scoreFunc = scoreFunc;

	ret.field = ascalloc(w * h, sizeof(Cell));

	// initialize 1st row and 1st column to 0
	// yes i dont rely on calloc >:(
	for (size_t x = 0; x < w; x++) {
		mtrxXY(&ret, x, 0)->score = 0;
	}

	for (size_t y = 1; y < h; y++) {
		mtrxXY(&ret, 0, y)->score = 0;
	}

	return ret;
}

static double getGapPenalty(SubData* what, size_t dist) {
	assert(what != NULL);
	return (what->gOpen + (what->gExt * dist));
}

static double getMatchScore(SubData* what, size_t x, size_t y) {
	assert(what != NULL);
	double score = what->scoreFunc( what->seq1[x - 1], what->seq2[y - 1] );
	return score;
}

static void calcSubMatrix(SubData* what) {
	assert(what != NULL);

	// start from (1, 1) since (0, 0) is already filled
	for (size_t x = 1; x < what->w; x++) {
		for (size_t y = 1; y < what->h; y++) {
			// https://i.imgur.com/J0Lj4Fa.png

			double adjScore = 0;
			size_t useX = 0;
			size_t useY = 0;
			size_t jumpDist = 0;
			char dir = 0;

			for (size_t nx = x; nx > 0; nx--) {
				size_t chkX = nx - 1;
				double gapPenalty = getGapPenalty(what, x - chkX - 1);
				Cell* cell = mtrxXY(what, chkX, y);
				double useScore = cell->score + gapPenalty;

				if (adjScore < useScore) {
					adjScore = useScore;
					useX = chkX;
					useY = y;
					dir = ADD_SIDE;
					jumpDist = x - chkX;
				}
			}

			for (size_t ny = y; ny > 0; ny--) {
				size_t chkY = ny - 1;
				double gapPenalty = getGapPenalty(what, y - chkY - 1);

				Cell* cell = mtrxXY(what, x, chkY);
				double useScore = cell->score + gapPenalty;

				if (adjScore < useScore) {
					adjScore = useScore;
					useX = x;
					useY = chkY;
					dir = ADD_UP;
					jumpDist = y - chkY;
				}
			}

			Cell* diagCell = mtrxXY(what, x - 1, y - 1);
			double diagScore = diagCell->score + getMatchScore(what, x, y);

			if (diagScore > adjScore) {
				adjScore = diagScore;
				useX = x - 1;
				useY = y - 1;
				dir = ADD_SIDE | ADD_UP;
				jumpDist = 1;
			}

			double newScore = MAX( // https://i.imgur.com/nZnu7dV.png
					adjScore,
					0
				);

			Cell* cur = mtrxXY(what, x, y);

			cur->score = newScore;
			cur->fromX = useX;
			cur->fromY = useY;
			cur->dir = dir;
			cur->jumpDist = jumpDist;
		}
	}

}


static char getPrevNode(SubData* what, size_t* xRet, size_t* yRet, size_t* dist,
	size_t curx, size_t cury) {

	assert(what != NULL);
	assert(xRet != NULL);
	assert(yRet != NULL);
	assert(dist != NULL);

	Cell* cell = mtrxXY(what, curx, cury);

	if (cell->score == 0) {
		return 0;
	}

	*xRet = cell->fromX;
	*yRet = cell->fromY;
	*dist = cell->jumpDist;

	return cell->dir;
}

static void backTrack(SubData* what, char** ret1, char** ret2, double* ret3) {
	assert(what != NULL);
	assert(ret1 != NULL);
	assert(ret2 != NULL);
	assert(ret3 != NULL);

	size_t bestX = 0;
	size_t bestY = 0;
	double bestScore = 0;

	for (size_t x = 1; x < what->w; x++) {
		for (size_t y = 1; y < what->h; y++) {
			if (bestScore < mtrxXY(what, x, y)->score) {
				bestScore = mtrxXY(what, x, y)->score;
				bestX = x;
				bestY = y;
			}
		}
	}

	*ret3 = bestScore;

	size_t xcur = MAX(what->w, what->h);
	size_t xadd = bestX;

	size_t ycur = MAX(what->w, what->h);
	size_t yadd = bestY;

	size_t jDist = 0;

	char exists = getPrevNode(what, &bestX, &bestY, &jDist, bestX, bestY);

	char* xarr = calloc(xcur + 2, sizeof(char));
	memset(xarr, '-', xcur);
	xarr[xcur] = '\0';

	char* yarr = calloc(ycur + 2, sizeof(char));
	memset(yarr, '-', ycur);
	yarr[ycur] = '\0';

	size_t iters = 0;

	while (exists) {

		/*printf("run #%d: moved ", iters);

		if (exists == (ADD_SIDE | ADD_UP)) {
			printf("diagonally (%u)\n", jDist);
		} else if (exists & ADD_UP) {
			printf("up by %u\n", jDist);
		} else {
			printf("left by %u\n", jDist);
		}*/

		for (size_t jmp = 0; jmp < jDist; jmp++) {
			if (exists & ADD_SIDE) {
				xarr[xcur] = what->seq1[xadd - jmp - 1];
			} else {
				xarr[xcur] = '-';
			}

			if (exists & ADD_UP) {
				yarr[ycur] = what->seq2[yadd - jmp - 1];
			} else {
				yarr[ycur] = '-';
			}
			xcur--;
			ycur--;
		}

		xadd = bestX;
		yadd = bestY;
		// double preScore = bestScore;

		exists = getPrevNode(what, &bestX, &bestY, &jDist, bestX, bestY);
		// printf("(%d, %d)[%.1lf] -> (%d, %d)[%.1lf]\n", xadd, yadd, preScore, bestX, bestY, bestScore);
		iters++;
	}

	// shift the chars down
	// ---ABCDEF  ->  ABCDEF--- -> ABCDEF-
	// --DEFABFC  ->  DEFABFC-- -> DEFABFC

	size_t remainsX = xcur;
	size_t remainsY = ycur;

	size_t initCur = MAX(what->w, what->h);
	size_t arrCursor = 0;

	for (; arrCursor < initCur - remainsX; arrCursor++) {
		xarr[arrCursor] = xarr[xcur + arrCursor + 1];
	}
	xarr[arrCursor] = '\0';

	for (arrCursor = 0; arrCursor < initCur - remainsY; arrCursor++) {
		yarr[arrCursor] = yarr[xcur + arrCursor + 1];
	}
	yarr[arrCursor] = '\0';

	// return!

	*ret1 = xarr;
	*ret2 = yarr;
}

void freeSmithResult(SmithRet* what) {
	free(what->sequence1);
	free(what->sequence2);
	free(what);
}



static void printMatrix(SubData* sDat) {
	printf("%7c", ' ');

	for (size_t x = 0; x < sDat->w; x++) {
		printf("%3c ", sDat->seq1[x]);
	}
	printf("\n");

	for (size_t y = 0; y < sDat->h; y++) {
		if (y > 0) {
			printf("%3c", sDat->seq2[y - 1]);
		} else {
			printf("%3c", ' ');
		}

		for (size_t x = 0; x < sDat->w; x++) {
			printf("%3.0lf ", mtrxXY(sDat, x, y)->score);
		}

		printf("\n");
	}
}


SmithRet* doTheThing(char* seq1, char* seq2,
	double (*scoreFunc) (char c1, char c2),
	double gapOpenPenalty, double gapExtPenalty) {

	assert(seq1 != NULL);
	assert(seq2 != NULL);

	size_t seq1len = strlen(seq1);
	size_t seq2len = strlen(seq2);

	SubData sDat = newMtrx(seq1, seq2,
		seq1len, seq2len, scoreFunc,
		gapOpenPenalty, gapExtPenalty);

	calcSubMatrix(&sDat);
	printMatrix(&sDat);

	char* ret1;
	char* ret2;
	double ret3;

	backTrack(&sDat, &ret1, &ret2, &ret3);

	free(sDat.field);

	SmithRet* ret = asmalloc(sizeof(SmithRet));
	ret->sequence1 = ret1;
	ret->sequence2 = ret2;
	ret->maxScore = ret3;

	return ret;
}
