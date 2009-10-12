#!/bin/bash
#c='fs/roster/anime@conference.jabber.ru/__chat'
export ad='Hi! I am HateFloodBot! To flood some conference, type ^flood <conference>, to stop - ^stopflood.'
echo $c
echo -n "$ad" >> $c
sleep 1
tail -n0 -f $c | 
sed -nu '/[0-9]* .*: ^.*/ s/.*: ^\(.*\)/\1/p' | (
        while read i; do
                bash -c 'case $0 in 
			"bot") echo -n ${ad};;
			"flood") c=$1; echo Flood $c >> floodlog; ./mt.sh > /dev/null;;
			"stopflood") ./kill.sh;;
		esac' $i &
        done) \
>> $c
