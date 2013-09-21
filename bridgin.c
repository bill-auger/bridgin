#include "bridgin.h"
#include "bridgin.dbg.h"


/* purple helpers */

PurpleCmdId registerCmd(const char* command , const char* format ,
                        PurpleCmdRet (* callback)() , const char* help)
{
  return purple_cmd_register(command , format , PURPLE_CMD_P_DEFAULT ,
      PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT , PLUGIN_ID , callback , help , NULL) ;
}

const char* getChannelName(PurpleConversation* aConv)
  { return purple_conversation_get_name(aConv) ; }

const char* getProtocol(PurpleAccount* anAccount)
  { return purple_account_get_protocol_name(anAccount) ; }

PurpleAccount* getAccount(PurpleConversation* aConv)
  { return purple_conversation_get_account(aConv) ; }

const char* getUsername(PurpleAccount* anAccount)
  { return purple_account_get_username(anAccount) ; }

void alert(char* msg)
{
  purple_notify_message(ThisPlugin , PURPLE_NOTIFY_MSG_INFO , msg ,
                        PLUGIN_VERSION , NULL , NULL , NULL) ;
}


/* model helpers */

gboolean doesBridgeExist(Bridge* aBridge) { return (aBridge != SentinelBridge) ; }

gboolean areReservedIds(char* bridgeName , char* channelUid)
{
  return (!strcmp(bridgeName  , SENTINEL_NAME) ||
          !strcmp(channelUid  , SENTINEL_NAME)) ;
}

void prepareBridgeKeys(char* bridgeName)
{
  snprintf(BridgeKeyBuffer  , UID_BUFFER_SIZE , BRIDGE_PREF_FMT ,
           BASE_PREF_KEY   , bridgeName) ;
  snprintf(EnabledKeyBuffer , UID_BUFFER_SIZE , ENABLED_PREF_FMT  ,
           BridgeKeyBuffer , ENABLED_PREF_KEY) ;
}

void prepareChannelUid(PurpleConversation* aConv)
{
  PurpleAccount* anAccount ;
  const char* protocol ; const char* username ; const char* channelName ;

  anAccount = getAccount(aConv) ;      protocol    = getProtocol(anAccount) ;
  username  = getUsername(anAccount) ; channelName = getChannelName(aConv) ;
  sprintf(ChannelUidBuffer , CHANNEL_ID_FMT , protocol , username , channelName) ;
}

Bridge* newBridge(char* bridgeName , Bridge* prevBridge)
{
  Bridge* newBridge ; Channel* newChannel ;

  newBridge  = (Bridge*) malloc(sizeof(Bridge)) ;  if (!newBridge)  return NULL ;
  newChannel = (Channel*)malloc(sizeof(Channel)) ; if (!newChannel) return NULL ;

  strcpy(newBridge->name     , bridgeName) ;
  newBridge->isEnabled       = TRUE ;
  newBridge->prev            = prevBridge ;
  newBridge->next            = NULL ;
  newBridge->sentinelChannel = newChannel ;

  strcpy(newChannel->uid , SENTINEL_NAME) ;
  newChannel->next       = NULL ;

  return newBridge ;
}

Channel* newChannel(Channel* prevChannel)
{
  Channel* newChannel ;

  newChannel = (Channel*)malloc(sizeof(Channel)) ; if (!newChannel) return NULL ;

  strcpy(newChannel->uid , ChannelUidBuffer) ;
  newChannel->prev       = prevChannel ;
  newChannel->next       = NULL ;

  return newChannel ;
}

