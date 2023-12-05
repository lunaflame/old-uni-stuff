#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h> // PRId64; GCC is dumb

void die(char* why) {
	printf("%s\n", why);
	exit(-1);
}

void dief(char* frmt, ...) {
	va_list args;
	va_start(args, frmt);
		vprintf(frmt, args);
	va_end(args);
	exit(-1);
}

void printBinary(char what[], unsigned char len, char skipBits) {
	for (int i = len - 1; i >= 0; i--) {
		char cur = what[i];

		for (int bitNum = 7; bitNum >= 0; bitNum--) {
			if (skipBits > 0) {
				skipBits--;
				continue;
			}
			int bit = (cur >> bitNum) & 1;
			printf("%d", bit);
		}
	}
	printf("\n");
}

char fromDouble() {
	double in = 0;

	printf("Input double: ");
	int amt = scanf("%lf", &in);
	if (amt < 1) {
		printf("Failed to read input as a double.");
		return 0;
	}

	unsigned long long int as_int = 0;
	memcpy(&as_int, &in, 8);

	int sign = as_int >> 63;	// can't print char, lol

	printf("\nSign: %d\n", sign);

	int exponent = (as_int >> 52) & ((1 << 11) - 1);

	char binArray[4] = {0};
	memcpy(&binArray, &exponent, 4);

	printf("Exponent: %d\n	binary: ", exponent);
	printBinary(binArray, 4, (32 - 11));

	long long int manMask = ((long long int)1 << 52) - 1;
	long long int mantissa = as_int & manMask;

	printf("Mantissa: %" PRId64	"\n	binary: ", mantissa);

	char mantArray[8] = {0};
	memcpy(&mantArray, &mantissa, 8);

	printBinary(mantArray, 7, 56 - 52);

	printf("Total double in binary: ");
	printBinary((char*)&in, 8, 0);

	return 1;
}

char readSign(char* where) {
	char sign;

	printf("Input sign: '+' or '-' : ");
		int amt = scanf("%c", &sign);
	printf("\n");

	if ( ((sign != '-') && (sign != '+')) || amt < 1 ) {
		printf("Failed to read input sign; not '+' nor '-'.");
		return 0;
	}

	char signNum = 0;
	if (sign == '-') {
		signNum = 1;
	}

	*where = signNum;

	return 1;
}

int readExponent(int* where) {

	char exp_char[12];

	printf("Input exponent in binary: ");
		scanf("%11s", (char*)&exp_char);
	printf("\n");

	int exponent = 0;
	int len = 11;

	for (int i = 0; i < 11; i++) {
		char cur = exp_char[i];
		if (cur == 0) {	// end of input
			len = i;
			break;
		}
		if ( (cur != '0') && (cur != '1') ) {
			printf("\nUnexpected '%c' in exponent input (allowed: '1', '0').", cur);
			return 0;
		}

		exponent += (unsigned int)(cur - '0') << (10 - i);
	}

	if (len < 11) {
		for (int i = len; i < 11; i++) {
			exp_char[i] = '0';
		}
		exp_char[11] = '\0';
		printf("Exponent not entered completely; assuming input = '%s'\n", exp_char);
	}

	char readChar = getchar();

	while ( (readChar != '\n') && (readChar != EOF) ) {
		// flush over-the-limit chars obtained from exponent input
		// (if there are any) because scanf sucks
		readChar = getchar();
	};

	*where = exponent;

	return 1;
}

char readMantissa(int64_t* where) {
	char man_char[53];

	printf("Input mantissa in binary: ");
	scanf("%52s", (char*)&man_char);
	printf("\n");

	int64_t mantissa = 0;
	int len = 52;

	for (int i = 0; i < 52; i++) {
		char cur = man_char[i];
		if (cur == 0) {
			len = i;
			break;
		}
		if ( (cur != '0') && (cur != '1') ) {
			dief("\nUnexpected '%c' in mantissa input (allowed: '1', '0').", cur);
		}
		mantissa += (uint64_t)(cur - '0') << (51 - i);
	}

	if (len < 52) {
		for (int i = len; i < 52; i++) {
			man_char[i] = '0';
		}
		man_char[52] = '\0';
		printf("Mantissa not entered completely; assuming input = '%s'\n", man_char);
	}

	*where = mantissa;

	return 1;
}

char toDouble() {
	char sign = 0;
	char signOK = readSign(&sign);

	if (!signOK) {
		printf("Failed to read sign.\n");
		return 1;
	}

	int exponent = 0;
	char expOK = readExponent(&exponent);

	if (!expOK) {
		printf("Failed to read exponent.\n");
		return 1;
	}

	int64_t mantissa = 0;
	char mOK = readMantissa(&mantissa);

	if (!mOK) {
		printf("Failed to read mantissa.\n");
		return 1;
	}

	int64_t dblAsInt = ((int64_t)sign << 63) + ((int64_t)exponent << 52) + mantissa;

	double dbl = 0;
	char asBinary[8] = {0};

	memcpy(&dbl, &dblAsInt, 8);
	memcpy(&asBinary, &dbl, 8);

	printf("\nOutput: %lf\n", dbl);
	printf("	binary:");
	printBinary(asBinary, 8, 0);

	return 0;
}

int main(int argc, char* args[]) {
	if (argc < 2) {
		die("Use -fromd or -tod arguments.");
	}

	char* mode = args[1];

	if (strcmp(mode, "-fromd") == 0) {
		char ok = fromDouble();
		if (!ok) {
			printf("Failed?\n");
		}
	} else if (strcmp(mode, "-tod") == 0) {
		char ok = toDouble();
		if (!ok) {
			printf("Failed?\n");
		}
	} else {
		printf("Unrecognized option: '%s'.", *mode);
		return 0;
	}
}