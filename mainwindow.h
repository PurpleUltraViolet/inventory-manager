#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "inventory.h"

GtkTreeView *setup_main_window_tree_view(struct Inventory *inv);
GtkWindow *setup_main_window(GtkWindow *owindow, struct Inventory *inv);

#endif
