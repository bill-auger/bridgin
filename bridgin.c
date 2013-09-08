
#include "bridgin.h"
#include "bridgin.dbg.h"

/* helpers */

PurpleCmdId RegisterCmd(const char* command , const char* format ,
                        PurpleCmdRet (* callback)() , const char* help)
{
  return purple_cmd_register(command , format , PURPLE_CMD_P_DEFAULT ,
      PURPLE_CMD_FLAG_IM | PURPLE_CMD_FLAG_CHAT , PLUGIN_ID , callback , help , NULL) ;
}

void SystemChat(PurpleConversation* conv , const char* msg)
{
  purple_conversation_write(conv , BRIDGIN_NICK , msg , PURPLE_MESSAGE_SYSTEM , time(0)) ;
}


/* event handlers */

void HandlePluginInit(PurplePlugin *plugin) {}

gboolean HandlePluginLoaded(PurplePlugin *aPlugin)
{
//  purple_notify_message(aPlugin , PURPLE_NOTIFY_MSG_INFO , "Hello Pidgin!" , PLUGIN_VERSION , NULL , NULL , NULL) ;

  purple_signal_connect(purple_conversations_get_handle() , "received-im-msg" ,
                        aPlugin , PURPLE_CALLBACK(HandleIm)            , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , "received-chat-msg" ,
                        aPlugin , PURPLE_CALLBACK(HandleChat)          , NULL) ;
  purple_signal_connect(purple_conversations_get_handle() , "deleting-conversation" ,
                        aPlugin , PURPLE_CALLBACK(HandleChannelClosed) , NULL) ;

  CommandIds[0]  = RegisterCmd(ADD_CMD     , UNARY_FMT  , ADD_CB    , ADDu_HELP) ;
  CommandIds[1]  = RegisterCmd(ADD_CMD     , BINARY_FMT , ADD_CB    , ADDb_HELP) ;
  CommandIds[2]  = RegisterCmd(REMOVE_CMD  , UNARY_FMT  , REMOVE_CB , REMOVE_HELP) ;
  CommandIds[3]  = RegisterCmd(DISABLE_CMD , UNARY_FMT  , ENABLE_CB , DISABLEu_HELP) ;
  CommandIds[4]  = RegisterCmd(DISABLE_CMD , BINARY_FMT , ENABLE_CB , DISABLEb_HELP) ;
  CommandIds[5]  = RegisterCmd(ENABLE_CMD  , UNARY_FMT  , ENABLE_CB , ENABLEu_HELP) ;
  CommandIds[6]  = RegisterCmd(ENABLE_CMD  , BINARY_FMT , ENABLE_CB , ENABLEb_HELP) ;
  CommandIds[7]  = RegisterCmd(ECHO_CMD    , BINARY_FMT , ECHO_CB   , ECHO_HELP) ;
  CommandIds[8]  = RegisterCmd(CHAT_CMD    , BINARY_FMT , CHAT_CB   , CHAT_HELP) ;
  CommandIds[9]  = RegisterCmd(BCAST_CMD   , BINARY_FMT , BCAST_CB  , BCAST_HELP) ;
  CommandIds[10] = RegisterCmd(STATUS_CMD  , UNARY_FMT  , STATUS_CB , STATUSu_HELP) ;
  CommandIds[11] = RegisterCmd(STATUS_CMD  , BINARY_FMT , STATUS_CB , STATUSb_HELP) ;
  CommandIds[12] = RegisterCmd(HELP_CMD    , UNARY_FMT  , HELP_CB   , HELP_HELP) ;

  return TRUE ;
}

gboolean HandlePluginUnloaded(PurplePlugin *plugin)
{
  int i ; for (i = 0 ; i < N_COMMANDS ; ++i) purple_cmd_unregister(CommandIds[i]) ;

  return TRUE ;
}

void HandleIm(PurpleAccount* account , char* sender , char* buffer ,
              PurpleConversation* conv , PurpleMessageFlags flags , void* data)
{
DBGchat("received-chat-msg" , account , sender , conv , buffer , flags , data) ;

//  purple_conv_chat_send(PURPLE_CONV_CHAT(conv) , formatMessage()) ;
}

void HandleChat(PurpleAccount* account , char* sender , char* buffer ,
                PurpleConversation* conv , PurpleMessageFlags flags , void* data)
{
DBGchat("received-chat-msg" , account , sender , conv , buffer , flags , data) ;

  if (flags & PURPLE_MESSAGE_SEND) return ;

//  purple_conv_chat_send(PURPLE_CONV_CHAT(conv) , formatMessage()) ;
}

void HandleChannelClosed(PurpleConversation* conv , void* data)
{
DBGchannelClosed(conv) ;

}


/* callbacks */

PurpleCmdRet HandleAddCmd(PurpleConversation* conv , const gchar* command ,
                          gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet HandleRemoveCmd(PurpleConversation* conv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet HandleEnableCmd(PurpleConversation* conv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet HandleEchoCmd(PurpleConversation* conv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet HandleChatCmd(PurpleConversation* conv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet HandleBroadcastCmd(PurpleConversation* conv , const gchar* command ,
                                gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet HandleStatusCmd(PurpleConversation* conv , const gchar* command ,
                             gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
}

PurpleCmdRet HandleHelpCmd(PurpleConversation* conv , const gchar* command ,
                           gchar** args , gchar** error , void* data)
{
DBGcmd(command , args[0]) ;

  return PURPLE_CMD_RET_OK ;
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

PURPLE_INIT_PLUGIN(PLUGIN_NAME , HandlePluginInit , PluginInfo)
