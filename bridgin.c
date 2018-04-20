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


#include "bridgin.h"
#include "bridgin.dbg.h"


/* globals */

static char*            Commands[N_UNIQ_CMDS] = COMMANDS ;
static unsigned int     NRelayChannels        = 0 ;
static PurplePluginInfo PluginInfo            =
{
  PURPLE_PLUGIN_MAGIC , PURPLE_MAJOR_VERSION , PURPLE_MINOR_VERSION ,
  PLUGIN_TYPE , PLUGIN_GUI_TYPE , 0 , NULL , PURPLE_PRIORITY_DEFAULT ,
  PLUGIN_ID , PLUGIN_NAME , PLUGIN_VERSION , PLUGIN_SHORT_DESC , PLUGIN_LONG_DESC ,
  PLUGIN_AUTHOR , PLUGIN_WEBSITE , PLUGIN_ONLOAD_CB , PLUGIN_ONUNLOAD_CB ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL
} ;


/* purple helpers */

PurpleCmdId registerCmd(const char* command , const char* format ,
                        PurpleCmdRet (*callback)() , const char* help)
{
  return purple_cmd_register(command , format , PURPLE_CMD_P_DEFAULT ,
      PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT , PLUGIN_ID , callback , help , NULL) ;
}

void registerCommands()
{
  CommandIds[0]  = registerCmd(ADD_CMD     , UNARY_FMT  , ADD_CB     , ADDu_HELP) ;
  CommandIds[1]  = registerCmd(ADD_CMD     , BINARY_FMT , ADD_CB     , ADDb_HELP) ;
  CommandIds[2]  = registerCmd(REMOVE_CMD  , UNARY_FMT  , REMOVE_CB  , REMOVE_HELP) ;
  CommandIds[3]  = registerCmd(DISABLE_CMD , UNARY_FMT  , DISABLE_CB , DISABLEu_HELP) ;
  CommandIds[4]  = registerCmd(DISABLE_CMD , BINARY_FMT , DISABLE_CB , DISABLEb_HELP) ;
  CommandIds[5]  = registerCmd(ENABLE_CMD  , UNARY_FMT  , ENABLE_CB  , ENABLEu_HELP) ;
  CommandIds[6]  = registerCmd(ENABLE_CMD  , BINARY_FMT , ENABLE_CB  , ENABLEb_HELP) ;
  CommandIds[7]  = registerCmd(ECHO_CMD    , BINARY_FMT , ECHO_CB    , ECHO_HELP) ;
  CommandIds[8]  = registerCmd(CHAT_CMD    , BINARY_FMT , CHAT_CB    , CHAT_HELP) ;
  CommandIds[9]  = registerCmd(BCAST_CMD   , BINARY_FMT , BCAST_CB   , BCAST_HELP) ;
  CommandIds[10] = registerCmd(STATUS_CMD  , UNARY_FMT  , STATUS_CB  , STATUSu_HELP) ;
  CommandIds[11] = registerCmd(STATUS_CMD  , BINARY_FMT , STATUS_CB  , STATUSb_HELP) ;
  CommandIds[12] = registerCmd(HELP_CMD    , UNARY_FMT  , HELP_CB    , HELP_HELP) ;
}

void registerCallbacks()
{
  void* convs = purple_conversations_get_handle() ;
  purple_signal_connect(convs , RECEIVING_IM_SIGNAL   , ThisPlugin ,
                        PURPLE_CALLBACK(CHAT_RECV_CB) , NULL) ;
  purple_signal_connect(convs , RECEIVING_CHAT_SIGNAL , ThisPlugin ,
                        PURPLE_CALLBACK(CHAT_RECV_CB) , NULL) ;
}

void unregisterCommands()
{
  unsigned int i ;

  for (i = 0 ; i < N_COMMANDS ; ++i) purple_cmd_unregister(CommandIds[i]) ;
}

void unregisterCallbacks()
{
  purple_prefs_disconnect_by_handle(purple_prefs_get_handle()) ;
  purple_signal_disconnect(purple_conversations_get_handle() , RECEIVING_IM_SIGNAL ,
                           ThisPlugin , PURPLE_CALLBACK(CHAT_RECV_CB)) ;
  purple_signal_disconnect(purple_conversations_get_handle() , RECEIVING_CHAT_SIGNAL ,
                           ThisPlugin , PURPLE_CALLBACK(CHAT_RECV_CB)) ;
}

const char* getChannelName(PurpleConversation* aConv)
  { return purple_conversation_get_name(aConv) ; }

