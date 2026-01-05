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

// how many free list blocks should be printed per line during dumping
#define BLOCKS_PER_LINE 5
#define MAX_OBJECT_NAME_LEN 25

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
 * OBJECT FUNCTIONS
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

void free_object_list(object_t *head) {
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
 * BLOCK LIST FUNCTIONS
 */

/* frees all blocks of the linked list */
void free_block_list(block_t *head) {
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

/* removes block if present in the list */
void block_list_remove(block_t *head, block_t *block) {
  if (head == NULL || block == NULL) {
    assert(false && "HEAP: Head or block is NULL pointer ;)");
  }
  // empty list
  if (head->next == head) {
    return;
  }
  block_t *prev = head;
  for (block_t *cur = head->next; cur != head; block = block->next) {
    if (cur == block) {
      prev->next = block->next;
      block->next = NULL;
      return;
    }
  }
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
  // empty list
  if (head->next == head) {
    block_list_add_back(head, block);
    return;
  }
  for (block_t *cur = head->next;; cur = cur->next) {
    if (cur == head) {
      // no block to merge found
      // block is added to the front to be reallocated first
      block_list_add_back(head, block);
      return;
    }
    if ((size_t)block->mem < (size_t)cur->mem) {
      block->next = cur->next;
      cur->next = block;
      return;
    }
  }
}

/* merges memory-adjacent blocks in the given list */
void block_list_merge(block_t *head) {
  /*for(block_t *cur=head->next;cur!=head;cur=cur->next){
      printf("cur [%p]-[%p]\n",(uint8_t*)cur->mem,(uint8_t*)cur->mem+cur->size);
      printf("block
  [%p]-[%p]\n",(uint8_t*)block->mem,(uint8_t*)block->mem+block->size); block_t
  *dealloc_b = NULL; block_t *resize_b = NULL;
      if((uint8_t*)cur->mem+cur->size==(uint8_t*)block->mem){
          resize_b = cur;
          dealloc_b = block;
      }else if((uint8_t*)block->mem+block->size==(uint8_t*)cur->mem){
          resize_b = cur;
          dealloc_b = block;
      }
  }*/
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

size_t print_free_blocks_fancy(block_t *fblock) {
  printf("Free list:\n");
  size_t free_bytes = 0;
  size_t i = 0;
  for (block_t *block = fblock->next; block != fblock; block = block->next) {
    printf("[%lu]@[%p]<--", block->size, (void *)block);
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

size_t print_used_blocks_fancy(block_t *ublock, object_t *objs) {
  printf("Allocated \"Objects\"\n");
  size_t used_bytes = 0;
  size_t i = 0;
  for (block_t *block = ublock->next; block != ublock; block = block->next) {
    uint64_t *obj_start = ((uint64_t *)(((size_t *)block->mem) + 1));
    object_t *obj =
        get_obj_by_type(objs, (int32_t *)MASK(*(size_t *)block->mem));
    if (obj == NULL) {
      assert(false && "Could not type indentifier during printing");
    }
    printf("[%lub]@[%p] Name:[%s] Data:[0x%lx]\n", block->size,
           (void *)obj_start, obj->name, *obj_start);
    used_bytes += block->size;
    i++;
  }
  printf("\n\nLive objects: %lu bytes\n", used_bytes);
  return used_bytes;
}

void print_marked_blocks(block_t *ublock, object_t *objects) {
  printf("HEAP: Marked objects after gc mark:\n");
  for (block_t *block = ublock->next; block != ublock; block = block->next) {
    if (TAGGED_TYPE(*(size_t *)block->mem)) {
      object_t *obj =
          get_obj_by_type(objects, (int32_t *)MASK(*(size_t *)block->mem));
      if (obj != NULL) {
        printf("HEAP: Marked object %s @ %p\n", obj->name,
               (void *)(((uint64_t *)block->mem) + 1));
      }
    }
  }
}
