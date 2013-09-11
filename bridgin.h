
#define PURPLE_PLUGINS

// plugin constants
#define PLUGIN_TYPE        PURPLE_PLUGIN_STANDARD
#define PLUGIN_GUI_TYPE    NULL
#define PLUGIN_ID          "core-mr-jonze-bridgin"
#define PLUGIN_NAME        "bridgin"
#define PLUGIN_VERSION     "0.5pre"
#define PLUGIN_SHORT_DESC  "short description"
#define PLUGIN_LONG_DESC   "long description"
#define PLUGIN_AUTHOR      "bill auger <mr.j.spam.me@gmail.com>"
#define PLUGIN_WEBSITE     "https://github.com/bill-auger/bridgin"
#define PLUGIN_ONLOAD_CB   handlePluginLoaded
#define PLUGIN_ONUNLOAD_CB handlePluginUnloaded

// app constants
#define BRIDGIN_NICK        "BRIDGIN"
#define DEFAULT_BRIDGE_NAME "default"
#define NICK_PREFIX         "(from" // dont use '<' some clients will supress it as html
#define NICK_POSTFIX        ")"

// admin commands
#define N_COMMANDS    13
#define BINARY_FMT    "s"
#define UNARY_FMT     ""
#define ADD_CMD      "add"
#define ADDu_HELP     "/ADD_CMD\nadd this channel to the default bridge"
#define ADDb_HELP     "/ADD_CMD 'a-bridge-name'\nadd this channel to the bridge 'a-bridge-name'"
#define ADD_CB        handleAddCmd
#define REMOVE_CMD    "rem"
#define REMOVE_HELP   "/REMOVE_CMD\nunbridge this channel"
#define REMOVE_CB     handleRemoveCmd
#define DISABLE_CMD  "disable"
#define DISABLEu_HELP "/DISABLE_CMD\ntemporarily disable all bridges"
#define DISABLEb_HELP "/DISABLE_CMD 'a-bridge-name'\ntemporarily disable the bridge 'a-bridge-name'"
#define ENABLE_CMD   "enable"
#define ENABLEu_HELP  "/ENABLE_CMD\nenable all bridges"
#define ENABLEb_HELP  "/ENABLE_CMD 'a-bridge-name'\nenable the bridge 'a-bridge-name'"
#define ENABLE_CB     handleEnableCmd
#define ECHO_CMD      "echo"
#define ECHO_HELP     "/ECHO_CMD\necho text to the same channel"
#define ECHO_CB       handleEchoCmd
#define CHAT_CMD      "chat"
#define CHAT_HELP     "/CHAT_CMD\nrelay text to the all channels on this bridge"
#define CHAT_CB       handleChatCmd
#define BCAST_CMD     "broadcast"
#define BCAST_HELP    "/BROADCAST_CMD\nrelay text to the all channels on all bridges as BRIDGIN_NICK"
#define BCAST_CB      handleBroadcastCmd
#define STATUS_CMD   "status"
#define STATUSu_HELP  "/STATUS_CMD\nshow status information for all bridges"
#define STATUSb_HELP  "/STATUS_CMD 'a-bridge-name'\nshow status information for the bridge 'a-bridge-name'"
#define STATUS_CB     handleStatusCmd
#define HELP_CMD      "help"
#define HELP_HELP     "/HELP_CMD\nshow avaiable admin commands"
#define HELP_CB       handleHelpCmd

// admin command responses
#define CHAT_BUFFER_SIZE    8192
#define CH_SET_MSG          "channel set to bridge"
#define THIS_CHANNEL_MSG    "this channel"
#define CHANNEL_EXISTS_MSG  "already exists on bridge"
#define BRIDGE_CONFLICT_MSG "each channel may only be on one bridge"
#define NO_SUCH_BRIDGE_MSG  "no such bridge"
#define NO_BRIDGES_MSG      "no bridges exist"
#define STATS_MSGa          "bridge"
#define STATS_DELETED_MSG   "- deleted"
#define STATS_ENABLED_MSG   "- enabled -"
#define STATS_DISABLED_MSG  "- disabled -"
#define STATS_MSGb          "channels bridged"
#define CH_ACTIVE_MSG       "    ( active ) "
#define CH_INACTIVE_MSG     "    (inactive) "
#define OOM_MSG             "out of memory"

// model constants
#define PROTOCOL_BUFFER_SIZE 256
#define IRC_PROTOCOL         "IRC"

