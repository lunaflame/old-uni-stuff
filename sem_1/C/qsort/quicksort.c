#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static void byteSwap(char* a, char* b, int width) {
	char byte = 0;
	for (int swapByte = 0; swapByte < width; swapByte++) {
		byte = a[swapByte];
		a[swapByte] = b[swapByte];
		b[swapByte] = byte;
	}
}


static int partition(char* data, int left, int right, int width,
	char (*compFunc)(const void*, const void*)) {

	void* pivot = &data[right * width];
	int i = (left - 1);

	for (int j = left; j < right; j++) {
		char ret = compFunc( &data[j * width], pivot );

		if ( ret ) {
			i++;

			byteSwap(&data[j * width], &data[i * width], width);
		}

	}

	byteSwap(&data[right * width], &data[(i + 1) * width], width);

	return i + 1;
}

static char quicksort_int(char* arr, int first, int last,
						  int width, int depthLeft, char (*compFunc)(const void*, const void*)) {

	if (first >= last || last <= 0) return 1;

	depthLeft--;

	if (depthLeft < 1) {
		return 0;
	}

	int split = partition(arr, first, last, width, compFunc);

	char ok = quicksort_int(arr, first, split - 1, width, depthLeft, compFunc);
	char ok2 = quicksort_int(arr, split + 1, last, width, depthLeft, compFunc);

	return ok & ok2;
}

char quicksort_qs(void* arr, int len, int width, int maxDepth, char (*compFunc)(const void*, const void*)) {
	char* in = (char*)arr;
									// 0-based
	char ok = quicksort_int(in, 0, len-1, width, maxDepth, compFunc);
	return ok;
}