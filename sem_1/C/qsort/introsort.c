#include <math.h>
#include <stddef.h>
#include <stdio.h>

#include "insertionsort.h"
#include "quicksort.h"
#include "heapsort.h"

#define MIN_ELEM_INSSORT (long long int) 16 	// 64 elements or less = begin insertion sort

static void introsort_util(void* arr, long long int len, int width, int depth, char (*compFunc)(const void*, const void*)) {

	if (len < MIN_ELEM_INSSORT) {
		insertionSort(arr, len, width, compFunc);
	} else {
		char ok = quicksort_qs(arr, len, width, depth, compFunc);

		if (!ok) {
			heapSort(arr, len, width, compFunc);
		}
	}
}

void introsort(void* arr, size_t len, size_t width, char (*compFunc)(const void*, const void*)) {
	int depth = len / 4;
	introsort_util(arr, len, width, depth, compFunc);
}