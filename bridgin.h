
#define PURPLE_PLUGINS

#define PLUGIN_TYPE PURPLE_PLUGIN_STANDARD
#define PLUGIN_GUI_TYPE NULL
#define PLUGIN_ID "core-mr-jonze-bridgin"
#define PLUGIN_NAME "bridgin"
#define PLUGIN_VERSION "0.5s"
#define PLUGIN_SHORT_DESC "short description"
#define PLUGIN_LONG_DESC "long description"
#define PLUGIN_AUTHOR "bill auger <mr.j.spam.me@gmail.com>"
#define PLUGIN_WEBSITE "http://bill-auger.github.com/bridgin"
#define PLUGIN_ONLOAD_CB plugin_load
#define PLUGIN_ONUNLOAD_CB NULL


//#include <glib.h>

//#include "conversation.h"
#include "debug.h"
#include "notify.h"
#include "plugin.h"
//#include "signals.h"
#include "version.h"


static PurplePluginInfo info ;

// helpers
static void systemChat(PurpleConversation* conv , const char* msg) ;

// event handlers
static gboolean plugin_load(PurplePlugin *plugin) ;
static void init_plugin(PurplePlugin *plugin) ;
static void received_im_msg_cb(PurpleAccount* account , char* sender , char* buffer ,
    PurpleConversation* conv , PurpleMessageFlags flags , void* data) ;
static void received_chat_msg_cb(PurpleAccount* account , char* sender , char* buffer ,
    PurpleConversation* conv , PurpleMessageFlags flags , void* data) ;
static void deleting_conversation_cb(PurpleConversation *conv, void *data) ;
