#!/bin/bash
export C=anime-talks@conference.jabber.ru
kill `cat bpid`
sudo killall -9 -u hatebot
for i in `sudo fuser fs 2>&1|sed -n 's/.*\/proc\/\(.*\)\/fd.*/\1/p'`
	do sudo kill -9 $i
done
sleep 1;
./getfup.sh
for i in $C; do 
	./cmdlex.sh "$i" &
	{
		tail -n0 -f fs/roster/$i/__chat |
		perl -ne 's/^([0-9]*) jid .* nick (.*) body {(.*)}/$1 $2: $3/ and print' >> "$i-log" 
	} &
done
echo $! > bpid

