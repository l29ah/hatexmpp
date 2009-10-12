#!/bin/bash
mp=makelogin
#s=${s:-deshalbfrei.org}
s=$(grep + servers|sed -ne "$((RANDOM%$(grep + servers | wc -l)+1))p" | sed "s/\+//")
u=${u:-`uuidgen | sed "s/-//g; s/^[^a-z]*//"`}
fusermount -u $mp
sleep 1
mkdir -p $mp
hatexmpp $mp/ >/dev/null
sleep 1
cp trollconfig/* $mp/config/
echo -n $s > $mp/config/server
echo -n $u > $mp/config/username
sleep 1
mkdir $mp/roster
echo $u@$s
fusermount -u makelogin
