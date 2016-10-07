#include <gtk/gtk.h>

/* Three callback functions for the toolbar button items. Here
 * I only print text to the screen. In an actual application, I
 * would do more; open_cb(), for instance would open a file chooser
 * dialog, GtkFileChooserDialog, discussed later this section.
 */
void new_cb(GtkWidget *button, void *data)
{
     printf("New\n");
}
void open_cb(GtkWidget *button, void *data)
{
     printf("Open\n");
}
void save_cb(GtkWidget *button, void *data)
{
     printf("Save\n");
}
/* This callback is called when the window is closed.
 * I must manually tell GTK+ to exit when this happens.
 */
void destroy_cb(GtkWidget *window, void *data)
{
	gtk_main_quit();
}
int main(int argc, char *argv[])
{
     GtkWidget *window, *toolbar, *tool_button;
     gtk_init(&argc, &argv);
     window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
     gtk_window_set_title(GTK_WINDOW(window), "Window");
     g_signal_connect(G_OBJECT(window), "destroy", destroy_cb, NULL);
     toolbar = gtk_toolbar_new();

     /* I create and add each toolbar one at a time */
     tool_button = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
     g_signal_connect(G_OBJECT(tool_button), "clicked", new_cb, NULL);
     gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(tool_button), -1);

     /* If you won't need to modify the tool button anymore, you can reuse its
        * variable. GTK+ manages its memomry. But I can only reuse the variable
        * after making sure I no longer need this. I must, for instance, attach
        * callbacks to its signal before reusing the variable. 
        */
       tool_button = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
     g_signal_connect(G_OBJECT(tool_button), "clicked", open_cb, NULL);
       gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(tool_button), -1);

       tool_button = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
     g_signal_connect(G_OBJECT(tool_button), "clicked", save_cb, NULL);
       gtk_toolbar_insert(GTK_TOOLBAR(toolbar), GTK_TOOL_ITEM(tool_button), -1);

       /* Now I add the toolbar to the window */
       gtk_container_add(GTK_CONTAINER(window), toolbar);

       /* And I show the window and its contents with gtk_window_show_all() */
     gtk_widget_show_all(window);
     gtk_main();
     return 0;
}
