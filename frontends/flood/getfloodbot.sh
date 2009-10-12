#!/bin/bash
export c=anime-talks@conference.jabber.ru
mp=fs
echo Server ${s:=jabber.ru}
fusermount -u $mp
sleep 1
mkdir -p $mp
../hatexmpp $mp/ >/dev/null
sleep 1
cp trollconfig/* $mp/config/
echo -n $s > $mp/config/server
s=x23.eu
nick=HateFloodBot
echo -n $nick > $mp/config/muc_default_nick
echo -n bugmenot > $mp/config/username
echo -n bugmenot > $mp/config/password
rm $mp/config/register
sleep 1
mkdir $mp/roster
sleep 3
mkdir "$mp/roster/$c"
./hatefloodbot.sh
