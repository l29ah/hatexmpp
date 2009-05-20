#!/bin/bash
fusermount -u fs && sleep 2
./hatexmpp fs/ -d &> log &
sleep 2
cp config/* fs/config/
sleep 1
mkdir fs/roster
sleep 5
for i in anime hatexmpp booboo
	do mkdir 'fs/roster/'$i'@conference.jabber.ru'
	sleep 3
done
disown %1
