#include <stdlib.h>
#include <errno.h>
#include <gtk/gtk.h>
#include "product.h"
#include "inventory.h"
#include "uitools.h"
#include "mainwindow.h"
#include "editdialog.h"

struct button_callback_struct {
    GtkWindow *window;
    struct Inventory **inv;
};

void update_status_bar(GtkWidget *nstatusbar, int *ncontextid, char *text);
void destroy_signal_cb(GtkWidget *widget, gpointer data);
void add_button_cb(GtkButton *button, gpointer data);
void edit_button_cb(GtkButton *button, gpointer data);
void remove_button_cb(GtkButton *button, gpointer data);
void open_button_cb(GtkButton *button, gpointer data);
void save_button_cb(GtkButton *button, gpointer data);

void update_status_bar(GtkWidget *nstatusbar, int *ncontextid, char *text)
{
    static GtkWidget *statusbar;
    static int contextid;

    if (nstatusbar) statusbar = nstatusbar;
    if (ncontextid) contextid = *ncontextid;

    gtk_statusbar_pop(GTK_STATUSBAR(statusbar), contextid);
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), contextid, text);
}

void destroy_signal_callback(GtkWidget *widget, gpointer data)
{
    gtk_main_quit();
}

void add_button_cb(GtkButton *button, gpointer data)
{
    GtkWidget *treeview = NULL, *treeviewparent = NULL;
    struct button_callback_struct *b_cb_s;
    struct Product *p;
    b_cb_s = data;

    treeview = get_child_by_name(GTK_WIDGET(b_cb_s->window), "itemlist");
    if (!treeview) {
        gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
        return;
    }

    p = edit_dialog_run(b_cb_s->window, NULL);
    if (p == NULL) return;
    if (inventory_add(*b_cb_s->inv, p)) return;
    inventory_sort(*b_cb_s->inv, 0, (*b_cb_s->inv)->l - 1);

    treeviewparent = gtk_widget_get_parent(treeview);
    gtk_widget_destroy(treeview);
    treeview = GTK_WIDGET(setup_main_window_tree_view(*b_cb_s->inv));
    if (!treeview) {
        gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
        return;
    }
    gtk_container_add(GTK_CONTAINER(treeviewparent), treeview);
    gtk_widget_show(treeview);
}

void edit_button_cb(GtkButton *button, gpointer data)
{
    GtkWidget *treeview = NULL, *treeviewparent = NULL;
    GtkTreeSelection *treeselection;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    struct Product *p;
    struct button_callback_struct *b_cb_s;
    int *index;
    b_cb_s = data;

    treeview = get_child_by_name(GTK_WIDGET(b_cb_s->window), "itemlist");
    if (!treeview) {
        gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
        return;
    }
    treeselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(treeselection, &model, &iter))
        return;
    path = gtk_tree_model_get_path(model, &iter);
    index = gtk_tree_path_get_indices(path);

    p = edit_dialog_run(b_cb_s->window, (*b_cb_s->inv)->i[*index]);
    if (p == NULL) return;
    (*b_cb_s->inv)->i[*index] = p;
    inventory_sort(*b_cb_s->inv, 0, (*b_cb_s->inv)->l - 1);

    treeviewparent = gtk_widget_get_parent(treeview);
    gtk_widget_destroy(treeview);
    treeview = GTK_WIDGET(setup_main_window_tree_view(*b_cb_s->inv));
    if (!treeview) {
        gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
        return;
    }
    gtk_container_add(GTK_CONTAINER(treeviewparent), treeview);
    gtk_widget_show(treeview);
}

