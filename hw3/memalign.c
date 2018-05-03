#include <errno.h>
#include "malloc.h"

void* memalign(size_t alignment, size_t size) {
  if ((alignment & (alignment - 1)) != 0) {
    printf("given alignment %ld is not a power of 2\n", alignment);
    errno = EINVAL;
    return NULL;
  }

  if (size == 0) {
    return NULL;
  }

  if (alignment <= 8) {
    void* res = malloc(size);
    if (res == (void*)-1) {
      return NULL;
    }  // errno was set by malloc
    return res;
  }

  // make extra space to find alignment
  void* node = malloc(alignment + size);
  if (node == (void*)-1) {
    return NULL;
  }  // errno was set by malloc

  // find alignment (although we are wasting space, the
  // amount of wasted space won't be more than 'alignment')
  size_t push_down = (size_t)node % alignment;
  void* res = node + push_down;

  if (push_down >= sizeof(int)) {
    int* back_pointer = (int*)(res - sizeof(int));
    *back_pointer = -push_down;  // negative of push_down bytes,
                                 // need to be handled in free()
  }
  return res;
}

int posix_memalign(void** memptr, size_t alignment, size_t size) {
  if (alignment % sizeof(void*) != 0) {
    *memptr = NULL;
    return EINVAL;
  }
  *memptr = memalign(alignment, size);
  return (*memptr) ? 0 : errno;
}
