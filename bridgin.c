
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
  Bridge* bridge ; Channel* channel ;

  bridge  = (Bridge*) malloc(sizeof(Bridge)) ;  if (!bridge)  return NULL ;
  channel = (Channel*)malloc(sizeof(Channel)) ; if (!channel) return NULL ;

  bridge->name            = bridgeName ;
  bridge->isEnabled       = TRUE ;
  bridge->next            = NULL ;
  bridge->sentinelChannel = channel ;

  channel->name        = "sentinel" ;
  channel->protocol[0] = '\0' ;
  channel->account     = NULL ;
  channel->username    = "" ;
  channel->conv        = NULL ;
  channel->isActive    = FALSE ;
  channel->next        = NULL ;

  return bridge ;
}

Channel* newChannel(PurpleConversation* conv)
{
  Channel* channel ; PurpleAccount* account ;
  const char* protocol ; char* network ;

  channel = (Channel*)malloc(sizeof(Channel)) ; if (!channel) return NULL ;

  // append network url to protocol
  account = getAccount(conv) ; protocol = getProtocol(account) ;
  if (!strcmp(protocol , IRC_PROTOCOL) && (network = strstr(getUsername(account) , "@")))
    snprintf(channel->protocol , PROTOCOL_BUFFER_SIZE , "%s%s" , protocol , network) ;
  else strncpy(channel->protocol , protocol , PROTOCOL_BUFFER_SIZE) ;

  channel->name     = getChannelName(conv) ;
  channel->account  = account ;
  channel->username = getNick(account) ;
  channel->conv     = conv ;
  channel->isActive = TRUE ;
  channel->next     = NULL ;

  return channel ;
}

Bridge* getBridgeByChannel(PurpleConversation* conv)
{
  Bridge* bridge ; Channel* channel ;

  bridge = SentinelBridge ;
  while (bridge->next)
  {
    channel = (bridge = bridge->next)->sentinelChannel ;
    while (channel->next)
      if ((channel = channel->next)->conv == conv)
{
DBGss("getBridgeByChannel() '" , getChannelName(conv) , "' found" , "") ;

        return bridge ;
}
  }
DBGss("getBridgeByChannel() '" , getChannelName(conv) , "' not found" , "") ;

  return SentinelBridge ;
}

Bridge* getBridgeByName(char* bridgeName)
{
  Bridge* bridge ;

  bridge = SentinelBridge ;
  while (bridge->next)
    if (!strcmp((bridge = bridge->next)->name , bridgeName))
{
DBGss("getBridgeByName() '" , bridgeName , "' found" , "") ;

      return bridge ;
}
DBGss("getBridgeByName() '" , bridgeName , "' not found" , "") ;

  return SentinelBridge ;
}

const char* getChannelName(PurpleConversation* conv) { return purple_conversation_get_name(conv) ; }

const char* getProtocol(PurpleAccount *account) { return purple_account_get_protocol_name(account) ; }

PurpleAccount* getAccount(PurpleConversation* conv) { return purple_conversation_get_account(conv) ; }

const char* getUsername(PurpleAccount *account) { return purple_account_get_username(account) ; }

const char* getNick(PurpleAccount *account) { return purple_account_get_name_for_display(account) ; }

void setChannel(char* bridgeName , PurpleConversation* conv)
{
  Bridge* bridge ; Channel* channel ;

  if ((bridge = getBridgeByName(bridgeName)) == SentinelBridge)
  {
    while (bridge->next) bridge = bridge->next ;
    bridge->next = newBridge(bridgeName) ; if (!bridge->next) { alert(OOM_MSG) ; return ; }

    bridge = bridge->next ;
  }
  channel = bridge->sentinelChannel ;
  while (channel->next) channel = channel->next ;
  channel->next = newChannel(conv) ; if (!channel->next) alert(OOM_MSG) ;
}

void storeSession() {} // TODO:

unsigned int getNBridges()
{
  Bridge* bridge ; unsigned int n ; bridge = SentinelBridge ; n = 0 ;
  while ((bridge = bridge->next)) ++n ;
  return n ;
}

unsigned int getNChannels(Bridge* bridge)
{
  Channel* channel ; unsigned int n ; channel = bridge->sentinelChannel ; n = 0 ;
  while ((channel = channel->next)) ++n ;
  return n ;
}


/* event handlers */

void handlePluginInit(PurplePlugin* plugin) { SentinelBridge = newBridge("sentinel") ; }

