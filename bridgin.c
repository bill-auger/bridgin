




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

const char* getNick(PurpleAccount* anAccount)
  { return purple_account_get_name_for_display(anAccount) ; }

void alert(char* msg)
{
  purple_notify_message(ThisPlugin , PURPLE_NOTIFY_MSG_INFO , msg ,
                        PLUGIN_VERSION , NULL , NULL , NULL) ;
}

gboolean isBlank(const char* aCstring) { return (!aCstring || !*aCstring) ; }


/* model helpers */

gboolean areReservedIds(char* bridgeName , char* channelUid)
{
  return (!strcmp(bridgeName , SENTINEL_NAME) ||
          !strcmp(channelUid  , SENTINEL_NAME)) ;
}

Bridge* newBridge(char* bridgeName)
{
  Bridge* aBridge ; Channel* aChannel ;

  aBridge  = (Bridge*) malloc(sizeof(Bridge)) ;  if (!aBridge)  return NULL ;
  aChannel = (Channel*)malloc(sizeof(Channel)) ; if (!aChannel) return NULL ;

  strcpy(aBridge->name     , bridgeName) ;
  aBridge->isEnabled       = TRUE ;
  aBridge->next            = NULL ;
  aBridge->sentinelChannel = aChannel ;

  strcpy(aChannel->uid , SENTINEL_NAME) ;
  aChannel->next = NULL ;

  return aBridge ;
}

Channel* newChannel()
{
  Channel* aChannel = (Channel*)malloc(sizeof(Channel)) ; if (!aChannel) return NULL ;

  strcpy(aChannel->uid , UidBuffer) ;
  aChannel->next       = NULL ;

  return aChannel ;
}

void makeChannelId(PurpleConversation* aConv)
{
  PurpleAccount* anAccount ;
  const char* protocol ; const char* username ; const char* channelName ;

  anAccount = getAccount(aConv) ;      protocol    = getProtocol(anAccount) ;
  username  = getUsername(anAccount) ; channelName = getChannelName(aConv) ;
  sprintf(UidBuffer , CHANNEL_ID_FMT , protocol , username , channelName) ;
}

gboolean addChannel(char* bridgeName)
{
  Bridge* aBridge ; Channel* aChannel ;
  char bridgePrefKey[UID_BUFFER_SIZE] ; char enabledPrefKey[UID_BUFFER_SIZE] ;
  GList* channelsList = NULL ;

DBGss("addChannel() bridgeName='" , bridgeName , "'" , "") ;

  // bridge pref key
  snprintf(bridgePrefKey  , UID_BUFFER_SIZE , BRIDGE_PREF_FMT ,
           BASE_PREF_KEY , bridgeName) ;
  snprintf(enabledPrefKey , UID_BUFFER_SIZE , ENABLED_PREF_FMT  ,
           bridgePrefKey , ENABLED_PREF_KEY) ;

  if ((aBridge = getBridgeByName(bridgeName)) == SentinelBridge)
  {
    // create new bridge
    while (aBridge->next) aBridge = aBridge->next ;
    aBridge->next = newBridge(bridgeName) ;
    if (!(aBridge = aBridge->next)) return FALSE ;

    // store bridge config
    purple_prefs_add_bool(enabledPrefKey , TRUE) ;
    purple_prefs_add_string_list(bridgePrefKey , channelsList) ;

DBGs("addChannel() adding new bridgeKey=" , bridgePrefKey) ;
  }
  else channelsList = purple_prefs_get_string_list(bridgePrefKey) ;

  // create new channel
  aChannel = aBridge->sentinelChannel ;
  while (aChannel->next) aChannel = aChannel->next ;
  aChannel->next = newChannel() ;
  if (!aChannel->next) { g_list_free(channelsList) ; return FALSE ; }

  // store bridge config
  channelsList = g_list_append(channelsList , (gpointer)UidBuffer) ;
  purple_prefs_set_string_list(bridgePrefKey , channelsList) ;

DBGs("addChannel() adding new channelUid=" , UidBuffer) ;

  g_list_free(channelsList) ; return TRUE ;
}

