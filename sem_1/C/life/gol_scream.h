#ifndef GoL_Scream
#define GoL_Scream

enum GoL_Levels {
	INFO 		= 1,
	WARNING		= 1 << 1,
	ERROR		= 1 << 2,
	CRITICAL	= 1 << 3,
};

typedef enum GoL_Levels GoL_Levels;

void GoL_Inform(char* frmt, ...);
void GoL_Warn(char* frmt, ...);
void GoL_Error(char* frmt, ...);
void GoL_Throw(char* frmt, ...);
#endif