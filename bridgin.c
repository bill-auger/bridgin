#include "bridgin.h"
#include "bridgin.dbg.h"


/* purple helpers */

PurpleCmdId registerCmd(const char* command , const char* format ,
                        PurpleCmdRet (*callback)() , const char* help)
{
  return purple_cmd_register(command , format , PURPLE_CMD_P_DEFAULT ,
      PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT , PLUGIN_ID , callback , help , NULL) ;
}
/*
gboolean restoreSession()
{
  GList* prefsList ; GList* prefsIter ; const char* prefKey ; char* bridgeName ;
  GList* channelsList ; GList* channelsIter ;

  SentinelBridge = newBridge(SENTINEL_NAME , NULL , FALSE) ;
  if (!SentinelBridge) { alert(OOM_MSG) ; return FALSE ; }

  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  prefsIter = g_list_first(prefsList) ;
  while (prefsIter)
  {
#ifdef DEBUG_VB
if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_BOOLEAN)
  DBGsds("restoreSession() found bool prefKey='" , (char*)prefsIter->data , "' val='" , purple_prefs_get_bool((char*)prefsIter->data) , "'" , "") ;
else if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_STRING)
  DBGsss("restoreSession() found string prefKey='" , (char*)prefsIter->data , "' val='" , purple_prefs_get_string((char*)prefsIter->data) , "'" , "") ;
#endif

    prefKey = (char*)prefsIter->data ;
    if (purple_prefs_get_type(prefKey) == PURPLE_PREF_STRING_LIST &&
        (bridgeName = strrchr(prefKey , '/')) && ++bridgeName)
    {
#ifdef DEBUG_VB
DBGss("restoreSession() found stored bridgeName='" , bridgeName , "'" , "") ;
#endif

      channelsList = purple_prefs_get_string_list(prefKey) ;
      channelsIter = g_list_first(channelsList) ;
      while (channelsIter)
      {
#ifdef DEBUG_VB
DBGss("restoreSession() found stored channelUid='" , (char*)channelsIter->data , "'" , "") ;
#endif

        strcpy(ChannelUidBuffer , (char*)channelsIter->data) ;
        if (!createChannel(bridgeName , TRUE))
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
*/
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
/*
void destroySession()
{
  Bridge* aBridge ; Channel* aChannel ;

  aBridge = SentinelBridge ;
  while ((aBridge = aBridge->next))
  {
    aChannel = aBridge->sentinelChannel ;
    while ((aChannel = aChannel->next)) free(aChannel->prev) ;
    free(aChannel) ; free(aBridge->prev) ;
  }
  free(aBridge) ;
}
*/
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

//unsigned int getNRelayChannels(Bridge* thisBridge , PurpleConversation* thisConv)
void getNRelayChannels(char* thisBridgeName , PurpleConversation* thisConv)
{
// NOTE: inputConv will be excluded from the count - pass in NULL to include it

  GList* activeChannelsIter ; PurpleConversation* aConv ;
  char aBridgeName[SM_BUFFER_SIZE] ;

  activeChannelsIter = g_list_first(purple_get_conversations()) ; NRelayChannels = 0 ;
  while (activeChannelsIter)
  {
    aConv = (PurpleConversation*)activeChannelsIter->data ;
//     if (aConv != thisConv && getBridgeName(aConv) == thisBridge) ++nChannels ;
    getBridgeName(aConv , aBridgeName) ;
    if (aConv != thisConv && aBridgeName == thisBridgeName) ++NRelayChannels ;
    activeChannelsIter = g_list_next(activeChannelsIter) ;
  }
}


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

gint isThisChanne(gconstpointer a , gconstpointer b)
  { return strcmp((char*)a , (char*)b) ; }