Bridge* getBridgeByChannel(PurpleConversation* aConv)
{
  Bridge* aBridge ; Channel* aChannel ;

/*DBG*/makeChannelId(aConv);DBGss("getBridgeByChannel() channel='" , UidBuffer , "'" , "") ;

  aBridge = SentinelBridge ; makeChannelId(aConv) ;
  while ((aBridge = aBridge->next))
  {
    aChannel = aBridge->sentinelChannel ;
    while ((aChannel = aChannel->next))
      if (!strcmp(aChannel->uid , UidBuffer))
        return aBridge ;
  }
DBGss("getBridgeByChannel() '" , UidBuffer , "' not found" , "") ;

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

void handlePluginInit(PurplePlugin* plugin) { purple_prefs_add_none(BASE_PREF_KEY) ; }

gboolean handlePluginLoaded(PurplePlugin* aPlugin)
{
  void* convs ;
  GList* prefsList ; GList* prefsIter ; const char* prefKey ; char* bridgeName ;
  GList* channelsList ; GList* channelsIter ;

DBG("handlePluginLoaded()") ;

  // init
  ThisPlugin = aPlugin ; SentinelBridge = newBridge(SENTINEL_NAME) ;
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

        strcpy(UidBuffer , (char*)channelsIter->data) ;
        if (!addChannel(bridgeName))
        {
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
  unsigned int i ;
  Bridge* prevBridge ; Bridge* nextBridge ;
  Channel* prevChannel ; Channel* nextChannel ;

DBG("handlePluginUnloaded()") ;

  // unregister commands
  for (i = 0 ; i < N_COMMANDS ; ++i) purple_cmd_unregister(CommandIds[i]) ;

  // unregister callbacks
  purple_prefs_disconnect_by_handle(purple_prefs_get_handle()) ;
  purple_signal_disconnect(purple_conversations_get_handle() , RECEIVED_IM_SIGNAL ,
                           ThisPlugin , PURPLE_CALLBACK(handleChat)) ;
  purple_signal_disconnect(purple_conversations_get_handle() , RECEIVED_CHAT_SIGNAL ,
                           ThisPlugin , PURPLE_CALLBACK(handleChat)) ;

  prevBridge = SentinelBridge ;
  while ((nextBridge = prevBridge->next))
  {
    prevChannel = nextBridge->sentinelChannel ;
    while ((nextChannel = prevChannel->next))
      { free(prevChannel) ; prevChannel = nextChannel ; }
    free(prevChannel) ; free(prevBridge) ; prevBridge = nextBridge ;
  }
  free(prevBridge) ;

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

PurpleCmdRet handleAddCmd(PurpleConversation* aConv , const gchar* command ,
                          gchar** args , gchar** error , void* data)
{
  char* bridgeName ; Bridge* thisBridge ;

DBGcmd(command , args[0]) ;

  bridgeName = (isBlank(args[0]))? DEFAULT_BRIDGE_NAME : args[0] ;
  if ((thisBridge = getBridgeByChannel(aConv)) != SentinelBridge)
  {
    if (thisBridge == getBridgeByName(bridgeName)) addExistsResp(aConv , bridgeName) ;
    else addConflictResp(aConv) ;
  }
  else
  {
    makeChannelId(aConv) ;
    if (!areReservedIds(bridgeName , UidBuffer))
      if (addChannel(bridgeName)) addResp(aConv , bridgeName) ;
      else addFailResp(aConv) ;
    else addReservedResp(aConv) ;
  }

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleRemoveCmd(PurpleConversation* aConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEnableCmd(PurpleConversation* aConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEchoCmd(PurpleConversation* aConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleChatCmd(PurpleConversation* aConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleBroadcastCmd(PurpleConversation* aConv , const gchar* command ,
                                gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleStatusCmd(PurpleConversation* aConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  statusResp(aConv , args[0]) ; return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleHelpCmd(PurpleConversation* aConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

/* admin command responses */

void addResp(PurpleConversation* aConv , char* bridgeName)
{
  chatBufferPutSS("%s '%s'" , CH_SET_MSG , bridgeName) ; chatBufferDump(aConv) ;
  chatBufferInit() ; bridgeStatsMsg(bridgeName) ; chatBufferDump(aConv) ;
}

void addExistsResp(PurpleConversation* aConv , char* bridgeName)
{
  chatBufferPutSSS("%s %s '%s'" , THIS_CHANNEL_MSG , CHANNEL_EXISTS_MSG , bridgeName) ;
  chatBufferDump(aConv) ;
}

void addConflictResp(PurpleConversation* aConv)
{
  chatBufferPutS("%s" , BRIDGE_CONFLICT_MSG) ; chatBufferDump(aConv) ;
}

void addReservedResp(PurpleConversation* aConv)
{
  chatBufferPutS("%s" , RESERVED_NAME_MSG) ; chatBufferDump(aConv) ;
}

void addFailResp(PurpleConversation* aConv)
{
  chatBufferPutS("%s" , OOM_MSG) ; chatBufferDump(aConv) ;
}

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

void statusResp(PurpleConversation* aConv , char* bridgeName)
{
  Bridge* aBridge ; unsigned int nBridges = getNBridges() ;

DBGs("statusResp() bridgeName=" , (!nBridges)? "no bridges" : ((!isBlank(bridgeName))? bridgeName : "unspecified - listing all")) ;

  if (!nBridges)                 { chatBufferInit() ; bridgeStatsMsg("") ; }
  else if (!isBlank(bridgeName)) { chatBufferInit() ; bridgeStatsMsg(bridgeName) ; }
  else
  {
    chatBufferPutSDS("%s %d %s\n\n" , STATS_MSGa , getNBridges() , STATS_MSGb) ;

    aBridge = SentinelBridge ;
    while ((aBridge = aBridge->next)) bridgeStatsMsg(aBridge->name) ;
  }
  if (nBridges) channelStateMsg(aConv) ;
  chatBufferDump(aConv) ;
}

void channelStateMsg(PurpleConversation* aConv)
{
// NOTE: callers of channelStateMsg() should first initialize ChatBuffer
//    then eventually call chatBufferDump() to flush to screen

  Bridge* aBridge ;

DBGs("statusResp() channelName=" , getChannelName(aConv)) ;

  if ((aBridge = getBridgeByChannel(aConv)) != SentinelBridge)
  {
    uidBufferPutSSS("%s %s '%s'" , THIS_CHANNEL_MSG , THIS_BRIDGE_MSG , aBridge->name) ;
    chatBufferCat(UidBuffer) ;
  }
  else
  {
    uidBufferPutSS("%s %s" , THIS_CHANNEL_MSG , UNBRIDGED_MSG) ;
    chatBufferCat(UidBuffer) ;
  }
}

void bridgeStatsMsg(const char* bridgeName)
{
// NOTE: callers of bridgeStatsMsg() should first initialize ChatBuffer
//    then eventually call chatBufferDump() to flush to screen

  gboolean doesBridgeExist ; Bridge* aBridge ; Channel* aChannel ;
  unsigned int nChannels ;
  GList* activeChannelsIter = NULL ; gboolean isActive = FALSE ;
  char* activeMsg ; char* protocol ; char* username ; const char* channelName ;
  char* network ; char nick[UID_BUFFER_SIZE] ;

DBG("bridgeStatsMsg()") ;

  aBridge = getBridgeByName(bridgeName) ;
  doesBridgeExist = (aBridge != SentinelBridge) ;
  if (!doesBridgeExist)
    if (!getNBridges()) statusBufferPutS("%s" , NO_BRIDGES_MSG) ;
    else statusBufferPutSS("%s '%s'" , NO_SUCH_BRIDGE_MSG , bridgeName) ;
  else statusBufferPutSS("%s '%s' - " , STATS_MSGc , bridgeName) ;
  chatBufferCat(StatusBuffer) ; if (!doesBridgeExist) return ;

  nChannels = getNChannels(aBridge) ;
  if (!nChannels) chatBufferCat(STATS_DELETED_MSG) ;
  else
  {
    if (aBridge->isEnabled) chatBufferCat(STATS_ENABLED_MSG) ;
    else chatBufferCat(STATS_DISABLED_MSG) ;
    statusBufferPutDS(" - %d %s\n" , nChannels , STATS_MSGd) ;
    chatBufferCat(StatusBuffer) ;
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

      makeChannelId((PurpleConversation*)activeChannelsIter->data) ;
      isActive |= !strcmp(aChannel->uid , UidBuffer) ;
      activeChannelsIter = g_list_next(activeChannelsIter) ;
    }
    activeMsg = (isActive)? CH_ACTIVE_MSG : CH_INACTIVE_MSG ;

DBGss("bridgeStatsMsg() aChannel=" , aChannel->uid , " " , activeMsg) ;

    // parse channel data from channelUid
    uidBufferPutS("%s" , aChannel->uid) ;
    if (!(protocol    = strtok(UidBuffer , UID_DELIMITER)) ||
        !(username    = strtok(NULL      , UID_DELIMITER)) ||
        !(channelName = strtok(NULL      , UID_DELIMITER)))
      continue ;

DBGsssss("bridgeStatsMsg() parsed channelId " , activeMsg , " protocol='" , protocol , "' username='" , username , "' channelName='" , channelName , "'" , "") ;

    // display channel data
    statusBufferPutSSS("    %s '%s' on '%s" , activeMsg , channelName , protocol) ;
    chatBufferCat(StatusBuffer) ;
    if (!strcmp(protocol , IRC_PROTOCOL) &&
        (network = strchr(username , '@')) &&
        (strncpy(nick , username , network - username)))
    {
      nick[network - username] = '\0' ;
      statusBufferPutSS("%s' as '%s'\n" , network , nick) ;
    }
    else statusBufferPutS("' as '%s'\n" , username) ;
    chatBufferCat(StatusBuffer) ;
  } // each aChannel

  chatBufferCat("\n") ;
}


/* text buffer helpers */

void uidBufferPutS(     const char* fmt , const char* s1)
  { snprintf(UidBuffer    , UID_BUFFER_SIZE  , fmt , s1) ; }

void uidBufferPutSS(    const char* fmt , const char* s1 , const char* s2)
  { snprintf(UidBuffer    , UID_BUFFER_SIZE  , fmt , s1 , s2) ; }

void uidBufferPutSSS(   const char* fmt , const char* s1 , const char* s2 , const char* s3)
  { snprintf(UidBuffer    , UID_BUFFER_SIZE  , fmt , s1 , s2 , s3) ; }

void statusBufferPutS(  const char* fmt , const char* s1)
  { snprintf(StatusBuffer , UID_BUFFER_SIZE  , fmt , s1) ; }

void statusBufferPutSS( const char* fmt , const char* s1 , const char* s2)
  { snprintf(StatusBuffer , UID_BUFFER_SIZE  , fmt , s1 , s2) ; }

void statusBufferPutDS( const char* fmt , int d1 , const char* s1)
  { snprintf(StatusBuffer , UID_BUFFER_SIZE  , fmt , d1 , s1) ; }

void statusBufferPutSSS(const char* fmt , const char* s1 , const char* s2 , const char* s3)
  { snprintf(StatusBuffer , UID_BUFFER_SIZE  , fmt , s1 , s2 , s3) ; }

void chatBufferInit() { ChatBuffer[0] = '\0' ; }

void chatBufferPutS(    const char* fmt , const char* s1)
  { snprintf(ChatBuffer   , CHAT_BUFFER_SIZE , fmt , s1) ; }

void chatBufferPutSS(   const char* fmt , const char* s1 , const char* s2)
  { snprintf(ChatBuffer   , CHAT_BUFFER_SIZE , fmt , s1 , s2) ; }

void chatBufferPutSSS(  const char* fmt , const char* s1 , const char* s2 , const char* s3)
  { snprintf(ChatBuffer   , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3) ; }

void chatBufferPutSDS(  const char* fmt , const char* s1 , int d1 , const char* s2)
  { snprintf(ChatBuffer   , CHAT_BUFFER_SIZE , fmt , s1 , d1 , s2) ; }

void chatBufferPutSSSS( const char* fmt , const char* s1 , const char* s2 ,
                        const char* s3  , const char* s4)
  { snprintf(ChatBuffer   , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3 , s4) ; }

void chatBufferCat(     const char* aCstring)
  { strncat(ChatBuffer , aCstring , CHAT_BUFFER_SIZE - strlen(ChatBuffer) - 1) ; }

void chatBufferDump(PurpleConversation* aConv)
{
DBGs("chatBufferDump() ChatBuffer\n=" , ChatBuffer) ;

  purple_conversation_write(aConv , BRIDGIN_NICK , ChatBuffer ,
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
