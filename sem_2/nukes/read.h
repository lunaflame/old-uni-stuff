#ifndef NukeReader_IG
#define NukeReader_IG

char readSubstitution(double* matchPtr, double* diffPtr);
char readSequences(char** seq1ret, char** seq2ret);
char readGapPenalties(double* open, double* ext);

#endif