//Bridge* getBridgeName(PurpleConversation* aConv)
void getBridgeName(PurpleConversation* aConv , char* thisBridgeNameBuffer)
{
/*
  Bridge* aBridge ; Channel* aChannel ;

#ifdef DEBUG_VB
prepareChannelUid(aConv) ; DBGss("getBridgeName() channel='" , ChannelUidBuffer , "'" , "") ;
#endif

  aBridge = SentinelBridge ; prepareChannelUid(aConv) ;
  while ((aBridge = aBridge->next))
  {
    aChannel = aBridge->sentinelChannel ;
    while ((aChannel = aChannel->next))
{
DBGsd("getBridgeName() aChannel->uid=" , aChannel->uid , " found=" , (!strcmp(aChannel->uid , ChannelUidBuffer))) ;

      if (!strcmp(aChannel->uid , ChannelUidBuffer))
        return aBridge ;
}
  }

#ifdef DEBUG_VB
DBGss("getBridgeName() '" , ChannelUidBuffer , "' not found" , "") ;
#endif

  return SentinelBridge ;
*/
  GList* prefsList ; GList* prefsIter ; char* prefKey ; GList* channelsList ;

  prepareChannelUid(aConv) ; thisBridgeNameBuffer[0] = '\0' ;
  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  prefsIter = g_list_first(prefsList) ;
  while (prefsIter)
  {
#ifdef DEBUG_VB
if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_BOOLEAN)
  DBGsds("getBridgeName() found bool prefKey='" , (char*)prefsIter->data , "' val='" , purple_prefs_get_bool((char*)prefsIter->data) , "'" , "") ;
else if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_STRING)
  DBGsss("getBridgeName() found string prefKey='" , (char*)prefsIter->data , "' val='" , purple_prefs_get_string((char*)prefsIter->data) , "'" , "") ;
else if (purple_prefs_get_type((char*)prefsIter->data) == PURPLE_PREF_STRING_LIST)
  DBGss("getBridgeName() found stored bridgeName='" , (strrchr((char*)prefsIter->data , '/') + 1 ) , "'" , "") ;
#endif

    prefKey = (char*)prefsIter->data ;
    if (isValidChannelsPref(prefKey))
    {
#ifdef DEBUG_VB
DBGs("getBridgeName() isValidChannelsPref() prefKey=" , prefKey) ;
#endif

      channelsList = purple_prefs_get_string_list(prefKey) ;
      if (g_list_find_custom(channelsList , ChannelUidBuffer , (GCompareFunc)isThisChanne))
      {
#ifdef DEBUG_VB
DBGs("getBridgeName() found channel=" , ChannelUidBuffer) ;
#endif

        strncpy(thisBridgeNameBuffer , strrchr(prefKey , '/') + 1 , SM_BUFFER_SIZE) ;
      }
#ifdef DEBUG_VB
else DBGs("getBridgeName() not found channel=" , ChannelUidBuffer) ;
#endif
    }
#ifdef DEBUG_VB
else DBGs("getBridgeName() not isValidChannelsPref() prefKey=" , prefKey) ;
#endif

    prefsIter = g_list_next(prefsIter) ;
  }
  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsList) ; g_list_free(prefsIter) ;
}

/*
Bridge* getBridgeByName(const char* bridgeName)
{
//return SentinelBridge ;

  Bridge* aBridge = SentinelBridge ; if (isBlank(bridgeName)) return SentinelBridge ;

  while ((aBridge = aBridge->next))
{
DBGssd("getBridgeByName() bridgeName=" , bridgeName , " aBridge->name=" , aBridge->name , " found=" , (!strcmp(aBridge->name , bridgeName))) ;

    if (!strcmp(aBridge->name , bridgeName))
      return aBridge ;
}
#ifdef DEBUG_VB
DBGsds("getBridgeByName() '" , bridgeName , "' not found - " , getNBridges() , " bridges exist" , "") ;
#endif

  return SentinelBridge ;
}
*/
unsigned int getNBridges()
{
/*
  Bridge* aBridge ; unsigned int n ;

  aBridge = SentinelBridge ; n = 0 ; while ((aBridge = aBridge->next)) ++n ;

DBGd("getNBridges() nBridges=" , n) ;

  return n ;
*/
  GList* prefsList ; unsigned int nBridges ;

  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  nBridges = g_list_length(prefsList) / 2 ;
  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsList) ;

  return nBridges ;
}

