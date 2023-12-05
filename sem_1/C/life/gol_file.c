#include "gol_cells.h"
#include "gol_scream.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STATE_READING_HEADER 0
#define STATE_GROUPING_DATA 1	// concatting all data into one line
#define STATE_READING_DATA 2	// actually parsing the data
#define STATE_READING_FINISHED 3

#define DEFAULT_FIELD_W 20
#define DEFAULT_FIELD_H 10

#define MAX_RLE_KEY_LEN 5
#define MAX_RLE_VAL_LEN 71

const char important[] = {'P', 'R', 'r'};

// see: https://www.conwaylife.com/wiki/Run_Length_Encoded

// #P, #R and #r actually mean something
// others don't

// UPD: #P, #R and #r don't need to be supported


#ifndef __unix__
	int getline(char** ptr, size_t* sizePtr, FILE* fp) {
		char* buf;
		size_t bufSize = 256;

		if (*sizePtr != 0) {
			buf = *ptr;
			bufSize = *sizePtr;
		} else {
			buf = malloc(256);
		}

		size_t cursor = 0;

		int curChar = 0;

		while (true) {
			curChar = fgetc(fp);

			if (curChar <= 0) {
				return curChar;
			}
			if (curChar == '\n') {
				buf[cursor] = '\n';
				cursor++;
				break;
			}

			buf[cursor] = curChar;
			cursor++;

			if (cursor == (bufSize - 2)) {
				buf = (char*)realloc(buf, bufSize * 2);
				bufSize = bufSize * 2;
			}
		}

		buf[cursor] = '\0';

		*ptr = buf;
		*sizePtr = bufSize;
		return cursor;
	}
#endif

static char isImportant(char in) {
	for (unsigned int i = 0; i < sizeof(important); i++) {
		if (in == important[i]) {
			return 1;
		}
	}
	return 0;
}

// #P and #R give the top-left coordinate to start the pattern from
static char isTopLeft(char in) {
	return in == 'P' || in == 'R';
}

// #r remains unimplemented for now (use the standard method of defining rules)
static void readComment(char* in) {
	if ( !isImportant(in[1]) ) {
		return;
	}

	if (isTopLeft(in[1])) {
		int tX = 0;
		int tY = 0;
		sscanf("#%*c %d %d", in, &tX, &tY);
		GoL_Warn("Top-left comment: Not implemented.");
	} else {
		GoL_Warn("Comment '%c': Not implemented.", in[1]);
	}
}

static unsigned char isNumber(char s) {
	if ( (s - '0' >= 0) && (s - '9' <= 0) ) return 1;
	return 0;
}

static char applyRule(char* key, char* value, unsigned char valueLen,	// required for parsing ruleset
	int* width, int* height, bool* survive, bool* reproduce, bool* rulesChanged) {	// required for output of rulesets

	char isX = strcmp(key, "x") == 0;
	char isY = strcmp(key, "y") == 0;

	if (isX || isY) {
		int num = atoi(value);

		if (num < 0) {
			GoL_Warn("Negative widths/heights are not supported."); // wat?
			return 1;
		}

		if (isX) {
			*width = num;
		} else {
			*height = num;
		}
	} else if (strcmp(key, "rule") == 0) {
		char currentRule = '\0';

		for (unsigned int i = 0; i < valueLen; i++) {
			char cur = value[i];
			if (!isNumber(cur)) {
				currentRule = cur;
			} else {
				switch (currentRule) {
					case 'B':	// 'B' = 'Born' = 'reproduce'
						reproduce[cur - '0'] = true;
						*rulesChanged = true;
						break;
					case 'S':	// 'S' = 'Survive'
						survive[cur - '0'] = true;
						*rulesChanged = true;
						break;

					default:
						GoL_Warn("Unsupported rule letter '%c'", currentRule);
				}

			}
		}
	} else {
		GoL_Warn("Unsupported key/value: %s = %s. Ignoring...", key, value);
	}

	return 0;
}

