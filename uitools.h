#ifndef UITOOLS_H_
#define UITOOLS_H_

GtkWidget *get_child_by_name(GtkWidget *widget, char *name);
void destroy_all_children(GtkContainer *container);
void copy_all_children(GtkContainer *src, GtkContainer *dest);

#endif
