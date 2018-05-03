#include <pthread.h>
#include <stdio.h>

typedef struct MemoryNode {
  unsigned int size;
  struct MemoryNode* next;
  short arena_id;
} MemoryNode;

typedef struct ArenaNode {
	pthread_mutex_t lock;
	MemoryNode* head;
	short id;
	int blk;
	int used_blk;
	int free_blk;
	int allo_req;
	int free_req;
} ArenaNode;

// global vars defined in utils.c
extern int MEM_NODE_OVERHEAD;
extern long PAGE_SIZE;
// extern __thread pthread_mutex_t malloc_lock;

extern pthread_mutex_t arena_list_lock;

// global vars defined in malloc.c
// extern __thread MemoryNode* Head;
extern __thread ArenaNode* Arena;
extern int arena_ct;
extern size_t total_alloc_size;

void initialize_arena();
// ArenaNode* create_arena(ArenaNode* a);

// pthread_mutex_t free_lock;
size_t round_up(size_t size);
size_t round_to_page(size_t size);

MemoryNode* next(MemoryNode* node);

MemoryNode* beside(MemoryNode* node);

// void split(MemoryNode* node);

void make_head(MemoryNode* a);

int is_free(MemoryNode* node);

void print(char* s);
void printp(void* p);
void printi(int i);
void print_arena(ArenaNode* a);
void print_mem_block(MemoryNode* m);

void print_mem_blocks();

void print_mem_block_mini(MemoryNode* n);
void print_free_list();

void print_stats();
void print_arena_stats();