static char readHeader(struct gameField** fieldptr, char* in, size_t len) {
	char key[MAX_RLE_KEY_LEN]; 		// max. : 'rule'
	unsigned char keyLen = 0;

	char value[MAX_RLE_VAL_LEN];
	unsigned char valueLen = 0;

	char readingValue = 0;

	int width = -1;
	int height = -1;

	bool survive[10] = {false};
	bool reproduce[10] = {false};
	bool rulesChanged = false;

	for (size_t i = 0; i < len; i++) {
		char chr = in[i];

		if (chr == ' ') { // ignore spaces
			continue;
		}

		if (chr == '=') { // change state so new chars go into 'value' buf
			readingValue = 1;
			continue;
		}

		if (readingValue) {
			value[valueLen] = chr;
			valueLen++;

			if (valueLen >= MAX_RLE_VAL_LEN) {
				GoL_Warn("Value is too long! (max. %d characters)\n	Possibly a malformed file?", MAX_RLE_VAL_LEN);
				return 1;
			}

		}
		else {
			key[keyLen] = chr;
			keyLen++;

			if (keyLen >= MAX_RLE_KEY_LEN) {
				GoL_Warn("Key is too long! (max. %d characters)\n	Possibly a malformed file?", MAX_RLE_KEY_LEN);
				return 1;
			}
		}

		if (chr == ',' || i == len - 1) {
			key[keyLen] = '\0';
			value[valueLen] = '\0';

			applyRule(key, value, valueLen, &width, &height, (bool*)&survive, (bool*)&reproduce, &rulesChanged);

			// reset reader

			keyLen = 0;
			valueLen = 0;
			readingValue = 0;
			continue;
		}

	}

	unsigned char bad_width = width <= 0;
	unsigned char bad_height = width <= 0;

	if (bad_width || bad_height) {

		char* what; // AAAAAAAAAAAAAAAAAAAAAAAAAA

		if (bad_width && bad_height) {
			char s[] = "Width and height";
			what = s;

			width = DEFAULT_FIELD_W;
			height = DEFAULT_FIELD_H;

		} else if (bad_width) {
			char s[] = "Width";
			what = s;

			width = DEFAULT_FIELD_W;

		} else {
			char s[] = "Height";
			what = s;

			height = DEFAULT_FIELD_H;
		};

		char* grammer;

		if (bad_width && bad_height) {
			char s[] = "were";
			grammer = s;
		} else {
			char s[] = "was";
			grammer = s;
		};

		GoL_Warn("%s of the field %s invalid; using default size (%dx%d).", what, grammer, width, height);
	}

	GoL_Inform("Creating a %dx%d game field.", width, height);

	struct gameField* g_Field = NULL;

	if (initField(&g_Field, width, height)) {
		GoL_Throw("Failed to create field while reading file.");
		return 1;
	}

	*fieldptr = g_Field;

	if (rulesChanged) {
		for (int i = 0; i < 10; i++) {
			g_Field->neighborsLive[i] = survive[i];
			g_Field->neighborsReproduce[i] = reproduce[i];
		}
	}

	return 0;
}

static char cellCharToBool(char s) {
	if (s == 'b') return 0;
	if (s == 'o') return 1;
	return -1;
}

static void mergeLine(struct gameField* g_Field, bool* line, int lineNum, int cellsFilled) { // merges the line of cells we've read into the game field
	int start = lineNum * g_Field->width;

	for (int i = start; i < start + cellsFilled; i++) {
		g_Field->field[i] = line[i - start];
	}
}

static char readData(struct gameField* g_Field, char* in, size_t inLen, int* state) {
	bool* curLine = calloc(g_Field->width + 1, 1);
	unsigned int curCell = 0; // current cell in the line
	unsigned int lineNum = 0;

	char buf[11]; 		// stores the cell number (repetitions number)
	unsigned char bufLen = 0;

	for (size_t i = 0; i <= inLen; i++) {
		char cur = in[i];
	
		if (isNumber(cur)) {
			buf[bufLen] = cur;
			bufLen++;

			if (bufLen >= sizeof(buf)) {
				GoL_Error("Could not read RLE data: cell amount is bigger than 10 chars!");
				return 1;
			}
		}

		if (cur == '$' || cur == '!' || cur == 0) {
			mergeLine(g_Field, curLine, lineNum, curCell);
			printf("Merged line #%d into field, going onto line #%d...\n", lineNum, lineNum + 1);
			lineNum++;
			curCell = 0;
			bufLen = 0;

			if (lineNum > g_Field->height) {
				GoL_Error("Could not read RLE data: amount of lines (%d) is bigger than the height of the field (%d)!", lineNum, g_Field->height);
				return 1;
			}

			if (cur == '!') {
				*state = STATE_READING_FINISHED;
				return 0;
			}
		}

		char bCell = cellCharToBool(cur);

		if (bCell > -1) {
			buf[bufLen] = '\0';

			int len;

			if (bufLen == 0) { // we haven't read any numbers, so that implies there's only 1 on that line
				len = 1;
			} else {
				len = atoi(buf);
			}
			printf("Repeating %d %d times\n", bCell, len);
			if (len <= 0) {
				GoL_Error("Negative number in data! You can't really write a negative amount of cells now, can you?...");
				return 1;
			}
			unsigned int loopTo = curCell + len;

			if (loopTo > g_Field->width) {
				GoL_Error("Amount of cells on one line exceeds field width!");
				return 1;
			}

			for (; curCell < loopTo; curCell++) {
				curLine[curCell] = bCell;
			}

			bufLen = 0;
		}
	}

	free(curLine);

	return 0;
}

