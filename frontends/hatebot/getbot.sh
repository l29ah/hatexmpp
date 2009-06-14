#!/bin/bash
kill `cat bpid`
sudo killall -9 -u hatebot
for i in `sudo fuser fs 2>&1|sed -n 's/.*\/proc\/\(.*\)\/fd.*/\1/p'`
	do sudo kill -9 $i
done
sleep 1;
./getfup.sh
./hatebot.sh &
echo $! > bpid

