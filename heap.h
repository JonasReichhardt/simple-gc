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
#include <stddef.h>
#include <stdint.h>

#define HEAP_SIZEB 32768 // 32KiB
#define DEBUG false

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
static void *heap_alloc(uint32_t alloc_size);

/*
 * PRIVATE METHODS
 */

// frees unmarked blocks and builds new free list
static void sweep() {
  // empty used block list
  if (ublock->next == ublock) {
    return;
  }

  for (block_t *block = ublock->next; block->mem != NULL; block = block->next) {
    // add unmarked block from used to freelist
    if (((uint64_t)block->mem & 0x0000000f) == 0) {
      block_list_remove(ublock, block);
      block_list_add_back(fblock, block);
    }
    block->mem = (void *)(((uint64_t)block->mem & 0xfffffffc)); // remove mark
  }
}

// Traverses all and marks used blocks which can be reached from given pointer
void mark(void *roots) {
  if (roots == NULL || TAGGED(roots)) {
    assert(false && "Cannot perform GC on NULL or tagged object");
  }
  // TAG(roots);
  void *prev, *cur, *p = NULL;
  cur = roots;
  int32_t *cur_t = GET_TYPE(roots)++; // skip first element of type descriptor
  for (;;) {
    cur_t += 1;
    int32_t off = *cur_t;
    if (off >= 0) {
      p = (void *)(*(((size_t *)cur) + off)); // set p to child
      if (p != NULL && !(TAGGED(p))) {
          (void*)(*(((size_t *)cur) + off)) = prev;

      }
      return;
    } else {
    }
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

static void *heap_alloc(uint32_t alloc_size) {
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
        block_list_add_new(fblock,
                           (void *)(((uint8_t *)block->mem) + alloc_size),
                           block->size - alloc_size);
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
void register_type(char *name, int32_t *desc_addr) {
  object_t *new_obj = (object_t *)malloc(sizeof(object_t));
  if (new_obj == NULL) {
    assert(false && "HEAP: Could not register type");
  }
  strcpy(new_obj->name, name);
  new_obj->type_desc = desc_addr;
  new_obj->next = objects;
  objects = new_obj;
}

void *gc_alloc(char *name) {
  // find type by name
  object_t *obj_type = get_obj_by_name(objects, name);
  if (obj_type == NULL) {
    printf("HEAP: Object \"%s\" not registered on heap\n", name);
    return NULL;
  }

  // allocate "Object" on our heap with size for the hidden type tag
  uint32_t alloc_size = obj_type->type_desc[0] + sizeof(size_t);
#if DEBUG
  printf("HEAP: Allocation request for [%s] with size [%u]+[%lu]\n",
         obj_type->name, obj_type->type_desc[0], sizeof(size_t));
#endif
  void *object = heap_alloc(alloc_size);

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

// Run Mark&Sweep algorithm, free unused blocks and compact heap
void gc(void *roots) {
  mark(roots);
  // sweep();
}

// Allocate and initialize heap area
void heap_init() {
  if (is_init) {
    printf("HEAP: already initialized\n");
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

void heap_dump() {
  printf("------------------------------------------\n");
  printf("   HEAP DUMP       %p .... %p\n", hstart, hend);
  printf("------------------------------------------\n");
  size_t free_bytes = print_free_blocks(fblock);
  printf("------------------------------------------\n");
  size_t used_bytes = print_used_blocks(ublock, objects);
  printf("------------------------------------------\n");
  printf("[Live Objects] %lu bytes [Free Blocks] %lu bytes [Combined] %lu "
         "bytes [Heap size] %d\n",
         used_bytes, free_bytes, used_bytes + free_bytes, HEAP_SIZEB);
}
