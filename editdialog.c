#include <stdio.h>
#include <gtk/gtk.h>
#include <errno.h>
#include "product.h"
#include "editdialog.h"

struct Product *edit_dialog_run(GtkWindow *parent, struct Product *p)
{
    GtkWidget *dialog = NULL, *content_area = NULL, *fields = NULL;
    struct Product *rp = NULL;
    int lerrno = 0;
    errno = 0;

    dialog = gtk_dialog_new_with_buttons("Product Editor", parent,
            GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
            "Cancel", GTK_RESPONSE_REJECT, "OK", GTK_RESPONSE_ACCEPT, NULL);
    if (!dialog)
        goto exit;

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    fields = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    if (!fields)
        goto exit;
    gtk_container_add(GTK_CONTAINER(content_area), fields);
    gtk_widget_show(fields);

    GtkWidget *namearea = NULL, *namelabel = NULL, *nametext = NULL;
    namearea = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); 
    if (!namearea)
        goto exit;
    gtk_box_pack_start(GTK_BOX(fields), namearea, FALSE, TRUE, 0);
    gtk_widget_show(namearea);
    namelabel = gtk_label_new("Name");
    if (!namelabel)
        goto exit;
    gtk_box_pack_start(GTK_BOX(namearea), namelabel, TRUE, TRUE, 0);
    gtk_widget_show(namelabel);
    nametext = gtk_entry_new();
    if (!nametext)
        goto exit;
    gtk_box_pack_start(GTK_BOX(namearea), nametext, TRUE, TRUE, 0);
    gtk_widget_show(nametext);

    GtkWidget *amtarea = NULL, *amtlabel = NULL, *amttext = NULL;
    amtarea = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); 
    if (!amtarea)
        goto exit;
    gtk_box_pack_start(GTK_BOX(fields), amtarea, FALSE, TRUE, 0);
    gtk_widget_show(amtarea);
    amtlabel = gtk_label_new("Amount");
    if (!amtlabel)
        goto exit;
    gtk_box_pack_start(GTK_BOX(amtarea), amtlabel, TRUE, TRUE, 0);
    gtk_widget_show(amtlabel);
    amttext = gtk_entry_new();
    if (!amttext)
        goto exit;
    gtk_box_pack_start(GTK_BOX(amtarea), amttext, TRUE, TRUE, 0);
    gtk_widget_show(amttext);

    GtkWidget *pricearea = NULL, *pricelabel = NULL, *pricetext = NULL;
    pricearea = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0); 
    if (!pricearea)
        goto exit;
    gtk_box_pack_start(GTK_BOX(fields), pricearea, FALSE, TRUE, 0);
    gtk_widget_show(pricearea);
    pricelabel = gtk_label_new("Price (eg $1.50)");
    if (!pricelabel)
        goto exit;
    gtk_box_pack_start(GTK_BOX(pricearea), pricelabel, TRUE, TRUE, 0);
    gtk_widget_show(pricelabel);
    pricetext = gtk_entry_new();
    if (!pricetext)
        goto exit;
    gtk_box_pack_start(GTK_BOX(pricearea), pricetext, TRUE, TRUE, 0);
    gtk_widget_show(pricetext);

    if (p) {
        char *amtstr, *pricestr;
        int n;
        size_t len;
        gtk_entry_set_text(GTK_ENTRY(nametext), p->name);

        n = snprintf(NULL, 0, "%lu", p->amt);
        if (n < 0)
            goto exit;
        len = n + 1;
        amtstr = malloc(len * sizeof(char));
        if (!amtstr)
            goto exit;
        snprintf(amtstr, len, "%lu", p->amt);
        gtk_entry_set_text(GTK_ENTRY(amttext), amtstr);
        free(amtstr);

        n = snprintf(NULL, 0, "$%lu.%02lu", p->cost / 100, p->cost % 100);
        if (n < 0)
            goto exit;
        len = n + 1;
        pricestr = malloc(len * sizeof(char));
        if (!pricestr)
            goto exit;
        snprintf(pricestr, len, "$%lu.%02lu", p->cost / 100, p->cost % 100);
        gtk_entry_set_text(GTK_ENTRY(pricetext), pricestr);
        free(pricestr);
    }

    int response = gtk_dialog_run(GTK_DIALOG(dialog));
    unsigned long amt, price, cents, dollars;
    if (response == GTK_RESPONSE_ACCEPT) {
        const char *namestr = gtk_entry_get_text(GTK_ENTRY(nametext));
        const char *amtstr = gtk_entry_get_text(GTK_ENTRY(amttext));
        const char *pricestr = gtk_entry_get_text(GTK_ENTRY(pricetext));

        if (pricestr[0] == '$')
            pricestr++;
        if (sscanf(pricestr, "%lu.%lu", &dollars, &cents) == 2)
            price = dollars * 100 + cents;
        else if (sscanf(pricestr, "%lu", &dollars) == 1)
            price = dollars * 100;
        else
            price = 0;
        if (sscanf(amtstr, "%lu", &amt) != 1)
            amt = 0;

        rp = product_create(namestr, price, amt);
        if (!rp)
            goto exit;
    } else {
        rp = p;
    }

exit: ;
    lerrno = errno;
    if (dialog)
        gtk_widget_destroy(dialog);
    errno = lerrno;
    return rp;
}
