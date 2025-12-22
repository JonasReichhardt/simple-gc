#pragma once

#include "stdint.h"

// this macro resolves the address to the type tag of an object
#define GET_TYPE(t) *(((size_t *)t) - 1)

typedef struct order_t {
  uint32_t size;
  int n;
} order_t;
