#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <ucontext.h>
#include <fcntl.h>
#include <string.h>
#include "utils.h"

void
save_memory_regions(int outfile_fd)
{
  // stat does not work for maps files
  // using a hardcoded length buffer instead.
  int read_size = 4000;
  char read_buf[read_size];
  my_read("/proc/self/maps", read_size, read_buf, 0);
  
  char* s = read_buf;
  char* next = read_buf;

  while (1) { // iterate throught maps file line by line
    if (*s == '\0') { break; } // if end of file, break

    while (*next++ != '\n'); // seek next line, end at starting of next sentence
    *(next-1) = '\0'; // end the previous line
    
    // processing the previous line
    struct MemoryRegion m = processRegion(s);
    s = next; // move s to next line

    // if not readable, don't bother
    if ( m.priv%2 == 0 ) { continue; }
    // write mem region header
    my_write(outfile_fd, &m, sizeof(struct MemoryRegion), 0);
    // write mem region
    my_write(outfile_fd, m.startAddr, m.seg_size, 0);
  }
}

void sig_handler(int sig) {
  
  int out_fd = open("./myckpt", O_WRONLY|O_CREAT, S_IRUSR);
  if (out_fd < 0) {
    print("output file opening failed.\n");
  }

  int restart = 1;
  save_memory_regions(out_fd);
  restart = 0;

  // save context 
  ucontext_t c;
  getcontext(&c);

  // I am restored, I'll just return from signal handler.
  if (restart) { return; }

  // I am actually not restored
  // I therefore act normal and save my context
  my_write(out_fd, &c, sizeof(c), 0);
  close(out_fd);
}

void __attribute__((constructor)) myconstructor(){
	signal(SIGUSR2, sig_handler);
  return;
}
int main() {sig_handler(1);return 0;}