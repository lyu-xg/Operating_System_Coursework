#include <assert.h>
#include "malloc.h"
#include "utils.h"

void find_buddy(MemoryNode* node);

void unite(MemoryNode* a, MemoryNode* b) {
  // given two buddies, merge them into one block
  assert(a < b);
  assert(a);
  assert(b);

  if (a->size == b->size) {
    connect(a, next(b));
    a->size += b->size;
    find_buddy(a);
  }
}

void find_buddy(MemoryNode* node) {
  if (node->size >= PAGE_SIZE) {
    // we do not care merging blocks that are bigger than page size
    pthread_mutex_unlock(&malloc_lock);
    return;
  }

  // the REASON why we must do page alignment when extending heap:
  // to LOCATE BUDDIES
  int in_block_offset = (int)((unsigned long int)node % PAGE_SIZE);
  int buddy_on_left = in_block_offset % (node->size * 2);
  MemoryNode* buddy = (buddy_on_left) ? node - node->size / MEM_NODE_OVERHEAD
                                      : node + node->size / MEM_NODE_OVERHEAD;

  if (is_free(buddy)) {
    if (buddy_on_left) {
      unite(buddy, node);
    } else {
      unite(node, buddy);
    }
  }
  pthread_mutex_unlock(&malloc_lock);
  return;
}

void free(void* ptr) {
  MemoryNode* node = (MemoryNode*)(ptr - MEM_NODE_OVERHEAD);
  // given pointer out of range
  if ((void*)Start > ptr || ptr > heap_end) {
    return;
  }
  // given pointer not pointing to a possible mem block
  if ((unsigned long)node % 16 != 0) {
    return;
  }

  // handling back_pointers created by memalign
  if (node->offset < 0) {
    free(ptr + node->offset);
  }

  if (node->size < 16) {
    print_mem_block(node);
  }
  assert(node->size >= 16);
  pthread_mutex_lock(&malloc_lock);

  // insert `node` back into our linked list
  MemoryNode* cur = Head;
  MemoryNode* prev = NULL;

  if (node < cur) {
    // when node appears to be ahead of 'Head'
    Head = node;
    connect(node, cur);
    find_buddy(node);
    return;
  }

  while (1) {
    // found node in between two blocks
    if (prev < node && node < cur) {
      connect(prev, node);
      connect(node, cur);
      find_buddy(node);
      return;
    }
    // found node outside last free block
    if (cur->offset == 0) {
      connect(cur, node);
      node->offset = 0;
      find_buddy(node);
      return;
    }
    prev = cur;
    cur = next(cur);
  }
}