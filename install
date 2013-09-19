#!/bin/bash

# This file is part of the Bridgin program
# Copyright 2013-2014 bill-auger <https://github.com/bill-auger/bridgin/issues>
#
# Bridgin is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Bridgin is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with Bridgin.  If not, see <http://www.gnu.org/licenses/>.


PLUGIN_OBJ=./bridgin.so
SUCCESS_MSG="compilation success"
FAILURE_MSG="compilation failure"
INSTALLING_MSG="installing to $HOME/.purple/plugins/"
NO_HOME_MSG='your $HOME environment var is not properly set'" - copy the file '$PLUGIN_OBJ' to YOUR_HOME_DIR/.purple/plugins/ or /usr/lib/purple-2/"
NOLAUNCH="nolaunch"
PIDGIN_BIN=`which pidgin`


make $PLUGIN_OBJ 1> /dev/null
if [ -f "$PLUGIN_OBJ" ]
then
  if [ -d "$HOME" ]
  then echo $(tput setaf 2)$SUCCESS_MSG" - "$INSTALLING_MSG
  else echo $(tput setaf 2)$SUCCESS_MSG" - "$NO_HOME_MSG ; exit
  fi
else echo $(tput setaf 1)$FAILURE_MSG ; exit
fi

mv $PLUGIN_OBJ $HOME/.purple/plugins/
if [ "$1" == $NOLAUNCH ] ; then exit ; fi ;

if ((`pidof pidgin`)) ; then kill `pidof pidgin` ; fi ;

if [ "$PIDGIN_BIN" == "" ] ; then PIDGIN_BIN=../../pidgin/pidgin ; fi ;
$PIDGIN_BIN &
