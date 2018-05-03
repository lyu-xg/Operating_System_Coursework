CFLAGS=-g -O3

all:	check

default: check
	
clean: 
	rm -rf *.o *.so data* cache_line_size mem_estimate  *.txt cache_size

%.o: %.c
	gcc ${CFLAGS} -c -Wall -Werror -fpic -o $@ $<


mem_estimate.o: mem_estimate.c
	gcc -g -O0 -c -Wall -Werror -fpic -o mem_estimate.o mem_estimate.c	

mem_estimate: mem_estimate.o getRealTime.o
	gcc ${CFLAGS} -o mem_estimate mem_estimate.o getRealTime.o

cache_line_size: cache_line_size.o getRealTime.o
	gcc ${CFLAGS} -o cache_line_size cache_line_size.o getRealTime.o

cache_size: cache_size.o getRealTime.o
	gcc ${CFLAGS} -o cache_size cache_size.o getRealTime.o

mem: clean mem_estimate
	./mem_estimate

visualize_mem: clean
	python visualize_mem.py mem_data.txt

mem_all: clean mem visualize_mem

line: clean cache_line_size
	./cache_line_size


cache: clean cache_size
	./cache_size >> cache_data.txt
	

visualize_arraysize: cache
	python visualize_cache.py cache_data.txt 64B

visualize_stride: cache
	python visualize_cache.py cache_data.txt