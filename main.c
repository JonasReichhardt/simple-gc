#include "stdbool.h"
#include "stdint.h"
#include "stdlib.h"

#include "heap.h"
#include "objects.h"
#include "type_desc.h"
#include <stdio.h>

void test_types() {
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
}

int main() {
  heap_init();
  for (uint32_t i = 0; i < 5; i++) {
    uint32_t *v = (uint32_t *)heap_alloc(sizeof(uint32_t));
    *v = 0xfeedbeef;
  }
  heap_dump();
  return 0;
}
