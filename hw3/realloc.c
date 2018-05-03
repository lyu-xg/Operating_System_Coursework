#include <string.h>
#include "malloc.h"

// The  realloc()  function changes the size of the memory block pointed to
// by ptr to size bytes.  The contents will be unchanged in the range  from
// the  start of the region up to the minimum of the old and new sizes.  If
// the new size is larger than the old size, the added memory will  not  be
// initialized.   If  ptr  is  NULL,  then  the  call is equivalent to mal‐
// loc(size), for all values of size; if size is equal to zero, and ptr  is
// not NULL, then the call is equivalent to free(ptr).  Unless ptr is NULL,
// it must have been returned by an earlier call to malloc(),  calloc()  or
// realloc().  If the area pointed to was moved, a free(ptr) is done.

// some thought:  maybe do it in-place when possible.
void *realloc(void *ptr, size_t size) {
  // printf("reallocing %p size %zd\n", ptr, size);
  if (ptr == NULL) {
    return malloc(size);
  }
  if (size <= 0) {
    free(ptr);
    return NULL;
  }
  unsigned int old_size = *(unsigned int *)(ptr - 8);
  unsigned int bytes_cp = (old_size > size) ? size : old_size;
  void *res = malloc(size);
  if (res == (void *)-1) {
    return NULL;
  }
  memcpy(res, ptr, bytes_cp);
  free(ptr);
  return res;
}

void *reallocarray(void *ptr, size_t nmemb, size_t size) {
  size_t t = nmemb * size;
  if (t == 0) {
    return NULL;
  } else if (nmemb != 0 && t / nmemb != size) {
    // overflow
    return NULL;
  }
  return realloc(ptr, t);
}