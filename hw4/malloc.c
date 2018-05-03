#include "malloc.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include "utils.h"

__thread ArenaNode* Arena = NULL;
int arena_ct = 0;
size_t total_alloc_size = 0;

// wrapper function for calling sbrk
MemoryNode* request_memory(size_t size) {
  assert(size >= PAGE_SIZE);
  assert(size % PAGE_SIZE == 0);

  MemoryNode* block_new = mmap(0, size, 7, MAP_PRIVATE|MAP_ANON, -1, 0);
  if (block_new == (void*)-1) {
    printf("mmap failed\n");
    printf("errno%d\n", errno);
    return block_new;
  }  // malloc handles error

  // stats
  total_alloc_size += size;
  Arena->free_blk++; Arena->blk++;

  block_new->size = (unsigned int)size;
  block_new->next = 0;
  block_new->arena_id = Arena->id;
  // assert((unsigned long)block_new % PAGE_SIZE == 0);
  return block_new;
}

// defined in free.c
void sink(MemoryNode* node);

void* fit(MemoryNode* node, size_t nbytes) {
  // fit `nbytes` in `node`
  // print("fit ");printi(nbytes);print(" into ");printp(node);print("\n");
  
  assert(node->size >= nbytes);
  if(nbytes == node->size) {
    sink(node);
    Arena->used_blk++; Arena->free_blk--;
    pthread_mutex_unlock(&(Arena->lock));
    return ((void*)node) + MEM_NODE_OVERHEAD;
  } else {
    MemoryNode* new_buddy = (MemoryNode*)((void*)node + node->size/2);
    new_buddy->size = node->size/2;
    node->size = node->size/2;
    new_buddy->arena_id = Arena->id;
    Arena->blk++; Arena->free_blk++;
    make_head(new_buddy);
    return fit(new_buddy, nbytes);
  }
}

void* malloc(size_t query) {
  if (Arena == NULL) {
    initialize_arena();
  }
  // assert (Arena->blk == Arena->free_blk + Arena->used_blk);
  // print("malloc ");printi(query);print("\n");
  // print_free_list();
  // print_arena_stats(Arena);
  // print("\n\n\n");
  size_t bytes_needed = round_up(query + MEM_NODE_OVERHEAD);
  pthread_mutex_lock(&(Arena->lock));
  Arena->allo_req++;

  // MemoryNode* perv_node = NULL;
  MemoryNode* cur = Arena->head;
  while (cur) {
    if (cur->size >= bytes_needed) {
      return fit(cur, bytes_needed);
    }
    cur = cur->next;
  }

  MemoryNode* new_block = request_memory(round_to_page(bytes_needed));
  if (new_block == (void*)-1) {
    pthread_mutex_unlock(&(Arena->lock));
    errno = ENOMEM;
    return NULL;
  }
  make_head(new_block);
  return fit(new_block, bytes_needed);
}


// void malloc_and_free() {
//   int total = 2;

//   void** ppool = malloc(total * sizeof(void*));
  
//   // print_free_list();

//   // int cur = 0;
//   int i = 0;
//   while (i < total) {
//     *(ppool + i) = malloc(100);
//     print("got ");printp(*(ppool+i));print("from malloc\n");
//     // sleep(2);
//     // free(p);
//     i++;
//     print_free_list();
//   }

//   print("starting to free\n\n\n");

//   i = 0;
//   while (i < total) {
//     print("freeing ");printp(*(ppool+i));print("\n");
//     free(*(ppool+i));
//     print_free_list();
//     i++;
//   }

//   free(ppool);
//   print_free_list();
// }

// #include <string.h>
// void* main2() {
//   sleep(3);
//   print("forking.");
//   int pid = fork();
//   if (pid) {
//     print("parent\n");
//     char * b = (char*)malloc(100);
//     strcpy(b, "some\0");
//     print("parent went through");
//   }
//   else {
//     print("child\n");
//     char * b = (char*)malloc(100);
//     strcpy(b, "some\0");
//     print("child went through");
//   }
  
//   // char * b = (char*)malloc(100);
//   // strcpy(b, "some\0");
//   // print(b); print("\n");
//   return 0;
// }


// // //   pthread_t second_thread;
// // //   pthread_create(&second_thread,0,main2,0);
// // //   while (1) {
// // //     malloc(100);
// // //   }



// #include <string.h>
// int main() {

//   char * a = (char*)malloc(100);
//   strcpy(a, "some");
//   printp(a);print(" is the first 100\n");
  
//   // print_free_list();

//   char * b = (char*)malloc(100);
//   strcpy(b, "some");
//   print_free_list();

//   free(b);
//   b = (char*)malloc(100);
//   strcpy(b, "some");

//   // print_free_list();

//   char * c = (char*)malloc(1000);
//   strcpy(c, "some");

//   print_free_list();

//   free(b); print("freeing the 100\n");
//   print_free_list();

//   free(a);print("freeing the other 100\n");
//   print_free_list();


//   free(c);print("freeing the 1000\n");
//   print_free_list();
//   free(c);

//   print_arena_stats(Arena);

//   return 0;
// }


#include "test1.c"
