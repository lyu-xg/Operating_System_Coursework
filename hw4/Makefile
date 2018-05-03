CFLAGS=-g -O0

all:	check

default: check
	
clean: 
	rm -rf *.o *.so malloc test1

%.o: %.c
	gcc ${CFLAGS} -c -Wall -Werror -fpic -o $@ $<


libmalloc.so: malloc.o calloc.o realloc.o memalign.o free.o utils.o itoa.o
	gcc ${CFLAGS} -shared  -pthread  -o libmalloc.so malloc.o calloc.o realloc.o memalign.o free.o utils.o itoa.o

# given test file has some warning, disabling warning-as-error
test%: test%.c
	gcc ${CFLAGS} -Wall -pthread $< -o $@

malloc:	malloc.o calloc.o realloc.o memalign.o free.o utils.o itoa.o
	gcc ${CFLAGS} -pthread -o malloc malloc.o calloc.o realloc.o memalign.o free.o utils.o itoa.o

check: clean libmalloc.so test1
	LD_PRELOAD=`pwd`/libmalloc.so ./test1

lib: libmalloc.so

main: clean malloc
	./malloc