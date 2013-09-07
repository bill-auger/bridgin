
#include "bridgin.h"


/* helpers */

void systemChat(PurpleConversation* conv , const char* msg)
{
  purple_conversation_write(conv , PLUGIN_NAME , msg , PURPLE_MESSAGE_SYSTEM , time(0)) ;
}


/* event handlers */

gboolean plugin_load(PurplePlugin *plugin)
{
//  purple_notify_message(plugin , PURPLE_NOTIFY_MSG_INFO , "Hello Pidgin!" , PLUGIN_VERSION , NULL , NULL , NULL) ;

  purple_signal_connect(purple_conversations_get_handle() , "received-im-msg" ,
    plugin , PURPLE_CALLBACK(received_im_msg_cb) , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , "received-chat-msg" ,
    plugin , PURPLE_CALLBACK(received_chat_msg_cb) , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , "deleting-conversation" ,
    plugin , PURPLE_CALLBACK(deleting_conversation_cb) , NULL) ;

  return TRUE ;
}

void init_plugin(PurplePlugin *plugin)
{
}

void received_im_msg_cb(PurpleAccount* account , char* sender , char* buffer ,
    PurpleConversation* conv , PurpleMessageFlags flags , void* data)
{
purple_debug_misc(PLUGIN_NAME , "received-im-msg from %s\n\taccount = %d\n\tsender  = %s\n\tchannel = %s (%d)\n\tmessage = %s\n\tflags   = %d\n\tdata    = %d\n" , sender , (int)account , sender , (conv != NULL) ? purple_conversation_get_name(conv) : "(null)" , (int)conv , buffer , flags , (int)data) ;
}

void received_chat_msg_cb(PurpleAccount* account , char* sender , char* buffer ,
    PurpleConversation* conv , PurpleMessageFlags flags , void* data)
{
purple_debug_misc(PLUGIN_NAME , "received-chat-msg from %s\n\taccount = %d\n\tsender  = %s\n\tchannel = %s (%d)\n\tmessage = %s\n\tflags   = %d\n\tdata    = %d\n" , sender , (int)account , sender , (conv != NULL) ? purple_conversation_get_name(conv) : "(null)" , (int)conv , buffer , flags , (int)data) ;
if (flags & PURPLE_MESSAGE_SEND) systemChat(conv , "SYSTEM: received-chat-msg - echo dropping") ; else systemChat(conv , "SYSTEM: received-chat-msg") ;

  if (flags & PURPLE_MESSAGE_SEND) return ;

  purple_conv_chat_send(PURPLE_CONV_CHAT(conv) , "echo") ;
}

void deleting_conversation_cb(PurpleConversation *conv, void *data)
{
purple_debug_misc(PLUGIN_NAME , "deleting-conversation (%s)\n" , purple_conversation_get_name(conv)) ;
}


/* main */

static PurplePluginInfo info =
{
  PURPLE_PLUGIN_MAGIC , PURPLE_MAJOR_VERSION , PURPLE_MINOR_VERSION ,
  PLUGIN_TYPE , PLUGIN_GUI_TYPE , 0 , NULL , PURPLE_PRIORITY_DEFAULT ,
  PLUGIN_ID , PLUGIN_NAME , PLUGIN_VERSION , PLUGIN_SHORT_DESC , PLUGIN_LONG_DESC ,
  PLUGIN_AUTHOR , PLUGIN_WEBSITE ,
  PLUGIN_ONLOAD_CB , PLUGIN_ONUNLOAD_CB , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL , NULL
} ;

PURPLE_INIT_PLUGIN(bridgin , init_plugin , info)
