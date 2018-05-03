#include <pthread.h>
#include <stdio.h>

typedef struct MemoryNode {
  unsigned int size;
  int offset;
} MemoryNode;

// global vars defined in utils.c
extern int MEM_NODE_OVERHEAD;
extern long PAGE_SIZE;
extern pthread_mutex_t malloc_lock;

// global vars defined in malloc.c
extern MemoryNode* Head;
extern MemoryNode* Start;
extern void* heap_end;

// pthread_mutex_t free_lock;
size_t round_up(size_t size);
size_t round_to_page(size_t size);

MemoryNode* next(MemoryNode* node);

MemoryNode* beside(MemoryNode* node);

void split(MemoryNode* node);

void connect(MemoryNode* a, MemoryNode* b);

int is_free(MemoryNode* node);

void print_mem_block(MemoryNode* m);

void print_mem_blocks();

void print_free_list();
