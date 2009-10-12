#!/bin/bash
mp=look
echo Server ${s:=alpha-labs.net}
	fusermount -u $mp
	sleep 1
	mkdir -p $mp
	../hatexmpp $mp/ >/dev/null
	sleep 1
	cp trollconfig/* $mp/config/
	echo -n $s > $mp/config/server
	#nick=`cat nickname``uuidgen | sed "s/-//g; s/^[^a-z]*//"`
	#echo -n $nick > $mp/config/muc_default_nick
	#echo -n `uuidgen | sed "s/-//g; s/^[^a-z]*//"` > $mp/config/muc_default_nick
	echo -n `uuidgen | sed "s/-//g; s/^[^a-z]*//"` > $mp/config/username
	sleep 1
	mkdir $mp/roster
