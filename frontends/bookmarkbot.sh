#!/bin/bash
jid='sandr1x@jabber.rikt.ru'
tail -f fs/roster/$jid | (
while true;
	do read rcmd
	cmd=`sed 's/[0-9]* '$jid': //' <<< $rcmd`
	case $cmd in
		show) cat bookmarks;;
		rm) rm bookmarks;;
		*) echo $cmd >> bookmarks;;
	esac
done ) >> fs/roster/$jid

