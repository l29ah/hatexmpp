#!/bin/bash
CMD=$1
shift
commands/$CMD "$@" 2> /dev/null | perl -e '
use utf8; 
$i=0; 
$max=1500; 
while (<STDIN>) {
	if (($i+=100+length($_)) < $max) { 
		print "$_"; 
	} else { 
		print substr("$_", 0, ($i-$max)); print "\n"; exit;
	}
}'