gboolean createChannel(char* bridgeName)
{
  Bridge* aBridge ; Channel* aChannel ; GList* channelsList = NULL ;

DBGss("createChannel() bridgeName='" , bridgeName , "'" , "") ;

  prepareBridgeKeys(bridgeName) ;
  if ((aBridge = getBridgeByName(bridgeName)) == SentinelBridge)
  {
    // create new bridge
    while (aBridge->next) aBridge = aBridge->next ;
    aBridge->next = newBridge(bridgeName , aBridge) ;
    if (!(aBridge = aBridge->next)) return FALSE ;

    // store bridge config
    purple_prefs_add_bool(EnabledKeyBuffer , TRUE) ;
    purple_prefs_add_string_list(BridgeKeyBuffer , channelsList) ;

DBGs("createChannel() added new bridgeKey=" , BridgeKeyBuffer) ;
  }
  else channelsList = purple_prefs_get_string_list(BridgeKeyBuffer) ;

  // create new channel
  aChannel = aBridge->sentinelChannel ;
  while (aChannel->next) aChannel = aChannel->next ;
  aChannel->next = newChannel(aChannel) ;
  if (!aChannel->next) { g_list_free(channelsList) ; return FALSE ; }

  // store bridge config
  channelsList = g_list_prepend(channelsList , (gpointer)ChannelUidBuffer) ;
  purple_prefs_set_string_list(BridgeKeyBuffer , channelsList) ;

DBGs("createChannel() added new channelUid=" , ChannelUidBuffer) ;

  g_list_free(channelsList) ; return TRUE ;
}

void destroyChannel(Bridge* aBridge , PurpleConversation* aConv)
{
  Channel* aChannel ; GList* channelsList = NULL ; GList* channelsIter = NULL ;

  // destroy channel struct
  aChannel = aBridge->sentinelChannel ; prepareChannelUid(aConv) ;
  while ((aChannel = aChannel->next))
    if (!strcmp(aChannel->uid , ChannelUidBuffer))
      { aChannel->prev->next = aChannel->next ; free(aChannel) ; }

  // store bridge config
  prepareBridgeKeys(aBridge->name) ;
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

  // destroy empty bridge struct and preference keys
  if (!g_list_length(channelsList))
  {
    aBridge->prev->next = aBridge->next ; free(aBridge) ;
    purple_prefs_remove(BridgeKeyBuffer) ; purple_prefs_remove(EnabledKeyBuffer) ;
  }

  g_list_free(channelsList) ; g_list_free(channelsIter) ;
}

Bridge* getBridgeByChannel(PurpleConversation* aConv)
{
  Bridge* aBridge ; Channel* aChannel ;

/*DBG*/prepareChannelUid(aConv);DBGss("getBridgeByChannel() channel='" , ChannelUidBuffer , "'" , "") ;

  aBridge = SentinelBridge ; prepareChannelUid(aConv) ;
  while ((aBridge = aBridge->next))
  {
    aChannel = aBridge->sentinelChannel ;
    while ((aChannel = aChannel->next))
      if (!strcmp(aChannel->uid , ChannelUidBuffer))
        return aBridge ;
  }

DBGss("getBridgeByChannel() '" , ChannelUidBuffer , "' not found" , "") ;

  return SentinelBridge ;
}

Bridge* getBridgeByName(const char* bridgeName)
{
  Bridge* aBridge = SentinelBridge ;

  while ((aBridge = aBridge->next))
    if (!strcmp(aBridge->name , bridgeName))
      return aBridge ;

DBGsd("getBridgeByName() '" , bridgeName , "' not found - nBridges=" , getNBridges()) ;

  return SentinelBridge ;
}

unsigned int getNBridges()
{
  Bridge* aBridge ; unsigned int n ; aBridge = SentinelBridge ; n = 0 ;
  while ((aBridge = aBridge->next)) ++n ;
  return n ;
}

unsigned int getNChannels(Bridge* aBridge)
{
  Channel* aChannel ; unsigned int n ; aChannel = aBridge->sentinelChannel ; n = 0 ;
  while ((aChannel = aChannel->next)) ++n ;
  return n ;
}


/* event handlers */

void handlePluginInit(PurplePlugin* aPlugin)
  { ThisPlugin = aPlugin ; purple_prefs_add_none(BASE_PREF_KEY) ; }

