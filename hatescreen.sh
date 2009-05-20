#!/bin/bash
tmprc=".hatescreen"

echo -e "\
screen tail -f '$1'\n\
split\n\
focus\n\
resize 5\n\
screen bash -c 'while :; do read s; echo \$s > '$1'; done'" > "$tmprc"

screen -t "$1" -c "$tmprc"
