#include <assert.h>
#include <unistd.h>
#include "malloc.h"
#include "utils.h"

MemoryNode* find_buddy(MemoryNode* node);

MemoryNode* prev(MemoryNode* node){
  // return prev_node, if node is head, return NULL
  MemoryNode* cur = Arena->head;
  MemoryNode* prv = NULL;
  while (cur != node) {
    prv = cur;
    cur = cur->next;
  }
  return prv;
}


void sink(MemoryNode* node){
  // PURE SIDE EFFECT: remove `node` from free list
  MemoryNode* prv = prev(node);
  if (prv) {
    prv->next = node->next;
  } else {
    Arena->head = node->next;
  }
  
}

MemoryNode* find_buddy(MemoryNode* node) {
  // SIDE EFFECT: remove buddies from free list
  // RETURNS: a MemoryNode which is the end product of Merging
  // NOTE: Merging might or might not happen

  // we do not merge blocks that are bigger than page size
  if (node->size >= PAGE_SIZE) { return node; }

  int in_block_offset = (int)((unsigned long int)node % PAGE_SIZE);
  int buddy_on_left = in_block_offset % (node->size * 2);
  MemoryNode* buddy = (buddy_on_left) ? (MemoryNode*)((void*)node - node->size)
                                      : (MemoryNode*)((void*)node + node->size);

  // if (!(buddy->size >= 16 && (buddy->size & (buddy->size - 1)) == 0)) {
  //   print_mem_block_mini(buddy);print("\n");
  //   print_mem_block_mini(node);print("\n");
  // }
  assert(buddy->size >= 16 && (buddy->size & (buddy->size - 1)) == 0);
  assert(node->size >= 16 && (node->size & (node->size - 1)) == 0);

  if (buddy->size == node->size && is_free(buddy)) {
    sink(buddy); Arena->free_blk--;
    if (buddy_on_left) { node = buddy; }
    node->size *= 2;
    return find_buddy(node);
  }
  return node;
}

void free(void* ptr) {
  // return;

  if (ptr == 0) {
    // print("null pointer!\n");
    return;
  }

  pthread_mutex_lock(&(Arena->lock));
  MemoryNode* node = (MemoryNode*)(ptr - MEM_NODE_OVERHEAD);
  Arena->free_req++;
  Arena->free_blk++;Arena->used_blk--;

  // given pointer not pointing to a possible mem block
  if ((unsigned long int)node % MEM_NODE_OVERHEAD||(unsigned long int)node % MEM_NODE_OVERHEAD == 0) {
    // printf("bad pointer %p\n",ptr);
    pthread_mutex_unlock(&(Arena->lock));
    return;
  }

  // handling back_pointers created by memalign TODO
  if (node->size < 0) {
    // unmask the mask
    node = node->next;
  }
  

  assert(node->size > MEM_NODE_OVERHEAD);
  
  MemoryNode* combined = find_buddy(node);
  make_head(combined);
  // print("combined being ");printp(combined);print("\n");
  // print("Arena->head: ");printp(Arena->head);print("\n");
  pthread_mutex_unlock(&(Arena->lock));
  return;
}