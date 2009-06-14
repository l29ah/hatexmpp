#!/bin/bash
while true
	do while [ -d /proc/`cat bpid` ]
		do sleep 20;
	done
	./getbot.sh
done
