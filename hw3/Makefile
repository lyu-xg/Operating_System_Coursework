CFLAGS=-g -O0

all:	check

default: check
	
clean: 
	rm -rf *.o *.so

%.o: %.c
	gcc ${CFLAGS} -c -Wall -Werror -fpic -o $@ $<


libmalloc.so: malloc.o calloc.o realloc.o memalign.o free.o utils.o 
	gcc ${CFLAGS} -shared  -pthread  -o libmalloc.so malloc.o calloc.o realloc.o memalign.o free.o utils.o 


# given test file has some warning, disabling warning-as-error
test%: test%.c
	gcc ${CFLAGS} -Wall -pthread $< -o $@

check:	libmalloc.so test1
	LD_PRELOAD=`pwd`/libmalloc.so ./test1

malloc: malloc.o utils.o
	gcc ${CFLAGS} -Wall -pthread $< -o $@