
#include "bridgin.h"
#include "bridgin.dbg.h"


/* helpers */

PurpleCmdId registerCmd(const char* command , const char* format ,
                        PurpleCmdRet (* callback)() , const char* help)
{
  return purple_cmd_register(command , format , PURPLE_CMD_P_DEFAULT ,
      PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT , PLUGIN_ID , callback , help , NULL) ;
}

void alert(char* msg)
{
  purple_notify_message(ThisPlugin , PURPLE_NOTIFY_MSG_INFO , msg ,
                        PLUGIN_VERSION , NULL , NULL , NULL) ;
}

gboolean isBlank(const char* aCstring) { return (!aCstring || !*aCstring) ; }


/* model helpers */

Bridge* newBridge(char* bridgeName)
{
  Bridge* aBridge ; Channel* aChannel ;

  aBridge  = (Bridge*) malloc(sizeof(Bridge)) ;  if (!aBridge)  return NULL ;
  aChannel = (Channel*)malloc(sizeof(Channel)) ; if (!aChannel) return NULL ;

  aBridge->name            = bridgeName ;
  aBridge->isEnabled       = TRUE ;
  aBridge->next            = NULL ;
  aBridge->sentinelChannel = aChannel ;

  aChannel->name        = "sentinel" ;
  aChannel->protocol[0] = '\0' ;
  aChannel->account     = NULL ;
  aChannel->username    = "" ;
  aChannel->conv        = NULL ;
  aChannel->isActive    = FALSE ;
  aChannel->next        = NULL ;

  return aBridge ;
}

Channel* newChannel(PurpleConversation* aConv)
{
  Channel* aChannel ; PurpleAccount* anAccount ;
  const char* protocol ; char* network ;

  aChannel = (Channel*)malloc(sizeof(Channel)) ; if (!aChannel) return NULL ;

  // append network url to protocol
  anAccount = getAccount(aConv) ; protocol = getProtocol(anAccount) ;
  if (!strcmp(protocol , IRC_PROTOCOL) && (network = strstr(getUsername(anAccount) , "@")))
    snprintf(aChannel->protocol , PROTOCOL_BUFFER_SIZE , "%s%s" , protocol , network) ;
  else strncpy(aChannel->protocol , protocol , PROTOCOL_BUFFER_SIZE) ;

  aChannel->name     = getChannelName(aConv) ;
  aChannel->account  = anAccount ;
  aChannel->username = getNick(anAccount) ;
  aChannel->conv     = aConv ;
  aChannel->isActive = TRUE ;
  aChannel->next     = NULL ;

  return aChannel ;
}

Bridge* getBridgeByChannel(PurpleConversation* aConv)
{
  Bridge* aBridge ; Channel* aChannel ;

  aBridge = SentinelBridge ;
  while (aBridge->next)
  {
    aChannel = (aBridge = aBridge->next)->sentinelChannel ;
    while (aChannel->next)
      if ((aChannel = aChannel->next)->conv == aConv)
{
DBGss("getBridgeByChannel() '" , getChannelName(aConv) , "' found" , "") ;

        return aBridge ;
}
  }
DBGss("getBridgeByChannel() '" , getChannelName(aConv) , "' not found" , "") ;

  return SentinelBridge ;
}

Bridge* getBridgeByName(char* bridgeName)
{
  Bridge* aBridge ;

  aBridge = SentinelBridge ;
  while (aBridge->next)
    if (!strcmp((aBridge = aBridge->next)->name , bridgeName))
{
DBGss("getBridgeByName() '" , bridgeName , "' found" , "") ;

      return aBridge ;
}
DBGss("getBridgeByName() '" , bridgeName , "' not found" , "") ;

  return SentinelBridge ;
}

const char* getChannelName(PurpleConversation* aConv) { return purple_conversation_get_name(aConv) ; }

const char* getProtocol(PurpleAccount* anAccount) { return purple_account_get_protocol_name(anAccount) ; }

PurpleAccount* getAccount(PurpleConversation* aConv) { return purple_conversation_get_account(aConv) ; }

const char* getUsername(PurpleAccount* anAccount) { return purple_account_get_username(anAccount) ; }

const char* getNick(PurpleAccount* anAccount) { return purple_account_get_name_for_display(anAccount) ; }

