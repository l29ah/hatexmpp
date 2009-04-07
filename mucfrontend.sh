#!/bin/bash
tail -f __chat & 
rlwrap -mc bash -c 'while true; do read i; echo -n $i >> __chat; done'
kill $!