// purple constants
#define RECEIVED_IM_SIGNAL     "received-im-msg"
#define RECEIVED_CHAT_SIGNAL   "received-chat-msg"
#define CHANNEL_CLOSING_SIGNAL "deleting-conversation"


#include <string.h>

#include "cmds.h"
//#include "conversation.h"
//#include "notify.h"
//#include "plugin.h"
//#include "signals.h"
#include "version.h"


typedef struct Channel
{
  const char*         name ;
  char                protocol[PROTOCOL_BUFFER_SIZE] ;
  PurpleAccount*      account ;
  const char*         username  ;
  PurpleConversation* conv ;
  gboolean            isActive ;
  struct Channel*     next ;
} Channel ;

typedef struct Bridge
{
  const char*    name ;
  gboolean       isEnabled ;
  Channel*       sentinelChannel ;
  struct Bridge* next ;
} Bridge ;

static PurplePluginInfo PluginInfo ;             // init pre main()
static PurplePlugin*    ThisPlugin ;             // init on handlePluginLoaded()
static PurpleCmdId      CommandIds[N_COMMANDS] ; // init on handlePluginLoaded()
static char             ChatBuffer[CHAT_BUFFER_SIZE] ;
static gboolean         ChatMutex ;              // init on handlePluginInit()
static Bridge*          SentinelBridge ;         // init on handlePluginInit()

// helpers
PurpleCmdId registerCmd(  const char* command , const char* format ,
                          PurpleCmdRet (*callback)() , const char* help) ;
void        alert(        char* msg) ;
gboolean    isBlank(      const char* aCstring) ;

// model helpers
Bridge*        newBridge(         char* bridgeName) ;
Channel*       newChannel(        PurpleConversation* aConv) ;
Bridge*        getBridgeByChannel(PurpleConversation* aConv) ;
Bridge*        getBridgeByName(   char* bridgeName) ;
const char*    getChannelName(    PurpleConversation* aConv) ;
const char*    getProtocol(       PurpleAccount* anAccount) ;
PurpleAccount* getAccount(        PurpleConversation* aConv) ;
const char*    getUsername(       PurpleAccount* anAccount) ;
const char*    getNick(           PurpleAccount* anAccount) ;
gboolean       addChannel(        char* bridgeName , PurpleConversation* aConv) ;
void           storeSession(      void) ;
unsigned int   getNBridges(       void) ;
unsigned int   getNChannels(Bridge* aBridge) ;

// event handlers
void     handlePluginInit(    PurplePlugin* plugin) ;
gboolean handlePluginLoaded(  PurplePlugin* plugin) ;
gboolean handlePluginUnloaded(PurplePlugin* plugin) ;
void     handleIm(            PurpleAccount* anAccount , char* sender ,
                              char* buffer , PurpleConversation* aConv ,
                              PurpleMessageFlags flags , void* data) ;
void     handleChat(          PurpleAccount* anAccount , char* sender ,
                              char* buffer , PurpleConversation* aConv ,
                              PurpleMessageFlags flags , void* data) ;
void     handleChannelClosed( PurpleConversation* aConv, void *data) ;

// admin command handlers */
PurpleCmdRet handleAddCmd(      PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;
PurpleCmdRet handleRemoveCmd(   PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;
PurpleCmdRet handleEnableCmd(   PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;
PurpleCmdRet handleEchoCmd(     PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;
PurpleCmdRet handleChatCmd(     PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;
PurpleCmdRet handleBroadcastCmd(PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;
PurpleCmdRet handleStatusCmd(   PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;
PurpleCmdRet handleHelpCmd(     PurpleConversation* aConv , const gchar* cmd ,
                                gchar** args , gchar** error , void* data) ;

// admin command responses
void         chatBufferDump( PurpleConversation* aConv) ;
void         addResp(        PurpleConversation* aConv , char* bridgeName) ;
void         addExistsResp(  PurpleConversation* aConv , char* bridgeName) ;
void         addConflictResp(PurpleConversation* aConv) ;
void         bridgeStatsMsg( PurpleConversation* aConv , char* bridgeName) ;

// chat buffer helpers
unsigned int chatBufferFillS(     const char* fmt , const char* s1) ;
unsigned int chatBufferFillSS(    const char* fmt , const char* s1 , const char* s2) ;
unsigned int chatBufferFillSSS(   const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3) ;
unsigned int chatBufferFillSSSS(  const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3 , const char* s4) ;
unsigned int chatBufferFillSSSDSD(const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3 , int d1 , const char* s4 , int d2) ;
unsigned int chatBufferCat(       const char* msg , unsigned int nChars) ;
