#!/bin/bash
tmprc="/tmp/.hatescreen"
f=`basename $1`
cd `dirname $1`
cat > $tmprc << EOF
bind Q quit
screen bash -c 'tail -f $f | (while read d m; do echo -E \$(date +%H:%M:%S -d @\$d) "\$m"; done)'
split
focus
resize 5
screen rlwrap -c bash -c 'while read s; do clear; echo -nE "\$s" >> '$f'; done'
EOF

screen -t "$f" -c "$tmprc"