const char* getProtocol(PurpleAccount* anAccount)
  { return purple_account_get_protocol_name(anAccount) ; }

PurpleAccount* getAccount(PurpleConversation* aConv)
  { return purple_conversation_get_account(aConv) ; }

const char* getUsername(PurpleAccount* anAccount)
  { return purple_account_get_username(anAccount) ; }

const char* getNick(PurpleConversation* aConv)
  { return purple_account_get_name_for_display(getAccount(aConv)) ; }


/* model helpers */

void prepareBridgeKeys(const char* bridgeName)
{
  snprintf(BridgeKeyBuffer  , SM_BUFFER_SIZE , BRIDGE_PREF_FMT ,
           BASE_PREF_KEY   , bridgeName) ;
  snprintf(EnabledKeyBuffer , SM_BUFFER_SIZE , ENABLED_PREF_FMT  ,
           BridgeKeyBuffer , ENABLED_PREF_KEY) ;
}

void prepareChannelUid(PurpleConversation* aConv)
{
  PurpleAccount* anAccount ;
  const char* protocol ; const char* username ; const char* channelName ;

  anAccount = getAccount(aConv) ;      protocol    = getProtocol(anAccount) ;
  username  = getUsername(anAccount) ; channelName = getChannelName(aConv) ;
  sprintf(ChannelUidBuffer , CHANNEL_UID_FMT , protocol , username , channelName) ;
}

gint isMatch(gconstpointer a , gconstpointer b) { return strcmp((char*)a , (char*)b) ; }

void prefKey2Name(const char* prefKey , char* nameBuffer)
  { snprintf(nameBuffer , SM_BUFFER_SIZE , "%s" , strrchr(prefKey , '/') + 1) ; }

void getBridgeName(PurpleConversation* aConv , char* bridgeNameBuffer)
{
  GList* prefsList ; GList* prefsIter ; char* prefKey ; GList* channelsList ;

  // search for this channelUid
  prepareChannelUid(aConv) ; bridgeNameBuffer[0] = '\0' ;
  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  prefsIter = g_list_first(prefsList) ;
  while (prefsIter)
  {
#if DEBUG_LOGIC
if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_STRING_LIST)
  DBGss("getBridgeName() found stored bridgeName='" , strrchr((char*)prefsIter->data , '/') + 1 , "'" , "") ;
#endif
#if DEBUG_VB
else if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_BOOLEAN)
  DBGsds("getBridgeName() found bool prefKey='" , (char*)prefsIter->data , "' val='" , purple_prefs_get_bool((char*)prefsIter->data) , "'" , "") ;
else if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_STRING)
  DBGsss("getBridgeName() found string prefKey='" , (char*)prefsIter->data , "' val='" , purple_prefs_get_string((char*)prefsIter->data) , "'" , "") ;
#endif

    prefKey = (char*)prefsIter->data ;
    if (!isChannelsPref(prefKey)) { prefsIter = g_list_next(prefsIter) ; continue ; }

    // set bridgeNameBuffer to channelUid if found
    channelsList = purple_prefs_get_string_list(prefKey) ;
    if (g_list_find_custom(channelsList , ChannelUidBuffer , (GCompareFunc)isMatch))
      prefKey2Name(prefKey , bridgeNameBuffer) ;

#if DEBUG_LOGIC
if (g_list_find_custom(channelsList , ChannelUidBuffer , (GCompareFunc)isMatch))
  DBGsss("getBridgeName() found channel=" , ChannelUidBuffer , "' on bridgeName='" , bridgeNameBuffer , "'" , "") ;
#endif
#if DEBUG_VB
else DBGsss("getBridgeName() not found channel='" , ChannelUidBuffer , "' on bridgeName='" , bridgeNameBuffer , "'" , "") ;
#endif

    prefsIter = g_list_next(prefsIter) ;
  }
  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsIter) ; g_list_free(prefsList) ;
}

unsigned int getNBridges()
{
  GList* prefsList ; unsigned int nBridges ;

  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  nBridges = g_list_length(prefsList) / 2 ;
  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsList) ;

  return nBridges ;
}

unsigned int getNChannels(const char* bridgePrefKey)
{
  GList* channelsList ; unsigned int nChannels ;

  channelsList = purple_prefs_get_string_list(bridgePrefKey) ;
  nChannels = g_list_length(channelsList) ;

  return nChannels ;
}

