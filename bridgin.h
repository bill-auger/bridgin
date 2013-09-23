
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef PURPLE_PLUGINS
#  define PURPLE_PLUGINS
#endif

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
#define CHAT_RECV_CB       handleChat

// app constants
#define BRIDGIN_NICK        "BRIDGIN"
#define DEFAULT_BRIDGE_NAME "default"
#define CHAT_OUT_FMT        "%s %s%s %s"
#define NICK_PREFIX         "(from" // dont use '<' - some clients will supress it as html
#define NICK_POSTFIX        ")"
#define BCAST_PREFIX        "(BROADCAST from"

// purple constants
#define RECEIVING_IM_SIGNAL   "receiving-im-msg"
#define RECEIVING_CHAT_SIGNAL "receiving-chat-msg"
#define IRC_PROTOCOL          "IRC"

// model constants
#define SM_BUFFER_SIZE   256
#define LG_BUFFER_SIZE   8192
#define BRIDGE_PREF_FMT  "%s/%s"
#define ENABLED_PREF_FMT "%s%s"
#define BASE_PREF_KEY    "/plugins/core/"PLUGIN_NAME
#define BASE_PREF_LABEL  PLUGIN_NAME" preferences"
#define ENABLED_PREF_KEY "-enabled"
#define SENTINEL_NAME    "sentinel"
#define UID_DELIMITER    "::"
#define CHANNEL_ID_FMT   "%s"UID_DELIMITER"%s"UID_DELIMITER"%s"

// admin commands
#define N_COMMANDS    13
#define N_UNIQ_CMDS   9
#define COMMANDS      { ADD_CMD , REMOVE_CMD , DISABLE_CMD , ENABLE_CMD , ECHO_CMD , CHAT_CMD , BCAST_CMD , STATUS_CMD , HELP_CMD }
#define BINARY_FMT    "s"
#define UNARY_FMT     ""
#define ADD_CMD       "add"
#define ADDu_HELP     "/"ADD_CMD"\n    - add this channel to the default bridge"
#define ADDb_HELP     "/"ADD_CMD" 'a-bridge-name'\n    - add this channel to the bridge 'a-bridge-name'"
#define ADD_CB        handleAddCmd
#define REMOVE_CMD    "rem"
#define REMOVE_HELP   "/"REMOVE_CMD"\n    - unbridge this channel"
#define REMOVE_CB     handleRemoveCmd
#define DISABLE_CMD   "disable"
#define DISABLEu_HELP "/"DISABLE_CMD"\n    - temporarily disable all bridges"
#define DISABLEb_HELP "/"DISABLE_CMD" 'a-bridge-name'\n    - temporarily disable the bridge 'a-bridge-name'"
#define DISABLE_CB    handleEnableCmd
#define ENABLE_CMD    "enable"
#define ENABLEu_HELP  "/"ENABLE_CMD"\n    - enable all bridges"
#define ENABLEb_HELP  "/"ENABLE_CMD" 'a-bridge-name'\n    - enable the bridge 'a-bridge-name'"
#define ENABLE_CB     handleEnableCmd
#define ECHO_CMD      "echo"
#define ECHO_HELP     "/"ECHO_CMD" 'some text'\n    - echo formatted text locally"
#define ECHO_CB       handleEchoCmd
#define CHAT_CMD      "chat"
#define CHAT_HELP     "/"CHAT_CMD" 'some text'\n    - relay text to the all channels on this bridge"
#define CHAT_CB       handleChatCmd
#define BCAST_CMD     "broadcast"
#define BCAST_HELP    "/"BCAST_CMD" 'some text'\n    - relay text to the all channels on all bridges"
#define BCAST_CB      handleBroadcastCmd
#define STATUS_CMD    "status"
#define STATUSu_HELP  "/"STATUS_CMD"\n    - show status information for all bridges"
#define STATUSb_HELP  "/"STATUS_CMD" 'a-bridge-name'\n    - show status information for the bridge 'a-bridge-name'"
#define STATUS_CB     handleStatusCmd
#define HELP_CMD      "?"
#define HELP_HELP     "/"HELP_CMD"\n    - show avaiable "PLUGIN_NAME" commands"
#define HELP_CB       handleHelpCmd

