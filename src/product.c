#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "product.h"

struct Product *product_create(const char *name, unsigned long cost,
        unsigned long amt)
{
    struct Product *p;
    size_t namelen;
    p = malloc(sizeof(struct Product));
    if (!p) {
        errno = ENOMEM;
        return NULL;
    }

    namelen = strlen(name) + 1;
    p->name = malloc(namelen * sizeof(char));
    if (!p->name) {
        free(p);
        errno = ENOMEM;
        return NULL;
    }
    strcpy(p->name, name);

    p->cost = cost;
    p->amt = amt;

    return p;
}

void product_destroy(struct Product *p)
{
    free(p->name);
    free(p);
}
