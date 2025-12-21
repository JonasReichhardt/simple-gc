#pragma once

#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"

#define HEAP_SIZEB 32768 // 32KiB
// who many free list blocks should be printed per line during dumping
#define BLOCKS_PER_LINE 3
#define DEBUG

typedef struct block {
  size_t size; // size in minimal block-sized memory chunks (e.g. 1=16bytes,
               // 2=32bytes)
  struct block *next;
} block_t;

static block_t *start;  // pointer to heap start
static block_t *fblock; // pointer to the first free block
static block_t *ublock; // pointer to the first used block

void heap_init() {
  start = (block_t *)malloc(HEAP_SIZEB);
  fblock = start;
  if (fblock == NULL) {
    printf("HEAP: allocation failed. Aborting...\n");
    assert(false);
  }
  fblock->size = HEAP_SIZEB / sizeof(block_t);
  fblock->next = fblock; // cyclic link
  printf("HEAP: size [%lu] bytes with base at %lx and block length %lu\n",HEAP_SIZEB, fblock, fblock->size);
}

void *heap_alloc(size_t alloc_size) {
  size_t num_blocks;
  block_t *p, *prev_p;

  num_blocks = (alloc_size + sizeof(block_t) - 1) / sizeof(block_t) + 1;
#ifdef DEBUG
  printf("HEAP: allocation request for %lu bytes => [block size = %lu] "
         "[allocated bytes = %lu]\n",
         alloc_size, num_blocks, num_blocks * sizeof(block_t));
#endif
  // start search at first free block
  prev_p = fblock;

  for (p = prev_p->next;; prev_p = p, p = p->next) {
    // find the first fitting block
    if (p->size >= num_blocks) {
      if (p->size == num_blocks) {
        prev_p->next = p->next;
        assert(false);
      } else {
        p->size -= num_blocks; // calculate new block size
        p += p->size;          // move pointer
        p->size = num_blocks;  // save size in new block
      }

      fblock = prev_p;

      // add p to used list
      if (ublock == NULL) {
        ublock = p->next = p;
        ublock->next = ublock;
      } else {
        p->next = ublock->next;
        ublock->next = p;
      }
      printf("HEAP: used block with size [%lu] at [%lx]\n", ublock->next->size,
             ublock->next);
      printf("HEAP: next free block with size [%lu] at [%lx]\n", fblock->size,
             fblock);

      return (void *)(p + 1);
    } else {
      printf("HEAP: Block to small\n");
    }
    if (p == fblock) { // reached the front again => no more space
      printf("HEAP: No more free space\n");
      assert(false);
    }
  }
  printf("HEAP: Something is wrong with the free list. There is no cyclic "
         "reference\n");
  assert(false);
}

void print_raw_heap(){
    printf("%.*s\n", HEAP_SIZEB, (unsigned char*)start);
}

size_t print_free_blocks() {
  printf("Free list:\n");
  printf("[%lu]@[0x%lx]", fblock->size * sizeof(block_t),fblock);
  size_t free_bytes = fblock->size * sizeof(block_t);
  block_t *block;
  size_t i = 0;
  for (block = fblock->next; block != fblock; block = block->next) {
    printf("<--[%lu]@[0x%lx]", block->size * sizeof(block_t),block);
    free_bytes += block->size * sizeof(block_t);
    i++;
    if (i == BLOCKS_PER_LINE) {
      printf("\n");
      i = 0;
    }
  }
  printf("\n\nFree heap space: %lu bytes\n",free_bytes);
  return free_bytes;
}

size_t print_used_blocks() {
    printf("Allocated \"Objects\":\n");
    printf("[%lu]@[0x%lx] Name: TODO Data: [0x%lx]\n", ublock->size * sizeof(block_t),ublock,*(ublock+1));
    printf("   -->[0x%lxTODO]\n", 0); //TODO
    size_t used_bytes = ublock->size * sizeof(block_t);
    block_t *block;
    size_t i = 0;
    for (block = ublock->next; block != ublock; block = block->next) {
      printf("[%lu]@[0x%lx] Name: TODO Data: [0x%lx]\n", block->size * sizeof(block_t),block,*(block+1));
      used_bytes += ublock->size * sizeof(block_t);
      i++;
      if (i == BLOCKS_PER_LINE) {
        printf("\n");
        i = 0;
      }
    }
    printf("\n\nLive objects: %lu bytes\n",used_bytes);
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
  printf("[Live Objects] %lu bytes [Free Blocks] %lu bytes [Combined] %lu bytes [Heap size] %lu\n",used_bytes,free_bytes,used_bytes+free_bytes,HEAP_SIZEB);
}
