#pragma once

#include "assert.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

#include "heap_util.h"
//#include "objects.h"
//#include "type_desc.h"

#define HEAP_SIZEB 32768 // 32KiB

static void *hstart;    // heap start
static void *hend;      // heap end
static block_t *fblock; // dummy block: points to the first free block
static block_t *ublock; // dummy block: points to the first free block
static bool is_init = false;

void heap_dump();

/*void test_types() {
  order_t ord_type = {.size = sizeof(Order), .n = 1};
  Order *order = malloc(sizeof(Order) + sizeof(size_t));

  // move order pointer
  order = (Order *)((size_t *)order + 1);

  // install new pointer to freed space
  order_t **tp = (order_t **)(((size_t *)order) - 1);
  *tp = &ord_type;

  // access type descriptor
  order_t *k = (order_t *)*(((size_t *)order) - 1);
  printf("%lu %lu\n", (k)->size, (k)->n);
  }*/

// This method should prevent user access to memory outside of our allocated
// heap
static void in_heap(void *block) {
  if (block < hstart && block > hend) {
    heap_dump();
    printf("HEAP: alloc return address [%p] outside [%p...%p]\n", block, hstart,
           (void *)(((uint8_t *)hstart) + HEAP_SIZEB));
    printf("HEAP: generated heap trace for error search\n");
    assert(false);
  }
}

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

void heap_deinit() {
  // free linked lists
  block_list_free(ublock);
  block_list_free(fblock);
  // free heap area
  free(hstart);
  is_init = false;
}

void *heap_alloc(size_t alloc_size) {
  block_t *prev_block = fblock;
  // loop through all free blocks until we reach the front again
  for (block_t *block = fblock->next;;
       prev_block = block, block = block->next) {
    // find the first fitting block
    if (block->size >= alloc_size) {
      if (block->size == alloc_size) {
        // take the whole block from the free list to the used list
        prev_block->next = block->next;
        block_list_add(ublock, block);
      } else {
        // new free block with shifted mem address and size by alloc_size
        block_list_add_new(
            fblock, (void *)(((uint8_t *)block->mem) + alloc_size),
            block->size - alloc_size);
        prev_block->next = block->next;
        block->size = alloc_size;
        block_list_add(ublock, block);
      }

      in_heap((void *)(block->mem));
      return (void *)(block->mem);
    }

    if (block->mem == NULL) { // reached the front again => no more space
      assert(false && "HEAP: No more free space");
    }
  }
  assert(false &&
         "HEAP: Something is wrong with the free list. There is no cyclic "
         "reference");
}

void heap_dump() {
  printf("------------------------------------------\n");
  printf("   HEAP DUMP       %p .... %p\n", hstart, hend);
  printf("------------------------------------------\n");
  size_t free_bytes = print_free_blocks(fblock);
  printf("------------------------------------------\n");
  size_t used_bytes = print_used_blocks(ublock);
  printf("------------------------------------------\n");
  printf("[Live Objects] %lu bytes [Free Blocks] %lu bytes [Combined] %lu "
         "bytes [Heap size] %d\n",
         used_bytes, free_bytes, used_bytes + free_bytes, HEAP_SIZEB);
}
