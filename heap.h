#pragma once

#include "assert.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "heap_util.h"
#include "objects.h"
#include "type_desc.h"

#define HEAP_SIZEB 32768 // 32KiB
#define DEBUG true

// Heap related variables
static void *hstart;    // heap start
static void *hend;      // heap end
static block_t *fblock; // dummy block: points to the first free block
static block_t *ublock; // dummy block: points to the first free block
static bool is_init = false;

// Object related variables

// linked list with all registered objects and their type descriptors
static object_t *objects = NULL;

// Function definitions
void heap_dump();
static void *alloc(uint32_t alloc_size);

/*
 * PRIVATE METHODS
 */

// frees unmarked blocks and builds new free list
static void sweep() {
  // empty used block list
  if (ublock->next == ublock) {
    return;
  }

  for (block_t *block = ublock->next;;) {
    if (block == ublock) {
      return;
    }
    size_t tag_addr = *(size_t *)block->mem;
    if (TAGGED_TYPE(tag_addr)) {
      *(size_t *)block->mem = (size_t)MASK(tag_addr); // remove mark
      block = block->next;
    } else {
      // as block is move to free list we need to preserve the old next
      block_t *temp = block->next;
      block_list_remove(ublock, block);
      block_list_add_sorted(fblock, block);
      printf("HEAP: freed object @ [0x%lx]\n", ((uint64_t)block->mem) + 1);
      block = temp;
      block_list_merge(fblock); // try merging after every free -> worse perf
                                // but cleaner free list
    }
  }
}

// Traverses all and marks used blocks which can be reached from given pointer
void mark(void *roots) {
  if (roots == NULL || TAGGED(roots)) {
    return;
  }
  void *p = NULL;
  void *prev = NULL;
  void *cur = roots;
  TAG(roots);
  uint32_t i = 0;
  for (;;) {
    *(uint32_t **)MASK(GET_TYPE_ADR(cur)) += 1; // move type desc pointer foward
    int32_t off = *(int32_t *)MASK(GET_TYPE(cur));
    if (off >= 0) { // advance
      size_t childaddr = ((size_t)cur) + off;
      p = *((void **)(childaddr)); // set p to child
      if (p != NULL && !(TAGGED(p))) {
        *(size_t *)(childaddr) = (size_t)prev;
        prev = cur;
        cur = p;
        TAG(cur);
      }
    } else { // retreat
      *(uint32_t **)MASK(GET_TYPE_ADR(cur)) +=
          off; // move type desc pointer back
      if (prev == NULL) {
        return;
      }
      p = cur;
      cur = prev;
      off = *(uint32_t *)MASK(GET_TYPE(cur));
      size_t childaddr = ((size_t)cur) + off;
      prev = (void *)*((void **)(childaddr));
      *((void **)(childaddr)) = p;
    }
    i++;
  }
}

// Check if block memory is inside allocated heap space
static void in_heap(block_t *block) {
  void *end = (void *)(((uint8_t *)block->mem) + block->size);
  if (block->mem < hstart || block->mem > hend || end < hstart || end > hend) {
    heap_dump();
    printf("HEAP: alloc block [%p] outside [%p...%p]\n", (void *)block, hstart,
           hend);
    printf("HEAP: generated heap trace for error search\n");
    assert(false);
  }
}

/* Tries to allocate a block with given size
 * Returns NULL if allocation failed
 */
static void *alloc(uint32_t alloc_size) {
  block_t *prev_block = fblock;
  // loop through all free blocks until we reach the front again
  for (block_t *block = fblock->next;;
       prev_block = block, block = block->next) {
    // find the first fitting block
    if (block->size >= alloc_size) {
      if (block->size == alloc_size) {
        // take the whole block from the free list to the used list
        prev_block->next = block->next;
      } else {
        // new free block with shifted mem address and size by alloc_size
        block_list_add_sorted(
            fblock,
            block_list_new((void *)(((uint8_t *)block->mem) + alloc_size),
                           block->size - alloc_size));
        ;
        prev_block->next = block->next;
        block->size = alloc_size;
      }
      block_list_add_front(ublock, block);

      in_heap(block);
      return (void *)(block->mem);
    }

    if (block == fblock) { // reached the front again => no more space
      return NULL;
    }
  }
  assert(false &&
         "HEAP: Something is wrong with the free list. There is no cyclic "
         "reference");
}

