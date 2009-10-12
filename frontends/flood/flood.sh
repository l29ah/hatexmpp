#!/bin/bash
# Usage: flood id rosterpathtoflood
mid=$1

mp=fs$mid
ff=tdplm.$((mid % 7))
sl=servers.list
#s=$(cat $sl | sed -ne "$((RANDOM % $(cat $sl | wc -l) + 1))p" | sed "s/\+//")

# Custom targets switch
target=jid
tfile=$2
{ grep -q '/__chat' <<< $2 && target=muc && echo -n > msg$mid; } || 
{ egrep -q '^#|^PM ' <<< $2 && target=juick && echo -n "$2 " > msg$mid; }

[[ $target == juick ]] && { tfile=juick@juick.com; }
echo Target $target

while true; do
	s=$(cat $sl | sed -ne "$((RANDOM % $(cat $sl | wc -l) + 1))p" | sed "s/\+//")


	# getting hatexmpp up
	fusermount -u $mp >/dev/null
	sleep 1
	mkdir -p $mp
	rm -rf $mp/*
	hatexmpp $mp/ >/dev/null
	sleep 1
	cp trollconfig/* $mp/config/
	echo -n $s > $mp/config/server
	[[ $target == muc ]] && { 
		nick=`./nickgen.sh`; 
		echo -n $nick > $mp/config/muc_default_nick; 
	}
	./nickgen.sh > $mp/config/username
	sleep 1
	mkdir $mp/roster
	sleep 4

	# for irc
	#grep -q '@irc' <<< $c && 
	#	c=$oldc.`sed -ne "$((RANDOM%$(wc -l < irc)+1))p" irc`
	#{ (for ((i=0; i<3; i++)); do
	#	mkdir -p "$mp/roster/$c" || exit
	#	sleep 2
	#        grep -q captcha "$mp/roster/$c/__chat" && (echo Captcha;
	#		url=`tail -n2 "$mp/roster/$c/__chat" | sed -ne 's/.*\(http.*[0-9]\).*/\1/p' | tail -n1`
	#		echo Url $url
	#        	(cd ~/projects/hatecaptcha; ./interface.sh $url && echo Captcha broken && break) || echo Captcha failed)
	#done); } || 

	[[ $target == muc ]] && {
		mkdir -p "$mp/roster/${2/__chat/}" || break;
		sleep 3; 
	}

	for ((i=0; i<10; i++)); do
		# spam!!111
		{
			cat msg$mid $ff | head -n 16 >> "$mp/roster/$tfile";
			#head -n $(($i*4)) | 
			#tail -n 4 
		} #2> /dev/null
		sleep 1
		[[ $target == muc  ]] && {
			nick=`echo -n $nick | sed 's/^\(.\)\(.*\)/\2\1/'`;
			echo -n $nick > "$mp/roster/${2/__chat/}/__nick"
		}
		[ -d "$mp/roster/$jid" ] || { echo Failure; break 1; }
		{ [[ $target == juick ]] && sleep 10; } || sleep 3;
	done
	echo Restart
done
