#include "gol_scream.h"

#include <stdio.h>
#include <stdarg.h>



#define REQUIRE_WARN(lv) if((GoL_WarningLevel & lv) == 0)return;

int GoL_WarningLevel = 0;

void GoL_Inform(char* frmt, ...)
{
	REQUIRE_WARN(INFO);
	fprintf(stdout, "=? ");
		va_list args;
		va_start(args, frmt);
			vprintf(frmt, args);
		va_end(args);
	fprintf(stdout, "\n");
}


void GoL_Warn(char* frmt, ...)
{
	REQUIRE_WARN(WARNING);
	fprintf(stdout, "=! ");
		va_list args;
		va_start(args, frmt);
			vprintf(frmt, args);
		va_end(args);
	fprintf(stdout, "\n");
}


void GoL_Error(char* frmt, ...)
{
	REQUIRE_WARN(ERROR);
	fprintf(stdout, "==!! ");
		va_list args;
		va_start(args, frmt);
			vprintf(frmt, args);
		va_end(args);
	fprintf(stdout, "\n");
}


void GoL_Throw(char* frmt, ...)
{
	REQUIRE_WARN(CRITICAL);
	fprintf(stdout, "===!!! ");
		va_list args;
		va_start(args, frmt);
			vprintf(frmt, args);
		va_end(args);
	fprintf(stdout, "\n");
}

