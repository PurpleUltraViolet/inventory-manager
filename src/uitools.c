#include <gtk/gtk.h>

void destroy_all_children(GtkContainer *container)
{
    GList *children, *iter;

    children = gtk_container_get_children(GTK_CONTAINER(container));
    for (iter = children; iter != NULL; iter = g_list_next(iter))
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    g_list_free(children);
}

void copy_all_children(GtkContainer *src, GtkContainer *dest)
{
    GList *children, *iter;
    GtkWidget *tmp;
    destroy_all_children(dest);

    children = gtk_container_get_children(src);
    for (iter = children; iter != NULL; iter = g_list_next(iter)) {
        tmp = GTK_WIDGET(g_object_ref(G_OBJECT(iter->data)));
        gtk_container_remove(src, tmp);
        gtk_container_add(dest, GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

GtkWidget *get_child_by_name(GtkWidget *widget, char *name)
{
    GList *children, *iter;
    GtkWidget *nwidget;

    if (strcmp(gtk_widget_get_name(widget), name) == 0)
        return widget;

    if (GTK_IS_CONTAINER(widget)) {
        children = gtk_container_get_children(GTK_CONTAINER(widget));
        for (iter = children; iter != NULL; iter = g_list_next(iter)) {
            nwidget = get_child_by_name(iter->data, name);
            if (nwidget) {
                return nwidget;
            }
        }
        g_list_free(children);
    }

    return NULL;
}
