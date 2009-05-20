#!/bin/bash
# HateXMPP frontend
# Dependencies: 
#  - bash
#  - screen
#  - rlwrap
#  - Perl with Date::Format (gentoo: dev-perl/TimeDate, other distros: http://search.cpan.org/~gbarr/TimeDate-1.16/lib/Date/Format.pm);

tmprc="/tmp/.hatescreen"
f=`basename $1`
cd `dirname $1`
mynick=`cat __nick`

# generate output filter on Perl
cat > $tmprc.pl << EOF
use Date::Format; 
while (<STDIN>) {
	if (\$_ =~ /(.+) (.*?): (.*$)/) {
		
		\$date=time2str("%T", "\$1"); 
		if (\$date != "") { print \$date; } 
		else { print \$1; } 

		\$msg = \$3;
#		\$msg =~ s/$mynick/\\033[1m${mynick}\\033[0m/gi;
		\$col = unpack("%32C*", "\$2")%7+1; 
		print "\e[3\${col}m \$2\e[0m"; 
		print ": \$msg\n";
	}
}
EOF

cat > $tmprc << EOF
bind Q quit
screen bash -c 'tail -f $f | perl $tmprc.pl'
split
focus
resize 5
screen rlwrap -c bash -c 'while read -re s; do clear; echo -nE "\$s" >> '$f'; done'
EOF
 
screen -t "$f" -c "$tmprc"
