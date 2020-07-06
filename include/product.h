#ifndef PRODUCT_H_
#define PRODUCT_H_

struct Product {
    char *name;
    unsigned long cost; /* In cents, avoid decimal errors */
    unsigned long amt; /* Number of product */
};

/* Creates new product with id of 0 (to be set by inventory) and amount of 1 */
struct Product *product_create(const char *name, unsigned long cost,
        unsigned long amt);
void product_destroy(struct Product *p);

#endif