void remove_button_cb(GtkButton *button, gpointer data)
{
    GtkWidget *treeview = NULL, *treeviewparent = NULL;
    GtkTreeSelection *treeselection;
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    struct button_callback_struct *b_cb_s;
    int *index;
    b_cb_s = data;

    treeview = get_child_by_name(GTK_WIDGET(b_cb_s->window), "itemlist");
    if (!treeview) {
        gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
        return;
    }
    treeselection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    if (!gtk_tree_selection_get_selected(treeselection, &model, &iter))
        return;
    path = gtk_tree_model_get_path(model, &iter);
    index = gtk_tree_path_get_indices(path);

    inventory_remove(*b_cb_s->inv, *index);

    treeviewparent = gtk_widget_get_parent(treeview);
    gtk_widget_destroy(treeview);
    treeview = GTK_WIDGET(setup_main_window_tree_view(*b_cb_s->inv));
    if (!treeview) {
        gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
        return;
    }
    gtk_container_add(GTK_CONTAINER(treeviewparent), treeview);
    gtk_widget_show(treeview);
}

void open_button_cb(GtkButton *button, gpointer data)
{
    GtkWidget *dialog, *treeview, *treeviewparent;
    struct button_callback_struct *b_cb_s;
    struct Inventory *tmp;
    char *filename = NULL;
    int res, lerrno;
    b_cb_s = data;
    dialog = gtk_file_chooser_dialog_new("Open Inventory File", b_cb_s->window,
            GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL,
            "Open", GTK_RESPONSE_ACCEPT, NULL);
    if (!dialog)
        goto exit;
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (inventory_deserialize(&tmp, filename)) {
            lerrno = errno;
            GtkWidget *warndialog;
            warndialog = gtk_message_dialog_new(b_cb_s->window,
                    GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                    "Error reading from %s: %s", filename, strerror(errno));
            if (!warndialog)
                goto exit;
            gtk_dialog_run(GTK_DIALOG(warndialog));
            gtk_widget_destroy(warndialog);
            errno = lerrno;
            goto exit;
        }
        inventory_destroy(*b_cb_s->inv);
        *b_cb_s->inv = tmp;
        update_status_bar(NULL, NULL, (*b_cb_s->inv)->title);

        treeview = get_child_by_name(GTK_WIDGET(b_cb_s->window), "itemlist");
        if (!treeview) {
            gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
            goto exit;
        }
        treeviewparent = gtk_widget_get_parent(treeview);
        gtk_widget_destroy(treeview);
        treeview = GTK_WIDGET(setup_main_window_tree_view(*b_cb_s->inv));
        if (!treeview) {
            gtk_widget_destroy(GTK_WIDGET(b_cb_s->window));
            goto exit;
        }
        gtk_container_add(GTK_CONTAINER(treeviewparent), treeview);
        gtk_widget_show(treeview);
    }

exit:
    lerrno = errno;
    if (dialog)
        gtk_widget_destroy(dialog);
    if (filename)
        g_free(filename);
    errno = lerrno;
}

void save_button_cb(GtkButton *button, gpointer data)
{
    GtkWidget *dialog;
    struct button_callback_struct *b_cb_s;
    char *filename = NULL;
    int res, lerrno;
    b_cb_s = data;
    dialog = gtk_file_chooser_dialog_new("Save Inventory File", b_cb_s->window,
            GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL,
            "Save", GTK_RESPONSE_ACCEPT, NULL);
    if (!dialog)
        goto exit;
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
            (*b_cb_s->inv)->title);
    res = gtk_dialog_run(GTK_DIALOG(dialog));
    if (res == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (inventory_serialize(*b_cb_s->inv, filename)) {
            lerrno = errno;
            GtkWidget *warndialog;
            warndialog = gtk_message_dialog_new(b_cb_s->window,
                    GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                    GTK_MESSAGE_ERROR, GTK_BUTTONS_CLOSE,
                    "Error saving to %s: %s", filename, strerror(errno));
            if (!warndialog)
                goto exit;
            gtk_dialog_run(GTK_DIALOG(warndialog));
            gtk_widget_destroy(warndialog);
            errno = lerrno;
            goto exit;
        }
        update_status_bar(NULL, NULL, (*b_cb_s->inv)->title);
    }

exit:
    lerrno = errno;
    if (filename)
        g_free(filename);
    if (dialog)
        gtk_widget_destroy(dialog);
    errno = lerrno;
}

