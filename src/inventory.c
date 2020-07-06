#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include "product.h"
#include "inventory.h"

#define INVENTORY_INC_AMT 5

int compare_two_elements_of_inventory(struct Product *p1, struct Product *p2);
size_t inventory_sort_partition(struct Inventory *inv, size_t lo, size_t hi);

struct Inventory *inventory_create(char *title)
{
    struct Inventory *inv;
    size_t titlelen;
    int lerrno;
    inv = malloc(sizeof(struct Inventory));
    if (!inv)
        return NULL;

    inv->l = 0;
    inv->s = INVENTORY_INC_AMT;
    inv->i = malloc(inv->s * sizeof(struct Inventory));
    if (!inv->i) {
        lerrno = ENOMEM;
        free(inv);
        errno = lerrno;
        return NULL;
    }

    titlelen = strlen(title) + 1;
    inv->title = malloc(titlelen * sizeof(char));
    if (!inv->title) {
        lerrno = ENOMEM;
        free(inv->i);
        free(inv);
        errno = lerrno;
        return NULL;
    }
    memcpy(inv->title, title, titlelen);

    return inv;
}

int inventory_add(struct Inventory *inv, struct Product *p)
{
    if (inv->l == inv->s) {
        struct Product **tmp;
        inv->s += INVENTORY_INC_AMT;
        tmp = realloc(inv->i, inv->s * sizeof(struct Product *));
        if (!tmp) {
            int lerrno = errno;
            inventory_destroy(inv);
            return -lerrno;
        }

        inv->i = tmp;
    }

    inv->i[inv->l++] = p;
    return 0;
}

int compare_two_elements_of_inventory(struct Product *p1, struct Product *p2)
{
    size_t i;
    int rval = 0;
    unsigned char c1, c2;

    for (i = 0; i < strlen(p1->name) + 1; i++) {
        c1 = tolower(p1->name[i]);
        c2 = tolower(p2->name[i]);

        if (c1 < c2) {
            rval = -1;
            break;
        } else if (c1 > c2) {
            rval = 1;
            break;
        } else {
            if (p1->name[i] < p2->name[i]) {
                rval = -1;
                break;
            } else if (p1->name[i] > p2->name[i]) {
                rval = 1;
                break;
            }
        }
    }

    return rval;
}

size_t inventory_sort_partition(struct Inventory *inv, size_t lo, size_t hi)
{
    struct Product *p= inv->i[hi], *tmp;
    size_t i = lo, j;
    for (j = lo; j <= hi; j++) {
        if (compare_two_elements_of_inventory(inv->i[j], p) < 0) {
            tmp = inv->i[i];
            inv->i[i] = inv->i[j];
            inv->i[j] = tmp;
            i++;
        }
    }
    tmp = inv->i[i];
    inv->i[i] = inv->i[hi];
    inv->i[hi] = tmp;
    return i;
}

void inventory_sort(struct Inventory *inv, size_t lo, size_t hi)
{
    size_t p;
    if (lo < hi) {
        p = inventory_sort_partition(inv, lo, hi);
        inventory_sort(inv, lo, p != 0 ? p - 1 : p);
        inventory_sort(inv, p + 1, hi);
    }
}

void inventory_remove(struct Inventory *inv, size_t id)
{
    size_t i;
    product_destroy(inv->i[id]);
    inv->l--;
    for (i = id; i < inv->l; i++) {
        inv->i[i] = inv->i[i + 1];
    }
}