gboolean handlePluginLoaded(PurplePlugin* aPlugin)
{
  ThisPlugin = aPlugin ;

  purple_signal_connect(purple_conversations_get_handle() , "received-im-msg" ,
                        aPlugin , PURPLE_CALLBACK(handleIm)            , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , "received-chat-msg" ,
                        aPlugin , PURPLE_CALLBACK(handleChat)          , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , "deleting-conversation" ,
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

void handleIm(PurpleAccount* account , char* sender , char* buffer ,
              PurpleConversation* conv , PurpleMessageFlags flags , void* data)
{
DBGchat("received-im-msg" , account , sender , conv , buffer , flags , data) ;

//  purple_conv_chat_send(PURPLE_CONV_CHAT(conv) , formatMessage()) ;
}

void handleChat(PurpleAccount* account , char* sender , char* buffer ,
                PurpleConversation* conv , PurpleMessageFlags flags , void* data)
{
DBGchat("received-chat-msg" , account , sender , conv , buffer , flags , data) ;

  if (flags & PURPLE_MESSAGE_SEND) return ;

//  purple_conv_chat_send(PURPLE_CONV_CHAT(conv) , formatMessage()) ;
}

void handleChannelClosed(PurpleConversation* conv , void* data)
{
DBGchannelClosed(conv) ;

}


/* admin command handlers */

PurpleCmdRet handleAddCmd(PurpleConversation* conv , const gchar* command ,
                          gchar** args , gchar** error , void* data)
{
  char* bridgeName ; Bridge* thisBridge ;

DBGcmd(command , args[0]) ;

  bridgeName = (isBlank(args[0]))? DEFAULT_BRIDGE_NAME : args[0] ;
  if ((thisBridge = getBridgeByChannel(conv)) != SentinelBridge)
  {
    if (thisBridge == getBridgeByName(bridgeName)) addExistsResp(conv , bridgeName) ;
    else addConflictResp(conv) ;
  }
  else { setChannel(bridgeName , conv) ; storeSession() ; addResp(conv , bridgeName) ; }

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleRemoveCmd(PurpleConversation* conv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEnableCmd(PurpleConversation* conv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEchoCmd(PurpleConversation* conv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleChatCmd(PurpleConversation* conv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleBroadcastCmd(PurpleConversation* conv , const gchar* command ,
                                gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleStatusCmd(PurpleConversation* conv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleHelpCmd(PurpleConversation* conv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

/* admin command responses */

void addResp(PurpleConversation* conv , char* bridgeName)
{
  chatBufferFillSS("%s '%s'" , CH_SET_MSG , bridgeName) ;
  chatBufferDump(conv) ; bridgeStatsMsg(conv , bridgeName) ;
}

void addExistsResp(PurpleConversation* conv , char* bridgeName)
{
  chatBufferFillSSS("%s %s '%s'" , THIS_CHANNEL_MSG , CHANNEL_EXISTS_MSG , bridgeName) ;
  chatBufferDump(conv) ;
}

void addConflictResp(PurpleConversation* conv)
  { chatBufferFillS("%s" , BRIDGE_CONFLICT_MSG) ; chatBufferDump(conv) ; }
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
void bridgeStatsMsg(PurpleConversation* conv , char* bridgeName)
{
  Bridge* bridge ; Channel* channel ; unsigned int nChars ;
  unsigned int nChannels ; char nchannels[5] ; char* activeMsg ;

DBG("bridgeStatsMsg()") ;

  if ((bridge = getBridgeByName(bridgeName)) == SentinelBridge)
  {
    if (getNBridges()) chatBufferFillSS("\n%s '%s'" , NO_SUCH_BRIDGE_MSG , bridgeName) ;
    else chatBufferFillS("\n%s" , NO_BRIDGES_MSG) ;
    chatBufferDump(conv) ; return ;
  }

  nChars = chatBufferFillSS("%s '%s' " , STATS_MSGa , bridgeName) ;
  nChannels = getNChannels(bridge) ;
  if (!nChannels) nChars += chatBufferCat(STATS_DELETED_MSG , nChars) ;
  else
  {
    if (bridge->isEnabled) nChars += chatBufferCat(STATS_ENABLED_MSG , nChars) ;
    else nChars += chatBufferCat(STATS_DISABLED_MSG , nChars) ;
    snprintf(nchannels , 5 , " %d " , nChannels) ; nChars += 4 ;
    nChars += chatBufferCat(nchannels , nChars) ;
    nChars += chatBufferCat(STATS_MSGb , nChars) ;
  }

  channel = bridge->sentinelChannel ;
  while ((channel = channel->next))
  {
    activeMsg = (channel->isActive)? CH_ACTIVE_MSG : CH_INACTIVE_MSG ;
    nChars += chatBufferCat("\n"               , nChars) ;
    nChars += chatBufferCat(activeMsg          , nChars) ;
    nChars += chatBufferCat("'"                , nChars) ;
    nChars += chatBufferCat(channel->name      , nChars) ;
    nChars += chatBufferCat("' on '"           , nChars) ;
    nChars += chatBufferCat(channel->protocol  , nChars) ;
    nChars += chatBufferCat("' as '"           , nChars) ;
    nChars += chatBufferCat(channel->username  , nChars) ;
    nChars += chatBufferCat("'"                , nChars) ;
  }
  chatBufferDump(conv) ;
}

/* chat buffer helpers */

unsigned int chatBufferFillS(     const char* fmt , const char* s1)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1) ; }

unsigned int chatBufferFillSS(    const char* fmt , const char* s1 , const char* s2)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2) ; }

unsigned int chatBufferFillSSS(   const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3) ; }

unsigned int chatBufferFillSSSDSD(const char* fmt , const char* s1 , const char* s2 ,
                                  const char* s3 , int d1 , const char* s4 , int d2)
  { return snprintf(ChatBuffer , CHAT_BUFFER_SIZE , fmt , s1 , s2 , s3 , d1 , s4 , d2) ; }

unsigned int chatBufferCat(       const char* msg , unsigned int nChars)
{
  strncat(ChatBuffer , msg , CHAT_BUFFER_SIZE - nChars - 1) ;
  return nChars + strlen(msg) ;
}

void chatBufferDump(PurpleConversation* conv)
{
  purple_conversation_write(conv , BRIDGIN_NICK , ChatBuffer ,
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
