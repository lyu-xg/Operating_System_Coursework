#include <string.h>
#include "malloc.h"

// The  calloc()  function allocates memory for an array of nmemb elements
// of size bytes each and returns a pointer to the allocated memory.   The
// memory  is  set  to ZERO.  If nmemb or size is 0, then calloc() returns
// either NULL.

void* calloc(size_t nmemb, size_t size) {
  // printf("callocing %zd x %zd\n", nmemb, size);
  size_t t = nmemb * size;
  if (!t) {
    return 0;
  }
  void* res = malloc(t);
  if (res == (void*)-1) {
    return NULL;
  }
  memset(res, 0, t);
  return res;
}