### <-- bridgin (with a pidgin) -->

&nbsp;&nbsp;&nbsp;&nbsp;a nifty purple plugin  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;to bridge multiple IM and chat sessions  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;across the various chat services supported by libpurple  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;(aim, facebook, googletalk, icq, identi.ca,  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;irc, twitter, and many others)  
  
this is a native purple plugin port of the [bridgin-php](https://github.com/bill-auger/bridgin-php) project  
ported to C in order to overcome the limitations of the original php/dbus script  
  
some new functionalities this allows:  
* supressing echo to public chats of admin commands and status reports
* better handling of special forms such as '/me' , smileys , etc ...
* issuing admin commands in the standard form ('/add' instead of '?/add')
* painfree installation and automatic loading with pidgin
* running on a nox box , windows , or mac
  
  
##build instructions for debian (ymmv)

cd into your build dir and load the pidgin sources and build deps

    apt-get source pidgin
    sudo apt-get build-dep pidgin

cd into ./pidgin-x.x.x/ and build pidgin and finch  
you do not need to install this build  
it is only used to simplify building the bridgin plugin  
if you already have pidgin , finch , or adium installed  
you can delete this entire directory after building the plugin  
if you want to use this newly compiled pidgin or finch  
you may need to remove some of the ./configure --disable* switches

    cd ./pidgin*
    ./configure --disable-gtkui --disable-vv --disable-meanwhile --disable-avahi --disable-dbus --disable-perl --disable-tk
    make

now go make some C(__) and when the build has completed  
copy the contents of this repo into ./libpurple/plugins  
cd into the plugins dir and run the install script

    cd ./libpurple/plugins/
    cp </path/to/bridgin/sources>/* .
    chmod a+x ./install.sh
    ./install.sh

if your $HOME environment variable is properly set  
the install script should reply with the following message  
and pidgin will launch automatically upon successful installation

    "compilation success - installing to YOUR_HOME_DIR/.purple/plugins/"

the bridgin plugin should now be available to pidgin in Tools->Plugins  
if it is not then use this command to check that the plugin was installed

    ls $HOME/.purple/plugins/bridgin.so

if there is no output then you will need to manually move the file 'bridgin.so'  
into YOUR_HOME_DIR/.purple/plugins/ or /usr/lib/purple-2/ then restart pidgin  
  
if you are running without X or you do not want pidgin to launch automatically  
use this comand to compile and install only

    ./install --nolaunch

if you do not want the plugin to be automatically installed into your home dir  
use this comand to compile only

    make bridgin.so
  
  
## build instructions for windows
follow [these instructions](https://test.developer.pidgin.im/wiki/BuildingWinPidgin) to build pidgin for windows  
then copy the contents of this repo into PIDGIN_SRC_DIR\libpurple\plugins  
cd into PIDGIN_SRC_DIR\libpurple\plugins then make and install with:

    make -f Makefile.mingw bridgin.dll
    copy bridgin.dll %APPDATA%\.purple\plugins