// unsigned int getNChannels(Bridge* aBridge)
unsigned int getNChannels(const char* bridgePrefKey)
{
/*
  Channel* aChannel ; unsigned int n ;

  aChannel = aBridge->sentinelChannel ; n = 0 ; while ((aChannel = aChannel->next)) ++n ;

if (aBridge) DBGd("getNChannels() nChannels=" , n) ; else DBG("getNChannels() bridge nfg") ;

  return n ;
*/
  GList* channelsList ; unsigned int nChannels ;

  channelsList = purple_prefs_get_string_list(bridgePrefKey) ;
  nChannels = g_list_length(channelsList) ;
  g_list_free(channelsList) ;

  return nChannels ;
}
/*
gboolean doesBridgeExist(Bridge* aBridge)
  { return (!!aBridge && aBridge != SentinelBridge) ; }
*/
gboolean doesBridgeExist(const char* bridgeName)
  { prepareBridgeKeys(bridgeName) ; return purple_prefs_exists(BridgeKeyBuffer) ; }

gboolean isBridgeEnabled(const char* bridgeName)
{
  prepareBridgeKeys(bridgeName) ;
  return purple_prefs_exists(EnabledKeyBuffer) &&
         purple_prefs_get_bool(EnabledKeyBuffer) ;
}

gboolean isChannelBridged(PurpleConversation* aConv)
{
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(aConv , thisBridgeName) ;
  return isBlank(thisBridgeName) ;
}

gboolean areReservedIds(char* bridgeName , char* channelUid , const char* channelName)
{
  return (!strcmp(bridgeName  , SENTINEL_NAME) ||
          !strcmp(channelUid  , SENTINEL_NAME) ||
          !strcmp(channelName , "NickServ")    ||
          !strcmp(channelName , "MemoServ")) ;
}

gboolean isValidChannelsPref(const char* bridgePrefKey)
  { return (purple_prefs_get_type(bridgePrefKey) == PURPLE_PREF_STRING_LIST) ; }

