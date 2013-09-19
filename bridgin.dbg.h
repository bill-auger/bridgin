/*\
|*| This file is part of the Bridgin program
|*| Copyright 2013-2014 bill-auger <https://github.com/bill-auger/bridgin/issues>
|*|
|*| Bridgin is free software: you can redistribute it and/or modify
|*| it under the terms of the GNU Affero General Public License as published by
|*| the Free Software Foundation, either version 3 of the License, or
|*| (at your option) any later version.
|*|
|*| Bridgin is distributed in the hope that it will be useful,
|*| but WITHOUT ANY WARRANTY; without even the implied warranty of
|*| MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
|*| GNU Affero General Public License for more details.
|*|
|*| You should have received a copy of the GNU Affero General Public License
|*| along with Bridgin.  If not, see <http://www.gnu.org/licenses/>.
\*/


#include "debug.h"

// arges for DBG*() functions are in pairs<string , var> (DBGd implies "n=" , n)
static void DBG(const char* s1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s\n" , s1) ; }

static void DBGs(const char* s1 , const char* s2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s\n" , s1 , s2) ; }

//static void DBGd(const char* s1 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d\n" , s1 , d1) ; }

static void DBGss(const char* s1 , const char* s2 , const char* s3 , const char* s4) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s\n" , s1 , s2 , s3 , s4) ; }

static void DBGsd(const char* s1 , const char* s2 , const char* s3 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d\n" , s1 , s2 , s3 , d1) ; }

//static void DBGdd(const char* s1 , int d1 , const char* s2 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d%s%d\n" , s1 , d1 , s2 , d2) ; }

//static void DBGsss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6) ; }

//static void DBGsdd(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%d\n" , s1 , s2 , s3 , d1 , s4 , d2) ; }

//static void DBGddd(const char* s1 , int d1 , const char* s2 , int d2 , const char* s3 , int d3) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%d%s%d%s%d\n" , s1 , d1 , s2 , d2 , s3 , d3) ; }

//static void DBGssss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , const char* s8) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , s8) ; }

//static void DBGsssd(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , int d1) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%d\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , d1) ; }

//static void DBGsdsd(const char* s1 , const char* s2 , const char* s3 , int d1 , const char* s4 , const char* s5 , const char* s6 , int d2) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%d%s%s%s%d\n" , s1 , s2 , s3 , d1 , s4 , s5 , s6 , d2) ; }

static void DBGsssss(const char* s1 , const char* s2 , const char* s3 , const char* s4 , const char* s5 , const char* s6 , const char* s7 , const char* s8 , const char* s9 , const char* s10) { if (isBlank(s1)) return ; purple_debug_misc(PLUGIN_NAME , "%s%s%s%s%s%s%s%s%s%s\n" , s1 , s2 , s3 , s4 , s5 , s6 , s7 , s8 , s9 , s10) ; }

static void DBGchat(char* convType , PurpleAccount* thisAccount , char* sender ,
                    PurpleConversation* thisConv , char* msg , PurpleMessageFlags flags)
{
  Bridge* thisBridge        = getBridgeByChannel(thisConv) ;
  const char* channelName   = purple_conversation_get_name(thisConv) ;
  gboolean isLocal          = (flags & PURPLE_MESSAGE_SEND) ;
  gboolean isRemote         = (flags & PURPLE_MESSAGE_RECV) ;
  gboolean isUnbridged      = (thisBridge == SentinelBridge) ;
  GList* activeChannelsIter = g_list_first(purple_get_conversations()) ;
  unsigned int nChannels    = 0 ; PurpleConversation* aConv ;
  while (activeChannelsIter)
  {
    aConv = (PurpleConversation*)activeChannelsIter->data ;
    if (aConv != thisConv && getBridgeByChannel(aConv) == thisBridge)
      ++nChannels ;

    activeChannelsIter = g_list_next(activeChannelsIter) ;
  }
  statusBufferPutDS("%d %s" , nChannels , "channels") ;

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
             ((isRemote && !isUnbridged)? StatusBuffer : "")) ;
}

// static void DBGchannelClosed(PurpleConversation* thisConv)
//   { purple_debug_misc(PLUGIN_NAME , "deleting-conversation (%s)\n" , purple_conversation_get_name(thisConv)) ; }

static void DBGcmd(const char* command , char* args)
  { purple_debug_misc(PLUGIN_NAME , "HandleCmd '/%s' args = %s\n" , command , args) ; }
