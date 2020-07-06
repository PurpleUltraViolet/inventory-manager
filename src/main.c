#include <stdio.h>
#include "product.h"
#include "inventory.h"
#include "ui.h"

int main(int argc, char *argv[])
{
    struct Inventory *inv;

    inv = inventory_create("Unsaved Inventory");
    if (!inv)
        return 1;

    return run(inv);
}
