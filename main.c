
#include "assert.h"
#include "stdbool.h"
#include "stdint.h"

#include "heap.h"
#include "objects.h"

#define NUM_ORDERS 3
#define NUM_ORDER_NODES 7
#define NUM_PRODUCTS 3

void register_types() {
  int32_t *desc = create_order_type();
  register_type("Order", desc);
  desc = create_order_node_type();
  register_type("OrderNode", desc);
  desc = create_product_type();
  register_type("Product", desc);
  desc = create_product_node_type();
  register_type("ProductNode", desc);
  desc = create_product_list_type();
  register_type("ProductList", desc);
}

int test_mini() {
  heap_init();
  register_types();

  Order *order = heap_alloc("Order");
  heap_dump();
  order = NULL;
  heap_gc((void *)order);

  heap_dump();

  heap_deinit();
  return 0;
}

int test_small() {
  heap_init();
  register_types();

  Order *order1 = (Order *)heap_alloc("Order");
  Order *order2 = (Order *)heap_alloc("Order");

  OrderNode *order_node1 = (OrderNode *)heap_alloc("OrderNode");
  OrderNode *order_node2 = (OrderNode *)heap_alloc("OrderNode");

  order1->id = 0xFEEDBEEF;
  strncpy(order1->date, "05/01/2026", ORDER_DATE_LEN);
  order1->completed = false;

  order2->id = 0xBEEFFEED;
  strncpy(order2->date, "05/01/2026", ORDER_DATE_LEN);
  order2->completed = true;

  order_node1->order = order1;
  order_node1->next = order_node2;
  order_node2->order = order2;
  order_node2->next = NULL;

  heap_dump();

  // Remove reference to order2 and run gc again
  order_node2->order = NULL;
  heap_gc((void *)order_node1);
  heap_dump();

  heap_deinit();
  return 0;
}

int test_big() {
  // PROLOGUE
  heap_init();
  register_types();

  // SETUP
  Order *orders[NUM_ORDERS];
  for (uint i = 0; i < NUM_ORDERS; i++) {
    orders[i] = (Order *)heap_alloc("Order");
    if (orders[i] == NULL) {
      assert(false && "Could not allocate Order");
    }
    strncpy(orders[i]->date, "25/12/2025", ORDER_DATE_LEN);
    orders[i]->completed = false;
  }
  orders[0]->id = 0xFEEDBEEF;
  orders[1]->id = 0xBEEFBEEF;
  orders[2]->id = 0xFEEDFEED;

  OrderNode *order_nodes[NUM_ORDER_NODES];
  for (uint i = 0; i < NUM_ORDER_NODES; i++) {
    order_nodes[i] = (OrderNode *)heap_alloc("OrderNode");
    if (order_nodes[i] == NULL) {
      assert(false && "Could not allocate OrderNode");
    }
  }

  order_nodes[0]->next = order_nodes[1];
  order_nodes[0]->order = orders[0];
  order_nodes[1]->next = order_nodes[2];
  order_nodes[1]->order = orders[1];
  order_nodes[2]->next = NULL;
  order_nodes[2]->order = orders[2];

  order_nodes[3]->next = order_nodes[4];
  order_nodes[3]->order = orders[0];
  order_nodes[4]->next = NULL;
  order_nodes[4]->order = orders[2];

  order_nodes[5]->next = order_nodes[6];
  order_nodes[5]->order = orders[1];
  order_nodes[6]->next = NULL;
  order_nodes[6]->order = orders[2];

  Product *prod0 = (Product *)heap_alloc("Product");
  Product *prod1 = (Product *)heap_alloc("Product");
  Product *prod2 = (Product *)heap_alloc("Product");

  if (prod0 == NULL || prod1 == NULL || prod2 == NULL) {
    assert(false && "Could not allocate Product");
  }

  prod0->id = 0xBEFEBEFE;
  strncpy(prod0->name, "Underwear", PRODUCT_NAME_LEN);
  prod0->order = order_nodes[0];

  prod1->id = 0xFEBEFEBE;
  strncpy(prod1->name, "Hat", PRODUCT_NAME_LEN);
  prod1->order = order_nodes[3];

  prod2->id = 0xCEFECEFE;
  strncpy(prod2->name, "Socks", PRODUCT_NAME_LEN);
  prod2->order = order_nodes[5];

  ProdNode *pnode0 = (ProdNode *)heap_alloc("ProductNode");
  ProdNode *pnode1 = (ProdNode *)heap_alloc("ProductNode");
  ProdNode *pnode2 = (ProdNode *)heap_alloc("ProductNode");

  if (pnode0 == NULL || pnode1 == NULL || pnode2 == NULL) {
    assert(false && "Could not allocate ProductNode");
  }

  pnode0->product = prod0;
  pnode0->next = pnode1;
  pnode1->product = prod1;
  pnode1->next = pnode2;
  pnode2->product = prod2;
  pnode2->next = NULL;

  ProductList *product_list = (ProductList *)heap_alloc("ProductList");
  if (product_list == NULL) {
    assert(false && "Could not allocate ProductList");
  }
  product_list->first = pnode0;

  heap_dump();

  // TEST
  product_list->first = NULL;
  heap_gc((void *)product_list);
  heap_dump();

  pnode0 = (ProdNode *)heap_alloc("ProductNode");
  pnode0->product = NULL;
  pnode0->next = NULL;
  product_list->first = pnode0;

  heap_gc((void *)product_list);
  heap_dump();

  // EPILOGUE
  heap_deinit();
  return 0;
}

int main() { return test_big(); }
