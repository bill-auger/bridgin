/*\
|*| This file is part of the Bridgin program
|*| Copyright 2013-2014 bill-auger <https://github.com/bill-auger/bridgin/issues>
|*|
|*| Bridgin is free software: you can redistribute it and/or modify
|*| it under the terms of the GNU Affero General Public License as published by
|*| the Free Software Foundation, either version 3 of the License, or
|*| (at your option) any later version.
|*|
|*| Bridgin is distributed in the hope that it will be useful,
|*| but WITHOUT ANY WARRANTY; without even the implied warranty of
|*| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*| GNU Affero General Public License for more details.
|*|
|*| You should have received a copy of the GNU Affero General Public License
|*| along with Bridgin.  If not, see <http://www.gnu.org/licenses/>.
\*/


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#ifndef PURPLE_PLUGINS
#  define PURPLE_PLUGINS
#endif


// plugin constants
#define PLUGIN_TYPE        PURPLE_PLUGIN_STANDARD
#define PLUGIN_GUI_TYPE    NULL
#define PLUGIN_ID          "core-bill-auger-bridgin"
#define PLUGIN_NAME        "Bridgin"
#define PLUGIN_VERSION     "0.5.0"
#define PLUGIN_SHORT_DESC  "Bridge multiple conversations"
#define PLUGIN_LONG_DESC   "Relays chat message from/to specified conversations.\n\tType /? into any conversation for available commands."
#define PLUGIN_AUTHOR      "bill-auger <bill-auger@programmer.net>"
#define PLUGIN_WEBSITE     "https://github.com/bill-auger/bridgin"
#define PLUGIN_ONLOAD_CB   handlePluginLoaded
#define PLUGIN_ONUNLOAD_CB handlePluginUnloaded
#define CHAT_RECV_CB       handleChat

// app constants
#define BRIDGIN_NICK        "BRIDGIN"
#define DEFAULT_BRIDGE_NAME "default"
#define CHAT_OUT_FMT        "%s%s%s%s" // prefix , sender , postfix , msg
#define NICK_PREFIX         "(from "   // dont use '<' - some clients will supress it as html
#define NICK_POSTFIX        ") "
#define BCAST_PREFIX        "(BROADCAST from "

// purple constants
#define RECEIVING_IM_SIGNAL   "writing-im-msg"
#define RECEIVING_CHAT_SIGNAL "writing-chat-msg"
#define IRC_PROTOCOL          "IRC"

// model constants
#define SM_BUFFER_SIZE   256
#define LG_BUFFER_SIZE   8192
#define BASE_PREF_KEY    "/plugins/core/"PLUGIN_NAME
#define BRIDGE_PREF_FMT  "%s/%s"
#define ENABLED_PREF_FMT "%s%s"
#define ENABLED_PREF_KEY "-enabled"
#define UID_DELIMITER    "::"
#define CHANNEL_UID_FMT  "%s"UID_DELIMITER"%s"UID_DELIMITER"%s"

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


/* global vars */

static char*            Commands[N_UNIQ_CMDS] ;            // init global
static unsigned int     NRelayChannels ;                   // init global
static PurplePluginInfo PluginInfo ;                       // init global
static PurplePlugin*    ThisPlugin ;                       // init handlePluginLoaded()
static PurpleCmdId      CommandIds[N_COMMANDS] ;           // init handlePluginLoaded()
static char             BridgeKeyBuffer[SM_BUFFER_SIZE] ;  // volatile
static char             EnabledKeyBuffer[SM_BUFFER_SIZE] ; // volatile
static char             ChannelUidBuffer[SM_BUFFER_SIZE] ; // volatile
static char             ChatBuffer[LG_BUFFER_SIZE] ;       // volatile


/* model-like functions */

// purple helpers
PurpleCmdId    registerCmd(        const char* command , const char* format ,
                                   PurpleCmdRet (*callback)() , const char* help) ;
void           registerCommands(   void) ;
void           registerCallbacks(  void) ;
void           unregisterCommands( void) ;
void           unregisterCallbacks(void) ;
const char*    getChannelName(     PurpleConversation* aConv) ;
const char*    getProtocol(        PurpleAccount* anAccount) ;
PurpleAccount* getAccount(         PurpleConversation* aConv) ;
const char*    getUsername(        PurpleAccount* anAccount) ;
const char*    getNick(            PurpleConversation* aConv) ;

// model helpers
void         prepareBridgeKeys(const char* bridgeName) ;
void         prepareChannelUid(PurpleConversation* aConv) ;
gint         isMatch(          gconstpointer a , gconstpointer b) ;
void         prefKey2Name(     const char* prefKey , char* nameBuffer) ;
void         getBridgeName(    PurpleConversation* aConv , char* thisBridgeNameBuffer) ;
unsigned int getNBridges(      void) ;
unsigned int getNChannels(     const char* bridgeName) ;
void         getNRelayChannels(char* bridgePrefKey , PurpleConversation* inputConv) ;
gboolean     doesBridgeExist(  const char* bridgeName) ;
gboolean     isBridgeEnabled(  const char* bridgeName) ;
gboolean     isChannelBridged( PurpleConversation* aConv) ;
gboolean     shouldBridgeAll(  void) ;
gboolean     areReservedIds(   char* bridgeName , PurpleConversation* thisConv) ;
gboolean     isChannelsPref(   const char* bridgePrefKey) ;
void         createChannel(    char* bridgeName) ;
void         destroyChannel(   PurpleConversation* aConv) ;
void         enableBridge(     char* bridgeName , gboolean shouldEnable) ;
void         enableBridgeEach( char* bridgePrefKey , gboolean* shouldEnable) ;


/* controller-like functions */

// event handlers
void     handlePluginInit(    PurplePlugin* plugin) ;
gboolean handlePluginLoaded(  PurplePlugin* plugin) ;
gboolean handlePluginUnloaded(PurplePlugin* plugin) ;
gboolean handleChat(          PurpleAccount* thisAccount , char* sender , char** message ,
                              PurpleConversation* thisConv , PurpleMessageFlags flags) ;

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
void enableRespEach(     char* bridgePrefKey , PurpleConversation* thisConv) ;
void adminEcho(          PurpleConversation* thisConv , char* msg) ;
void adminChat(          PurpleConversation* thisConv , char* msg , char* thisBridgeName) ;
void adminBroadcast(     PurpleConversation* thisConv , char* msg) ;
void broadcastResp(      PurpleConversation* thisConv) ;
void statusResp(         PurpleConversation* aConv , char* bridgeName) ;
void helpResp(           PurpleConversation* thisConv) ;
void channelStateMsg(    PurpleConversation* aConv) ;
void bridgeStatsMsg(     const char* bridgePrefKey) ;
void bridgeStatsMsgEach( const char* bridgePrefKey , void* unusedGListEachData) ;

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
void     relayMessage(        char* bridgeName , PurpleConversation* thisConv) ;
void     relayMessageEach(    const char* bridgePrefKey , PurpleConversation* thisConv) ;
void     alert(               char* msg) ;
