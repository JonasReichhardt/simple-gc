#pragma once

#include "stdbool.h"
#include "stdint.h"

// C naming conventions are ignored here for clarity

/*
 * "Object" can be allocated on the GC heap
 */

typedef struct Order {
  uint64_t id;
  // max 10 chars needed for format 22/12/2025
  char date[10];
  bool completed;
} Order;

typedef struct OrderNode {
  struct OrderNode *next;
  struct Order *order;
} OrderNode;

typedef struct Product {
  uint64_t id;
  // NOTE: not the best way but the easiest
  char name[25];
  struct OrderNode *order;
} Product;

typedef struct ProdNode {
  struct ProdNode *next;
  struct Product *product;
} ProdNode;

typedef struct ProductList {
  struct ProdNode *first;
} ProductList;
