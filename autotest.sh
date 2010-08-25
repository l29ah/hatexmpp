#!/bin/bash
#rm -rf test/fs
mkdir -p test/fs
fusermount -u test/fs
cd test
../hatexmpp fs
sleep 1;
cp -r config/* fs/config/
sleep 3;
cd fs
mkdir roster
#sleep 6;
#mkdir 'roster/hatexmpp@conference.jabber.ru'
#sleep 3;
#echo -n autotest >> roster/hatexmpp@conference.jabber.ru/__chat
#sleep 3;
#rmdir 'roster/hatexmpp@conference.jabber.ru'
#sleep 1;
#cd ..
#fusermount -u fs
#
