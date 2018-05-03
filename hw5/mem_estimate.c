#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "getRealTime.h"

typedef struct listnode
{
	struct listnode* next;
} listnode;

size_t page_size;
size_t mmapped_size = 0;
listnode* head = 0;

void globals_init() {
	page_size = sysconf(_SC_PAGESIZE);
}

void get_new_page() {
	// SIDE EFFECT: map a new page and add it to list of pages
	void* n_page = mmap(0, page_size, 7, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
	if (n_page != (void*)(-1)) {
		mmapped_size += page_size;
		((listnode*)n_page)->next = head;
		head = n_page;
	} else {
		printf("MMAP page failed, ERRNO %d\n", errno);
		_exit(errno);
	}
}

void get_new_pages() {
	int i;
	for (i = 0; i<65536; i++) {
		get_new_page();
	}
}


void* get_tail() {
	// return the tail of the list of pages
	listnode* cur = head;
	while (cur && cur->next) {
		cur = cur->next;
	}
	return cur;
}

double measure_list() {
	// return the time taken to traverse the list of pages
	double start, end;
	start = getRealTime();
	get_tail();
	end = getRealTime();
	return end-start;
}

void iterate_map() {
	// iterate to request new memories
	// and measure the time of touching all requested pages (linked-list)
	// until the derivative of such time
	//   (which should remain zero with infinite memory size)
	// is considered significantly larger than running mean
	
	long i = 0;
	double time_took = 0;
	double d_time_took = 0;
	double d_time_mean = 0;

	FILE* f = fopen("mem_data.txt", "w");

	int tail = 5;
	int finished = 0;

	while (++i && tail) {
		get_new_pages();
		double prev_time_took = time_took;
		
		time_took = measure_list();
		d_time_took = time_took - prev_time_took;
		
		d_time_mean = (d_time_mean * (i-1) + d_time_took)/i;

		fprintf(f, "%ld\t%.31g\t%.31g\t%.31g\n", mmapped_size, time_took, d_time_took, d_time_mean);
		if (i>1 && d_time_took > d_time_mean*30) {
			// should use a running variance if assume a Gaussian 
			int gb = mmapped_size/1073741824 + ((mmapped_size%1073741824)? 1: 0);
			printf("memory should be along the magnitude of %ld bytes (round-up to %d GB).\n", mmapped_size, gb);
			finished = 1;
		}

		if (finished) {
			// some ghetto outlier handling
			if (time_took < prev_time_took/2) {
				printf("oh wait, there's more to it\n");
				finished = 0;
				tail = 5;
			} else {
				tail--;
			}
		}
		
	}
}


int main() {
	globals_init();

	iterate_map();
	return 0;
}