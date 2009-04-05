#!/bin/bash
mkdir -p test/fs
fusermount -u test/fs/
cp hatexmpp test
cd test
valgrind ./hatexmpp fs
sleep 6;
cd fs
mkdir 'roster/hatexmpp@conference.jabber.ru'
sleep 1;
echo -n autotest >> roster/hatexmpp@conference.jabber.ru/__chat
sleep 1;
rmdir 'roster/hatexmpp@conference.jabber.ru'
sleep 1;
cd ..
fusermount -u fs