void getNRelayChannels(char* bridgePrefKey , PurpleConversation* thisConv)
{
// NOTE: thisConv will be excluded from the count - pass in NULL to include it

  GList* activeChannelsIter ; PurpleConversation* aConv ;
  char thisBridgeName[SM_BUFFER_SIZE] ; char aBridgeName[SM_BUFFER_SIZE] ;

  prefKey2Name(bridgePrefKey , thisBridgeName) ;
  if (!isChannelsPref(bridgePrefKey)) return ;

  activeChannelsIter = g_list_first(purple_get_conversations()) ;
  while (activeChannelsIter)
  {
    aConv = (PurpleConversation*)activeChannelsIter->data ;
    getBridgeName(aConv , aBridgeName) ;
    if (aConv != thisConv && !strcmp(aBridgeName , thisBridgeName)) ++NRelayChannels ;
    activeChannelsIter = g_list_next(activeChannelsIter) ;
  }
}

gboolean isChannelsPref(const char* bridgePrefKey)
{
  return (purple_prefs_exists(bridgePrefKey) &&
          purple_prefs_get_type(bridgePrefKey) == PURPLE_PREF_STRING_LIST) ;
}

gboolean doesBridgeExist(const char* bridgeName)
  { prepareBridgeKeys(bridgeName) ; return isChannelsPref(BridgeKeyBuffer) ; }

gboolean isBridgeEnabled(const char* bridgeName)
{
  prepareBridgeKeys(bridgeName) ;
  return purple_prefs_exists(EnabledKeyBuffer) &&
         purple_prefs_get_bool(EnabledKeyBuffer) ;
}

gboolean isChannelBridged(PurpleConversation* aConv)
{
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(aConv , thisBridgeName) ;
  return !isBlank(thisBridgeName) ;
}

gboolean areReservedIds(char* bridgeName , char* channelUid , const char* channelName)
{
  return (!strcmp(bridgeName  , "")         || // TODO: better validations?
          !strcmp(channelUid  , "")         ||
          !strcmp(channelName , "NickServ") ||
          !strcmp(channelName , "MemoServ")) ;
}

void createChannel(char* bridgeName)
{
  GList* channelsList = NULL ;

#if DEBUG_VB
DBGss("createChannel() bridgeName='" , bridgeName , "'" , "") ;
#endif

  prepareBridgeKeys(bridgeName) ;

  // add bridge to store if necessary
  if (!doesBridgeExist(bridgeName))
  {
    purple_prefs_add_bool(EnabledKeyBuffer , TRUE) ;
    purple_prefs_add_string_list(BridgeKeyBuffer , channelsList) ;

#if DEBUG_LOGIC
DBGss("createChannel() added new bridgeKey='" , BridgeKeyBuffer , "'" , "") ;
#endif
  }

  // add channel to store
  channelsList = purple_prefs_get_string_list(BridgeKeyBuffer) ;
  channelsList = g_list_prepend(channelsList , (gpointer)ChannelUidBuffer) ;
  purple_prefs_set_string_list(BridgeKeyBuffer , channelsList) ;

#if DEBUG_LOGIC
DBGss("createChannel() added new channelUid='" , ChannelUidBuffer , "'" , "") ;
#endif
}

void destroyChannel(PurpleConversation* aConv)
{
  GList* channelsList = NULL ; GList* channelsIter = NULL ;
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(aConv , thisBridgeName) ;

  // remove channel from store
  prepareBridgeKeys(thisBridgeName) ; prepareChannelUid(aConv) ;
  channelsList = purple_prefs_get_string_list(BridgeKeyBuffer) ;
  channelsIter = g_list_first(channelsList) ;
  while (channelsIter)
    if (!strcmp(channelsIter->data , ChannelUidBuffer))
    {
      channelsList = g_list_delete_link(channelsList , channelsIter) ;
      channelsIter = g_list_first(channelsList) ;
    }
    else channelsIter = g_list_next(channelsIter) ;
  purple_prefs_set_string_list(BridgeKeyBuffer , channelsList) ;

#if DEBUG_LOGIC
DBGssss("destroyChannel() removed channel='" , getChannelName(aConv) , "' from bridge='" , thisBridgeName , ((getNChannels(BridgeKeyBuffer))? "" : "' also removing bridge='") , ((getNChannels(BridgeKeyBuffer))? "" : thisBridgeName) , "'" , "") ;
#endif

  if (getNChannels(BridgeKeyBuffer)) return ;

  // remove empty bridge from store
  purple_prefs_remove(BridgeKeyBuffer) ; purple_prefs_remove(EnabledKeyBuffer) ;
}

