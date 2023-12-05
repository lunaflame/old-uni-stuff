#include <string.h>
#include <stdlib.h>

static void byteSwap(char* a, char* b, int width) {
	char byte = 0;
	for (int swapByte = 0; swapByte < width; swapByte++) {
		byte = a[swapByte];
		a[swapByte] = b[swapByte];
		b[swapByte] = byte;
	}
}

void heapify(char* arr, int n, int i, int width, char (*compFunc)(const void*, const void*))
{
	int largest = i * width;
	int l = 2 * i * width + width;
	int r = 2 * i * width + 2 * width;

	//printf("heapify: n is %d, width is %d; therefore max. is %d\n", n, width, n * width);
	if (l < n * width) {
		//printf("l: comparing %d and %d\n", l, i * width);
		if (compFunc (&arr[l], &arr[largest])) {
			largest = l;
		}
	}

	if (r < n * width) {
		//printf("r: comparing %d and %d\n", r, largest);
		if (compFunc (&arr[r], &arr[largest]))
			largest = r;
	}

	if (largest != i * width) {
		byteSwap(&arr[i * width], &arr[largest], width);
		heapify(arr, n, largest, width, compFunc);
	}
}


void heapSort(void* arr, int n, int width, char (*compFunc)(const void*, const void*))
{
	char* in = (char*)arr; // bruh

	for (int i = n / 2 - 1; i >= 0; i--) {
		heapify(in, n, i, width, compFunc);
	}

	for (int i = n - 1; i > 0; i--) {
		byteSwap(&in[0], &in[i * width], width);

		heapify(in, i, 0, width, compFunc);
	}
}