/*
Bridge* newBridge(char* bridgeName , Bridge* prevBridge , gboolean isEnabled)
{
  Bridge* newBridge ; Channel* newChannel ;

  newBridge  = (Bridge*) malloc(sizeof(Bridge)) ;  if (!newBridge)  return NULL ;
  newChannel = (Channel*)malloc(sizeof(Channel)) ; if (!newChannel) return NULL ;

  strcpy(newBridge->name     , bridgeName) ;
  newBridge->isEnabled       = isEnabled ;
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
*/
//gboolean createChannel(char* bridgeName , gboolean isStored)
void createChannel(char* bridgeName)
{
//  Bridge* aBridge ; Channel* aChannel ;
GList* channelsList = NULL ;

#ifdef DEBUG_VB
DBGss("createChannel() bridgeName='" , bridgeName , "'" , "") ;
#endif

  prepareBridgeKeys(bridgeName) ;

  // create bridge if necessary
//  if (!doesBridgeExist(aBridge = getBridgeByName(bridgeName)))
  if (!doesBridgeExist(bridgeName))
  {
/*
    // restore stored state
    gboolean isEnabled = (!isStored || purple_prefs_get_bool(EnabledKeyBuffer)) ;

    // create new bridge struct
    while (aBridge->next) aBridge = aBridge->next ;
    aBridge->next = newBridge(bridgeName , aBridge , isEnabled) ;
    if (!(aBridge = aBridge->next)) return FALSE ;

    // store bridge
    if (!isStored)
    {
*/
      purple_prefs_add_bool(EnabledKeyBuffer , TRUE) ;
      purple_prefs_add_string_list(BridgeKeyBuffer , channelsList) ;

DBGss("createChannel() added new bridgeKey='" , BridgeKeyBuffer , "'" , "") ;
//    }

//DBGss("createChannel() added new bridge struct='" , BridgeKeyBuffer , "'" , "") ;
  }
/*
  // create new channel struct
  aChannel = aBridge->sentinelChannel ;
  while (aChannel->next) aChannel = aChannel->next ;
  if (!(aChannel->next = newChannel(aChannel))) return FALSE ;

  // store new channel
  if (!isStored)
  {
*/
    channelsList = purple_prefs_get_string_list(BridgeKeyBuffer) ;
    channelsList = g_list_prepend(channelsList , (gpointer)ChannelUidBuffer) ;
    purple_prefs_set_string_list(BridgeKeyBuffer , channelsList) ;
    g_list_free(channelsList) ;

DBGss("createChannel() added new channelUid='" , ChannelUidBuffer , "'" , "") ;
//  }

//DBGss("createChannel() added new Channel struct='" , strrchr(ChannelUidBuffer , ':') , "'" , "") ;

//  return TRUE ;
}
//static int DBGN = 0 ;
//if (!DBGN) { ++DBGN ; //else return PURPLE_CMD_RET_OK ;
//void destroyChannel(Bridge* aBridge , PurpleConversation* aConv)
void destroyChannel(PurpleConversation* aConv)
{
//  Channel* aChannel ; GList* channelsList = NULL ; GList* channelsIter = NULL ;
  GList* channelsList = NULL ; GList* channelsIter = NULL ;
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(aConv , thisBridgeName) ;
/*
  // destroy channel struct
  aChannel = aBridge->sentinelChannel ; prepareChannelUid(aConv) ;
  while ((aChannel = aChannel->next))
    if (!strcmp(aChannel->uid , ChannelUidBuffer))
      { aChannel->prev->next = aChannel->next ; free(aChannel) ; break ; }

  // remove channel from store
  prepareBridgeKeys(aBridge->name) ;
*/
  prepareBridgeKeys(thisBridgeName) ; prepareChannelUid(aConv) ;

DBGd("destroyChannel() nChannels  IN=" , getNChannels(BridgeKeyBuffer)) ;
DBGd("destroyChannel() nBridges  IN=" , getNBridges()) ;

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
  g_list_free(channelsList) ; g_list_free(channelsIter) ;

DBGssss("destroyChannel() removed channel='" , getChannelName(aConv) , "' from bridge='" , thisBridgeName , ((getNChannels(BridgeKeyBuffer))? "" : "' also removing bridge='") , ((getNChannels(BridgeKeyBuffer))? "" : thisBridgeName) , "'" , "") ;
DBGd("destroyChannel() nChannels OUT=" , getNChannels(BridgeKeyBuffer)) ;

  // destroy empty bridge struct and storage
//  if (!getNChannels(aBridge))
  if (!getNChannels(BridgeKeyBuffer))
  {
//    aBridge->prev->next = aBridge->next ; free(aBridge) ;
    purple_prefs_remove(BridgeKeyBuffer) ; purple_prefs_remove(EnabledKeyBuffer) ;
  }

DBGd("destroyChannel() nBridges OUT=" , getNBridges()) ;
}

//void enableBridge(Bridge* aBridge , gboolean shouldEnable)
void enableBridge(char* bridgeName , gboolean shouldEnable)
{
//  if (aBridge->isEnabled == shouldEnable) return ;

//  aBridge->isEnabled = shouldEnable ;
//   prepareBridgeKeys(aBridge->name) ;
  prepareBridgeKeys(bridgeName) ;
  purple_prefs_set_bool(EnabledKeyBuffer , shouldEnable) ;
}

void enableBridgeEach(char* bridgePrefKey , gboolean* shouldEnable)
{
  if (isValidChannelsPref(bridgePrefKey))
    enableBridge(strrchr(bridgePrefKey , '/') + 1 , *shouldEnable) ;
}

/* event handlers */

void handlePluginInit(PurplePlugin* aPlugin)
  { ThisPlugin = aPlugin ; purple_prefs_add_none(BASE_PREF_KEY) ; }

