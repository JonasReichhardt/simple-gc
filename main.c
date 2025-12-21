#include "heap.h"
#include <stdint.h>

int main() {
  heap_init();
  for(uint32_t i=0;i<1024;i++){
      uint32_t *v = (uint32_t*)heap_alloc(sizeof(uint32_t));
      *v = 0xfeedbeef;
  }
  heap_dump();
  return 0;
}
