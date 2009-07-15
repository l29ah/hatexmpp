#!/bin/bash
fusermount -u fs
rm -rf fs/*
valgrind ./hatexmpp fs/ -d &> log &
sleep 5
cp config/* fs/config/
sleep 1
mkdir fs/roster
sleep 15
for i in anime hatexmpp booboo
	do mkdir 'fs/roster/'$i'@conference.jabber.ru'
	sleep 6
done
disown %1