// returns 0 if no grouping necessary and everything went a-ok
// returns 1 if grouping is required
// returns 2 if we should burn down to the ground

static char parseLine(struct gameField** fieldPtr, char* in, size_t len, int* state, char* groupedLines, size_t groupedLinesLen) {
	printf("Parsing line: %s\n", in);
	if (*state == STATE_READING_HEADER) {
		if (in[0] == '#') {
			readComment(in);
			printf("Read as a comment\n");
			return 0;
		} else {
			char prank_gone_wrong = readHeader(fieldPtr, in, len);
			printf("Read as a header\n");
			*state = STATE_GROUPING_DATA;
			if (prank_gone_wrong) {
				printf("DYING\n");
				return 2;
			}
			return 0;
		}
	}

	if (*state == STATE_GROUPING_DATA) {
		printf("Grouping data %d %s\n", groupedLinesLen, groupedLines);
		unsigned char found = 0;

		for (size_t i = 0; i < len; i++) {

			if (in[i] == '!') { // ! marks the end of the data input, so we can stop grouping now
				found = 1;
				memcpy(groupedLines + groupedLinesLen, in, i);

				groupedLines[i + 1] = '\0';
				groupedLinesLen = i;
				break;
			}
		}

		if (!found) { // no `!` = keep grouping until we find it
			return 1;
		} else {
			*state = STATE_READING_DATA;
		}
	}

	if (*state == STATE_READING_DATA) {
		printf("Read as data: %s %d\n", groupedLines, groupedLinesLen);
		char prank_gone_wrong = readData(*fieldPtr, groupedLines, groupedLinesLen, state);

		if (prank_gone_wrong) {
			return 2;
		}
		return 0;
	}

	return 0;
}

struct gameField* readConfig(char* path) {
	FILE* cfg = fopen(path, "r");

	if (NULL == cfg) {
		printf("Failed to open the config file @ %s\n", path); // this is a good time to perish
		exit(-1);

		return NULL;
	}

	char* groupedLines = calloc(256, sizeof(char));
	size_t groupedLinesLen = 0;
	size_t curLinesLen = 256;

	char* line = NULL;
	size_t lineBufSize = 0;

	int state = STATE_READING_HEADER;
	int lineLen = getline(&line, &lineBufSize, cfg);	// can't use size_t because getline can return a negative value for an error

	bool grouping = false;

	struct gameField* g_Field = NULL;

	while (lineLen != -1 && state != STATE_READING_FINISHED) {
		printf("Line len: %d / %s\n", lineLen, line);
		char status = parseLine(&g_Field, line, lineLen, &state, groupedLines, groupedLinesLen);

		if (status == 2) {
			free(groupedLines);
			fclose(cfg);
			return NULL; // nyope
		}

		if (status == 1) {
			// when parsing data it's simpler to just group up every line into one big string
			grouping = true;
			groupedLinesLen += lineLen;

			if (groupedLinesLen > curLinesLen) {
				groupedLines = realloc(groupedLines, groupedLinesLen);
				curLinesLen = groupedLinesLen;
			}

			int inLen = 0;

			// copy the new line onto the end of groupedLines array
			for (size_t i = groupedLinesLen - lineLen; i < groupedLinesLen; i++) {
				groupedLines[i] = line[inLen];
				inLen++;
			}

		} else {
			grouping = false;
		}

		lineLen = getline(&line, &lineBufSize, cfg);
		//printf("reading new line: %d", lineLen);
	}

	if (grouping) { // parse one last time with the fully-grouped lines
		printf("one last boys");
		char status = parseLine(&g_Field, line, strlen(line), &state, groupedLines, groupedLinesLen);
		if (status == 2) {
			free(groupedLines);
			fclose(cfg);

			return NULL; // nyope
		}
	}

	return g_Field;
}