gboolean handlePluginLoaded(PurplePlugin* aPlugin)
{
DBG("handlePluginLoaded()") ;

//  if (!restoreSession()) return FALSE ;

 registerCommands() ; registerCallbacks() ; chatBufferClear() ;

  return TRUE ;
}

gboolean handlePluginUnloaded(PurplePlugin* plugin)
{
DBG("handlePluginUnloaded()") ;

//  unregisterCommands() ; unregisterCallbacks() ; destroySession() ;
  unregisterCommands() ; unregisterCallbacks() ;

  return TRUE ;
}

gboolean handleChat(PurpleAccount* thisAccount , char** sender , char** msg ,
                    PurpleConversation* thisConv , PurpleMessageFlags* flags , void* data)
{
//  Bridge* thisBridge ;
char thisBridgeName[SM_BUFFER_SIZE] ;

#ifdef DEBUG_CHAT // NOTE: DBGchat() should mirror changes to logic here
if (thisConv) DBGchat(thisAccount , *sender , thisConv , *msg , *flags) ;
#endif

  if (!thisConv) return TRUE ; // supress rogue msgs (autojoined server msgs maybe unbound)

//  thisBridge = getBridgeName(thisConv) ;
  getBridgeName(thisConv , thisBridgeName) ;
  if (!isBridgeEnabled(thisBridgeName))          return FALSE ; // input channel bridge is disabled
  if (*flags & PURPLE_MESSAGE_SEND)    return FALSE ; // never relay unprefixed local chat
  if (!(*flags & PURPLE_MESSAGE_RECV)) return FALSE ; // TODO: handle special message types
//   if (!doesBridgeExist(thisBridge))    return FALSE ; // input channel is unbridged
//   if (!thisBridge->isEnabled)          return FALSE ; // input channel bridge is disabled

  prepareRelayChat(NICK_PREFIX , *sender , *msg) ;
//  relayMessage(thisBridge , thisConv) ; chatBufferClear() ;
  relayMessage(thisBridgeName , thisConv) ; chatBufferClear() ;

  return FALSE ;
}


/* admin command handlers */

