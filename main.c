#include "objects.h"
#include "stdbool.h"
#include "stdint.h"

#include "heap.h"
#include "objects.h"

int main() {
  heap_init();
  Order *order = gc_alloc("Order");
  heap_dump();
  // access type descriptor
  order_t *k = (order_t *)*(((size_t *)order) - 1);
  // test access to type descripter
  printf("Order Descriptor: %d %d\n",(k)->size, (k)->n);;
  heap_deinit();
  return 0;
}
