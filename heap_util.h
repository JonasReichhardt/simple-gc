#pragma once

#include "assert.h"
#include "stdlib.h"

/*
 * This file contains some utility for the heap
 */

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
  block_t *tmp;

  while (head != NULL) {
    tmp = head;
    head = head->next;
    free(tmp);
  }
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
