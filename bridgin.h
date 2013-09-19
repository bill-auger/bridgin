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


#define PREFS_GUI 0 // nyi - we strictly only need prefs for persistence

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

// app constants
#define BRIDGIN_NICK        "BRIDGIN"
#define DEFAULT_BRIDGE_NAME "default"
#define CHAT_OUT_FMT        "%s %s%s %s"
#define NICK_PREFIX         "(from" // dont use '<' - some clients will supress it as html
#define NICK_POSTFIX        ")"

// model constants
#define BRIDGE_PREF_FMT    "%s/%s"
#define ENABLED_PREF_FMT   "%s%s"
#define BASE_PREF_KEY      "/plugins/core/"PLUGIN_NAME
#define BASE_PREF_LABEL    PLUGIN_NAME" preferences"
#if PREFS_GUI
#  define BRIDGES_PREF_NAME  "selected-bridge"
#  define BRIDGES_PREF_KEY   BASE_PREF_KEY"/"BRIDGES_PREF_NAME
#  define BRIDGES_PREF_LABEL "select bridge:"
#endif
#define ENABLED_PREF_KEY   "-enabled"
#define SENTINEL_NAME      "sentinel"
#define UID_BUFFER_SIZE    256
#define UID_DELIMITER      "::"
#define CHANNEL_ID_FMT     "%s"UID_DELIMITER"%s"UID_DELIMITER"%s"

// purple constants
#define RECEIVED_IM_SIGNAL   "received-im-msg"
#define RECEIVED_CHAT_SIGNAL "received-chat-msg"
#define IRC_PROTOCOL         "IRC"

// admin commands
#define N_COMMANDS    13
#define BINARY_FMT    "s"
#define UNARY_FMT     ""
#define ADD_CMD      "add"
#define ADDu_HELP     "/ADD_CMD\nadd this channel to the default bridge"
#define ADDb_HELP     "/ADD_CMD 'a-bridge-name'\nadd this channel to the bridge 'a-bridge-name'"
#define ADD_CB        handleAddCmd
#define REMOVE_CMD    "remove"
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
#define BCAST_HELP    "/BCAST_CMD\nrelay text to the all channels on all bridges as BRIDGIN_NICK"
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
#define RESERVED_NAME_MSG   "invalid name - not adding"
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
  char            uid[UID_BUFFER_SIZE] ;
  struct Channel* next ;
} Channel ;

typedef struct Bridge
{
  char           name[UID_BUFFER_SIZE] ;
  gboolean       isEnabled ;
  Channel*       sentinelChannel ;
  struct Bridge* next ;
} Bridge ;

static PurplePluginInfo   PluginInfo ;                    // init pre main()
#if PREFS_GUI
static PurplePluginUiInfo PrefsInfo  ;                    // init pre main()
//static PurplePluginPrefFrame* PrefsFrame ;                 // init initPrefs()
#endif
static PurplePlugin*      ThisPlugin ;                    // init handlePluginLoaded()
static PurpleCmdId        CommandIds[N_COMMANDS] ;        // init handlePluginLoaded()
static Bridge*            SentinelBridge ;                // init handlePluginInit()
static char               UidBuffer[UID_BUFFER_SIZE] ;    // volatile
static char               StatusBuffer[UID_BUFFER_SIZE] ; // volatile
static char               ChatBuffer[CHAT_BUFFER_SIZE] ;  // volatile

// purple helpers
#if PREFS_GUI
PurplePluginPrefFrame* initPrefs(     PurplePlugin* plugin) ;
#endif
PurpleCmdId            registerCmd(   const char* command , const char* format ,
                                      PurpleCmdRet (*callback)() , const char* help) ;
const char*            getChannelName(PurpleConversation* aConv) ;
const char*            getProtocol(   PurpleAccount* anAccount) ;
PurpleAccount*         getAccount(    PurpleConversation* aConv) ;
const char*            getUsername(   PurpleAccount* anAccount) ;
const char*            getNick(       PurpleAccount* anAccount) ;
void                   alert(         char* msg) ;
gboolean               isBlank(       const char* aCstring) ;

// model helpers
gboolean     areReservedIds(    char* bridgeName , char* channelUid) ;
gboolean     isReservedId(      char* aCstring) ;
Bridge*      newBridge(         char* bridgeName) ;
Channel*     newChannel(        void) ;
void         makeChannelId(     PurpleConversation* aConv) ;
gboolean     addChannel(        char* bridgeName) ;
Bridge*      getBridgeByChannel(PurpleConversation* aConv) ;
Bridge*      getBridgeByName(   const char* bridgeName) ;
unsigned int getNBridges(       void) ;
unsigned int getNChannels(      Bridge* aBridge) ;

// event handlers
void     handlePluginInit(    PurplePlugin* plugin) ;
gboolean handlePluginLoaded(  PurplePlugin* plugin) ;
gboolean handlePluginUnloaded(PurplePlugin* plugin) ;
#if PREFS_GUI
void     handlePrefChanged(   const char *name , PurplePrefType type ,
                              gconstpointer val , gpointer data) ;
#endif
void     handleChat(          PurpleAccount* anAccount , char* sender ,
                              char* buffer , PurpleConversation* aConv ,
                              PurpleMessageFlags flags , void* data) ;

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
// NOTE: callers of channelStateMsg() or bridgeStatsMsg()
//    should first initialize ChatBuffer using one of the text buffer helpers
//    then eventually call chatBufferDump() to flush to screen
void chatBufferDump( PurpleConversation* aConv) ;
void addResp(        PurpleConversation* aConv , char* bridgeName) ;
void addExistsResp(  PurpleConversation* aConv , char* bridgeName) ;
void addConflictResp(PurpleConversation* aConv) ;
void addReservedResp(PurpleConversation* aConv) ;
void addFailResp(    PurpleConversation* aConv) ;
void statusResp(     PurpleConversation* aConv , char* bridgeName) ;
void channelStateMsg(PurpleConversation* aConv) ;
void bridgeStatsMsg( const char* bridgeName) ;

// text buffer helpers
void uidBufferPutS(     const char* fmt , const char* s1) ;
void uidBufferPutSS(    const char* fmt , const char* s1 , const char* s2) ;
void uidBufferPutSSS(   const char* fmt , const char* s1 , const char* s2 , const char* s3) ;
void statusBufferPutS(  const char* fmt , const char* s1) ;
void statusBufferPutSS( const char* fmt , const char* s1 , const char* s2) ;
void statusBufferPutDS( const char* fmt , int d1 , const char* s1) ;
void statusBufferPutSSS(const char* fmt , const char* s1 , const char* s2 , const char* s3) ;
void chatBufferInit(    void) ;
void chatBufferPutS(    const char* fmt , const char* s1) ;
void chatBufferPutSS(   const char* fmt , const char* s1 , const char* s2) ;
void chatBufferPutSSS(  const char* fmt , const char* s1 , const char* s2 , const char* s3) ;
void chatBufferPutSDS(  const char* fmt , const char* s1 , int d1 , const char* s2) ;
void chatBufferPutSSSS( const char* fmt , const char* s1 , const char* s2 ,
                        const char* s3 , const char* s4) ;
void chatBufferCat(     const char* msg) ;