/*
 * PUBLIC METHODS
 */

/* Registers the given type descriptor under the given name */
void register_type(char *name, int32_t *desc_addr) {
  object_t *new_obj = (object_t *)malloc(sizeof(object_t));
  if (new_obj == NULL) {
    assert(false && "HEAP: Could not register type");
  }
  strcpy(new_obj->name, name);
  new_obj->type_desc = desc_addr;
  new_obj->next = objects;
  objects = new_obj;
#if DEBUG
  printf("HEAP: new type %s with type descriptor: ", new_obj->name);
  print_type_desc(new_obj->type_desc);
#endif
}

/* Tries to allocate the given object
 * Return NULL if allocation failed
 *
 * NOTE: Object name has to be registered via register_type()
 */
void *heap_alloc(char *name) {
  // find type by name
  object_t *obj_type = get_obj_by_name(objects, name);
  if (obj_type == NULL) {
    printf("HEAP: Object \"%s\" not registered on heap\n", name);
    return NULL;
  }

  // allocate "Object" on our heap with size for the hidden type tag
  uint32_t alloc_size = obj_type->type_desc[0] + sizeof(size_t);
#if DEBUG
  // printf("HEAP: Allocation request for [%s] with size [%u]+[%lu]\n",
  //      obj_type->name, obj_type->type_desc[0], sizeof(size_t));
#endif
  void *object = alloc(alloc_size);

  if (object == NULL) {
    printf("HEAP: Could not allocate %s\n", obj_type->name);
    return NULL;
  }
  // move pointer
  object = ((size_t *)object + 1);

  // install new pointer to freed space
  int32_t **tp = (int32_t **)(((size_t *)object) - 1);
  *tp = obj_type->type_desc;

  return object;
}

/* Run garbage collection and compact heap */
void heap_gc(void *roots) {
  printf("-------------------------------------------\n");
  printf("          HEAP GARBAGE COLLECTION          \n");
  printf("-------------------------------------------\n");
  mark(roots);
  print_marked_blocks(ublock, objects);
  printf("-------------------------------------------\n");
  sweep();
  printf("-----HEAP GARBAGE COLLECTION FINISHED------\n");
}

/* Allocate and initialize heap */
void heap_init() {
  if (is_init) {
    printf("HEAP already initialized\n");
    return;
  }
  hstart = malloc(HEAP_SIZEB);
  if (hstart == NULL) {
    printf("HEAP: Could not allocate heap area with size %ux. Aborting...\n",
           HEAP_SIZEB);
    assert(false);
  }
  hend = (void *)(((uint8_t *)hstart) + HEAP_SIZEB);

  // zero-sized dummy block
  fblock = (block_t *)malloc(sizeof(block_t));
  fblock->size = 0;
  fblock->next = (block_t *)malloc(sizeof(block_t));
  fblock->mem = NULL;

  // free block
  fblock->next->size = HEAP_SIZEB;
  fblock->next->next = fblock; // cyclic list
  fblock->next->mem = hstart;  // points to the actual memory

  // zero_sized dummy block
  ublock = (block_t *)malloc(sizeof(block_t));
  ublock->size = 0;
  ublock->next = ublock;
  ublock->mem = NULL;

  is_init = true;
  printf("HEAP: [%d] bytes size at base [%p] allocated\n", HEAP_SIZEB,
         fblock->next->mem);
}

// Deallocate heap area and related memory
void heap_deinit() {
  // free linked lists
  free_object_list(objects);
  free_block_list(ublock);
  free_block_list(fblock);
  // free heap area
  free(hstart);
  is_init = false;
}

/* Prints free/used blocks and available space on the heap */
void heap_dump() {
  printf("------------------------------------------\n");
  printf("   HEAP DUMP       %p .... %p\n", hstart, hend);
  printf("------------------------------------------\n");
  size_t free_bytes = print_free_blocks_fancy(fblock);
  printf("------------------------------------------\n");
  size_t used_bytes = print_used_blocks_fancy(ublock, objects);
  printf("------------------------------------------\n");
  printf("[Live Objects] %lu bytes [Free Blocks] %lu bytes [Combined] %lu "
         "bytes [Heap size] %d\n",
         used_bytes, free_bytes, used_bytes + free_bytes, HEAP_SIZEB);
}

/* Prints a raw representation of the used block list */
void heap_print_used_blocks() { block_list_print(ublock); }
