/*
	[x] die выкинуть
	[ ] однострочные в {}
	[ ] пробельчики исправить
	[x] пустые строки
	[x] почекать RLE
	[ ] магические числа задефить
	[ ] чекать на -Wall
	[x] conio.h

*/

#define _CRT_SECURE_NO_WARNINGS
#include "gol_cells.h"
#include "gol_file.h"
#include "gol_scream.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define READCHAR getchar

#define DEFAULT_FIELD_W 20
#define DEFAULT_FIELD_H 10

extern int GoL_WarningLevel;

int main(int argc, char* args[]) {
	struct gameField* g_Field;

	// ideally this should parse args to decide the warning level, but eh.
	GoL_WarningLevel = INFO | WARNING | ERROR | CRITICAL;

	if (argc > 1) {
		char* file = args[1];
		GoL_Inform("Reading cfg @ %s", file);

		g_Field = readConfig(file);
		if (NULL == g_Field) {
			GoL_Throw("Failed to read config.    Y O U R    L I F E   I S   O V E R");
			return 1;
		}
	} else {
		GoL_Inform("No file provided; using a random field.\n");
		srand(time(0));

		if (initField(&g_Field, DEFAULT_FIELD_W, DEFAULT_FIELD_H)) {
			GoL_Throw("Failed to create default field.    Y O U R    L I F E   I S   O V E R");
			return 1;
		}

		// default rules
		g_Field->neighborsLive[2] = true;
		g_Field->neighborsLive[3] = true;

		g_Field->neighborsReproduce[3] = true;

		generateRandomField(g_Field);
	}

	while (1) {
		printField(g_Field);
		printf("Press any key to simulate...\n");

		int chr = READCHAR();

		if (chr == 3) { // CTRL+C
			break;
		}

		simulateField(g_Field);
	}

	return 0;
}