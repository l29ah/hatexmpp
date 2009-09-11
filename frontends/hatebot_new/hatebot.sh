#!/bin/bash
fsroot="fs"
while :; do
	hatexmpp $fsroot/
	cp .hatexmpp/* $fsroot/config
	mkdir -p $fsroot/roster/$1
	sleep 5
	tail -f -n1 "$fsroot/roster/$1/__chat" | perl hatebot.pl "`cat $fsroot/roster/$1/__nick`" > "$fsroot/roster/$1/__chat"
	fusermount -u $fsroot
done