int inventory_serialize(struct Inventory *inv, char *fname)
{
    /* The file format:
     *     size_t - number of products
     *     products - dump of products, name values are arbitrary
     *     names - dump of strings, separated by NUL*/
    FILE *fp = NULL;
    size_t i, namesize = 0, bufloc = 0, nameloc = 0, namelen, titlelen;
    int rval = 0;
    char *buf = NULL, *titleptr, *tmp;

    /* Get size of all name strings */
    for (i = 0; i < inv->l; i++) {
        namesize += strlen(inv->i[i]->name) + 1;
    }
    nameloc = sizeof(inv->l) + (inv->l * sizeof(struct Product));

    buf = malloc(sizeof(inv->l) + (inv->l * sizeof(struct Product)) + namesize);
    if (!buf) {
        rval = -errno;
        goto exit;
    }

    memcpy(buf, &inv->l, sizeof(inv->l));
    bufloc += sizeof(inv->l);
    for (i = 0; i < inv->l; i++, bufloc += sizeof(struct Product)) {
        memcpy(buf + bufloc, inv->i[i], sizeof(struct Product));
        namelen = strlen(inv->i[i]->name) + 1;
        memcpy(buf + nameloc, inv->i[i]->name, namelen);
        nameloc += namelen;
    }

    fp = fopen(fname, "wb");
    if (!fp) {
        rval = -errno;
        goto exit;
    }

    if ((rval = fwrite(buf, sizeof(inv->l) + (inv->l * sizeof(struct Product))
                        + namesize, 1, fp)) != 1) {
        rval = -errno;
        goto exit;
    }
    rval = 0;

    titleptr = fname;
    while((tmp = strchr(titleptr, '/')) != NULL) titleptr = ++tmp;
    titlelen = strlen(titleptr) + 1;
    if (inv->title) free(inv->title);
    inv->title = malloc(titlelen * sizeof(char));
    if (!inv->title) {
        rval = -errno;
        goto exit;
    }
    memcpy(inv->title, titleptr, titlelen * sizeof(char));

exit:
    if (fp)
        fclose(fp);
    if (buf)
        free(buf);
    return rval;
}

int inventory_deserialize(struct Inventory **inv, char *fname)
{
    FILE *fp = NULL;
    size_t i, bufsize, bufloc = 0, bufnameloc, namelen, titlelen;
    int rval = 0;
    char *buf = NULL, *titleptr, *tmp;
    struct Inventory *ninv = NULL;

    ninv = malloc(sizeof(struct Inventory));
    if (!ninv) {
        rval = -errno;
        goto exit_err;
    }
    ninv->title = NULL;
    ninv->i = NULL;
    ninv->l = 0;
    ninv->s = 0;

    fp = fopen(fname, "rb");
    if (!fp) {
        rval = -errno;
        goto exit_err;
    }

    titleptr = fname;
    while((tmp = strchr(titleptr, '/')) != NULL) titleptr = ++tmp;
    titlelen = strlen(titleptr) + 1;
    ninv->title = malloc(titlelen * sizeof(char));
    if (!ninv->title) {
        rval = -errno;
        goto exit_err;
    }
    memcpy(ninv->title, titleptr, titlelen * sizeof(char));

    fseek(fp, 0, SEEK_END);
    bufsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buf = malloc(bufsize);
    if (!buf) {
        rval = -errno;
        goto exit_err;
    }

    if ((rval = fread(buf, bufsize, 1, fp)) != 1) {
        rval = 1;
        goto exit_err;
    }
    rval = 0;

    memcpy(&ninv->s, buf, sizeof(ninv->s));
    ninv->i = malloc(ninv->s * sizeof(struct Product *));
    if (!ninv->i) {
        rval = -errno;
        goto exit_err;
    }
    memset(ninv->i, 0, ninv->s * sizeof(struct Product *));
    ninv->l = ninv->s;

    bufloc += sizeof(ninv->l);
    bufnameloc = bufloc + (ninv->s * sizeof(struct Product));
    for (i = 0; i < ninv->l; i++, bufloc += sizeof(struct Product),
            bufnameloc += namelen) {
        ninv->i[i] = malloc(sizeof(struct Product));
        if (!ninv->i[i]) {
            rval = -errno;
            goto exit_err;
        }
        memcpy(ninv->i[i], buf + bufloc, sizeof(struct Product));

        namelen = strlen(buf + bufnameloc) + 1;
        ninv->i[i]->name = malloc(namelen);
        if (!ninv->i[i]->name) {
            rval = -errno;
            goto exit_err;
        }
        memcpy(ninv->i[i]->name, buf + bufnameloc, namelen);
    }

    *inv = ninv;

    goto exit_noerr;
exit_err:
    if (ninv)
        inventory_destroy(ninv);
exit_noerr:
    if (buf)
        free(buf);
    if (fp)
        fclose(fp);
    errno = -rval;
    return rval;
}

void inventory_destroy(struct Inventory *inv)
{
    size_t i;
    if (inv->title)
        free(inv->title);
    if (inv->i) {
        for (i = 0; i < inv->l; i++) {
            product_destroy(inv->i[i]);
        }
    }
    free(inv);
}
