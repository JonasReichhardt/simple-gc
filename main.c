#include "stdbool.h"
#include "stdint.h"

#include "heap.h"
#include "heap_util.h"
#include "objects.h"
#include "objects.h"
#include "type_desc.h"

int main() {
  heap_init();
  void *order = NULL;
  do{
      order = gc_alloc("Order");
      //printf("%p\n",order);
  }while (order!=NULL);
  /*heap_dump();
  // access type descriptor
  order_t *k = (order_t*)GET_TYPE(order);
  // test access to type descripter
  printf("Order Descriptor: %d %d\n",(k)->size, (k)->n);;*/
  heap_deinit();
  return 0;
}