gboolean handlePluginLoaded(PurplePlugin* aPlugin)
{
  void* convs ;
  GList* prefsList ; GList* prefsIter ; const char* prefKey ; char* bridgeName ;
  GList* channelsList ; GList* channelsIter ;

DBG("handlePluginLoaded()") ;

  // init
  SentinelBridge = newBridge(SENTINEL_NAME , NULL) ;
  if (!SentinelBridge) { alert(OOM_MSG) ; return FALSE ; }

  // register admin commands
  CommandIds[0]  = registerCmd(ADD_CMD     , UNARY_FMT  , ADD_CB    , ADDu_HELP) ;
  CommandIds[1]  = registerCmd(ADD_CMD     , BINARY_FMT , ADD_CB    , ADDb_HELP) ;
  CommandIds[2]  = registerCmd(REMOVE_CMD  , UNARY_FMT  , REMOVE_CB , REMOVE_HELP) ;
  CommandIds[3]  = registerCmd(DISABLE_CMD , UNARY_FMT  , ENABLE_CB , DISABLEu_HELP) ;
  CommandIds[4]  = registerCmd(DISABLE_CMD , BINARY_FMT , ENABLE_CB , DISABLEb_HELP) ;
  CommandIds[5]  = registerCmd(ENABLE_CMD  , UNARY_FMT  , ENABLE_CB , ENABLEu_HELP) ;
  CommandIds[6]  = registerCmd(ENABLE_CMD  , BINARY_FMT , ENABLE_CB , ENABLEb_HELP) ;
  CommandIds[7]  = registerCmd(ECHO_CMD    , BINARY_FMT , ECHO_CB   , ECHO_HELP) ;
  CommandIds[8]  = registerCmd(CHAT_CMD    , BINARY_FMT , CHAT_CB   , CHAT_HELP) ;
  CommandIds[9]  = registerCmd(BCAST_CMD   , BINARY_FMT , BCAST_CB  , BCAST_HELP) ;
  CommandIds[10] = registerCmd(STATUS_CMD  , UNARY_FMT  , STATUS_CB , STATUSu_HELP) ;
  CommandIds[11] = registerCmd(STATUS_CMD  , BINARY_FMT , STATUS_CB , STATUSb_HELP) ;
  CommandIds[12] = registerCmd(HELP_CMD    , UNARY_FMT  , HELP_CB   , HELP_HELP) ;

  // register conversation callbacks
  convs = purple_conversations_get_handle() ;
  purple_signal_connect(convs , RECEIVED_IM_SIGNAL           , ThisPlugin ,
                        PURPLE_CALLBACK(handleChat)          , NULL) ;
  purple_signal_connect(convs , RECEIVED_CHAT_SIGNAL         , ThisPlugin ,
                        PURPLE_CALLBACK(handleChat)          , NULL) ;

  // restore session
  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  prefsIter = g_list_first(prefsList) ;
  while (prefsIter)
  {
if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_BOOLEAN)
DBGsd("handlePluginLoaded() found bool prefKey=" , (char*)prefsIter->data , " val=" , purple_prefs_get_bool((char*)prefsIter->data)) ;
else if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_STRING)
DBGss("handlePluginLoaded() found string prefKey=" , (char*)prefsIter->data , " val=" , purple_prefs_get_string((char*)prefsIter->data)) ;

    prefKey = (char*)prefsIter->data ;
    if (purple_prefs_get_type(prefKey) == PURPLE_PREF_STRING_LIST &&
        (bridgeName = strrchr(prefKey , '/')) && ++bridgeName)
    {
DBGs("handlePluginLoaded() found stored bridgeName=" , bridgeName) ;

      channelsList = purple_prefs_get_string_list(prefKey) ;
      channelsIter = g_list_first(channelsList) ;
      while (channelsIter)
      {
DBGs("handlePluginLoaded() found stored channelUid=" , (char*)channelsIter->data) ;

        strcpy(ChannelUidBuffer , (char*)channelsIter->data) ;
        if (!createChannel(bridgeName))
        {
          // malloc error - cleanup and bail
          g_list_free(channelsList) ; g_list_free(channelsIter) ;
          g_list_free(prefsList) ; g_list_free(prefsIter) ;
          alert(OOM_MSG) ; return FALSE ;
        }

        channelsIter = g_list_next(channelsIter) ;
      }
      g_list_free(channelsList) ; g_list_free(channelsIter) ;
    }

    prefsIter = g_list_next(prefsIter) ;
  }

  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsList) ; g_list_free(prefsIter) ;

  return TRUE ;
}

