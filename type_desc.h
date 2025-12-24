#pragma once

#include "stdint.h"
#include "stdlib.h"

#include "objects.h"

// this macro resolves the address to the type tag of an object
#define GET_TYPE(t) (int32_t *)(*(((size_t *)t) - 1))

int32_t *alloc_type_desc(int32_t size, int32_t num_p) {
  // reserve space for n pointer offsets
  // the objects size and negative end of array marker
  int32_t *desc = (int32_t *)malloc((sizeof(int32_t) * (num_p + 2)));
  desc[0] = size;
  // last element is negative to mark the end of the pointer offset list
  desc[(num_p + 2) - 1] = -(num_p + 2) - 1;
  return desc;
}
