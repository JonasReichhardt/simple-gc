#include "stdbool.h"
#include "stdint.h"

#include "heap.h"

int main() {
  heap_init();
  for (uint32_t i = 0; i < 4096; i++) {
    uint64_t *v = (uint64_t *)heap_alloc(sizeof(uint64_t));
    *v = 0xfeedbeef;
  }
  heap_dump();
  heap_deinit();
  return 0;
}