void enableBridge(char* bridgeName , gboolean shouldEnable)
{
  prepareBridgeKeys(bridgeName) ;
  if (purple_prefs_get_bool(EnabledKeyBuffer) == shouldEnable) return ;

  purple_prefs_set_bool(EnabledKeyBuffer , shouldEnable) ;
}

void enableBridgeEach(char* bridgePrefKey , gboolean* shouldEnable)
{
  char bridgeName[SM_BUFFER_SIZE] ; prefKey2Name(bridgePrefKey , bridgeName) ;

  if (isChannelsPref(bridgePrefKey)) enableBridge(bridgeName , *shouldEnable) ;
}


/* event handlers */

void handlePluginInit(PurplePlugin* aPlugin)
  { ThisPlugin = aPlugin ; purple_prefs_add_none(BASE_PREF_KEY) ; }

gboolean handlePluginLoaded(PurplePlugin* aPlugin)
{
#if DEBUG_EVS
DBG("handlePluginLoaded()") ;
#endif

  registerCommands() ; registerCallbacks() ; chatBufferClear() ; return TRUE ;
}

gboolean handlePluginUnloaded(PurplePlugin* plugin)
{
#if DEBUG_EVS
DBG("handlePluginUnloaded()") ;
#endif

  unregisterCommands() ; unregisterCallbacks() ; return TRUE ;
}

gboolean handleChat(PurpleAccount* thisAccount , char** sender , char** msg ,
                    PurpleConversation* thisConv , PurpleMessageFlags* flags , void* data)
{
  char thisBridgeName[SM_BUFFER_SIZE] ;

  if (!thisConv) return TRUE ; // supress rogue msgs (autojoined server msgs maybe unbound)

  getBridgeName(thisConv , thisBridgeName) ;

#if DEBUG_CHAT // NOTE: DBGchat() should mirror changes to logic here
DBGchat(thisAccount , *sender , thisConv , *msg , *flags , thisBridgeName) ;
#endif

  if (!isBridgeEnabled(thisBridgeName)) return FALSE ; // input channel bridge is disabled
  if (*flags & PURPLE_MESSAGE_SEND)     return FALSE ; // never relay unprefixed local chat
  if (!(*flags & PURPLE_MESSAGE_RECV))  return FALSE ; // TODO: handle special message types

  prepareRelayChat(NICK_PREFIX , *sender , *msg) ;
  relayMessage(thisBridgeName , thisConv) ; chatBufferClear() ;

  return FALSE ;
}


/* admin command handlers */

