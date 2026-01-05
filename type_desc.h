#pragma once

#include "assert.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

#define MASK(t) (void *)(((size_t)t) & 0xfffffffc)
// Resolves the address to the type tag of an object
#define GET_TYPE(t) (int32_t *)(*GET_TYPE_ADR(t))
// Resolves the address to the address of type tag of an object
#define GET_TYPE_ADR(t) (int32_t **)(((size_t *)t) - 1)
// Takes object address and unsets the mark bit of the type tag
#define UNTAG(p)                                                               \
  *GET_TYPE_ADR(p) = (int32_t *)(((uint64_t)(GET_TYPE(p))) & 0xfffffffc)
// Takes object address and sets the mark bit of the type tag
#define TAG(p) *GET_TYPE_ADR(p) = (int32_t *)(((uint64_t)(GET_TYPE(p))) | 1)
// Checks if an objects type descriptor is tagged
#define TAGGED(p) (((uint64_t)(GET_TYPE(p))) & 0x0000000f) == 1
// Check if a type descriptor is tagged
#define TAGGED_TYPE(p) (((uint64_t)p) & 0x0000000f) == 1

int32_t *alloc_type_desc(int32_t size, int32_t num_p) {
  // reserve space for n pointer offsets
  // the objects size and negative end of array marker
  int32_t *desc = (int32_t *)malloc((sizeof(int32_t) * (num_p + 2)));
  desc[0] = size;
  // last element is negative to mark the end of the pointer offset list
  desc[num_p + 1] = -(num_p + 1);
  return desc;
}

void print_type_desc(int32_t *desc) {
  printf("size[%d]@[%p] ", *desc, (void *)desc);
  for (;;) {
    desc += 1;
    printf("[%d]@[%p] ", *desc, (void *)desc);
    if (*desc < 0) { // sentinel reached
      desc += (*desc);
      printf("\n");
      return;
    }
  }
}
