
#include "debug.h"

static void DBGchat(char* convType , PurpleAccount* account , char* sender , PurpleConversation* conv , char* buffer , PurpleMessageFlags flags , void* data)
{
  char dbg[64] ;

  purple_debug_misc(PLUGIN_NAME ,
      "%s from %s\n\taccount = %d\n\tsender  = %s\n\tchannel = %s (%d)\n\tmessage = %s\n\tflags   = %d\n\tdata    = %d\n" ,
      convType , sender , (int)account , sender , ((conv != NULL) ? purple_conversation_get_name(conv) : "(null)") ,
      (int)conv , buffer , flags , (int)data) ;

  sprintf(dbg , ((flags & PURPLE_MESSAGE_SEND)? "%s - loopback - dropping" : "%s") , convType) ;
  SystemChat(conv , dbg) ;
}

static void DBGchannelClosed(PurpleConversation* conv)
  { purple_debug_misc(PLUGIN_NAME , "deleting-conversation (%s)\n" , purple_conversation_get_name(conv)) ; }

static void DBGcmd(const gchar* command , gchar* args)
  { purple_debug_misc(PLUGIN_ID , "HandleCmd '/%s' args = %s\n" , command , args) ; }