gboolean addChannel(char* bridgeName , PurpleConversation* aConv)
{
  Bridge* aBridge ; Channel* aChannel ;

  if ((aBridge = getBridgeByName(bridgeName)) == SentinelBridge)
  {
    while (aBridge->next) aBridge = aBridge->next ;
    aBridge->next = newBridge(bridgeName) ;
    if (aBridge->next) aBridge = aBridge->next ; else { alert(OOM_MSG) ; return FALSE ; }
  }
  aChannel = aBridge->sentinelChannel ;
  while (aChannel->next) aChannel = aChannel->next ;
  aChannel->next = newChannel(aConv) ;
  if (!aChannel->next) { alert(OOM_MSG) ; return FALSE ; }

  storeSession() ; return TRUE ;
}

void storeSession() {} // TODO:

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

void handlePluginInit(PurplePlugin* plugin)
  { ChatMutex = TRUE ; SentinelBridge = newBridge("sentinel") ; }

gboolean handlePluginLoaded(PurplePlugin* aPlugin)
{
  ThisPlugin = aPlugin ;

  purple_signal_connect(purple_conversations_get_handle() , RECEIVED_IM_SIGNAL ,
                        aPlugin , PURPLE_CALLBACK(handleChat)          , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , RECEIVED_CHAT_SIGNAL ,
                        aPlugin , PURPLE_CALLBACK(handleChat)          , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , CHANNEL_CLOSING_SIGNAL ,
                        aPlugin , PURPLE_CALLBACK(handleChannelClosed) , NULL) ;

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

  if (SentinelBridge) return TRUE ; else { alert(OOM_MSG) ; return FALSE ; }
}

gboolean handlePluginUnloaded(PurplePlugin* plugin)
{
  int i ; for (i = 0 ; i < N_COMMANDS ; ++i) purple_cmd_unregister(CommandIds[i]) ;

  return TRUE ;
}

void handleChat(PurpleAccount* anAccount , char* sender , char* msg ,
                PurpleConversation* aConv , PurpleMessageFlags flags , void* data)
{
  Bridge* aBridge ; unsigned int convType ; Channel* aChannel ;

DBGchat(((purple_conversation_get_type(aConv) == PURPLE_CONV_TYPE_IM)? RECEIVED_IM_SIGNAL : RECEIVED_CHAT_SIGNAL) , anAccount , sender , aConv , msg , flags , data) ;

  if (flags & PURPLE_MESSAGE_SEND) return ;
  if ((aBridge = getBridgeByChannel(aConv)) == SentinelBridge) return ;

  chatBufferFillSSSS("%s %s%s %s" , NICK_PREFIX , sender , NICK_POSTFIX , msg) ;
  convType = purple_conversation_get_type(aConv) ;
  aChannel = aBridge->sentinelChannel ;
  while ((aChannel = aChannel->next)) if (aChannel->conv != aConv)
  {
    if (convType == PURPLE_CONV_TYPE_IM)
      purple_conv_im_send(PURPLE_CONV_IM(aChannel->conv) , ChatBuffer) ;
    else if (convType == PURPLE_CONV_TYPE_CHAT)
      purple_conv_chat_send(PURPLE_CONV_CHAT(aChannel->conv) , ChatBuffer) ;
  }
}

void handleChannelClosed(PurpleConversation* aConv , void* data)
{
DBGchannelClosed(aConv) ;

}


/* admin command handlers */

PurpleCmdRet handleAddCmd(PurpleConversation* aConv , const gchar* command ,
                          gchar** args , gchar** error , void* data)
{
  char* bridgeName ; Bridge* thisBridge ; unsigned int convType ;

DBGcmd(command , args[0]) ;

  convType = purple_conversation_get_type(aConv) ;
  if (convType != PURPLE_CONV_TYPE_IM && convType != PURPLE_CONV_TYPE_CHAT)
    return PURPLE_CMD_RET_OK ;

  bridgeName = (isBlank(args[0]))? DEFAULT_BRIDGE_NAME : args[0] ;
  if ((thisBridge = getBridgeByChannel(aConv)) != SentinelBridge)
  {
    if (thisBridge == getBridgeByName(bridgeName)) addExistsResp(aConv , bridgeName) ;
    else addConflictResp(aConv) ;
  }
  else if (addChannel(bridgeName , aConv)) addResp(aConv , bridgeName) ;

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

  return PURPLE_CMD_RET_OK ;
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
  chatBufferFillSS("%s '%s'" , CH_SET_MSG , bridgeName) ;
  chatBufferDump(aConv) ; bridgeStatsMsg(aConv , bridgeName) ;
}