// admin command responses
#define CH_SET_MSG          "channel set to bridge"
#define THIS_CHANNEL_MSG    "this channel"
#define CHANNEL_EXISTS_MSG  "already exists on bridge"
#define BRIDGE_CONFLICT_MSG "each channel may only be on one bridge"
#define RESERVED_NAME_MSG   "invalid name - not adding"
#define CHANNEL_REMOVED_MSG "channel removed from bridge"
#define BRIDGE_REMOVED_MSG  "bridge removed"
#define ENABLING_ALL_MSG    "enabling all bridges"
#define DISABLING_ALL_MSG   "disabling all bridges"
#define ENABLE_MSG          "bridge"
#define ENABLED_MSG         "is enabled"
#define DISABLED_MSG        "is disabled"
#define BROADCAST_MSGa      "broadcasting message on"
#define BROADCAST_MSGb      "channels"
#define THIS_BRIDGE_MSG     "is on bridge"
#define UNBRIDGED_MSG       "is not bridged"
#define NO_SUCH_BRIDGE_MSG  "no such bridge"
#define NO_BRIDGES_MSG      "no bridges exist"
#define STATS_MSGa          "status:"
#define STATS_MSGb          "bridges defined"
#define STATS_MSGc          "bridge"
#define STATS_DELETED_MSG   "deleted"
#define STATS_ENABLED_MSG   "enabled"
#define STATS_DISABLED_MSG  "disabled"
#define STATS_MSGd          "channels bridged"
#define CH_ACTIVE_MSG       "( active )"
#define CH_INACTIVE_MSG     "(inactive)"
#define OOM_MSG             "out of memory"


#include <string.h>

#include "cmds.h"
//#include "conversation.h"
//#include "notify.h"
//#include "plugin.h"
//#include "signals.h"
#include "version.h"


typedef struct Channel
{
  char            uid[SM_BUFFER_SIZE] ;
  struct Channel* prev ;
  struct Channel* next ;
} Channel ;

typedef struct Bridge
{
  char           name[SM_BUFFER_SIZE] ;
  gboolean       isEnabled ;
  Channel*       sentinelChannel ;
  struct Bridge* prev ;
  struct Bridge* next ;
} Bridge ;


// global vars
static PurplePluginInfo PluginInfo ;                       // init pre main()
static PurplePlugin*    ThisPlugin ;                       // init handlePluginLoaded()
static PurpleCmdId      CommandIds[N_COMMANDS] ;           // init handlePluginLoaded()
static char*            Commands[N_UNIQ_CMDS] ;            // init pre main()
static Bridge*          SentinelBridge ;                   // init handlePluginLoaded()
static char             BridgeKeyBuffer[SM_BUFFER_SIZE] ;  // volatile
static char             EnabledKeyBuffer[SM_BUFFER_SIZE] ; // volatile
static char             ChannelUidBuffer[SM_BUFFER_SIZE] ; // volatile
static char             ChatBuffer[LG_BUFFER_SIZE] ;       // volatile


/* model-like functions */

// purple helpers
PurpleCmdId    registerCmd(        const char* command , const char* format ,
                                   PurpleCmdRet (*callback)() , const char* help) ;
gboolean       restoreSession(     void) ;
void           registerCommands(   void) ;
void           registerCallbacks(  void) ;
void           unregisterCommands( void) ;
void           unregisterCallbacks(void) ;
void           destroySession(     void) ;
const char*    getChannelName(     PurpleConversation* aConv) ;
const char*    getProtocol(        PurpleAccount* anAccount) ;
PurpleAccount* getAccount(         PurpleConversation* aConv) ;
const char*    getUsername(        PurpleAccount* anAccount) ;
const char*    getNick(            PurpleConversation* aConv) ;
unsigned int   getNRelayChannels(  Bridge* thisBridge , PurpleConversation* thisConv) ;

// model helpers
Bridge*      getBridgeByChannel(PurpleConversation* aConv) ;
Bridge*      getBridgeByName(   const char* bridgeName) ;
unsigned int getNBridges(       void) ;
unsigned int getNChannels(      Bridge* aBridge) ;
gboolean     doesBridgeExist(   Bridge* aBridge) ;
gboolean     isServerChannel(   PurpleConversation* aConv) ;
gboolean     areReservedIds(    char* bridgeName , char* channelUid ,
                                PurpleConversation* aConv) ;
