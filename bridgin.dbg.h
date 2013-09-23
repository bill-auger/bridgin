#define DEBUG
//#define DEBUG_VB
#define DEBUG_CHAT


#include "debug.h"

// arges for DBG*() functions are in pairs<string , var> (DBGd implies "n=" , n)
static void DBG(const char* s1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s\n" , s1) ; }

static void DBGs(const char* s1 , const char* s2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s\n" , s1 , s2) ; }

//static void DBGd(const char* s1 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d\n" , s1 , d1) ; }

static void DBGss(const char* s1 , const char* s2 , const char* s3 , const char* s4) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s\n" , s1 , s2 , s3 , s4) ; }
#ifdef DEBUG_VB
static void DBGsd(const char* s1 , const char* s2 , const char* s3 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d\n" , s1 , s2 , s3 , d1) ; }
#endif
//static void DBGdd(const char* s1 , int d1 , const char* s2 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d%s%d\n" , s1 , d1 , s2 , d2) ; }

static void DBGsss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6) ; }

//static void DBGsdd(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%d\n" , s1 , s2 , s3 , d1 , s4 , d2) ; }

//static void DBGddd(const char* s1 , int d1 , const char* s2 , int d2 , const char* s3 , int d3) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d%s%d%s%d\n" , s1 , d1 , s2 , d2 , s3 , d3) ; }

//static void DBGssss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , const char* s8) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , s8) ; }

//static void DBGsssd(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%d\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , d1) ; }

//static void DBGsdsd(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , const char* s5 , const char* s6 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%s%s%d\n" , s1 , s2 , s3 , d1 , s4 , s5 , s6 , d2) ; }
#ifdef DEBUG_VB
static void DBGsssss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , const char* s8 , const char* s9 , const char* s10) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , s8 , s9 , s10) ; }
#endif


static void DBGcmd(const char* command , char* args)
  { purple_debug_misc(PLUGIN_NAME , "HandleCmd '/%s' args = %s\n" , command , args) ; }


static void DBGchat(PurpleAccount* thisAccount , char* sender ,
                    PurpleConversation* thisConv , char* msg , PurpleMessageFlags flags)
{
  char* convType = ((purple_conversation_get_type(thisConv) == PURPLE_CONV_TYPE_IM)?
      RECEIVING_IM_SIGNAL : RECEIVING_CHAT_SIGNAL) ;
  Bridge* thisBridge        = getBridgeByChannel(thisConv) ;
  const char* channelName   = purple_conversation_get_name(thisConv) ;
  gboolean isLocal          = (flags & PURPLE_MESSAGE_SEND) ;
  gboolean isRemote         = (flags & PURPLE_MESSAGE_RECV) ;
  gboolean isUnbridged      = (thisBridge == SentinelBridge) ;
  char dbgBuffer[SM_BUFFER_SIZE] ; unsigned int nChannels ;

  if (!strcmp(channelName , "NickServ") || !strcmp(channelName , "MemoServ"))
  {
    purple_debug_misc(PLUGIN_NAME , "%s from %s - dropping\n" , convType , channelName) ;
    return ;
  }

  if (isLocal && !strcmp(msg , ChatBuffer))
  {
    DBGss(convType , " relay on channel '" , channelName , "' - ignoring") ;
    return ;
  }

  nChannels = getNRelayChannels(thisBridge , thisConv) ;
  snprintf(dbgBuffer , SM_BUFFER_SIZE , "%d %s" , nChannels , "channels") ;

  purple_debug_misc(PLUGIN_NAME ,
      "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%d%s%s%s\n" ,
      convType , ((isLocal)? " from admin " : " from user ") , sender ,
      "\n\taccount = " , getProtocol(thisAccount) , " as " , getUsername(thisAccount) ,
      "\n\tsender  = " , sender ,
      "\n\tchannel = " , ((!thisConv) ? "(null)" : channelName) ,
      "\n\tmessage = " , msg ,
      "\n\tflags   = " , flags ,
      "\n" , ((isLocal)?     "local message - dropping" :
             ((!isRemote)?   "special message - dropping" :
             ((isUnbridged)? "unbridged - dropping" : "relaying to "))) ,
             ((isRemote && !isUnbridged)? dbgBuffer : "")) ;
}
