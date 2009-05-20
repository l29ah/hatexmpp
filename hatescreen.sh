#!/bin/bash
tmprc="/tmp/.hatescreen"

cat > $tmprc << EOF
screen tail -f $1
split
focus
resize 5
screen bash -c 'while :; do read s; echo -n \$s > '$1'; done'
EOF

screen -t "$1" -c "$tmprc"
