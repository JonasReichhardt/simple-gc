
#include "stdbool.h"
#include "stdint.h"

#include "heap.h"
#include "objects.h"
#include "type_desc.h"

void register_types() {
  int32_t *order_desc = alloc_type_desc(sizeof(Order), 0);
  register_type("Order", order_desc);
}

int main() {
  heap_init();
  register_types();

  Order *order = (Order *)gc_alloc("Order");
  if (order != NULL) {
    order->id = 0xFEEDBEEF;
    strncpy(order->date, "24/12/2025", ORDER_DATE_LEN);
    order->completed = true;
  }

  heap_dump();
  heap_deinit();
  return 0;
}
