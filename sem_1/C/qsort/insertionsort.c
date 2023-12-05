#include <stdlib.h>
#include <string.h>

void insertionSort(void* arr, long long int n, int width, char (*compFunc)(const void*, const void*))
{
	char* in = (char*)arr;
	char* key = calloc(width, 1);

	for (int i = 1; i < n; i++) {
		int j = i - 1;

		memcpy(key, &in[i * width], width);

		while (j >= 0 && compFunc(&in[j * width], key)) {
			memcpy(&in[(j + 1) * width], &in[j * width], width);
			j = j - 1;
		}

		memcpy(&in[(j + 1) * width], key, width);
	}

	free(key);
}