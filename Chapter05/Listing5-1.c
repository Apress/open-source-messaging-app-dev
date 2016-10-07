#include <gtk/gtk.h>
 
static void activate_cb(GtkWidget *entry, void *data)
{
     const char *text = gtk_entry_get_text(entry);
     printf("%s\n", text);
}

int main(int argc, char *argv[])
{
     GtkWidget *window, *hbox, *entry, *label;
     gtk_init(&argc, &argv);
     window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title(GTK_WINDOW(window), "Window");
     hbox = gtk_hbox_new(FALSE, 6);
     gtk_container_add(GTK_CONTAINER(window), hbox);
     label = gtk_label_new("Text");
     gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
     entry = gtk_entry_new();
     g_signal_connect(G_OBJECT(entry), "activate", activate_cb, NULL);
     gtk_box_pack_start(GTK_BOX(hbox), entry, TRUE, TRUE, 0);
     gtk_widget_show_all(window);
     gtk_main();
     return 0;
}
