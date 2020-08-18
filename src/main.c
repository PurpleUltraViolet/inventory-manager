#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "inventory.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    struct Inventory *inv;

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

    gtk_init(NULL, NULL);

    GtkWidget *window;
    window = GTK_WIDGET(setup_main_window(NULL, inv));
    if (!window)
        return -1;
    gtk_widget_show(window);

    gtk_main();

    return 0;
}
