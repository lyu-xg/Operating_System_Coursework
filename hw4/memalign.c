#include <errno.h>
#include "malloc.h"
#include "utils.h"

void* memalign(size_t alignment, size_t size) {
  if ((alignment & (alignment - 1)) != 0) {
    print("given alignment ");printi(alignment);print(" is not a power of 2\n");
    errno = EINVAL;
    return NULL;
  }

  if (size == 0) {
    return NULL;
  }

  if (alignment <= MEM_NODE_OVERHEAD) {
    void* res = malloc(size);
    if (res == (void*)-1) {
      return NULL;
    }  // errno was set by malloc
    return res;
  }

  // make extra space to find alignment
  void* node = malloc(alignment*2 + size);
  if (node == (void*)-1) {
    return NULL;
  }  // errno was set by malloc

  // find alignment (although we are wasting space, the
  // amount of wasted space won't be more than 'alignment')
  size_t push_down = (size_t)node % alignment;

  if (push_down < MEM_NODE_OVERHEAD) {
    push_down += alignment;
  }

  void* res = node + push_down;
  
  MemoryNode* mask_node = (MemoryNode*)(res - sizeof(MemoryNode));
  mask_node->size = -1;
  mask_node->next = node;
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