void addExistsResp(PurpleConversation* aConv , char* bridgeName)
{
  chatBufferFillSSS("%s %s '%s'" , THIS_CHANNEL_MSG , CHANNEL_EXISTS_MSG , bridgeName) ;
  chatBufferDump(aConv) ;
}

void addConflictResp(PurpleConversation* aConv)
{
  chatBufferFillS("%s" , BRIDGE_CONFLICT_MSG) ; chatBufferDump(aConv) ;
}
/*
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

function statusResp($accountId , $channelId , $bridgeName)
{
  global $Bridges ;

  $resp = channelStateMsg($accountId , $channelId) ;
  if (!count($Bridges)) $resp .= "\n" . bridgeStatsMsg("") ;
  else if ($bridgeName) $resp .= "\n" . bridgeStatsMsg($bridgeName) ;
  else foreach ($Bridges as $name => $aBridge) $resp .= bridgeStatsMsg($name) ;
  return $resp ;
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
void bridgeStatsMsg(PurpleConversation* aConv , char* bridgeName)
{
  Bridge* aBridge ; Channel* aChannel ; unsigned int nChars ;
  unsigned int nChannels ; char nchannels[5] ; char* activeMsg ;

DBG("bridgeStatsMsg()") ;

  if ((aBridge = getBridgeByName(bridgeName)) == SentinelBridge)
  {
    if (getNBridges()) chatBufferFillSS("\n%s '%s'" , NO_SUCH_BRIDGE_MSG , bridgeName) ;
    else chatBufferFillS("\n%s" , NO_BRIDGES_MSG) ;
    chatBufferDump(aConv) ; return ;
  }

  nChars = chatBufferFillSS("%s '%s' " , STATS_MSGa , bridgeName) ;
  nChannels = getNChannels(aBridge) ;
  if (!nChannels) nChars += chatBufferCat(STATS_DELETED_MSG , nChars) ;
  else
  {
    if (aBridge->isEnabled) nChars += chatBufferCat(STATS_ENABLED_MSG , nChars) ;
    else nChars += chatBufferCat(STATS_DISABLED_MSG , nChars) ;
    snprintf(nchannels , 5 , " %d " , nChannels) ; nChars += 4 ;
    nChars += chatBufferCat(nchannels , nChars) ;
    nChars += chatBufferCat(STATS_MSGb , nChars) ;
  }

  aChannel = aBridge->sentinelChannel ;
  while ((aChannel = aChannel->next))
  {
    activeMsg = (aChannel->isActive)? CH_ACTIVE_MSG : CH_INACTIVE_MSG ;
    nChars += chatBufferCat("\n"               , nChars) ;
    nChars += chatBufferCat(activeMsg          , nChars) ;
    nChars += chatBufferCat("'"                , nChars) ;
    nChars += chatBufferCat(aChannel->name     , nChars) ;
    nChars += chatBufferCat("' on '"           , nChars) ;
    nChars += chatBufferCat(aChannel->protocol , nChars) ;
    nChars += chatBufferCat("' as '"           , nChars) ;
    nChars += chatBufferCat(aChannel->username , nChars) ;
    nChars += chatBufferCat("'"                , nChars) ;
  }
  chatBufferDump(aConv) ;
}

/* chat buffer helpers */

unsigned int chatBufferFillS(     const char* fmt , const char* s1)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1) ; }

unsigned int chatBufferFillSS(    const char* fmt , const char* s1 , const char* s2)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2) ; }

unsigned int chatBufferFillSSS(   const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3) ; }

unsigned int chatBufferFillSSSS(  const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3 , const char* s4)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3 , s4) ; }

unsigned int chatBufferFillSSSDSD(const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3 , int d1 , const char* s4 , int d2)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3 , d1 , s4 , d2) ; }

unsigned int chatBufferCat(       const char* msg , unsigned int nChars)
{
  strncat(ChatBuffer , msg , CHAT_BUFFER_SIZE - nChars - 1) ;
  return nChars + strlen(msg) ;
}

void chatBufferDump(PurpleConversation* aConv)
{
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
