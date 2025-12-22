#pragma once

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
  OrderNode *next;
  Order order;
} OrderNode;

typedef struct Product {
  uint64_t id;
  // NOTE: not the best way but the easiest
  char name[25];
  OrderNode order;
} Product;

typedef struct ProdNode {
  ProdNode *next;
  Product product;
} ProdNode;

typedef struct ProductList {
  ProdNode *first;
} ProductList;
