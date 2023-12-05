#ifndef Smith_IG
#define Smith_IG

typedef struct smith_ret_t {
	char* sequence1;
	char* sequence2;

	double maxScore;
} SmithRet;

SmithRet* doTheThing(char* seq1, char* seq2,	// pointers to the sequences
	double (*scoreFunc) (char c1, char c2),	// must return the score from a substitution matrix for char1 and char2
	double gapOpenPenalty, double gapExtPenalty); // see: affine gap penalty

void freeSmithResult(SmithRet* what);


#endif
