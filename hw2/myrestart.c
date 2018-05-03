#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "utils.h"

char ckpt_image_filename[1000];
ucontext_t context;
struct MemoryRegion old_stack;

void save_filename(char* arg_filename) {
  // straight copy absolute path.
  if (*arg_filename == '/' || *arg_filename == '.') {
    strcpy(ckpt_image_filename, arg_filename);
  } else {
    // implicit path, we add "./" prefix
    *ckpt_image_filename = '.';
    *(ckpt_image_filename + 1) = '/';
    strcpy(ckpt_image_filename+2, arg_filename);
  }
}

struct MemoryRegion find_stack()
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
    struct MemoryRegion m;
    if (strstr(s, "[stack") != NULL) {
      return processRegion(s);
    }
    s = next; // move s to next line
  }
}



restore_memory() {
  int ckpt_fd = open(ckpt_image_filename, O_RDONLY);
  if (ckpt_fd < 0) {
    print("checkpoint file fail to open.");
    exit(1);
  }

  size_t filesize = get_file_size(ckpt_image_filename);
  size_t read_already = 0;

  struct MemoryRegion m;

  while ((filesize - read_already) != sizeof(ucontext_t)) {
    // read mem header
    read_already += read_exact(ckpt_fd, sizeof(struct MemoryRegion), &m, 0);

    // map memory pages, give rwx(7) for now.
    if (
      mmap(m.startAddr, m.seg_size, 7, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0)
      != m.startAddr
    ) { 
      print("failed to map memory"); exit(1); 
    }
    
    // fill memory pages with contents from checkpoint
    read_already += read_exact(ckpt_fd, m.seg_size, m.startAddr, 0);

    // restore the privileges
    mprotect(m.startAddr, m.seg_size, m.priv);
  }
  // end-of-loop invariant: the only bytes left in the file is the content.

  // read the last thing: the saved context
  // this is where the side effect comes in.
  read_exact(ckpt_fd, sizeof(ucontext_t), &context, 0);
  close(ckpt_fd);
}


void restore() {
  // unmap the old stack
  if (munmap(old_stack.startAddr, old_stack.seg_size) < 0){
    print("unmap stack failed."); exit(1);
  }

  // copy memory pages from checkpoint
  // SIDE EFFECT: populate context from checkpoint file
  restore_memory();

  // hello, old friend.
  setcontext(&context);

}



int main(int argc, char *argv[]) {
  if (argc < 2) {
    print("Usage: ./restart myckpt"); exit(0);
  } 
  save_filename(argv[1]);

  // map some memeory for the new stack
  void* new_stack = mmap(strptr("5300000"), 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
  new_stack += 4096;

  // after switching stack pointer, old stack is no longer marked as "[stack]"
  // need to save old stack address in data segment.
  old_stack = find_stack();

  asm volatile ("mov %0,%%rsp" : : "g" (new_stack) : "memory");
  restore();
}