PurpleCmdRet handleAddCmd(PurpleConversation* thisConv , const gchar* command ,
                          gchar** args , gchar** error , void* data)
{
  char thisBridgeName[SM_BUFFER_SIZE] ; char* thatBridgeName ;

#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  thatBridgeName = (isBlank(*args))? DEFAULT_BRIDGE_NAME : *args ;
  if (isChannelBridged(thisConv))
  {
    getBridgeName(thisConv , thisBridgeName) ;
    if (strcmp(thisBridgeName , thatBridgeName)) addConflictResp(thisConv) ;
    else addExistsResp(thisConv , thisBridgeName) ;
  }
  else
  {
    prepareChannelUid(thisConv) ;
    if (!areReservedIds(thatBridgeName , ChannelUidBuffer , getChannelName(thisConv)))
      { createChannel(thatBridgeName) ; addResp(thisConv , thatBridgeName) ; }
    else addReservedResp(thisConv) ;
  }

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleRemoveCmd(PurpleConversation* thisConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(thisConv , thisBridgeName) ;

#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  if (isBlank(thisBridgeName)) removeUnbridgedResp(thisConv) ;
  else { destroyChannel(thisConv) ; removeResp(thisConv , thisBridgeName) ; }

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEnableCmd(PurpleConversation* thisConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
  gboolean shouldEnable ; char* bridgeName ; GList* prefsList ;

#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  shouldEnable = !strcmp(command , ENABLE_CMD) ; bridgeName = *args ;
  if (!getNBridges()) enableNoneResp(thisConv , "") ;
  else if (isBlank(bridgeName))
  {
    enableAllResp(thisConv , shouldEnable) ;
    prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
    g_list_foreach(prefsList , (GFunc)enableBridgeEach , &shouldEnable) ;
    g_list_foreach(prefsList , (GFunc)enableRespEach , thisConv) ;
    g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
    g_list_free(prefsList) ;
  }
  else if (doesBridgeExist(bridgeName))
  {
    enableBridge(bridgeName , shouldEnable) ;
    enableResp(thisConv , bridgeName , shouldEnable) ;
  }
  else enableNoneResp(thisConv , bridgeName) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEchoCmd(PurpleConversation* thisConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  adminEcho(thisConv , *args) ; return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleChatCmd(PurpleConversation* thisConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(thisConv , thisBridgeName) ;

#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  if (isChannelBridged(thisConv)) adminChat(thisConv , *args , thisBridgeName) ;
  else { channelStateMsg(thisConv) ; chatBufferDump(thisConv) ; }

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleBroadcastCmd(PurpleConversation* thisConv , const gchar* command ,
                                gchar** args , gchar** error , void* data)
{
#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  broadcastResp(thisConv) ; adminBroadcast(thisConv , *args) ; return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleStatusCmd(PurpleConversation* thisConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  statusResp(thisConv , *args) ; return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleHelpCmd(PurpleConversation* thisConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
#if DEBUG_EVS
DBGcmd(command , *args) ;
#endif

  helpResp(thisConv) ; return PURPLE_CMD_RET_OK ;
}

/* admin command responses */

void addResp(PurpleConversation* thisConv , char* thisBridgeName)
{
  chatBufferPutSS("%s '%s'\n" , CH_SET_MSG , thisBridgeName) ;
  bridgeStatsMsg(thisBridgeName) ; chatBufferDump(thisConv) ;
}

void addExistsResp(PurpleConversation* thisConv , char* thisBridgeName)
{
  chatBufferPutSSS("%s %s '%s'" , THIS_CHANNEL_MSG , CHANNEL_EXISTS_MSG , thisBridgeName) ;
  chatBufferDump(thisConv) ;
}

void addConflictResp(PurpleConversation* thisConv)
  { chatBufferPut(BRIDGE_CONFLICT_MSG) ; chatBufferDump(thisConv) ; }

void addReservedResp(PurpleConversation* thisConv)
  { chatBufferPut(RESERVED_NAME_MSG) ; chatBufferDump(thisConv) ; }

void addFailResp(PurpleConversation* thisConv)
  { chatBufferPut(OOM_MSG) ; chatBufferDump(thisConv) ; }

void removeResp(PurpleConversation* thisConv , char* thisBridgeName)
{
  chatBufferPutSS("%s '%s'" , CHANNEL_REMOVED_MSG , thisBridgeName) ;
  if (doesBridgeExist(thisBridgeName))
    { chatBufferCat("\n") ; bridgeStatsMsg(thisBridgeName) ; }
  else { chatBufferCatSSSS("\n'" , thisBridgeName , "' " , BRIDGE_REMOVED_MSG) ; }
  chatBufferDump(thisConv) ;
}

void removeUnbridgedResp(PurpleConversation* thisConv)
  { channelStateMsg(thisConv) ; chatBufferDump(thisConv) ; }

void enableNoneResp(PurpleConversation* thisConv , char* bridgeName)
  {  bridgeStatsMsg(bridgeName) ; chatBufferDump(thisConv) ; }

void enableAllResp(PurpleConversation* thisConv , gboolean shouldEnable)
{
  chatBufferPut(((shouldEnable)? ENABLING_ALL_MSG : DISABLING_ALL_MSG)) ;
  chatBufferDump(thisConv) ;
}

void enableResp(PurpleConversation* thisConv , char* bridgeName , gboolean shouldEnable)
{
  char* enabledMsg = ((shouldEnable)? ENABLED_MSG : DISABLED_MSG) ;
  chatBufferPutSSS("%s '%s' %s" , ENABLE_MSG , bridgeName , enabledMsg) ;
  chatBufferDump(thisConv) ;
}

void enableRespEach(char* bridgePrefKey , PurpleConversation* thisConv)
{
  char bridgeName[SM_BUFFER_SIZE] ; prefKey2Name(bridgePrefKey , bridgeName) ;

  if (isChannelsPref(bridgePrefKey))
    enableResp(thisConv , bridgeName , isBridgeEnabled(bridgeName)) ;
}

void adminEcho(PurpleConversation* thisConv , char* msg)
{
  prepareRelayChat(NICK_PREFIX , getNick(thisConv) , msg) ; chatBufferDump(thisConv) ;
}

void adminChat(PurpleConversation* thisConv , char* msg , char* thisBridgeName)
{
  prepareRelayChat(NICK_PREFIX , getNick(thisConv) , msg) ;
  relayMessage(thisBridgeName , NULL) ; chatBufferClear() ;
}

void adminBroadcast(PurpleConversation* thisConv , char* msg)
{
  GList* prefsList ;

  prepareRelayChat(BCAST_PREFIX , getNick(thisConv) , msg) ;
  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  g_list_foreach(prefsList , (GFunc)relayMessageEach , NULL) ;
  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsList) ; chatBufferClear() ;
}

void broadcastResp(PurpleConversation* thisConv)
{
  GList* prefsList ; NRelayChannels = 0 ;

  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  g_list_foreach(prefsList , (GFunc)getNRelayChannels , NULL) ;
  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsList) ;

  if (!NRelayChannels) chatBufferPut(NO_BRIDGES_MSG) ;
  else chatBufferPutSDS("%s %d %s" , BROADCAST_MSGa , NRelayChannels , BROADCAST_MSGb) ;
  chatBufferDump(thisConv) ;
}

void statusResp(PurpleConversation* thisConv , char* bridgeName)
{
  unsigned int nBridges ; GList* prefsList ;

#if DEBUG_VB
if (!getNBridges()) DBG("statusResp() no bridges") ; else if (isBlank(bridgeName)) DBG("statusResp() bridge unspecified - listing all") ; else DBGss("statusResp() bridgeName='" , bridgeName , "'" , "") ;
#endif

  nBridges = getNBridges() ;
  chatBufferPutSDS("%s %d %s" , STATS_MSGa , nBridges , STATS_MSGb) ;
  if (nBridges) chatBufferCat("\n") ; else { chatBufferDump(thisConv) ; return ; }

  if (!isBlank(bridgeName)) { bridgeStatsMsg(bridgeName) ; chatBufferCat("\n") ; }
  else
  {
    prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
    g_list_foreach(prefsList , (GFunc)bridgeStatsMsgEach , NULL) ;
    g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
    g_list_free(prefsList) ;
  }

  channelStateMsg(thisConv) ; chatBufferDump(thisConv) ;
}

void helpResp(PurpleConversation* thisConv)
{
  unsigned int i ; GList* helpIter ;

  for (i = 0 ; i < N_UNIQ_CMDS ; ++i)
  {
    helpIter = g_list_first(purple_cmd_help(NULL , Commands[i])) ;
    while (helpIter)
    {
      chatBufferPut(helpIter->data) ; chatBufferDump(thisConv) ;
      helpIter = g_list_next(helpIter) ;
    }
  }
}

void channelStateMsg(PurpleConversation* thisConv)
{
// NOTE: callers of channelStateMsg() should eventually call chatBufferDump()

  char thisBridgeName[SM_BUFFER_SIZE] ;

  chatBufferCatSS(THIS_CHANNEL_MSG , " ") ;
  getBridgeName(thisConv , thisBridgeName) ;
  if (isBlank(thisBridgeName)) chatBufferCat(UNBRIDGED_MSG) ;
  else chatBufferCatSSSS(THIS_BRIDGE_MSG , " '" , thisBridgeName , "'") ;
}

void bridgeStatsMsg(const char* bridgeName)
{
// NOTE: callers of bridgeStatsMsg() should eventually call chatBufferDump()

  unsigned int nChannels ;
  GList* channelsList ; GList* channelsIter ; char* aChannelUid ;
  GList* activeChannelsIter = NULL ; gboolean isActive = FALSE ;
  char* activeMsg ; char* protocol ; char* username ; const char* channelName ;
  char* network ; char nick[SM_BUFFER_SIZE] ;

#if DEBUG_LOGIC
DBGss("bridgeStatsMsg() bridgeName='" , bridgeName , "' " , ((doesBridgeExist(bridgeName))? "exists" : "not exists - bailing")) ;
#endif

  // display non-existent bridge state
  if (!doesBridgeExist(bridgeName))
  {
    if (!getNBridges()) chatBufferCatSS(NO_BRIDGES_MSG , "") ;
    else chatBufferCatSSSS(NO_SUCH_BRIDGE_MSG , " '" , bridgeName , "'") ;
    return ;
  }
  else chatBufferCatSSSS(STATS_MSGc , " '" , bridgeName , "' - ") ;

  // display bridge state
  prepareBridgeKeys(bridgeName) ;
  nChannels = getNChannels(BridgeKeyBuffer) ;
  if (!nChannels) chatBufferCat(STATS_DELETED_MSG) ;
  else
  {
    if (isBridgeEnabled(bridgeName)) chatBufferCat(STATS_ENABLED_MSG) ;
    else chatBufferCat(STATS_DISABLED_MSG) ;
    channelUidBufferPutD("%d" , nChannels) ;
    chatBufferCatSSSS(" - " , ChannelUidBuffer , " " , STATS_MSGd) ;
  }

  // display channels
  channelsList = purple_prefs_get_string_list(BridgeKeyBuffer) ;
  channelsIter = g_list_first(channelsList) ;
  while (channelsIter)
  {
#if DEBUG_LOGIC
DBGss("bridgeStatsMsg() found stored channelUid='" , (char*)channelsIter->data , "'" , "") ;
#endif

    aChannelUid = (char*)channelsIter->data ;

    // determine if bridged aChannel is opened or closed
    activeChannelsIter = g_list_first(purple_get_conversations()) ;
    while (activeChannelsIter)
    {
#if DEBUG_VB
if (channelsIter == g_list_first(channelsList))
  DBGss("bridgeStatsMsg() got active channelName='" , getChannelName((PurpleConversation*)activeChannelsIter->data) , "'" , "") ;
#endif

      prepareChannelUid((PurpleConversation*)activeChannelsIter->data) ;
      isActive |= !strcmp(ChannelUidBuffer , aChannelUid) ;
      activeChannelsIter = g_list_next(activeChannelsIter) ;
    }
    activeMsg = (isActive)? CH_ACTIVE_MSG : CH_INACTIVE_MSG ;

#if DEBUG_VB
DBGss("bridgeStatsMsg() aChannel='" , aChannelUid , "' " , activeMsg) ;
#endif

    // parse channel data from aChannelUid
    channelUidBufferPutS("%s" , aChannelUid) ;
    if (!(protocol    = strtok(ChannelUidBuffer , UID_DELIMITER)) ||
        !(username    = strtok(NULL      , UID_DELIMITER)) ||
        !(channelName = strtok(NULL      , UID_DELIMITER)))
      continue ;

#if DEBUG_VB
DBGsssss("bridgeStatsMsg() parsed channelUid " , activeMsg , " protocol='" , protocol , "' username='" , username , "' channelName='" , channelName , "'" , "") ;
#endif

    // display channel data
    chatBufferCatSSSSSS("\n    " , activeMsg , " '" , channelName , "' on '" , protocol) ;
    if (!strcmp(protocol , IRC_PROTOCOL) &&
        (network = strchr(username , '@')) &&
        (strncpy(nick , username , network - username)))
    {
      nick[network - username] = '\0' ;
      chatBufferCatSSSS(network , "' as '" , nick , "'") ;
    }
    else chatBufferCatSSS("' as '" , username , "'") ;

    channelsIter = g_list_next(channelsIter) ;
  }
}

void bridgeStatsMsgEach(const char* bridgePrefKey , void* unusedGListEachData)
{
  char bridgeName[SM_BUFFER_SIZE] ; prefKey2Name(bridgePrefKey , bridgeName) ;

  if (isChannelsPref(bridgePrefKey)) { bridgeStatsMsg(bridgeName) ; chatBufferCat("\n") ; }
}


/* text buffer helpers */

gboolean isBlank(const char* aCstring) { return (!aCstring || !*aCstring) ; }

void channelUidBufferPutS(const char* fmt , const char* s1)
  { snprintf(ChannelUidBuffer , SM_BUFFER_SIZE , fmt , s1) ; }

void channelUidBufferPutD(const char* fmt , int d1)
  { snprintf(ChannelUidBuffer , SM_BUFFER_SIZE , fmt , d1) ; }

void chatBufferClear() { ChatBuffer[0] = '\0' ; }

void chatBufferPut(    const char* s)
  { snprintf(ChatBuffer , LG_BUFFER_SIZE , "%s" , s) ; }

void chatBufferPutSS(  const char* fmt , const char* s1 , const char* s2)
  { snprintf(ChatBuffer , LG_BUFFER_SIZE , fmt , s1 , s2) ; }

void chatBufferPutSSS( const char* fmt , const char* s1 , const char* s2 , const char* s3)
  { snprintf(ChatBuffer , LG_BUFFER_SIZE , fmt , s1 , s2 , s3) ; }

void chatBufferPutSDS( const char* fmt , const char* s1 , int d1 , const char* s2)
  { snprintf(ChatBuffer , LG_BUFFER_SIZE , fmt , s1 , d1 , s2) ; }

void chatBufferPutSSSS(const char* fmt , const char* s1 , const char* s2 ,
                       const char* s3  , const char* s4)
  { snprintf(ChatBuffer , LG_BUFFER_SIZE , fmt , s1 , s2 , s3 , s4) ; }

void chatBufferCat(      const char* s)
  { strncat(ChatBuffer , s , LG_BUFFER_SIZE - strlen(ChatBuffer) - 1) ; }

void chatBufferCatSS(    const char* s1 , const char* s2)
  { chatBufferCat(s1) ; chatBufferCat(s2) ; }

void chatBufferCatSSS(   const char* s1 , const char* s2 , const char* s3)
  { chatBufferCat(s1) ; chatBufferCat(s2) ; chatBufferCat(s3) ; }

void chatBufferCatSSSS(  const char* s1 , const char* s2 , const char* s3 ,
                         const char* s4)
  { chatBufferCat(s1) ; chatBufferCat(s2) ; chatBufferCat(s3) ; chatBufferCat(s4) ; }

void chatBufferCatSSSSS( const char* s1 , const char* s2 , const char* s3 ,
                         const char* s4 , const char* s5)
  { chatBufferCat(s1) ; chatBufferCat(s2) ; chatBufferCat(s3) ; chatBufferCat(s4) ;
    chatBufferCat(s5) ; }

void chatBufferCatSSSSSS(const char* s1 , const char* s2 , const char* s3 ,
                         const char* s4 , const char* s5 , const char* s6)
  { chatBufferCat(s1) ; chatBufferCat(s2) ; chatBufferCat(s3) ; chatBufferCat(s4) ;
    chatBufferCat(s5) ; chatBufferCat(s6) ; }

void prepareRelayChat(char* prefix , const char* sender , char* msg)
  { chatBufferPutSSSS(CHAT_OUT_FMT , prefix , sender , NICK_POSTFIX , msg) ; }

void chatBufferDump(PurpleConversation* thisConv)
{
#if DEBUG_CHAT
DBGs("chatBufferDump() ChatBuffer=\n" , ChatBuffer) ;
#endif

  purple_conversation_write(thisConv , BRIDGIN_NICK , ChatBuffer ,
                            PURPLE_MESSAGE_SYSTEM , time(0)) ;
  chatBufferClear() ;
}

void relayMessage(char* thisBridgeName , PurpleConversation* thisConv)
{
// NOTE: thisConv will be excluded from the relay - pass in NULL to include it

  GList* activeChannelsIter = NULL ; PurpleConversation* aConv ;
  char aBridgeName[SM_BUFFER_SIZE] ; unsigned int convType ;

  activeChannelsIter = g_list_first(purple_get_conversations()) ;
  while (activeChannelsIter)
  {
    aConv = (PurpleConversation*)activeChannelsIter->data ;
    getBridgeName(aConv , aBridgeName) ;

#if DEBUG_VB
DBGss("relayMessage() got active channelName='" , getChannelName(aConv) ,
      ((aConv == thisConv)? " isThisChannel - skipping" :
      ((!strcmp(aBridgeName , thisBridgeName))? "' isThisBridge - relaying" : "' notThisBridge - skipping" )) , "") ;
#endif

    if (aConv != thisConv && !strcmp(aBridgeName , thisBridgeName))
    {
      convType = purple_conversation_get_type(aConv) ;
      if (convType == PURPLE_CONV_TYPE_IM)
        purple_conv_im_send(purple_conversation_get_im_data(aConv) , ChatBuffer) ;
      else if (convType == PURPLE_CONV_TYPE_CHAT)
        purple_conv_chat_send(purple_conversation_get_chat_data(aConv) , ChatBuffer) ;
    }
    activeChannelsIter = g_list_next(activeChannelsIter) ;
  }
}

void relayMessageEach(const char* bridgePrefKey , PurpleConversation* thisConv)
{
  char bridgeName[SM_BUFFER_SIZE] ; prefKey2Name(bridgePrefKey , bridgeName) ;

  if (isChannelsPref(bridgePrefKey)) relayMessage(bridgeName , thisConv) ;
}

void alert(char* msg)
{
  purple_notify_message(ThisPlugin , PURPLE_NOTIFY_MSG_INFO , msg ,
                        PLUGIN_VERSION , NULL , NULL , NULL) ;
}


/* main */

PURPLE_INIT_PLUGIN(PLUGIN_NAME , handlePluginInit , PluginInfo)
