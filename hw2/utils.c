#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "utils.h"

struct MemoryRegion
processRegion(char* lineBuf)
{
  // take a line buffer; instantiate, populate and return a MemoryRegion
  struct MemoryRegion mr;

  mr.startAddr = strptr(strtok_r(lineBuf, "-", &lineBuf));
  mr.seg_size = strptr(strtok_r(NULL, " ", &lineBuf)) - mr.startAddr;
  char* privileges = strtok_r(NULL, " ", &lineBuf);

  int isReadable   = *(privileges) == 'r' ? 1 : 0;
  int isWriteable  = *(privileges+1) == 'w' ? 2 : 0;
  int isExecutable = *(privileges+2) == 'x' ? 4: 0;

  // hardcoding, set vsyscall to not readable. hence ignoring it.
  if (strstr(lineBuf, "vsyscall") != NULL) { isReadable = 0; }

  mr.priv = isReadable|isWriteable|isExecutable;
  return mr;
}

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

size_t get_file_size(const char* filename) {
  struct stat st;
  stat(filename, &st);
  return st.st_size;
}


void print(const char* s) {
  write(1, s, strlen(s));
  write(1, "\n", 2);
}


// "has jujube, no jujube, hit three times" magic

void my_read(const char* filename, size_t readbyte, char* buf, int retry) {
  if (retry > 3) {
    print("read failed after 3rd retry");
    exit(1);
  }
  
  int fd = open(filename, O_RDONLY);
  if (fd < 0) { return my_read(filename, readbyte, buf, retry + 1); }
  
  ssize_t bytesread = read(fd, buf, readbyte);
  if ( bytesread < 0) {
    return my_read(filename, readbyte, buf, retry + 1);
  }

  // if buf is not filled up by the read,
  // end the buf with null
  if (bytesread < readbyte) {
    buf[bytesread] = '\0';
  }
  close(fd);
}

ssize_t read_exact(int fd, size_t nbyte_to_read, void* buf, int retry) {
  // read exactly 'nbyte to read', FAIL on insufficient btye from fd
  if (retry > 3) {
    print("read_exact failed after 3rd retry");
    exit(1);
  }
  ssize_t nbyte_read = read(fd, buf, nbyte_to_read);
  if (nbyte_read < nbyte_to_read) {
    return nbyte_read + read_exact(fd, nbyte_to_read-nbyte_read, buf+nbyte_read, retry+1);
  }
  return nbyte_read;
}


ssize_t my_write(int fd, const void* buf, size_t nbyte, int retry) {
  if (retry > 3) {
    print("write could't write everything after 3rd retry");
    exit(1);
  }
  ssize_t written = write(fd, buf, nbyte);
  if (written == nbyte) {
    return written;
  }
  else if (written < 0){
    return my_write(fd, buf, nbyte, retry+1);
  } else if (written < nbyte) {
    return written + my_write(fd, buf+written, nbyte-written, retry+1);
  } else {
    print("write failed at random (which happens btw)");
    exit(errno);
  }
}
