#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>


struct MemoryRegion {
  void *startAddr;
  void *endAddr;
  int isReadable;
  int isWriteable;
  int isExecutable;
};

void*
strptr(char* address)
{
  // takes a memory address string; returns the address as pointer

  char c;
  unsigned long int res = 0;
  int i = 0;
  while (1) {
    c = * (address + i++);
    if ((c >= '0') && (c <= '9')) c -= '0';
    else if ((c >= 'a') && (c <= 'f')) c -= 'a' - 10;
    else if ((c >= 'A') && (c <= 'F')) c -= 'A' - 10;
    else break;
    res = res * 16 + c;
  }
  return (void *)res;
}

struct MemoryRegion
processRegion(char* lineBuf)
{
  // take a line buffer; instantiate, populate and return a MemoryRegion
  struct MemoryRegion mr;

  mr.startAddr = strptr(strtok_r(lineBuf, "-", &lineBuf));
  mr.endAddr = strptr(strtok_r(NULL, " ", &lineBuf));
  char* privileges = strtok_r(NULL, " ", &lineBuf);

  mr.isReadable = (int)*(privileges++) == 'r';
  mr.isWriteable = (int)*(privileges++) == 'w';
  mr.isExecutable = (int)*(privileges++) == 'x';

  return mr;
}

void print(const char* s) {
  write(1, s, strlen(s));
}

int
main(int argc, char* argv[])
{
  int in_fd = open("./proc/self/maps", O_RDONLY);
  int out_fd = open("./myckpt", O_WRONLY|O_CREAT);
  if (in_fd < 0) {
    print("memory maps file opening failed.\n");
  }
  if (out_fd < 0) {
    print("output file opening failed.\n");
  }

  int read_size = 10000;
  char read_buf[read_size];
  ssize_t bytes_read = read(in_fd, read_buf, read_size);

  printf("%zd\n", bytes_read);

  char* s = read_buf;
  char* next = read_buf;

  while (1) {
    while (*next != '\n') { next++; }
    *next++ = '\0';
    if (*s == '\0') { break; }
    struct MemoryRegion m = processRegion(s);
    ssize_t written = write(out_fd, &m, sizeof(struct MemoryRegion));
    printf("%zd written", written);
    written = write(out_fd, m.startAddr, m.endAddr - m.startAddr);
    printf("%zd written", written);
    s = ++next;
  }
  
  return 0;
}
