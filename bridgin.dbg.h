
#include "debug.h"

// arges for DBG*() functions are in pairs<string , var> (DBGd implies "n=" , n)
static void DBG(const char* s1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s\n" , s1) ; }

static void DBGs(const char* s1 , const char* s2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s\n" , s1 , s2) ; }

static void DBGd(const char* s1 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d\n" , s1 , d1) ; }

static void DBGss(const char* s1 , const char* s2 , const char* s3 , const char* s4) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s\n" , s1 , s2 , s3 , s4) ; }

static void DBGsd(const char* s1 , const char* s2 , const char* s3 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d\n" , s1 , s2 , s3 , d1) ; }

static void DBGsss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6) ; }

static void DBGsdd(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%d\n" , s1 , s2 , s3 , d1 , s4 , d2) ; }

static void DBGsssd(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%d\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , d1) ; }

static void DBGchat(char* convType , PurpleAccount* anAccount , char* sender , PurpleConversation* aConv , char* msg , PurpleMessageFlags flags , void* data)
{
  // call these from somewhere just to make the compiler stop barking when unused
  DBG("") ; DBGs("" , "") ; DBGd("" , 0) ; DBGss("" , "" , "" , "") ; DBGsd("" , "" , "" , 0) ;
  DBGsss("" , "" , "" , "" , "" , "") ; DBGsdd("" , "" , "" , 0 , "" , 0) ;
  DBGsssd("" , "" , "" , "" , "" , "" , "" , 0) ;

  purple_debug_misc(PLUGIN_NAME ,
      "%s from %s\n\taccount = %d\n\tsender  = %s\n\tchannel = %s (%d)\n\tmessage = %s\n\tflags   = %d\n\tdata    = %d\n%s" ,
      convType , sender , (int)anAccount , sender , ((aConv != NULL) ? purple_conversation_get_name(aConv) : "(null)") ,
      (int)aConv , msg , flags , (int)data , ((flags & PURPLE_MESSAGE_SEND)? "loopback - dropping" : "")) ;
}

static void DBGchannelClosed(PurpleConversation* aConv)
  { purple_debug_misc(PLUGIN_NAME , "deleting-conversation (%s)\n" , purple_conversation_get_name(aConv)) ; }

static void DBGcmd(const char* command , char* args)
  { purple_debug_misc(PLUGIN_NAME , "HandleCmd '/%s' args = %s\n" , command , args) ; }
