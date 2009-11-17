#!/bin/bash
prefix='\^'
mynick="`cat config/muc_default_nick`"
export C="$1"
tail -n0 -f fs/roster/$1/__chat | 
perl -e '$|=1;' -ne 's/^[0-9]* jid .* nick (?!'"$mynick"').* body {'"$prefix"'(.*?)(\\n.*|)}/$1/ and print' | {
	while read i; do
		./cmdparse.sh "$i" &
	done; 
} >> fs/roster/$1/__chat
