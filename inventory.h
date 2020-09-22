#ifndef INVENTORY_H_
#define INVENTORY_H_

#include "product.h"

struct Inventory {
    char *title;
    size_t l;
    size_t s;
    struct Product **i;
};

struct Inventory *inventory_create(char *title);
int inventory_add(struct Inventory *inv, struct Product *p);
void inventory_sort(struct Inventory *inv, size_t lo, size_t hi);
void inventory_remove(struct Inventory *inv, size_t id);
int inventory_serialize(struct Inventory *inv, char *fname);
int inventory_deserialize(struct Inventory **inv, char *fname);
void inventory_destroy(struct Inventory *inv);

#endif
