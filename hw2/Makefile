CFLAGS=-g -O0

all:	check

default: check
	
clean-ckpt:
	rm -rf myckpt

clean: clean-ckpt
	rm -rf restart hello hello.o ckpt ckpt.o libckpt.so myrestart utils.o

utils.o: utils.c
	gcc ${CFLAGS} -c -Wall -Werror -fpic -o utils.o utils.c

ckpt.o: ckpt.c
	gcc ${CFLAGS} -c -Wall -Werror -fpic -o ckpt.o ckpt.c

ckpt: ckpt.o utils.o
	gcc ${CFLAGS} -o ckpt ckpt.o utils.o

libckpt.so: ckpt.o utils.o
	gcc ${CFLAGS} -shared -o libckpt.so ckpt.o utils.o

hello.o: hello.c
	gcc ${CFLAGS} -c -Wall -Werror -fpic -o hello.o hello.c

hello:	hello.o
	gcc ${CFLAGS} -o hello hello.o

myrestart: myrestart.c utils.o
	gcc ${CFLAGS} -g -static -Wl,-Ttext-segment=5000000 -Wl,-Tdata=5100000 -Wl,-Tbss=5200000 -o myrestart myrestart.c utils.o

restart: myrestart
	./myrestart myckpt

gdb:
	gdb --args ./restart myckpt

check:	clean libckpt.so hello myrestart
	(sleep 3 && pkill -12 hello && sleep 2 && pkill -9 hello) & 
	(LD_PRELOAD=`pwd`/libckpt.so ./hello)
	
dist:
	dir=`basename $$PWD`; cd ..; tar cvf $$dir.tar ./$$dir; gzip $$dir.tar
