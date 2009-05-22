#!/bin/bash
rsstail -n 1 -u http://kernel.org/kdist/rss.xml | 
sed -u 's/Title: //' |
(while true
	do read i
	echo -n 'Вышел новый Linux:' $i >> 'fs/roster/anime@conference.jabber.ru/__chat'
done)
