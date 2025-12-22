#pragma once

#include "assert.h"
#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"

/*
 * This file contains some function utility for the heap
 */

 // how many free list blocks should be printed per line during dumping
 #define BLOCKS_PER_LINE 3

typedef struct block {
  size_t size;
  void *mem;
  struct block *next;
} block_t;

/*
 * frees all blocks of the linked list
 */
void block_list_free(block_t *head) {
  if (head == NULL) {
    assert(false && "HEAP: Cannot free NULL pointer ;)");
  }
  // if free list is empty only a self referenced head exists
  if(head==head->next){
      free(head);
      return;
  }

  block_t *tmp;
  block_t *block = head->next; // skip dummy block

  while (block->next != head) {
    tmp = block;
    block = block->next;
    free(tmp);
  }
  // free the last two elements manually
  free(block);
  free(head);
}

/*
 * adds a block to the end of the linked list
 */
void block_list_add(block_t *head, block_t *block) {
  if (head == NULL || block == NULL) {
    assert(false && "HEAP: Head or block is NULL pointer ;)");
  }
  for (block_t *temp = head;; temp = temp->next) {
    if (temp->next == head) {
      temp->next = block;
      block->next = head;
      return;
    }
  }
}

/*
 * adds a block to the end of the linked list
 */
void used_list_add(block_t *head, block_t *block) {
  if (head == NULL || block == NULL) {
    assert(false && "HEAP: Head or block is NULL pointer ;)");
  }
  block_t* temp = head->next;
  head->next = block;
  block->next = temp;
}

/*
 * creates a new block and adds it to the end of the list
 * also returns a reference to the new block
 */
block_t *block_list_add_new(block_t *head, void *mem, size_t size) {
  if (head == NULL) {
    assert(false && "HEAP: Head is NULL pointer ;)");
  }
  block_t *block = (block_t *)malloc(sizeof(block_t));
  if (block == NULL) {
    assert(false && "HEAP: Could not allocate block for list");
  }
  block->size = size;
  block->mem = mem;
  block_list_add(head, block);
  return block;
}

size_t print_free_blocks(block_t* fblock) {
  printf("Free list:\n");
  size_t free_bytes = 0;
  size_t i = 0;
  for (block_t *block = fblock->next; block != fblock; block = block->next) {
    printf("[%lu]@[%p]<--", block->size, (void*)block);
    free_bytes += block->size;
    i++;
    if (i == BLOCKS_PER_LINE) {
      printf("\n");
      i = 0;
    }
  }
  printf("\n\nFree heap space: %lu bytes\n", free_bytes);
  return free_bytes;
}

size_t print_used_blocks(block_t *ublock) {
  printf("Allocated \"Objects\"\n");
  size_t used_bytes = 0;
  size_t i = 0;
  for (block_t *block = ublock->next; block != ublock; block = block->next) {
    printf("[%lub]@[%p] Name: [TODO] Data: [0x%lx]\n", block->size, (void*)block,
           *((uint64_t *)block->mem));
    used_bytes += block->size;
    i++;
  }
  printf("\n\nLive objects: %lu bytes\n", used_bytes);
  return used_bytes;
}
