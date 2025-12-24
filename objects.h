#pragma once

#include "stdbool.h"
#include "stdint.h"

// C naming conventions are ignored here for clarity

/*
 * "Object" can be allocated on the GC heap
 */

#define ORDER_DATE_LEN 11
typedef struct Order {
  uint64_t id;
  // max 11 chars needed for format 22/12/2025\0
  char date[ORDER_DATE_LEN];
  bool completed;
} Order;

typedef struct OrderNode {
  struct OrderNode *next;
  struct Order *order;
} OrderNode;

#define RRODUCT_NAME_LEN 25
typedef struct Product {
  uint64_t id;
  // NOTE: not the best way but the easiest
  char name[RRODUCT_NAME_LEN];
  struct OrderNode *order;
} Product;

typedef struct ProdNode {
  struct ProdNode *next;
  struct Product *product;
} ProdNode;

typedef struct ProductList {
  struct ProdNode *first;
} ProductList;