GtkTreeView *setup_main_window_tree_view(struct Inventory *inv)
{
    GtkWidget *itemlist = NULL;
    GtkTreeViewColumn *itemlistcolumn = NULL;
    GtkCellRenderer *cellrenderer = NULL;
    GtkListStore *itemliststore = NULL;
    GtkTreeIter iter;
    char *pricestr = NULL;
    size_t i, pricestrlen;
    int lerrno;

    itemlist = gtk_tree_view_new();
    if (!itemlist)
        goto exit_err;
    gtk_widget_set_name(itemlist, "itemlist");
    itemliststore = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_ULONG, G_TYPE_STRING);
    if (!itemliststore)
        goto exit_err;

    for (i = 0; i < inv->l; i++) {
        pricestrlen = snprintf(NULL, 0, "$%lu.%02lu", inv->i[i]->cost / 100,
                inv->i[i]->cost % 100) + 1;
        pricestr = malloc(pricestrlen * sizeof(char));
        if (!pricestr)
            goto exit_err;
        snprintf(pricestr, pricestrlen, "$%lu.%02lu", inv->i[i]->cost / 100,
                inv->i[i]->cost % 100);

        gtk_list_store_append(itemliststore, &iter);
        gtk_list_store_set(itemliststore, &iter, 0, inv->i[i]->name, 1,
                inv->i[i]->amt, 2, pricestr, -1);
    }

    gtk_tree_view_set_model(GTK_TREE_VIEW(itemlist), GTK_TREE_MODEL(itemliststore));

    cellrenderer = gtk_cell_renderer_text_new();
    if (!cellrenderer)
        goto exit_err;
    gtk_cell_renderer_set_alignment(cellrenderer, 1, 0.5);

    itemlistcolumn = gtk_tree_view_column_new_with_attributes("Name",
            cellrenderer, "text", 0, NULL);
    if (!itemlistcolumn)
        goto exit_err;
    gtk_tree_view_column_set_expand(itemlistcolumn, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(itemlist), itemlistcolumn);
    itemlistcolumn = gtk_tree_view_column_new_with_attributes("Amount",
            cellrenderer, "text", 1, NULL);
    if (!itemlistcolumn)
        goto exit_err;
    gtk_tree_view_column_set_expand(itemlistcolumn, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(itemlist), itemlistcolumn);
    itemlistcolumn = gtk_tree_view_column_new_with_attributes("Price",
            cellrenderer, "text", 2, NULL);
    if (!itemlistcolumn)
        goto exit_err;
    gtk_tree_view_column_set_expand(itemlistcolumn, TRUE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(itemlist), itemlistcolumn);

    goto exit_clean;
exit_err:
    lerrno = errno;
    if (cellrenderer)
        g_object_unref(G_OBJECT(cellrenderer));
    if (itemliststore)
        g_object_unref(G_OBJECT(itemliststore));
    if (itemlistcolumn)
        g_object_unref(G_OBJECT(itemlistcolumn));
    if (itemlist)
        g_object_unref(G_OBJECT(itemlist));
    errno = lerrno;
    return NULL;
exit_clean:
    return GTK_TREE_VIEW(itemlist);
}

