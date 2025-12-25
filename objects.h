#pragma once

#include "stdbool.h"
#include "stdint.h"

#include "type_desc.h"

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

#define PRODUCT_NAME_LEN 25
typedef struct Product {
  uint64_t id;
  // NOTE: not the best way but the easiest
  char name[PRODUCT_NAME_LEN];
  struct OrderNode *order;
} Product;

typedef struct ProdNode {
  struct ProdNode *next;
  struct Product *product;
} ProdNode;

typedef struct ProductList {
  struct ProdNode *first;
} ProductList;

int32_t *create_order_type() { return alloc_type_desc(sizeof(Order), 0); }

int32_t *create_order_node_type() {
  int32_t *type_desc = alloc_type_desc(sizeof(OrderNode), 2);
  type_desc[1] = 0;
  type_desc[2] = 8;
  return type_desc;
}

int32_t *create_product_type() {
  int32_t *type_desc = alloc_type_desc(sizeof(Product), 1);
  type_desc[1] = 40;
  return type_desc;
}

int32_t *create_product_node_type() {
  int32_t *type_desc = alloc_type_desc(sizeof(ProdNode), 2);
  type_desc[1] = 0;
  type_desc[2] = 8;
  return type_desc;
}

int32_t *create_product_list_type() {
  int32_t *type_desc = alloc_type_desc(sizeof(ProductList), 1);
  type_desc[1] = 0;
  return type_desc;
}
