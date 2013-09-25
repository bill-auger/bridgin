#define DEBUG       1
#define DEBUG_EVS   DEBUG && 1
#define DEBUG_LOGIC DEBUG && 1
#define DEBUG_CHAT  DEBUG && 1
#define DEBUG_VB    DEBUG && 0


#include "debug.h"


// NOTE: DBG*() functions args are in pairs <string,var> - e.g. DBGd("n=" , 42) ;
static void DBG(const char* s1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s\n" , s1) ; }
#if DEBUG_CHAT
static void DBGs(const char* s1 , const char* s2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s\n" , s1 , s2) ; }
#endif
//static void DBGd(const char* s1 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d\n" , s1 , d1) ; }
#if DEBUG_LOGIC
static void DBGss(const char* s1 , const char* s2 , const char* s3 , const char* s4) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s\n" , s1 , s2 , s3 , s4) ; }
#endif
//static void DBGsd(const char* s1 , const char* s2 , const char* s3 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d\n" , s1 , s2 , s3 , d1) ; }

//static void DBGdd(const char* s1 , int d1 , const char* s2 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d%s%d\n" , s1 , d1 , s2 , d2) ; }

static void DBGsss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6) ; }
#if DEBUG_VB
static void DBGsds(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , const char* s5) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%s\n" , s1 , s2 , s3 , d1 , s4 , s5) ; }
#endif
//static void DBGssd(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%d\n" , s1 , s2 , s3 , s4 , s5 , d1) ; }

//static void DBGsdd(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%d\n" , s1 , s2 , s3 , d1 , s4 , d2) ; }

//static void DBGddd(const char* s1 , int d1 , const char* s2 , int d2 , const char* s3 , int d3) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d%s%d%s%d\n" , s1 , d1 , s2 , d2 , s3 , d3) ; }

static void DBGssss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , const char* s8) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , s8) ; }

//static void DBGsssd(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%d\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , d1) ; }

//static void DBGsdsd(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , const char* s5 , const char* s6 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%s%s%d\n" , s1 , s2 , s3 , d1 , s4 , s5 , s6 , d2) ; }
#if DEBUG_VB
static void DBGsssss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , const char* s8 , const char* s9 , const char* s10) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , s8 , s9 , s10) ; }
#endif

#if DEBUG_EVS
static void DBGcmd(const char* command , char* args)
  { purple_debug_misc(PLUGIN_NAME , "handle*Cmd() cmd='/%s' args='%s'\n" , command , args) ; }
#endif

#if DEBUG_CHAT
static void DBGchat(PurpleAccount* thisAccount , char* sender , PurpleConversation* thisConv ,
                    char* msg , PurpleMessageFlags flags , char* thisBridgeName)
{
  char* convType = ((purple_conversation_get_type(thisConv) == PURPLE_CONV_TYPE_IM)?
      RECEIVING_IM_SIGNAL : RECEIVING_CHAT_SIGNAL) ;
  const char* channelName   = purple_conversation_get_name(thisConv) ;
  gboolean isLocal          = (flags & PURPLE_MESSAGE_SEND) ;
  gboolean isRemote         = (flags & PURPLE_MESSAGE_RECV) ;
  gboolean isBridged        = isChannelBridged(thisConv) ;
  gboolean isEnabled        = isBridgeEnabled(thisBridgeName) ;
  char dbgBuffer[SM_BUFFER_SIZE] ;

  // server msgs
  if (!strcmp(channelName , "NickServ") || !strcmp(channelName , "MemoServ"))
  {
    purple_debug_misc(PLUGIN_NAME , "%s from %s - dropping\n" , convType , channelName) ;
    return ;
  }

  // relay echos
  if (isLocal && !strcmp(msg , ChatBuffer))
  {
    DBGss(convType , " relay echo on channel '" , channelName , "' - ignoring") ;
    return ;
  }

  // count channels
  prepareBridgeKeys(thisBridgeName) ; NRelayChannels = 0 ;
  getNRelayChannels(BridgeKeyBuffer , thisConv) ;
  snprintf(dbgBuffer , SM_BUFFER_SIZE ,
           "%d channels on bridge '%s'" , NRelayChannels , thisBridgeName) ;

  // out
  purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s\n" ,
      convType , ((isLocal)? " from admin " : " from user ") , sender ,
      "\n\taccount = " , getProtocol(thisAccount) , " as " , getUsername(thisAccount) ,
      "\n\tsender  = " , sender ,
      "\n\tchannel = " , ((!thisConv) ? "(null)" : channelName) ,
      "\n\tmessage = " , msg ,
      "\n\tflags   = " , flags ,
      "\n" , ((isLocal)?    "local message - dropping" :
             ((!isRemote)?  "special message - dropping" :
             ((!isBridged)? "unbridged - dropping" :
             ((!isEnabled)? "bridge disabled - dropping" : "relaying to ")))) ,
             ((!isRemote || !isBridged || !isEnabled)? "" : dbgBuffer)) ;
}
#endif
