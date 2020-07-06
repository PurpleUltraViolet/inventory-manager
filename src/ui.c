#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "inventory.h"
#include "product.h"
#include "mainwindow.h"
#include "ui.h"

int run(struct Inventory *inv)
{
    gtk_init(NULL, NULL);

    GtkWidget *window;
    window = GTK_WIDGET(setup_main_window(NULL, inv));
    if (!window)
        return -1;
    gtk_widget_show(window);

    gtk_main();

    return 0;
}