GtkWindow *setup_main_window(GtkWindow *owindow, struct Inventory *oinv)
{
    GtkWidget *window = NULL;
    int lerrno;
    static struct Inventory *inv;
    inv = oinv;

    if (!owindow) {
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        if (!window)
            goto exit_err;
        gtk_window_set_title(GTK_WINDOW(window), "Inventory Manager");
        gtk_window_set_default_size(GTK_WINDOW(window), 640, 400);
    } else {
        destroy_all_children(GTK_CONTAINER(owindow));
        window = GTK_WIDGET(owindow);
    }
    g_signal_connect(window, "destroy", G_CALLBACK(destroy_signal_callback),
            NULL);

    GtkWidget *windowbox = NULL;
    windowbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    if (!windowbox)
        goto exit_err;
    gtk_container_add(GTK_CONTAINER(window), windowbox);
    gtk_widget_show(windowbox);

    GtkWidget *mainbox = NULL;
    mainbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    if (!mainbox)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(windowbox), mainbox, TRUE, TRUE, 0);
    gtk_widget_show(mainbox);

    GtkWidget *statusbar = NULL;
    int statusbar_context_id;
    statusbar = gtk_statusbar_new();
    if (!statusbar)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(windowbox), statusbar, FALSE, TRUE, 0);
    gtk_widget_show(statusbar);
    statusbar_context_id = gtk_statusbar_get_context_id(
            GTK_STATUSBAR(statusbar), "Inventory name");
    update_status_bar(statusbar, &statusbar_context_id, inv->title);

    GtkWidget *buttonbox = NULL;
    buttonbox = gtk_button_box_new(GTK_ORIENTATION_VERTICAL);
    if (!buttonbox)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(mainbox), buttonbox, FALSE, TRUE, 0);
    gtk_box_set_spacing(GTK_BOX(buttonbox), 5);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(buttonbox), GTK_BUTTONBOX_START);
    gtk_widget_show(buttonbox);

    static struct button_callback_struct b_cb_s;
    b_cb_s.window = GTK_WINDOW(window);
    b_cb_s.inv = &inv;

    GtkWidget *addbutton = NULL;
    addbutton = gtk_button_new_from_icon_name("list-add", GTK_ICON_SIZE_BUTTON);
    if (!addbutton)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(buttonbox), addbutton, FALSE, TRUE, 0);
    g_signal_connect(G_OBJECT(addbutton), "clicked", G_CALLBACK(add_button_cb),
            &b_cb_s);
    gtk_widget_show(addbutton);

    GtkWidget *editbutton = NULL;
    editbutton = gtk_button_new_from_icon_name("edit", GTK_ICON_SIZE_BUTTON);
    if (!editbutton)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(buttonbox), editbutton, FALSE, TRUE, 0);
    g_signal_connect(G_OBJECT(editbutton), "clicked", G_CALLBACK(edit_button_cb), &b_cb_s);
    gtk_widget_show(editbutton);

    GtkWidget *removebutton = NULL;
    removebutton = gtk_button_new_from_icon_name("list-remove",
            GTK_ICON_SIZE_BUTTON);
    if (!removebutton)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(buttonbox), removebutton, FALSE, TRUE, 0);
    g_signal_connect(G_OBJECT(removebutton), "clicked",
            G_CALLBACK(remove_button_cb), &b_cb_s);
    gtk_widget_show(removebutton);

    GtkWidget *openbutton = NULL;
    openbutton = gtk_button_new_from_icon_name("fileopen",
            GTK_ICON_SIZE_BUTTON);
    if (!openbutton)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(buttonbox), openbutton, FALSE, TRUE, 0);
    g_signal_connect(G_OBJECT(openbutton), "clicked",
            G_CALLBACK(open_button_cb), &b_cb_s);
    gtk_widget_show(openbutton);

    GtkWidget *savebutton = NULL;
    savebutton = gtk_button_new_from_icon_name("filesave",
            GTK_ICON_SIZE_BUTTON);
    if (!savebutton)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(buttonbox), savebutton, FALSE, TRUE, 0);
    g_signal_connect(G_OBJECT(savebutton), "clicked",
            G_CALLBACK(save_button_cb), &b_cb_s);
    gtk_widget_show(savebutton);

    GtkWidget *scrolledwindow = NULL;
    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    if (!scrolledwindow)
        goto exit_err;
    gtk_box_pack_start(GTK_BOX(mainbox), scrolledwindow, TRUE, TRUE, 0);
    gtk_widget_show(scrolledwindow);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow),
            GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    GtkWidget *treeview = NULL;
    treeview = GTK_WIDGET(setup_main_window_tree_view(inv));
    if (!treeview)
        goto exit_err;
    gtk_container_add(GTK_CONTAINER(scrolledwindow), treeview);
    gtk_widget_show(treeview);

    goto exit_clean;
exit_err:
    lerrno = errno;
    if (inv)
        inventory_destroy(inv);
    if (window)
        gtk_widget_destroy(window);
    errno = lerrno;
    return NULL;

exit_clean:
    return GTK_WINDOW(window);
}
