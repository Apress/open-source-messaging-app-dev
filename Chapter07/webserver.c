#include "internal.h"
#include "plugin.h"
#include "notify.h"
#include "version.h"

#include "gtkconv.h"
#include "gtkimhtml.h"

GaimPlugin *plugin_handle = NULL;

static char *create_response()	
{
	GList *convs = gaim_get_conversations();
	GString *str = g_string_new("HTTP/1.1 200 OK\r\n"
				    "Content-Type: text/html\r\n"
				    "\r\n"
				    "<html><head><title>"
				    "Gaim Conversations</title></head><body>");
	while (convs) {	
		GaimConversation *conv = convs->data;
		GaimGtkConversation *gtkconv = GAIM_GTK_CONVERSATION(conv);
		if (gaim_conversation_get_type(conv) != GAIM_CONV_IM) {
			convs = convs->next;
			continue;
		}
		char *html = gtk_imhtml_get_markup(GTK_IMHTML(gtkconv->imhtml));
		str = g_string_append(str, "<h1>");
		str = g_string_append(str, gaim_conversation_get_name(conv));
		str = g_string_append(str, "</h1>");
		str = g_string_append(str, html);
		g_free(html);
		convs = convs->next;
	}
	str = g_string_append(str, "</body></html>");
	return g_string_free(str, FALSE);
}	


static gboolean incoming_cb(GIOChannel *source,
			    GIOCondition condition,
			    gpointer data)
{
	int cfd;
	struct sockaddr_in sockaddr;
	int socklen = sizeof(sockaddr);
	char buf[256];
	char *resp=create_response();
	int fd = g_io_channel_unix_get_fd(source);
	if (condition != G_IO_IN)
		return TRUE;


	if ((cfd = accept(fd, (struct sockaddr*)&sockaddr, &socklen)) < 0)
		return TRUE;
	recv(cfd, buf, sizeof(buf), 0);
	/* Send response here */
	send(cfd, resp, strlen(resp), 0);
	close(cfd);
	return TRUE;
}

static gboolean
plugin_load(GaimPlugin *plugin)
{
	struct sockaddr_in sockaddr;
	int fd;

	plugin_handle = plugin;
	fd = socket(PF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		return FALSE;
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = INADDR_ANY;
	sockaddr.sin_port = htons(1118);

	if ((bind(fd, (struct sockaddr*)&sockaddr, sizeof(sockaddr))) < 0)
		return FALSE;
	listen(fd, 1);
	g_io_add_watch(g_io_channel_unix_new(fd), G_IO_IN, incoming_cb, NULL);
	return TRUE;
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
     "webserver",
     "Remote Web Access",
     "1.0",
     "Sends pending messages over the web when requested",
     "Implements a basic web server which listens on the network "
     "for incoming connections, and processes requests by sending "
     "the complete text within each open IM conversation",
     "Sean Egan <seanegan@gmail.com>",
     "http://apress.com",
     plugin_load,
     NULL,
     NULL,
     NULL,
     NULL,
     NULL
};

 static void
 init_plugin(GaimPlugin *plugin)
 {
 }

GAIM_INIT_PLUGIN(webserver, init_plugin, info)
