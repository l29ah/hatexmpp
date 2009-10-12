#!/bin/bash
for i in `seq 1 50`; do
	./flood.sh $i "$*" &
	sleep 2
done