PurpleCmdRet handleAddCmd(PurpleConversation* thisConv , const gchar* command ,
                          gchar** args , gchar** error , void* data)
{
//  char* bridgeName ; Bridge* thisBridge ;
  char thisBridgeName[SM_BUFFER_SIZE] ; char* thatBridgeName ;

DBGcmd(command , args[0]) ;

  thatBridgeName = (isBlank(args[0]))? DEFAULT_BRIDGE_NAME : args[0] ;
//  if (doesBridgeExist(thisBridge = getBridgeName(thisConv)))
  if (isChannelBridged(thisConv))
  {
//    if (thisBridge != getBridgeByName(bridgeName)) addConflictResp(thisConv) ;
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
//  Bridge* thisBridge ; char thisBridgeName[SM_BUFFER_SIZE] ;
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(thisConv , thisBridgeName) ;

DBGcmd(command , args[0]) ;
/*
  if (doesBridgeExist(thisBridge = getBridgeName(thisConv)))
  {
    strncpy(thisBridgeName , thisBridge->name , SM_BUFFER_SIZE) ;
    destroyChannel(thisBridge , thisConv) ; removeResp(thisConv , thisBridgeName) ;
  }
*/
  if (isBlank(thisBridgeName)) removeUnbridgedResp(thisConv) ;
  else { destroyChannel(thisConv) ; removeResp(thisConv , thisBridgeName) ; }

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleEnableCmd(PurpleConversation* thisConv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
//  gboolean shouldEnable ; char* bridgeName ; Bridge* aBridge ;
  gboolean shouldEnable ; char* bridgeName ; GList* prefsList ;

DBGcmd(command , args[0]) ;

  shouldEnable = !strcmp(command , ENABLE_CMD) ; bridgeName = args[0] ;
  if (!getNBridges()) enableNoneResp(thisConv , "") ;
  else if (isBlank(bridgeName))
  {
/*
    aBridge = SentinelBridge ; enableAllResp(thisConv , shouldEnable) ;
    while ((aBridge = aBridge->next))
    {
      enableBridge(aBridge , shouldEnable) ;
      enableResp(thisConv , aBridge->name , shouldEnable) ;
    }
*/
    enableAllResp(thisConv , shouldEnable) ;
    prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
    g_list_foreach(prefsList , (GFunc)enableBridgeEach , &shouldEnable) ;
    g_list_foreach(prefsList , (GFunc)enableRespEach , thisConv) ;
    g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
    g_list_free(prefsList) ;
  }
/*
  else if (doesBridgeExist(aBridge = getBridgeByName(bridgeName)))
  {
    enableBridge(aBridge , shouldEnable) ;
    enableResp(thisConv , aBridge->name , shouldEnable) ;
  }
*/
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
DBGcmd(command , args[0]) ;

  adminEcho(thisConv , *args) ; return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleChatCmd(PurpleConversation* thisConv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
//   Bridge* thisBridge ;
  char thisBridgeName[SM_BUFFER_SIZE] ; getBridgeName(thisConv , thisBridgeName) ;

DBGcmd(command , args[0]) ;

//   thisBridge = getBridgeName(thisConv) ;
//   if (doesBridgeExist(thisBridge)) adminChat(thisConv , *args , thisBridge) ;
  if (isChannelBridged(thisConv)) adminChat(thisConv , *args , thisBridgeName) ;
  else channelStateMsg(thisConv) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet handleBroadcastCmd(PurpleConversation* thisConv , const gchar* command ,
                                gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  broadcastResp(thisConv) ; adminBroadcast(thisConv , *args) ; return PURPLE_CMD_RET_OK ;
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

  helpResp(thisConv) ; return PURPLE_CMD_RET_OK ;
}

/* admin command responses */

void addResp(PurpleConversation* thisConv , char* thisBridgeName)
{
  chatBufferPutSS("%s '%s'\n\n" , CH_SET_MSG , thisBridgeName) ;
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
//   if (doesBridgeExist(getBridgeByName(thisBridgeName)))
  if (doesBridgeExist(thisBridgeName))
    { chatBufferCat("\n\n") ; bridgeStatsMsg(thisBridgeName) ; }
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
  char* bridgeName = strrchr(bridgePrefKey , '/') + 1 ;
  if (isValidChannelsPref(bridgePrefKey))
    enableResp(thisConv , bridgeName , isBridgeEnabled(bridgeName)) ;
}

void adminEcho(PurpleConversation* thisConv , char* msg)
{
  prepareRelayChat(NICK_PREFIX , getNick(thisConv) , msg) ; chatBufferDump(thisConv) ;
}

// void adminChat(PurpleConversation* inputConv , char* msg , Bridge* aBridge)
void adminChat(PurpleConversation* thisConv , char* msg , char* thisBridgeName)
{
  prepareRelayChat(NICK_PREFIX , getNick(thisConv) , msg) ;
//   relayMessage(aBridge , NULL) ; chatBufferClear() ;
  relayMessage(thisBridgeName , thisConv) ; chatBufferClear() ;
}

void adminBroadcast(PurpleConversation* thisConv , char* msg)
{
//  Bridge* aBridge = SentinelBridge ;
  GList* prefsList ;

  prepareRelayChat(BCAST_PREFIX , getNick(thisConv) , msg) ;
//  while ((aBridge = aBridge->next)) relayMessage(aBridge , NULL) ; chatBufferClear() ;
  prefsList = purple_prefs_get_children_names(BASE_PREF_KEY) ;
  g_list_foreach(prefsList , (GFunc)relayMessage , thisConv) ;
  g_list_foreach(prefsList , (GFunc)g_free , NULL) ;
  g_list_free(prefsList) ; chatBufferClear() ;
}

void broadcastResp(PurpleConversation* thisConv)
{
//   Bridge* aBridge ; unsigned int nChannels = 0 ;

//  aBridge = SentinelBridge ;
//   while ((aBridge = aBridge->next)) nChannels += getNRelayChannels(aBridge , NULL) ;

  GList* prefsList ;

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
//   Bridge* aBridge ; unsigned int nBridges = getNBridges() ;
  unsigned int nBridges = getNBridges() ; GList* prefsList ;

#ifdef DEBUG_VB
if (!nBridges) DBG("statusResp() no bridges") ; else if (isBlank(bridgeName)) DBG("statusResp() bridge unspecified - listing all") ; else DBGss("statusResp() bridgeName='" , bridgeName , "'" , "") ;
#endif

//   aBridge = SentinelBridge ; nBridges = getNBridges() ;
  nBridges = getNBridges() ;

  chatBufferPutSDS("%s %d %s" , STATS_MSGa , nBridges , STATS_MSGb) ;
  if (nBridges) chatBufferCat("\n\n") ; else { chatBufferDump(thisConv) ; return ; }

  if (!isBlank(bridgeName)) bridgeStatsMsg(bridgeName) ;
//   else while ((aBridge = aBridge->next))
//       { bridgeStatsMsg(aBridge->name) ; chatBufferCat("\n\n") ; }
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

//   Bridge* aBridge ;
char thisBridgeName[SM_BUFFER_SIZE] ;

#ifdef DEBUG_VB
DBGss("channelStateMsg() channelName='" , getChannelName(thisConv) , "'" , "") ;
#endif

  chatBufferCatSS(THIS_CHANNEL_MSG , " ") ;
//   if (doesBridgeExist(aBridge = getBridgeName(thisConv)))
//     chatBufferCatSSSS(THIS_BRIDGE_MSG , " '" , aBridge->name , "'") ;
  getBridgeName(thisConv , thisBridgeName) ;
  if (isBlank(thisBridgeName)) chatBufferCat(UNBRIDGED_MSG) ;
  else chatBufferCatSSSS(THIS_BRIDGE_MSG , " '" , thisBridgeName , "'") ;
}

void bridgeStatsMsg(const char* bridgePrefKey) // bridgePrefKey or bridgeName
{
// NOTE: callers of bridgeStatsMsg() should eventually call chatBufferDump()

//   Bridge* aBridge ; Channel* aChannel ; unsigned int nChannels ;
  const char* bridgeName ; unsigned int nChannels ;
  GList* channelsList ; GList* channelsIter ; char* aChannelUid ;
  GList* activeChannelsIter = NULL ; gboolean isActive = FALSE ;
  char* activeMsg ; char* protocol ; char* username ; const char* channelName ;
  char* network ; char nick[SM_BUFFER_SIZE] ;

  // ensure bridgePrefKey and bridgeName are properly set
// TODO: most calls to this function will have BridgeKeyBuffer set properly
//    so we could work it so that bridgeName is passed in consistantly
//    parsing bridgeName in nextBridgeStatsMsg() and calling prepareBridgeKeys() elsewhere
//    then pass BridgeKeyBuffer to isValidChannelsPref() and purple_prefs_get_string_list()
  bridgeName = strrchr(bridgePrefKey , '/') ;
  if (bridgeName) ++bridgeName ; // called with bridgePrefKey (e.g. via g_list_foreach())
  else                           // called with bridgeName (e.g. via chat command)
  {
DBG("bridgeStatsMsg() shouold not be here on forEach") ;

    bridgeName = bridgePrefKey ;
    prepareBridgeKeys(bridgeName) ; bridgePrefKey = BridgeKeyBuffer ;
  }

  // bail if we are called with an uninteresting key (e.g. via g_list_foreach())
  if (!isValidChannelsPref(bridgePrefKey)) return ;

//   if (!doesBridgeExist(aBridge = getBridgeByName(bridgeName)))
  if (!doesBridgeExist(bridgeName))
  {
    if (!getNBridges()) chatBufferCatSS(NO_BRIDGES_MSG , "") ;
    else chatBufferCatSSSS(NO_SUCH_BRIDGE_MSG , " '" , bridgeName , "'") ;
    return ;
  }
  else chatBufferCatSSSS(STATS_MSGc , " '" , bridgeName , "' - ") ;

//   nChannels = getNChannels(aBridge) ;
  nChannels = getNChannels(bridgePrefKey) ;
  if (!nChannels) chatBufferCat(STATS_DELETED_MSG) ;
  else
  {
//     if (aBridge->isEnabled) chatBufferCat(STATS_ENABLED_MSG) ;
    if (isBridgeEnabled(bridgeName)) chatBufferCat(STATS_ENABLED_MSG) ;
    else chatBufferCat(STATS_DISABLED_MSG) ;
    channelUidBufferPutD("%d" , nChannels) ;
    chatBufferCatSSSS(" - " , ChannelUidBuffer , " " , STATS_MSGd) ;
  }
/*
  aChannel = aBridge->sentinelChannel ;
  while ((aChannel = aChannel->next))
*/
  channelsList = purple_prefs_get_string_list(bridgePrefKey) ;
  channelsIter = g_list_first(channelsList) ;
  while (channelsIter)
  {
#ifdef DEBUG_VB
DBGss("bridgeStatsMsg() found stored channelUid='" , (char*)channelsIter->data , "'" , "") ;
#endif

    aChannelUid = (char*)channelsIter->data ;

    // determine if bridged aChannel is opened or closed
    activeChannelsIter = g_list_first(purple_get_conversations()) ;
    while (activeChannelsIter)
    {
#ifdef DEBUG_VB
if (channelsIter == g_list_first(channelsList))
  DBGss("bridgeStatsMsg() got active channelName='" , getChannelName((PurpleConversation*)activeChannelsIter->data) , "'" , "") ;
#endif

      prepareChannelUid((PurpleConversation*)activeChannelsIter->data) ;
      isActive |= !strcmp(ChannelUidBuffer , aChannelUid) ;
      activeChannelsIter = g_list_next(activeChannelsIter) ;
    }
    activeMsg = (isActive)? CH_ACTIVE_MSG : CH_INACTIVE_MSG ;

#ifdef DEBUG_VB
DBGss("bridgeStatsMsg() aChannel='" , aChannelUid , "' " , activeMsg) ;
#endif

    // parse channel data from aChannelUid
    channelUidBufferPutS("%s" , aChannelUid) ;
    if (!(protocol    = strtok(ChannelUidBuffer , UID_DELIMITER)) ||
        !(username    = strtok(NULL      , UID_DELIMITER)) ||
        !(channelName = strtok(NULL      , UID_DELIMITER)))
      continue ;

#ifdef DEBUG_VB
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
  { bridgeStatsMsg(bridgePrefKey) ; chatBufferCat("\n") ; }


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
#ifdef DEBUG_CHAT
DBGs("chatBufferDump() ChatBuffer=\n" , ChatBuffer) ;
#endif

  purple_conversation_write(thisConv , BRIDGIN_NICK , ChatBuffer ,
                            PURPLE_MESSAGE_SYSTEM , time(0)) ;
  chatBufferClear() ;
}

//void relayMessage(Bridge* outputBridge , PurpleConversation* inputConv)
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

#ifdef DEBUG_VB
DBGss("relayMessage() got active channelName='" , getChannelName(aConv) ,
      ((aConv == thisConv)? " isThisChannel - skipping" :
      ((!strcmp(aBridgeName , thisBridgeName))? "' isThisBridge - relaying" : "' notThisBridge - skipping" )) , "") ;
#endif

//    if (aConv != inputConv && getBridgeName(aConv) == outputBridge)
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

void alert(char* msg)
{
  purple_notify_message(ThisPlugin , PURPLE_NOTIFY_MSG_INFO , msg ,
                        PLUGIN_VERSION , NULL , NULL , NULL) ;
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

static char* Commands[N_UNIQ_CMDS] = COMMANDS ;
static unsigned int NRelayChannels = 0 ;


PURPLE_INIT_PLUGIN(PLUGIN_NAME , handlePluginInit , PluginInfo)
