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


readonly PLUGIN_OBJ=./bridgin.so
readonly SUCCESS_MSG="compilation success"
readonly FAILURE_MSG="compilation failure"
readonly INSTALLING_MSG="installing to $HOME/.purple/plugins/"
readonly NO_HOME_MSG='your $HOME environment var is not properly set'" - copy the file '$PLUGIN_OBJ' to YOUR_HOME_DIR/.purple/plugins/ or /usr/lib/purple-2/"
readonly NOLAUNCH_SWITCH="--nolaunch"


make $PLUGIN_OBJ 1> /dev/null
if   [ ! -f "$PLUGIN_OBJ" ]
then echo $(tput setaf 1)$FAILURE_MSG ; exit ;
elif [ ! -d "$HOME" ]
then echo $(tput setaf 2)$SUCCESS_MSG" - "$NO_HOME_MSG ; exit ;
else echo $(tput setaf 2)$SUCCESS_MSG" - "$INSTALLING_MSG
fi

mv $PLUGIN_OBJ $HOME/.purple/plugins/
[ "$1" != "$NOLAUNCH_SWITCH" ] || exit

pkill pidgin
which pidgin &> /dev/null && pidgin || ../../pidgin/pidgin &
