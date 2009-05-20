#!/bin/bash
tmprc="/tmp/.hatescreen"
f=`basename $1`
cd `dirname $1`
cat > $tmprc << EOF
bind Q quit
screen bash -c 'tail -f $f | (while read d n m; do if dd=\$(date +%H:%M:%S -d @\$d); then echo -En "\$dd"; else echo -En "\$d"; fi; echo -en "\E[3\$((\$(echo \$n | sum | cut -c1) % 7 +1));40m"; echo -ne " \$n\E[37;40m"; echo -E " \$m";  done)'
split
focus
resize 5
screen rlwrap -c bash -c 'while read s; do clear; echo -nE "\$s" >> '$f'; done'
EOF

screen -t "$f" -c "$tmprc"
