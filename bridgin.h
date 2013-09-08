
#define PURPLE_PLUGINS

// app constants
#define PLUGIN_TYPE        PURPLE_PLUGIN_STANDARD
#define PLUGIN_GUI_TYPE    NULL
#define PLUGIN_ID          "core-mr-jonze-bridgin"
#define PLUGIN_NAME        "bridgin"
#define PLUGIN_VERSION     "0.5s"
#define PLUGIN_SHORT_DESC  "short description"
#define PLUGIN_LONG_DESC   "long description"
#define PLUGIN_AUTHOR      "bill auger <mr.j.spam.me@gmail.com>"
#define PLUGIN_WEBSITE     "https://github.com/bill-auger/bridgin"
#define PLUGIN_ONLOAD_CB   HandlePluginLoaded
#define PLUGIN_ONUNLOAD_CB HandlePluginUnloaded
#define BRIDGIN_NICK       "BRIDGIN"

// admin commands
#define N_COMMANDS    13
#define BINARY_FMT    "s"
#define UNARY_FMT     ""
#define ADD_CMD      "add"
#define ADDu_HELP     "/ADD_CMD\nadd this channel to the default bridge"
#define ADDb_HELP     "/ADD_CMD 'a-bridge-name'\nadd this channel to the bridge 'a-bridge-name'"
#define ADD_CB        HandleAddCmd
#define REMOVE_CMD    "rem"
#define REMOVE_HELP   "/REMOVE_CMD\nunbridge this channel"
#define REMOVE_CB     HandleRemoveCmd
#define DISABLE_CMD  "disable"
#define DISABLEu_HELP "/DISABLE_CMD\ntemporarily disable all bridges"
#define DISABLEb_HELP "/DISABLE_CMD 'a-bridge-name'\ntemporarily disable the bridge 'a-bridge-name'"
#define ENABLE_CMD   "enable"
#define ENABLEu_HELP  "/ENABLE_CMD\nenable all bridges"
#define ENABLEb_HELP  "/ENABLE_CMD 'a-bridge-name'\nenable the bridge 'a-bridge-name'"
#define ENABLE_CB     HandleEnableCmd
#define ECHO_CMD      "echo"
#define ECHO_HELP     "/ECHO_CMD\necho text to the same channel"
#define ECHO_CB       HandleEchoCmd
#define CHAT_CMD      "chat"
#define CHAT_HELP     "/CHAT_CMD\nrelay text to the all channels on this bridge"
#define CHAT_CB       HandleChatCmd
#define BCAST_CMD     "broadcast"
#define BCAST_HELP    "/BROADCAST_CMD\nrelay text to the all channels on all bridges as BRIDGIN_NICK"
#define BCAST_CB      HandleBroadcastCmd
#define STATUS_CMD   "status"
#define STATUSu_HELP  "/STATUS_CMD\nshow status information for all bridges"
#define STATUSb_HELP  "/STATUS_CMD 'a-bridge-name'\nshow status information for the bridge 'a-bridge-name'"
#define STATUS_CB     HandleStatusCmd
#define HELP_CMD      "help"
#define HELP_HELP     "/HELP_CMD\nshow avaiable admin commands"
#define HELP_CB       HandleHelpCmd


//#include <glib.h>

#include "cmds.h"
//#include "conversation.h"
//#include "notify.h"
//#include "plugin.h"
//#include "signals.h"
#include "version.h"


static PurplePluginInfo PluginInfo ;
static PurpleCmdId CommandIds[N_COMMANDS] ;


// helpers
static PurpleCmdId RegisterCmd(const char* command , const char* format ,
                               PurpleCmdRet (* callback)() , const char* help) ;
static void        SystemChat(PurpleConversation* conv , const char* msg) ;

// event handlers
static void     HandlePluginInit(    PurplePlugin* plugin) ;
static gboolean HandlePluginLoaded(  PurplePlugin* plugin) ;
static gboolean HandlePluginUnloaded(PurplePlugin* plugin) ;
static void     HandleIm(  PurpleAccount* account , char* sender , char* buffer ,
                           PurpleConversation* conv , PurpleMessageFlags flags , void* data) ;
static void     HandleChat(PurpleAccount* account , char* sender , char* buffer ,
                           PurpleConversation* conv , PurpleMessageFlags flags , void* data) ;
static void     HandleChannelClosed(PurpleConversation* conv, void *data) ;

// callbacks
static PurpleCmdRet HandleAddCmd(      PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
static PurpleCmdRet HandleRemoveCmd(   PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
static PurpleCmdRet HandleEnableCmd(   PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
static PurpleCmdRet HandleEchoCmd(     PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
static PurpleCmdRet HandleChatCmd(     PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
static PurpleCmdRet HandleBroadcastCmd(PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
static PurpleCmdRet HandleStatusCmd(   PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
static PurpleCmdRet HandleHelpCmd(     PurpleConversation* conv , const gchar* cmd ,
                                       gchar** args , gchar** error , void* data) ;
