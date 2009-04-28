#!/bin/bash
tail -f "$1" & 
rlwrap -mc bash -c 'while true; do read i; echo -n $i >> '"$1"'; done'
#kill $!

