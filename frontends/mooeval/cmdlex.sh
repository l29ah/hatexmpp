#!/bin/bash
tail -n0 -f fs/roster/$1/__chat | 
#tee -a animelog | 
sed -nu '/[0-9]* .*: <.*/ s/.*: <\(.*\)/\1/p' | {
	while read i; do
		mueval --expression "$i" | tail | tr -d "\n" >> fs/roster/$1/__chat
	done; }
