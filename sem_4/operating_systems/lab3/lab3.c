#include <stdio.h>
#include <unistd.h>

void printIDs() {
	printf("%-30s: %d\n", "Real ID", getuid());
	printf("%-30s: %d\n", "Effective ID", geteuid());
}

void tryOpen(char* path) {
	FILE* fp = fopen(path, "r");
	if (fp == NULL) {
		// hacky concatenation, lol
		perror("	Failed to open file: ");
	} else {
		printf("	File %s opened successfully. Continuing.\n", path);
		fclose(fp);
	}
}

int main(int argc, char** argv) {
	printIDs();

	if (argc < 2) {
		perror("-- No file supplied. Exiting. --\n");
		return 0;
	}

	tryOpen(argv[1]);

	seteuid(getuid());
	printIDs();

	tryOpen(argv[1]);

	return 0;
}

// some_rando lol

/*

Функция gets* считывает ввод с stdin и записывает в буфер,
пока не встретится символ новой строки или EOF

*запрещённая в C террористическая функция

*/