#include "internal.h"
#include "plugin.h"
#include "notify.h"
#include "version.h"

static gboolean
plugin_load(GaimPlugin *plugin)
{
     gaim_notify_info(NULL, NULL, "Hello World!", "I've written a plugin");
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
     "helloworld",
     "Hello World",
     "1.0",
     "Example plugin",
     "Merely display a \"hello world\" dialog when loaded.",
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

GAIM_INIT_PLUGIN(history, init_plugin, info)

