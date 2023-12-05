#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../util/idk.h"

// FORCE values aren't automatically negated where necessary

// #define FORCE_VALUES 0


#define FORCE_SEQ1 "ATGAGTCTCTCTGATAAGGACAAGGCTGCTGTGAAAGCCCTATGG"
#define FORCE_SEQ2 "CTGTCTCCTGCCGACAAGACCAACGTCAAGGCCGCCTGGGGTAAG"
#define FORCE_SUBMATCH  5
#define FORCE_SUBDIFF  -4
#define FORCE_GAPOPEN  -10
#define FORCE_GAPEXT  -0.5


/*
#define FORCE_SEQ1 "ACGACATCAGCTACGATCAGCTACGATCAGCTACGATCGACTAGCATCGATCAGAGAGAGCTAG"
#define FORCE_SEQ2 "ACGACATCAGCTACGATCAGCGACTATCGACTAGCATGAGAGCTCG"
#define FORCE_SUBMATCH  5
#define FORCE_SUBDIFF  -4
#define FORCE_GAPOPEN  -10
#define FORCE_GAPEXT  -0.5
*/

/*
#define FORCE_SEQ1 "AGGTGCGATG"
#define FORCE_SEQ2 "AGTGGATCCC"
#define FORCE_SUBMATCH  5
#define FORCE_SUBDIFF  -4
#define FORCE_GAPOPEN  -5
#define FORCE_GAPEXT  -1
*/


// wiki example:

/*
#define FORCE_SEQ1 "TGTTACGG"
#define FORCE_SEQ2 "GGTTGACTA"
#define FORCE_SUBMATCH  3
#define FORCE_SUBDIFF  -3
#define FORCE_GAPOPEN  -2
#define FORCE_GAPEXT  -2
*/

char readSequences(char** seq1ret, char** seq2ret) {
	#ifdef FORCE_VALUES
		if (1) {
			*seq1ret = ascalloc(sizeof(FORCE_SEQ1) + 1, sizeof(char));
			memcpy(*seq1ret, &(FORCE_SEQ1), sizeof(FORCE_SEQ1));

			*seq2ret = ascalloc(sizeof(FORCE_SEQ2) + 1, sizeof(char));
			memcpy(*seq2ret, &(FORCE_SEQ2), sizeof(FORCE_SEQ2));

			return 1;
		}
	#endif

	assert(seq1ret != NULL);
	assert(seq2ret != NULL);

	char* buf = asmalloc(64);

	size_t bufSz = 64;

	printf("Enter sequence 1: ");

	int chr = fgetc(stdin);
	size_t cursor = 0;

	while (chr != EOF && chr != '\n') {
		if (bufSz <= (cursor + 1)) {
			buf = asrealloc(buf, bufSz * 2);
			bufSz *= 2;
		}

		buf[cursor] = chr;
		chr = fgetc(stdin);
		cursor++;
	}

	buf[cursor] = '\0';

	char* seq1 = asmalloc(cursor + 1);

	memcpy(seq1, buf, cursor + 1);


	printf("Enter sequence 2: ");


	cursor = 0;
	chr = fgetc(stdin);

	while (chr != EOF && chr != '\n') {
		if (bufSz <= (cursor + 1)) {
			buf = asrealloc(buf, bufSz * 2);
			bufSz *= 2;
		}

		buf[cursor] = chr;
		chr = fgetc(stdin);
		cursor++;
	}

	buf[cursor] = '\0';

	char* seq2 = asmalloc(cursor + 1);

	memcpy(seq2, buf, cursor + 1);

	free(buf);

	*seq1ret = seq1;
	*seq2ret = seq2;

	// todo: are the sequences somehow limited?
	return 1;
}

char readSubstitution(double* matchPtr, double* diffPtr) {
	assert(matchPtr != NULL);
	assert(diffPtr != NULL);

	#ifdef FORCE_VALUES
		if (1) {
			*matchPtr = FORCE_SUBMATCH;
			*diffPtr = FORCE_SUBDIFF;
			return 1;
		}
	#endif

	double match = 0;
	double diff = 0;

	printf("Enter substitution matrix (numbers):\n"
		"	If cell matches: ");

	int read = scanf("%lf", &match);
	if (read != 1) {
		printf("Failed to read match score.\n");
		return 0;
	}

	printf("	If cell mismatches: -");

	read = scanf("%lf", &diff);
	if (read != 1) {
		printf("Failed to read mismatch score.\n");
		return 0;
	}

	*matchPtr = match;
	*diffPtr = -diff;

	return 1;
}

char readGapPenalties(double* openPtr, double* extPtr) {
	assert(openPtr != NULL);
	assert(extPtr != NULL);

	#ifdef FORCE_VALUES
		if (1) {
			*openPtr = FORCE_GAPOPEN;
			*extPtr = FORCE_GAPEXT;
			return 1;
		}
	#endif

	double open = 0;
	double ext = 0;

	printf("Enter gap penalties (numbers):\n"
		"	Gap opening: -");

	int read = scanf("%lf", &open);
	if (read != 1) {
		printf("Failed to read gap opening penalty.\n");
		return 0;
	}

	printf("	Gap extension: -");

	read = scanf("%lf", &ext);
	if (read != 1) {
		printf("Failed to read gap extension penalty.\n");
		return 0;
	}

	*openPtr = -open;
	*extPtr = -ext;

	return 1;
}
