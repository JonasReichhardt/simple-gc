#pragma once

#include "assert.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "type_desc.h"

/*
 * This file contains utility structs and functions for the heap
 */

#define BLOCKS_PER_LINE 5
#define MAX_OBJECT_NAME_LEN 25
// #define VERBOSE

/* General purpose struct used for used blocks and free list */
typedef struct block {
  size_t size;
  void *mem;
  struct block *next;
} block_t;

/* Object that can be instantiated by the Heap */
typedef struct object {
  char name[MAX_OBJECT_NAME_LEN];
  int32_t *type_desc;
  struct object *next;
} object_t;

/*
 * OBJECT FUNCTIONS START
 */

object_t *get_obj_by_name(object_t *head, char *name) {
  for (object_t *obj = head;; obj = obj->next) {
    if (strcmp(obj->name, name) == 0) {
      return obj;
    }
    if (obj->next == NULL) {
      continue;
    }
  }
  return NULL;
}

object_t *get_obj_by_type(object_t *head, int32_t *type) {
  for (object_t *obj = head;; obj = obj->next) {
    // equal if they point to the same area
    if (obj->type_desc == type) {
      return obj;
    }
    if (obj->next == NULL) {
      return NULL;
    }
  }
  return NULL;
}

void object_list_free(object_t *head) {
  if (head == NULL) {
    return;
  }
  object_t *temp;
  while (head != NULL) {
    temp = head->next;
    free(head->type_desc);
    free(head);
    head = temp;
  }
}

void print_type_list(object_t *head) {
  printf("Registered types:\n");
  for (object_t *obj = head;; obj = obj->next) {
    printf("Name[%s]-", obj->name);
    print_type_desc(obj->type_desc);
    if (obj->next == NULL) {
      continue;
    }
  }
  printf("\n");
}

/*
 * OBJECT FUNCTIONS END
 */

/*
 * BLOCK LIST FUNCTIONS
 */

/* frees all blocks of the linked list */
void block_list_free(block_t *head) {
  if (head == NULL) {
    return;
  }
  // if free list is empty only a self referenced head exists
  if (head == head->next) {
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

/* Print a raw representation of the given block list */
void block_list_print(block_t *head) {
  if (head == NULL) {
    return;
  }
  size_t i = 0;
  printf("Block list:\n");
  for (block_t *block = head;; block = block->next, i++) {
    printf("[%p]-[%lx]-[%p]->", block->mem, block->size,
           (int8_t *)block->mem + block->size);
    if (i >= BLOCKS_PER_LINE) {
      i = 0;
      printf("\n");
    }
    if (block->next == head) {
      printf("\n");
      return;
    }
  }
}

/* prints the first 64bit value of the block */
/* when given a used list it prints all type desc pointers */
void block_list_print_values(block_t *head) {
  if (head == NULL) {
    return;
  }
  size_t i = 0;
  printf("Type descriptors:\n");
  for (block_t *block = head;; block = block->next, i++) {
    if (block->mem == NULL) {
      continue;
    }
    printf("[%lx]->", *(uint64_t *)block->mem);
    if (i >= BLOCKS_PER_LINE) {
      i = 0;
      printf("\n");
    }
    if (block->next == head) {
      printf("\n");
      return;
    }
  }
}

/* removes block from list and return previous block or NULL */
block_t *block_list_remove(block_t *head, block_t *block) {
  if (head == NULL || block == NULL) {
    assert(false && "HEAP: Head or block is NULL pointer ;)");
  }
  // empty list
  if (head->next == head) {
    return NULL;
  }
  block_t *prev = head;
  for (block_t *cur = head->next; cur != head; prev = cur, cur = cur->next) {
    if (cur == block) {
      prev->next = block->next;
      block->next = NULL;
      return prev;
    }
  }
  return NULL;
}

/* adds a block to the end of the linked list */
void block_list_add_back(block_t *head, block_t *block) {
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

/* adds a block to the front of the linked list */
void block_list_add_front(block_t *head, block_t *block) {
  if (head == NULL || block == NULL) {
    assert(false && "HEAP: Head or block is NULL pointer)");
  }
  block_t *temp = head->next;
  head->next = block;
  block->next = temp;
}

/* adds a block to the list sorted by referenced memory address */
void block_list_add_sorted(block_t *head, block_t *block) {
  if (head == NULL || block == NULL) {
    assert(false && "HEAP: Head or block is NULL pointer)");
  }
  block_t *prev = head;
  // empty list
  if (head->next == head) {
    block_list_add_front(head, block);
    return;
  }
  for (block_t *cur = head->next;; prev = cur, cur = cur->next) {
    if (cur == head) {
      // no block to merge found
      // block is added to the front to be reallocated first
      block_list_add_back(head, block);
      return;
    }
    if (block->mem < cur->mem) {
      prev->next = block;
      block->next = cur;
      return;
    }
  }
}

/* merges memory-adjacent blocks in the given list */
void block_list_merge(block_t *head) {
  for (block_t *cur = head->next; cur != head; cur = cur->next) {
    if (cur->next == head) {
      return;
    }
    if ((uint8_t *)cur->mem + cur->size == (uint8_t *)cur->next->mem) {
#ifdef VERBOSE
      printf("HEAP: merged free blocks with size [%lu]+[%lu] to new block "
             "[%lu] @ [%p]\n",
             cur->size, cur->next->size, cur->size + cur->next->size, cur->mem);
#endif
      block_t *temp = cur->next;
      cur->size += cur->next->size;
      cur->next = cur->next->next;
      free(temp);
    }
  }
}

/* instantiate a new block with given values */
block_t *block_list_new(void *mem, size_t size) {
  block_t *block = (block_t *)malloc(sizeof(block_t));
  if (block == NULL) {
    assert(false && "HEAP: Could not allocate block for list");
  }
  block->size = size;
  block->mem = mem;
  return block;
}

/*
 * BLOCK LIST FUNCTIONS END
 */
