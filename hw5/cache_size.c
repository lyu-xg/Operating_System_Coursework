#include <stdio.h>
#include <stdlib.h>
#include "getRealTime.h"

#define KB 1024
#define MB KB*KB

// below is the expected maximum size of cache this program 
// should encounter, if machine have cache larger than 32MB,
// this program will not work as expected.
#define MAX_CACHE_SIZE 32 * MB
char JUNK[MAX_CACHE_SIZE];

char RES;

void clear_cache() {
	// fill cache with junk
	// by adding every integer in JUNK by 1
	int i = 0;
	while (i < MAX_CACHE_SIZE) {
		RES += JUNK[i++]++;
	}
}

char sum(char* a, int length) {
	int i = 0;
	char res = 0;
	while (i < length) {
		res += a[i++];
	}
	return res;
}


void measure_time() {
	
	int max_array_size = 64*MB;
	int max_stride = max_array_size / 2;

	// test out different strides
	int stride;
	for (stride = 1; stride	<= max_stride; stride *= 2) {
		printf("%d\t", stride);
		// test out different array size
		int array_size;
		for (array_size = 8*KB; array_size <= max_array_size; array_size *= 2) {
			char *array = malloc(array_size * sizeof(char));
			// char *junk_array = malloc(MAX_CACHE_SIZE);

			int i;
			double start, end, time_total;

			int t;
			for (t = 0; t < 32; t++) {
				start = getRealTime();
				for (i = 0; i < array_size; i += stride) {
					array[i]++;
				}
				end = getRealTime();
				time_total = end - start;

			}
			clear_cache();

			double avg_t = time_total / (array_size/stride);

			printf("%.31g\t", avg_t);
			// this is to ensure that compiler doesn't throw out
			// accessing and storing done on the arrays
			RES += sum(array, array_size);
			free(array);
		}
		printf("\n");
	}
}

int main() {
	// RES = 1;
	// printf("%ld",sizeof(char));
	measure_time();
	printf("RES: %d\n", RES);
	return 0;
}