#include "internal.h"
#include "plugin.h"
#include "notify.h"
#include "util.h"
#include "version.h"
#include "gtkutils.h"
#include "gtkimhtml.h"

enum {
     TIME_COLUMN,
     SENDER_COLUMN,
     URL_COLUMN,
     DATA_COLUMN,
     MESSAGE_COLUMN,
     COLUMN_COUNT
};
struct url_data {
     char *url;
     char *message;
     char *sender;
     char *time;
};

GaimPlugin *plugin_handle = NULL;
GSList *urls = NULL;
GtkWidget *url_window = NULL;

static void
window_destroy_cb(GtkWidget *widget, gpointer data)
{
     url_window = NULL;
}

static void
view_url_cb(GtkWidget *widget, gpointer data)
{
     GtkTreeSelection *selection = GTK_TREE_SELECTION(data);
     GtkTreeIter iter;
     GtkTreeModel *model;
     gchar *url;

     if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
          gtk_tree_model_get (model, &iter,
			      URL_COLUMN, &url, -1);
	  gaim_notify_uri(plugin_handle, url);
     }
}

static void
tree_selection_changed_cb (GtkTreeSelection *selection, gpointer data)
{
        GtkWidget *imhtml = GTK_WIDGET(data);
        GtkTreeIter iter;
        GtkTreeModel *model;
        gchar *sender, *text, *message;

        if (gtk_tree_selection_get_selected (selection, &model, &iter)) {
           	gtk_tree_model_get (model, &iter, MESSAGE_COLUMN, &text,
				    SENDER_COLUMN, &sender, -1);
		message = g_strdup_printf("<font color=\"#A82F2F\"><b>%s: </b></font>%s",
					  sender, text);
		gtk_imhtml_clear(GTK_IMHTML(imhtml));
		gtk_imhtml_append_text(GTK_IMHTML(imhtml), message, 0);
		g_free(message);
        }
}

static void show_dialog(GaimPluginAction *action)
{
  GtkWidget *vbox, *bbox, *sw, *tree, *imhtml, *button;
  GtkListStore *list_store;
  GtkTreeViewColumn *col;
  GtkCellRenderer *rend;
  GSList *l;
  GtkTreeIter iter;
  GtkTreeSelection *sel;
  
     if (url_window != NULL) {
       gtk_window_present(GTK_WINDOW(url_window));    
          return;
     }
     url_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
     gtk_window_set_default_size(GTK_WINDOW(url_window), 400, 500);
     gtk_window_set_title(GTK_WINDOW(url_window), "URL Catcher");
     gtk_container_set_border_width(GTK_CONTAINER(url_window), 12);
     g_signal_connect(G_OBJECT(url_window), "destroy", G_CALLBACK(window_destroy_cb), NULL);     
     vbox = gtk_vbox_new(FALSE, 6);
     gtk_container_add(GTK_CONTAINER(url_window), vbox);
     list_store = gtk_list_store_new(COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING,
				     G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
     tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));

     rend = gtk_cell_renderer_text_new ();
     col = gtk_tree_view_column_new_with_attributes ("Date",
						     rend,
						     "text", TIME_COLUMN,
						     NULL);
     gtk_tree_view_append_column (GTK_TREE_VIEW (tree), col);

     rend = gtk_cell_renderer_text_new ();
     col = gtk_tree_view_column_new_with_attributes ("Sender",
						     rend,
						     "text", SENDER_COLUMN,
						     NULL);
     gtk_tree_view_append_column (GTK_TREE_VIEW (tree), col);

     rend = gtk_cell_renderer_text_new ();
     col = gtk_tree_view_column_new_with_attributes ("URL",
						     rend,
						     "text", URL_COLUMN,
						     NULL);
     gtk_tree_view_append_column (GTK_TREE_VIEW (tree), col);

     l = urls;
     while (l) {
       struct url_data *data = (struct url_data*)(l->data);
       gtk_list_store_append (list_store, &iter);
       gtk_list_store_set (list_store, &iter,
			   TIME_COLUMN, data->time,
			   SENDER_COLUMN, data->sender,
			   MESSAGE_COLUMN, data->message,
			   URL_COLUMN, data->url,
			   DATA_COLUMN, data,
			   -1);
       l = l->next;
     }
     
     sw = gtk_scrolled_window_new(NULL, NULL);
     gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

     gtk_container_add(GTK_CONTAINER(sw), tree);
     gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);

     imhtml = gtk_imhtml_new(NULL, NULL);
     gaim_setup_imhtml(imhtml);
     sw = gtk_scrolled_window_new(NULL, NULL);
     gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw), GTK_SHADOW_IN);
     gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);

     gtk_container_add(GTK_CONTAINER(sw), imhtml);
     gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, 0);
     
     sel = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
     gtk_tree_selection_set_mode (sel, GTK_SELECTION_SINGLE);
     g_signal_connect (G_OBJECT (sel), "changed",
		       G_CALLBACK (tree_selection_changed_cb),
		       imhtml); 

     bbox = gtk_hbutton_box_new();
     gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
     gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 6);
     gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

     button = gtk_button_new_with_label("View URL");
     g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(view_url_cb), sel);
     gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);
     
     button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
     g_signal_connect_swapped(G_OBJECT(button), "clicked", 
			      G_CALLBACK(gtk_widget_destroy), url_window);
     gtk_box_pack_start(GTK_BOX(bbox), button, FALSE, FALSE, 0);

     
     gtk_widget_show_all(url_window);
}

