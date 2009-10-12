#!/bin/bash
kill `ps ax | egrep 'flood.sh|mt.sh' | awk {'print $1'}`
sleep 2
for i in fs*; do fusermount -u $i; done
