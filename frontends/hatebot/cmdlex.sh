#!/bin/bash
tail -n0 -f fs/roster/anime@conference.jabber.ru/__chat | 
sed -nu '/[0-9]* .*: ^.*/ s/.*: ^\(.*\)/\1/p' | (
	while true
		do read i
		./cmdparse.sh $i &
	done) \
>> fs/roster/anime@conference.jabber.ru/__chat
