#include "utils.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

long PAGE_SIZE;
pthread_mutex_t malloc_lock;
pthread_mutexattr_t mutexattr;

int MEM_NODE_OVERHEAD = sizeof(MemoryNode);

void __attribute__((constructor)) myconstructor() {
  pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
  assert(pthread_mutex_init(&malloc_lock, &mutexattr) == 0);
  // assert (pthread_mutex_init(&free_lock, &mutexattr) == 0);
  PAGE_SIZE = sysconf(_SC_PAGESIZE);
  return;
}

size_t round_up(size_t size) {
  size_t c = 16;
  while (c < size) {
    c *= 2;
  }
  return c;
}

// return nearest page limit in bytes WHICH IS POWER OF 2
// why POWER OF 2? to use buddies to manage
size_t round_to_page(size_t size) {
  size_t res = PAGE_SIZE;
  while (res < size) {
    res *= 2;
  }
  return res;
}

MemoryNode* next(MemoryNode* node) {
  return (node->offset) ? node + (node->offset / sizeof(MemoryNode)) : NULL;
}

MemoryNode* beside(MemoryNode* node) {
  return node + (node->size) / sizeof(MemoryNode);
}

void connect(MemoryNode* a, MemoryNode* b) {
  // connect two memory nodes
  a->offset = (b == 0) ? 0 : (void*)b - (void*)a;
}

void split(MemoryNode* node) {
  // split `node` into two buddies
  assert(node->size >= 16);
  unsigned int new_size = node->size / 2;

  node->size = new_size;
  MemoryNode* new_buddy = beside(node);
  new_buddy->size = new_size;

  new_buddy->offset = (node->offset) ? node->offset - new_size : 0;
  node->offset = new_size;
}

int is_free(MemoryNode* node) {
  // not thread safe
  // return true iff given node is inside the free list
  MemoryNode* cur = Head;
  while (cur && cur < node) {
    cur = next(cur);
  }
  // printf("isfree(): cur ended up at %p",cur);
  return (cur == node);
}

void print_mem_block(MemoryNode* m) {
  if (m == NULL) {
    printf("%p\n", NULL);
  } else {
    printf("block at %p with size %d with offset %d\n", m, m->size, m->offset);
  }
}

void print_mem_blocks() {
  MemoryNode* cur = Start;
  while ((void*)cur < heap_end) {
    // printf("cur %p, start %p, heap_end %p\n", cur, Start, heap_end);
    print_mem_block(cur);
    printf("is free %d\n", is_free(cur));
    assert(cur->size > 16);
    cur = beside(cur);
  }
}
void print_free_list() {
  MemoryNode* cur = Head;
  while (1) {
    print_mem_block(cur);
    if (cur->offset == 0) {
      break;
    }
    cur = next(cur);
  }
}
