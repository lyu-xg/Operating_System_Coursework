#include "utils.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "itoa.h"

long PAGE_SIZE;
long CORE_COUNT;
pthread_mutexattr_t MUTEX_ATTR;
pthread_mutex_t arena_list_lock;

int MEM_NODE_OVERHEAD = sizeof(MemoryNode);


ArenaNode* ArenaList = NULL;
ArenaNode temp_arena;

void fork_prepare() {
  // to prepare for fork, we (the forking thread) need to acquire the locks
  if (Arena == NULL) {
    initialize_arena();
  }
  ArenaNode* cur = ArenaList;
  while (cur < ArenaList + CORE_COUNT) {
    pthread_mutex_lock(&(cur->lock)); cur++;
  }
  pthread_mutex_lock(&(arena_list_lock));
}

void after_fork() {
  // parent_p and chile_p both need after_fork to release the locks
  ArenaNode* cur = ArenaList;
  while (cur < ArenaList + CORE_COUNT) {
    pthread_mutex_unlock(&(cur->lock)); cur++;
  }
  pthread_mutex_unlock(&(arena_list_lock));
}


void __attribute__((constructor)) myconstructor() {
  pthread_mutexattr_settype(&MUTEX_ATTR, PTHREAD_MUTEX_RECURSIVE);
  assert(pthread_mutex_init(&arena_list_lock, &MUTEX_ATTR) == 0);
  PAGE_SIZE = sysconf(_SC_PAGESIZE);
  CORE_COUNT = sysconf(_SC_NPROCESSORS_ONLN);
  assert(CORE_COUNT > 0);

  pthread_atfork(fork_prepare, after_fork, after_fork);

  return;
}


ArenaNode* create_arena(ArenaNode* a) {
  // for other fields, no need to init, calloc already set them to 0.
  assert(pthread_mutex_init(&(a->lock), &MUTEX_ATTR) == 0);
  return a;
}

void initialize_arena() {
  // called when thread Arena is NULL
  pthread_mutex_lock(&arena_list_lock);
  if (ArenaList == NULL) {
    // first Arena, we need to init ArenaList as well
    // initialize arena list
    Arena = create_arena(&temp_arena); // having an Arena enables malloc to work
    ArenaList = calloc(CORE_COUNT, sizeof(ArenaNode));
    // printp(ArenaList);
    // print("after creating Arena LIST from temp ");print_arena(&temp_arena);
    // print("ArenaList created at ");printp(ArenaList);print("\n");
    int i = 1;
    while (i < CORE_COUNT) {
      create_arena(ArenaList + i++);
    }
    print("after init Arena LIST complete ");print_arena(&temp_arena);
    *ArenaList = *Arena;
    Arena = ArenaList;
    
  } else {
    Arena = ArenaList + arena_ct % CORE_COUNT;
  }
  arena_ct++;
  pthread_mutex_unlock(&arena_list_lock);
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

// MemoryNode* next(MemoryNode* node) {
//   return (node != NULL && node->offset) ? node + (node->offset / sizeof(MemoryNode)) : NULL;
// }

MemoryNode* beside(MemoryNode* node) {
  return node + (node->size) / sizeof(MemoryNode);
}

void make_head(MemoryNode* a) {
  a->next = Arena->head;
  Arena->head = a;
}


int is_free(MemoryNode* node) {
  // not thread safe
  // return true iff given node is inside the free list
  MemoryNode* cur = Arena->head;
  while (cur) {
    if (cur == node) { return 1; }
    cur = cur->next;
  }
  return 0;
}

void print(char* s) {
  write(1, s, strlen(s));
}

void printi(int i) {
  char buf[128];
  print(itoa(i, buf, 10));
}

void printp(void* p) {
  char buf[128];
  print(itoa((long int)p, buf, 16));
}

void lb() { print("\n"); }

void print_mem_block(MemoryNode* n) {
  if (n == NULL) {
    printf("%p\n", NULL);
  } else {
    // printf("block at %p with size %d with offset %d\n", n, n->size, n->offset);
  }
}


void print_mem_block_mini(MemoryNode* n) {
  assert(n);
  printp((void*)n);
  print("(");
  printi(n->size);
  print(") -> ");
}

void print_free_list() {
  MemoryNode* cur = Arena->head;
  while (cur) {
    print_mem_block_mini(cur);
    cur = cur->next;
  }
  print("END\n\n");
}

void print_arena(ArenaNode* a) {
  print("Arena at ");printp(a);
  print(" with a free list of ");print_free_list();
  print("\n");
}

void malloc_stats() {
  int i = 0;
  ArenaNode* cur = ArenaList;
  while (cur < ArenaList + CORE_COUNT) {
    print("Arena ");printi(i++);lb();
    print_arena_stats(cur++);lb();
  }
  print("Total: ");lb();
  print("total size of arenas:   ");printi(total_alloc_size);lb();
  print("total number of arenas: ");printi((arena_ct>CORE_COUNT)?CORE_COUNT:arena_ct);lb();
}

void print_arena_stats(ArenaNode* a) {
  print("total number of blocks: "); printi(a->blk); lb();
  print("used blocks:            "); printi(a->used_blk); lb();
  print("free blocks:            "); printi(a->free_blk); lb();
  print("total alloc requests:   "); printi(a->allo_req); lb();
  print("total free requests:    "); printi(a->free_req); lb();
}