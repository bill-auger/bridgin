### <-- bridgin (with a pidgin) -->

&nbsp;&nbsp;&nbsp;&nbsp;a nifty purple plugin that enables you  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;to bridge multiple IM and chat sessions  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;across the various services supported by libpurple  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(icq , yahoo , aim , msn , myspace , google talk ,  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;twitter , facebook , identi.ca , and many others)  
  
this is a native purple plugin port of the [bridgin-php](https://github.com/bill-auger/bridgin-php) project - once the initial port is completed  
further development will be able to overcome the limitations of the original php/dbus script  
  
some new functionalities this will allow:  
* supressing echo to public chats of admin commands and status reports
* better handling of special forms such as '/me' , smileys , etc ...
* issuing admin commands in the standard form ('/add' instead of '?/add')
* painfree installation and automatic loading with pidgin
* running on a nox box
* running on a windows box


##build instructions for debian (ymmv)

cd into your build dir and load sources and deps

    apt-get source pidgin
    sudo apt-get build-dep pidgin

cd into ./pidgin-x.x.x/ and build pidgin and finch  
if you already have pidgin or finch installed  
you can delete this build afterward  
it is needed only to simplify building the bridgin plugin

    ./configure
    make all

now go make some C(__) and when the build has completed  
copy the contents of this repo into ./libpurple/plugins  
cd into ./libpurple/plugins and run the install script

    chmod a+x ./install && ./install

if your $HOME environment variable is properly set  
the install script should reply with the following message  
and pidgin will launch automatically

    "compilation success - installing to YOUR_HOME_DIR/.purple/plugins/"

check that the install location mentioned points to inside you home dir  
and that the plugin was installed properly

    ls $HOME/.purple/plugins/bridgin.so

if there is no output then you will need to manually copy the file 'bridgin.so'
into YOUR_HOME_DIR/.purple/plugins/ or /usr/lib/purple-2/  
  
if you are running without X or you do not want pidgin to launch automatically  
use this comand to compile and install only

    ./install nolaunch

if you do not want the plugin to be automatically installed into your home dir  
use this comand to compile only

    make bridgin.so
  
  
## window build instructions
follow [these instructions](https://test.developer.pidgin.im/wiki/BuildingWinPidgin) to build pidgin for windows  
then copy the contents of this repo into PIDGIN_SRC_DIR\libpurple\plugins  
cd into PIDGIN_SRC_DIR\libpurple\plugins then make and install with:

    make -f Makefile.mingw bridgin.dll
    copy bridgin.dll %APPDATA%\.purple\plugins
