#!/bin/bash
cfile='fs/roster/anime@conference.jabber.ru/__chat'
rm chatstream
mkfifo chatstream
tail -n0 -f "$cfile" | 
perl -e '$|=1; open(F,">>$ARGV[0]"); while(<STDIN>) {print $_; print F $_; }' -- animelog |
sed -nu '/[0-9]* .*: ^.*/ s/.*: ^\(.*\)/\1/p' | (
while read cmd lulz
	do (echo $cmd $lulz >> cmdlog 
	commands/$cmd $lulz 2> /dev/null | 
	perl -e '
use utf8;
$i = 0;
$max = 1000;
while (<STDIN>) {
	if (($i += 100 + length($_)) < $max) { 
		print "$_";
	} else { 
		print substr("$_", 0, ($i-$max));
		exit;
	}
}')&
done) \
>> "$cfile"

