#!/bin/bash
tmprc="/tmp/.hatescreen"

cat > $tmprc << EOF
bind Q quit
screen bash -c 'tail -f $1 | (while read d m; do echo -E \$(date +%H:%M:%S -d @\$d) "\$m"; done)'
split
focus
resize 5
screen rlwrap bash -c 'while read s; do clear; echo -nE "\$s" >> '$1'; done'
EOF

screen -t "$1" -c "$tmprc"
