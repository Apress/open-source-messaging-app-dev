#include <gtk/gtk.h>
#include <stdio.h>

void button_clicked_cb(GtkWidget *button, GtkWidget *entry)
{
    const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
     printf("%s\n", text);
     
     /* gtk_main_quit causes the GTK+ main loop to terminate */
     gtk_main_quit();
}

int main(int argc, char **argv)
{
     GtkWidget *window;
     GtkWidget *vbox;
     GtkWidget *entry;
     GtkWidget *button;

     gtk_init(&argc, &argv);

     /* After initializing GTK+, but before entering the main loop,
      * we must create our intial GUI */
     window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title(GTK_WINDOW(window), "Example");
     vbox = gtk_vbox_new(FALSE, 0);
     
     /* Because a GtkWindow is also a GtkContainer, add the main
      * box used for layout into it as such.  Pack the other
      * widgets into that box */
     gtk_container_add(GTK_CONTAINER(window), vbox);
     entry = gtk_entry_new();
     gtk_box_pack_start(GTK_BOX(vbox), entry, FALSE, FALSE, 0);
     button = gtk_button_new_from_stock(GTK_STOCK_OK);
     gtk_box_pack_start(GTK_BOX(vbox), button, TRUE, TRUE, 0);

     /* Here connect to the "clicked" signal, causing clicked_cb
      * to be called when the button is clicked. By attaching
      * 'entry' as the data, clicked_cb will be able to get the
      * text from it. */
     g_signal_connect(G_OBJECT(button), "clicked", button_clicked_cb, entry);

     /* Remember that all widgets are invisible until 
      * gtk_widget_show() is called on them. gtk_widget_show_all()
      * recursively calls this on a container and all the children
      * within. */
     gtk_widget_show_all(window);

     /* Finally, all gtk_main(). This function will not return
      * until gtk_main_quit() is called from some callback. In
      * this example, it's called in clicked_cb(). */
     gtk_main();
     return 0;
}
