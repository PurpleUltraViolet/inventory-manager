#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "inventory.h"
#include "ui.h"

int main(int argc, char *argv[])
{
    struct Inventory *inv;
    int rval;

    if (argc >= 2) {
        if (inventory_deserialize(&inv, argv[1])) {
            fprintf(stderr, "Error opening file %s: %s\nUsage: %s [file]\n",
                    argv[1], strerror(errno), argv[0]);
            return errno;
        }
    } else {
        inv = inventory_create("Unsaved Inventory");
        if (!inv)
            return 1;
    }

    rval = run(inv);
    return rval;
}