void         prepareBridgeKeys( char* bridgeName) ;
void         prepareChannelUid( PurpleConversation* aConv) ;
Bridge*      newBridge(         char* bridgeName , Bridge* prevBridge ,
                                gboolean isEnabled) ;
Channel*     newChannel(        Channel* prevChannel) ;
gboolean     createChannel(     char* bridgeName) ;
void         destroyChannel(    Bridge* aBridge , PurpleConversation* aConv) ;
void         enableBridge(      Bridge* aBridge , gboolean isEnable) ;


/* controller-like functions */

// event handlers
void     handlePluginInit(    PurplePlugin* plugin) ;
gboolean handlePluginLoaded(  PurplePlugin* plugin) ;
gboolean handlePluginUnloaded(PurplePlugin* plugin) ;
gboolean handleChat(          PurpleAccount* anAccount , char** sender ,
                              char** msg , PurpleConversation* aConv ,
                              PurpleMessageFlags* flags , void* data) ;

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


/* view-like functions */

// admin command responses
// NOTE: callers of channelStateMsg() or bridgeStatsMsg()
//    should eventually call chatBufferDump() to flush to screen
void addResp(            PurpleConversation* aConv , char* thisBridgeName) ;
void addExistsResp(      PurpleConversation* aConv , char* thisBridgeName) ;
void addConflictResp(    PurpleConversation* aConv) ;
void addReservedResp(    PurpleConversation* aConv) ;
void addFailResp(        PurpleConversation* aConv) ;
void removeResp(         PurpleConversation* thisConv , char* thisBridgeName) ;
void removeUnbridgedResp(PurpleConversation* thisConv) ;
void enableNoneResp(     PurpleConversation* thisConv , char* bridgeName) ;
void enableAllResp(      PurpleConversation* thisConv , gboolean shouldEnable) ;
void enableResp(         PurpleConversation* thisConv , char* bridgeName ,
                         gboolean shouldEnable) ;
void adminEcho(          PurpleConversation* inputConv , char* msg) ;
void adminChat(          PurpleConversation* inputConv , char* msg ,
                         Bridge* aBridge) ;
void adminBroadcast(     PurpleConversation* inputConv , char* msg) ;
void broadcastResp(      PurpleConversation* thisConv) ;
void statusResp(         PurpleConversation* aConv , char* bridgeName) ;
void helpResp(           PurpleConversation* thisConv) ;
void channelStateMsg(    PurpleConversation* aConv) ;
void bridgeStatsMsg(     const char* bridgeName) ;

// text buffer helpers
gboolean isBlank(             const char* aCstring) ;
void     channelUidBufferPutS(const char* fmt , const char* s1) ;
void     channelUidBufferPutD(const char* fmt , int d1) ;
void     chatBufferClear(     void) ;
void     chatBufferPut(       const char* s) ;
void     chatBufferPutSS(     const char* fmt , const char* s1 , const char* s2) ;
void     chatBufferPutSSS(    const char* fmt , const char* s1 , const char* s2 ,
                              const char* s3) ;
void     chatBufferPutSDS(    const char* fmt , const char* s1 , int d1 , const char* s2) ;
void     chatBufferPutSSSS(   const char* fmt , const char* s1 , const char* s2 ,
                              const char* s3 , const char* s4) ;
void     chatBufferCat(       const char* s) ;
void     chatBufferCatSS(     const char* s1 , const char* s2) ;
void     chatBufferCatSSS(    const char* s1 , const char* s2 , const char* s3) ;
void     chatBufferCatSSSS(   const char* s1 , const char* s2 , const char* s3 ,
                              const char* s4) ;
void     chatBufferCatSSSSS(  const char* s1 , const char* s2 , const char* s3 ,
                              const char* s4 , const char* s5) ;
void     chatBufferCatSSSSSS( const char* s1 , const char* s2 , const char* s3 ,
                              const char* s4 , const char* s5 , const char* s6) ;
void     prepareRelayChat(    char* prefix , const char* sender , char* msg) ;
void     chatBufferDump(      PurpleConversation* thisConv) ;
void     relayMessage(        Bridge* outputBridge , PurpleConversation* inputConv) ;
void     alert(               char* msg) ;
