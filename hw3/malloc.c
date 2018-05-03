#include "malloc.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"

MemoryNode* Head = NULL;
MemoryNode* Start = NULL;
void* heap_end;  // heap size includes the gap between blocks
int init = 1;

// wrapper function for calling sbrk
MemoryNode* extend_heap(size_t size) {
  assert(size >= PAGE_SIZE);
  assert(size % PAGE_SIZE == 0);

  void* cur_heap = sbrk(0);  // get bottom of current heap
  if (cur_heap == (void*)-1) {
    return cur_heap;
  }  // malloc handles error
  int white_space = (int)(((unsigned long int)cur_heap) % PAGE_SIZE);

  // extend the heap
  MemoryNode* block = (MemoryNode*)sbrk(size + white_space);
  if (block == (void*)-1) {
    return block;
  }  // malloc handles error

  // maybe, we can do something with the white space, just maybe

  // create the new whole block.
  MemoryNode* block_new = block + white_space / MEM_NODE_OVERHEAD;
  heap_end = (void*)block_new + size;
  block_new->size = (unsigned int)size;
  block_new->offset = 0;
  // print_mem_block(block_new);
  assert((unsigned long)block_new % PAGE_SIZE == 0);
  return block_new;
}

void* fit(MemoryNode* node, size_t nbytes, MemoryNode* prev_node) {
  // fit `nbytes` in `node`, and connect `prev_node` with next node
  // printf("fitting query %zd into size %d\n", nbytes, node->size);

  if (node->size < nbytes) {
    // print_free_list();
    printf("fitting nbytes %zd into size %d\n", nbytes, node->size);
  }
  // print_mem_block(node);
  assert(node->size >= nbytes);
  if (nbytes == node->size) {
    if (prev_node) {
      prev_node->offset = (node->offset) ? prev_node->offset + node->offset : 0;
    } else {
      Head = (node->offset) ? node + (node->offset) / MEM_NODE_OVERHEAD : 0;
    }
    // print_mem_blocks();
    pthread_mutex_unlock(&malloc_lock);
    return ((void*)node) + MEM_NODE_OVERHEAD;
  } else {
    split(node);
    return fit(node, nbytes, prev_node);
  }
}

void* malloc(size_t query) {
  if (!query) {
    return NULL;
  }

  size_t bytes_needed = round_up(query + MEM_NODE_OVERHEAD);
  pthread_mutex_lock(&malloc_lock);
  MemoryNode* perv_node = NULL;
  MemoryNode* cur = Head;
  while (cur) {
    if (cur->size >= bytes_needed) {
      return fit(cur, bytes_needed, perv_node);
    }
    if (!cur->offset) {
      break;
    }
    perv_node = cur;
    cur = next(cur);
  }
  MemoryNode* new_block = extend_heap(round_to_page(bytes_needed));
  if (new_block == (void*)-1) {
    pthread_mutex_unlock(&malloc_lock);
    errno = ENOMEM;
    return NULL;
  }
  if (cur) {
    // cur being the tail of the linked list
    cur->offset = (unsigned int)((void*)new_block - (void*)cur);
  } else {
    // head is empty, new_block is the new head
    Head = new_block;
    if (init) {
      Start = Head;
      init = 0;
    }
  }
  return fit(new_block, bytes_needed, cur);
}