gboolean handlePluginUnloaded(PurplePlugin* plugin)
{
  unsigned int i ; Bridge* aBridge ; Channel* aChannel ;

DBG("handlePluginUnloaded()") ;

  // unregister commands
  for (i = 0 ; i < N_COMMANDS ; ++i) purple_cmd_unregister(CommandIds[i]) ;

  // unregister callbacks
  purple_prefs_disconnect_by_handle(purple_prefs_get_handle()) ;
  purple_signal_disconnect(purple_conversations_get_handle() , RECEIVED_IM_SIGNAL ,
                           ThisPlugin , PURPLE_CALLBACK(handleChat)) ;
  purple_signal_disconnect(purple_conversations_get_handle() , RECEIVED_CHAT_SIGNAL ,
                           ThisPlugin , PURPLE_CALLBACK(handleChat)) ;

  aBridge = SentinelBridge ;
  while ((aBridge = aBridge->next))
  {
    aChannel = aBridge->sentinelChannel ;
    while ((aChannel = aChannel->next)) free(aChannel->prev) ;
    free(aChannel) ; free(aBridge->prev) ;
  }
  free(aBridge) ;

  return TRUE ;
}

void handleChat(PurpleAccount* thisAccount , char* sender , char* msg ,
                PurpleConversation* thisConv , PurpleMessageFlags flags , void* data)
{
  GList* activeChannelsIter = NULL ;
  Bridge* thisBridge ; PurpleConversation* aConv ; unsigned int convType ;

// NOTE: DBGchat() should mirror changes to logic here
DBGchat(((purple_conversation_get_type(thisConv) == PURPLE_CONV_TYPE_IM)? RECEIVED_IM_SIGNAL : RECEIVED_CHAT_SIGNAL) , thisAccount , sender , thisConv , msg , flags) ;

  if (flags & PURPLE_MESSAGE_SEND) return ;
  if (!(flags & PURPLE_MESSAGE_RECV)) return ;
  if ((thisBridge = getBridgeByChannel(thisConv)) == SentinelBridge) return ;

  chatBufferPutSSSS(CHAT_OUT_FMT , NICK_PREFIX , sender , NICK_POSTFIX , msg) ;

  // relay chat to all opened channels on this bridge
  activeChannelsIter = g_list_first(purple_get_conversations()) ;
  while (activeChannelsIter)
  {
DBGs("handleChat() got active channelName=" , getChannelName((PurpleConversation*)activeChannelsIter->data)) ;

    aConv = (PurpleConversation*)activeChannelsIter->data ;
    if (aConv != thisConv && getBridgeByChannel(aConv) == thisBridge)
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


/* admin command handlers */

PurpleCmdRet handleAddCmd(PurpleConversation* thisConv , const gchar* command ,
                          gchar** args , gchar** error , void* data)
{
  char* bridgeName ; Bridge* thisBridge ;

DBGcmd(command , args[0]) ;

  bridgeName = (isBlank(args[0]))? DEFAULT_BRIDGE_NAME : args[0] ;
  if (doesBridgeExist(thisBridge = getBridgeByChannel(thisConv)))
  {
    if (thisBridge != getBridgeByName(bridgeName)) addConflictResp(thisConv) ;
    else addExistsResp(thisConv , bridgeName) ;
  }
  else
  {
    prepareChannelUid(thisConv) ;
    if (!areReservedIds(bridgeName , ChannelUidBuffer))
      if (createChannel(bridgeName)) addResp(thisConv , bridgeName) ;
      else addFailResp(thisConv) ;
    else addReservedResp(thisConv) ;
  }

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleRemoveCmd(PurpleConversation* thisConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
  Bridge* thisBridge ; char thisBridgeName[UID_BUFFER_SIZE] ;

DBGcmd(command , args[0]) ;

  if (doesBridgeExist(thisBridge = getBridgeByChannel(thisConv)))
  {
    strncpy(thisBridgeName , thisBridge->name , UID_BUFFER_SIZE) ;
    destroyChannel(thisBridge , thisConv) ; removeResp(thisConv , thisBridgeName) ;
  }
  else removeUnbridgedResp(thisConv) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEnableCmd(PurpleConversation* thisConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEchoCmd(PurpleConversation* thisConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleChatCmd(PurpleConversation* thisConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleBroadcastCmd(PurpleConversation* thisConv , const gchar* command ,
                                gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleStatusCmd(PurpleConversation* thisConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  statusResp(thisConv , args[0]) ; return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleHelpCmd(PurpleConversation* thisConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

/* admin command responses */

void addResp(PurpleConversation* thisConv , char* thisBridgeName)
{
  chatBufferPutSS("%s '%s'" , CH_SET_MSG , thisBridgeName) ;
  bridgeStatsMsg(thisBridgeName) ; chatBufferDump(thisConv) ;
}

void addExistsResp(PurpleConversation* thisConv , char* thisBridgeName)
{
  chatBufferPutSSS("%s %s '%s'" , THIS_CHANNEL_MSG , CHANNEL_EXISTS_MSG , thisBridgeName) ;
  chatBufferDump(thisConv) ;
}

void addConflictResp(PurpleConversation* thisConv)
{
  chatBufferPutS("%s" , BRIDGE_CONFLICT_MSG) ; chatBufferDump(thisConv) ;
}

void addReservedResp(PurpleConversation* thisConv)
{
  chatBufferPutS("%s" , RESERVED_NAME_MSG) ; chatBufferDump(thisConv) ;
}

void addFailResp(PurpleConversation* thisConv)
{
  chatBufferPutS("%s" , OOM_MSG) ; chatBufferDump(thisConv) ;
}

void removeResp(PurpleConversation* thisConv , char* thisBridgeName)
{
  chatBufferPutSS("%s '%s'" , CHANNEL_REMOVED_MSG , thisBridgeName) ;
  if (doesBridgeExist(getBridgeByName(thisBridgeName))) bridgeStatsMsg(thisBridgeName) ;
  else { chatBufferCatSSSS("\n'" , thisBridgeName , "' " , BRIDGE_REMOVED_MSG) ; }
  chatBufferDump(thisConv) ;
}

void removeUnbridgedResp(PurpleConversation* thisConv)
  { chatBufferInit() ; channelStateMsg(thisConv) ; chatBufferDump(thisConv) ; }

/* TODO: the remaining admin commands and responses
function removeResp($bridgeName)
{
  global $CHANNEL_REMOVED_MSG , $BRIDGE_REMOVED_MSG ; $resp = $CHANNEL_REMOVED_MSG ;

  if (array_key_exists($bridgeName , $Bridges)) $resp .= bridgeStatsMsg($bridgeName) ;
  else $resp .= $BRIDGE_REMOVED_MSG ;
  return $resp ;
}

function removeUnbridgedResp($accountId , $channelId)
  { return channelStateMsg($accountId , $channelId) ; }

function enableNoneResp($bridgeName) { return bridgeStatsMsg($bridgeName) ; }

function enableAllResp($isEnable)
{
  global $ENABLING_ALL_MSG , $DISABLING_ALL_MSG ;
  return ($isEnable)? $ENABLING_ALL_MSG : $DISABLING_ALL_MSG ;
}

function enableResp($bridgeName , $enabledMsg)
  { global $ENABLE_MSG ; return "$ENABLE_MSG '$bridgeName' $enabledMsg" ; }

function echoResp($nick , $msg)
  { global $NICK_PREFIX ; return chatOut($NICK_PREFIX , $nick , $msg) ; }

function chatResp() { return "" ; }

function chatUnbridgedResp($accountId , $channelId)
  { return channelStateMsg($accountId , $channelId) ; }

function broadcastResp($channels)
{
  global $BROADCAST_MSGa , $BROADCAST_MSGb ;
  return $BROADCAST_MSGa . (nActiveChannels($channels) - 1) . $BROADCAST_MSGb ;
}

function helpResp()
{
  global $HELP_MSG , $HELP_MSGS ; $resp = $HELP_MSG ;
  foreach ($HELP_MSGS as $trigger => $desc) $resp .= "\n$trigger\n\t=> $desc" ;
  return $resp ;
}

function exitResp() { global $EXIT_MSG ; return $EXIT_MSG ; }

function defaultResp()
{
  global $UNKNOWN_MSG , $TRIGGER_PREFIX ;
  return "$UNKNOWN_MSG '$TRIGGER_PREFIX$trigger'" ;
}
*/

void statusResp(PurpleConversation* thisConv , char* bridgeName)
{
  Bridge* aBridge ; unsigned int nBridges = getNBridges() ;

DBGs("statusResp() bridgeName=" , (!nBridges)? "no bridges" : ((!isBlank(bridgeName))? bridgeName : "unspecified - listing all")) ;

  if (!nBridges)                 { chatBufferInit() ; bridgeStatsMsg("") ; }
  else if (!isBlank(bridgeName)) { chatBufferInit() ; bridgeStatsMsg(bridgeName) ; }
  else
  {
    chatBufferPutSDS("%s %d %s" , STATS_MSGa , getNBridges() , STATS_MSGb) ;

    aBridge = SentinelBridge ;
    while ((aBridge = aBridge->next)) bridgeStatsMsg(aBridge->name) ;
  }
  if (nBridges) { chatBufferCat("\n\n") ; channelStateMsg(thisConv) ; }
  chatBufferDump(thisConv) ;
}

void channelStateMsg(PurpleConversation* thisConv)
{
// NOTE: callers of channelStateMsg() should first initialize ChatBuffer
//    then eventually call chatBufferDump() to flush to screen

  Bridge* aBridge ;

DBGs("channelStateMsg() channelName=" , getChannelName(thisConv)) ;

  chatBufferCatSS(THIS_CHANNEL_MSG , " ") ;
  if (doesBridgeExist(aBridge = getBridgeByChannel(thisConv)))
    chatBufferCatSSSS(THIS_BRIDGE_MSG , " '" , aBridge->name , "'") ;
  else chatBufferCat(UNBRIDGED_MSG) ;
}

void bridgeStatsMsg(const char* bridgeName)
{
// NOTE: callers of bridgeStatsMsg() should first initialize ChatBuffer
//    then eventually call chatBufferDump() to flush to screen

  Bridge* aBridge ; Channel* aChannel ; unsigned int nChannels ;
  GList* activeChannelsIter = NULL ; gboolean isActive = FALSE ;
  char* activeMsg ; char* protocol ; char* username ; const char* channelName ;
  char* network ; char nick[UID_BUFFER_SIZE] ;

DBG("bridgeStatsMsg()") ;

  chatBufferCat("\n\n") ;
  if (!doesBridgeExist(aBridge = getBridgeByName(bridgeName)))
  {
    if (!getNBridges()) chatBufferCatSS(NO_BRIDGES_MSG , "\n") ;
    else chatBufferCatSSSS(NO_SUCH_BRIDGE_MSG , " '" , bridgeName , "'\n") ;
    return ;
  }
  else chatBufferCatSSSS(STATS_MSGc , " '" , bridgeName , "' - ") ;

  nChannels = getNChannels(aBridge) ;
  if (!nChannels) chatBufferCat(STATS_DELETED_MSG) ;
  else
  {
    if (aBridge->isEnabled) chatBufferCat(STATS_ENABLED_MSG) ;
    else chatBufferCat(STATS_DISABLED_MSG) ;
    channelUidBufferPutD("%d" , nChannels) ;
    chatBufferCatSSSS(" - " , ChannelUidBuffer , " " , STATS_MSGd) ;
  }

  aChannel = aBridge->sentinelChannel ;
  while ((aChannel = aChannel->next))
  {
    // determine if bridged aChannel is opened or closed
    activeChannelsIter = g_list_first(purple_get_conversations()) ;
    while (activeChannelsIter)
    {
if (aBridge == SentinelBridge->next && aChannel == aBridge->sentinelChannel->next)
DBGs("bridgeStatsMsg() got active channelName=" , getChannelName((PurpleConversation*)activeChannelsIter->data)) ;

      prepareChannelUid((PurpleConversation*)activeChannelsIter->data) ;
      isActive |= !strcmp(aChannel->uid , ChannelUidBuffer) ;
      activeChannelsIter = g_list_next(activeChannelsIter) ;
    }
    activeMsg = (isActive)? CH_ACTIVE_MSG : CH_INACTIVE_MSG ;

DBGss("bridgeStatsMsg() aChannel=" , aChannel->uid , " " , activeMsg) ;

    // parse channel data from channelUid
    channelUidBufferPutS("%s" , aChannel->uid) ;
    if (!(protocol    = strtok(ChannelUidBuffer , UID_DELIMITER)) ||
        !(username    = strtok(NULL      , UID_DELIMITER)) ||
        !(channelName = strtok(NULL      , UID_DELIMITER)))
      continue ;

DBGsssss("bridgeStatsMsg() parsed channelId " , activeMsg , " protocol='" , protocol , "' username='" , username , "' channelName='" , channelName , "'" , "") ;

    // display channel data
    chatBufferCatSSSSSS("\n    " , activeMsg , "'" , channelName , "' on '" , protocol) ;
    if (!strcmp(protocol , IRC_PROTOCOL) &&
        (network = strchr(username , '@')) &&
        (strncpy(nick , username , network - username)))
    {
      nick[network - username] = '\0' ;
      chatBufferCatSSSS(network , "' as '" , nick , "'") ;
    }
    else chatBufferCatSSS("' as '" , username , "'") ;
  } // each aChannel
}


/* text buffer helpers */

gboolean isBlank(const char* aCstring) { return (!aCstring || !*aCstring) ; }

void channelUidBufferPutS(const char* fmt , const char* s1)
  { snprintf(ChannelUidBuffer , UID_BUFFER_SIZE , fmt , s1) ; }

void channelUidBufferPutD(const char* fmt , int d1)
  { snprintf(ChannelUidBuffer , UID_BUFFER_SIZE , fmt , d1) ; }

void chatBufferInit() { ChatBuffer[0] = '\0' ; }

void chatBufferPutS(   const char* fmt , const char* s1)
  { snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1) ; }

void chatBufferPutSS(  const char* fmt , const char* s1 , const char* s2)
  { snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2) ; }

void chatBufferPutSSS( const char* fmt , const char* s1 , const char* s2 , const char* s3)
  { snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3) ; }

void chatBufferPutSDS( const char* fmt , const char* s1 , int d1 , const char* s2)
  { snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , d1 , s2) ; }

void chatBufferPutSSSS(const char* fmt , const char* s1 , const char* s2 ,
                       const char* s3  , const char* s4)
  { snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3 , s4) ; }

void chatBufferCat(      const char* s)
  { strncat(ChatBuffer , s , CHAT_BUFFER_SIZE - strlen(ChatBuffer) - 1) ; }

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

void chatBufferDump(PurpleConversation* thisConv)
{
DBGs("chatBufferDump() ChatBuffer = -->\n" , ChatBuffer) ;

  purple_conversation_write(thisConv , BRIDGIN_NICK , ChatBuffer ,
                            PURPLE_MESSAGE_SYSTEM , time(0)) ;
}


/* main */

static PurplePluginInfo PluginInfo =
{
  PURPLE_PLUGIN_MAGIC , PURPLE_MAJOR_VERSION , PURPLE_MINOR_VERSION ,
  PLUGIN_TYPE , PLUGIN_GUI_TYPE , 0 , NULL , PURPLE_PRIORITY_DEFAULT ,
  PLUGIN_ID , PLUGIN_NAME , PLUGIN_VERSION , PLUGIN_SHORT_DESC , PLUGIN_LONG_DESC ,
  PLUGIN_AUTHOR , PLUGIN_WEBSITE , PLUGIN_ONLOAD_CB , PLUGIN_ONUNLOAD_CB ,
  NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL
} ;


PURPLE_INIT_PLUGIN(PLUGIN_NAME , handlePluginInit , PluginInfo)