static char *check_for_urls(char *message)
{
     /* character pointers are often used as the
      * beginning of a string, but can also point
      * anywhere within a string. I'm trying to
      * isolate the URL from the message argument.
      * I'll use n to point to the start of the URL and
      * m to point to the end. ret is where I'll store
      * the string I'll return.
      */
  char *n, *m, *ret;
     
     /* I'll assume that all URLs will start with "http://"
      * strstr() sets n to the location in the string where
      * "http://" starts or, if it's not found, NULL. If it's
      * not found, the message doesn't contain a link, and so the
      * function returns NULL.
      */
     n = strstr(message, "http://");
     if (n == NULL)
          return NULL;

     /* Now that n points to the beginning of the link, I will set
      * m to that location (plus strlen("http://") and traverse the
      * string until I get to a charcter that's invalid in a URL.
      * That will be the end of the link.
      */
     m = n + strlen("http://");
     while(*m != '\0' && 
	   *m != ' ' &&
	   *m != ',' &&
	   *m != '\n' &&
	   *m != '<' &&
	   *m != '>' &&
	   *m != '"' &&
	   *m != '\\')
       m++;

     /* Now I should have pointers to the beginning and end of the URL, which
      * means the length is everything between the two. I'll allocate a string for
      * it. I need to add 1 to store the terminating NUL character.
      */     
     ret = g_malloc(m - n + 1);

     /* And then I copy the URL into this buffer and return it. You often need to
      * manually NULL-terminate strings after using strncpy() because strncpy()
      * won't necessarily do it itself. */
     strncpy(ret, n, m-n);
     ret[m-n] = '\0';
     return ret;
}

static void incoming_msg_cb(GaimAccount *account, char *sender, char *message,
			    GaimConversation *conv, int flags)
{
     struct url_data *url;
     const char *timestamp = gaim_date();
     char *link;
     if ((link = check_for_urls(message)) == NULL)
          return;

     url = g_malloc(sizeof(struct url_data));
     url->url = link;
     url->message = g_strdup(message);
     url->sender = g_strdup(sender);
     url->time = g_strdup(timestamp);
     urls = g_slist_append(urls, url);
}

static gboolean
plugin_load(GaimPlugin *plugin)
{       
     void *handle = gaim_conversations_get_handle();
     plugin_handle = plugin;
     gaim_signal_connect(handle, "received-im-msg", plugin, 
			 GAIM_CALLBACK(incoming_msg_cb), NULL);
     gaim_signal_connect(handle, "received-chat-msg", plugin, 
			 GAIM_CALLBACK(incoming_msg_cb), NULL);
     return TRUE;
}

static GList *
actions(GaimPlugin *plugin, gpointer context)
{
	GList *l = NULL;
	GaimPluginAction *act = NULL;

	act = gaim_plugin_action_new("Show received URL's",
	                             show_dialog);
	l = g_list_append(l, act);

	return l;
}

static GaimPluginInfo info =
{
     GAIM_PLUGIN_MAGIC,
     GAIM_MAJOR_VERSION,
     GAIM_MINOR_VERSION,     
     GAIM_PLUGIN_STANDARD,
     NULL,
     0,
     NULL,
     GAIM_PRIORITY_DEFAULT,
     "urlcatcher",
     "URL Catcher",
     "1.0",
     "Catches URLs from incoming messages",
     "Detects URLs from within incoming messages and provides an "
     "interface for viewing them.",
     "Sean Egan <seanegan@gmail.com>",
     "http://apress.com",
     plugin_load,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL,
     actions
};

 static void
 init_plugin(GaimPlugin *plugin)
 {
 }

GAIM_INIT_PLUGIN(urlcatcher, init_plugin, info)
