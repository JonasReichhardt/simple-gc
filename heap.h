#pragma once

#include "assert.h"
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdlib.h>

#include "heap_util.h"

#define HEAP_SIZEB 32768 // 32KiB
// who many free list blocks should be printed per line during dumping
#define BLOCKS_PER_LINE 3
#define DEBUG

static void *start;     // zero sized block at the beginning
static block_t *fblock; // pointer to the first free block
static block_t *ublock; // pointer to the first used block
static bool is_init = false;

void heap_dump();

// This method should prevent memory overwrite outside of our allocated memory
static void in_heap(void *block) {
  if (block < (void *)start && block > (void *)(start + HEAP_SIZEB)) {
    heap_dump();
    printf("HEAP: alloc return address [%lx] outside [%lx...%lx]\n", block,
           start, start + HEAP_SIZEB);
    printf("HEAP: generated heap trace for error search\n");
    assert(false);
  }
}

void heap_init() {
  if (is_init) {
    printf("HEAP: already initialized\n");
    return;
  }
  start = malloc(HEAP_SIZEB);
  if (start == NULL) {
    printf("HEAP: Could not allocate heap area with size %ux. Aborting...\n",
           HEAP_SIZEB);
    assert(false);
  }

  // zero-sized dummy block
  fblock = (block_t *)malloc(sizeof(block_t));
  fblock->size = 0;
  fblock->next = (block_t *)malloc(sizeof(block_t));
  fblock->mem = NULL;

  // free block
  fblock->next->size = HEAP_SIZEB;
  fblock->next->next = fblock; // cyclic list
  fblock->next->mem = start;   // points to the actual memory

  // zero_sized dummy block
  ublock = (block_t *)malloc(sizeof(block_t));
  ublock->size = 0;
  ublock->next = ublock;
  ublock->mem = NULL;

  is_init = true;
  printf("HEAP: size [%lu] bytes with base at %lx allocated\n", HEAP_SIZEB,
         fblock->mem);
}

void heap_deinit() {
  // free heap area
  free(start);
  // free linked lists
  block_list_free(ublock);
  block_list_free(fblock);
  is_init = false;
  printf("HEAP: size [%lu] bytes with base at %lx de-allocated\n", HEAP_SIZEB,
         fblock->mem);
}

void *heap_alloc(size_t alloc_size) {
#ifdef DEBUG
  printf("HEAP: allocation request for %lu bytes\n", alloc_size);
#endif
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
#ifdef DEBUG
        printf("HEAP: found a perfectly fitting block @ [0x%lx]\n", block->mem);
#endif
      } else {
#ifdef DEBUG
        printf("HEAP: found a big enough block @ [0x%lx]\n", block->mem);
#endif
        // new free block with shifted mem address and size by alloc_size
        block_t *new_freeb = block_list_add_new(
            fblock, (void *)(((uint8_t *)block->mem) + alloc_size),
            block->size - alloc_size);
        prev_block->next = block->next;
        block->size = alloc_size;
        block_list_add(ublock, block);
#ifdef DEBUG
        printf("HEAP: new free block [%lu]@[0x%lx]\n", new_freeb->size,
               new_freeb->mem);
#endif
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

void print_raw_heap() { printf("%.*s\n", HEAP_SIZEB, (unsigned char *)start); }

size_t print_free_blocks() {
  printf("Free list:\n");
  size_t free_bytes = 0;
  size_t i = 0;
  for (block_t *block = fblock->next; block != fblock; block = block->next) {
    printf("[%lu]@[0x%lx]<--", block->size, block);
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

size_t print_used_blocks() {
  printf("Allocated \"Objects\"\n");
  size_t used_bytes = 0;
  size_t i = 0;
  for (block_t *block = ublock->next; block != ublock; block = block->next) {
    printf("[%lub]@[0x%lx] Name: [TODO] Data: [0x%lx]\n", block->size, block,
           *((uint64_t *)block->mem));
    used_bytes += block->size;
    i++;
  }
  printf("\n\nLive objects: %lu bytes\n", used_bytes);
  return used_bytes;
}

void heap_dump() {
  printf("------------------------------------------\n");
  printf("   HEAP DUMP       %lx .... %lx\n", start, start + HEAP_SIZEB);
  printf("------------------------------------------\n");
  size_t free_bytes = print_free_blocks();
  printf("------------------------------------------\n");
  size_t used_bytes = print_used_blocks();
  printf("------------------------------------------\n");
  printf("[Live Objects] %lu bytes [Free Blocks] %lu bytes [Combined] %lu "
         "bytes [Heap size] %lu\n",
         used_bytes, free_bytes, used_bytes + free_bytes, HEAP_SIZEB);
}
