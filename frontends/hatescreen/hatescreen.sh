#!/bin/bash
# HateXMPP frontend
# Dependencies: 
#  - bash
#  - screen
#  - rlwrap
#  - Perl with Date::Format (gentoo: dev-perl/TimeDate, other distros: http://search.cpan.org/~gbarr/TimeDate-1.16/lib/Date/Format.pm);

[ -e "$1" ] || { echo "Usage: $0 <hatexmpp_chatlog>"; exit 1; }

tmprc="/tmp/.hatescreen"
name=`basename $1`
dir=`dirname $1`
mynick=`cat $dir/__nick`

# generate output filter on Perl
cat > $tmprc.pl << EOF
use Date::Format; 
while (<STDIN>) {
	if (/(\\d+) jid (.*?) nick (.*?) body {(.*)}\$/) {
		print time2str("%T", "\$1");
		(\$nick, \$msg) = (\$3, \$4);
		\$msg =~ s/$mynick/\\033[1m${mynick}\\033[0m/gi;
		\$col = unpack("%32C*", "\$nick")%6+1; 
		print "\e[3\${col}m \$nick\e[0m:"; 
		print " \$msg\n";
	}
	elsif (\$_ =~ /(\\d+) (.*)$/) {
		print time2str("%T", "\$1");
		print " \$2\n";
	}
}
EOF

cat > $tmprc << EOF
bind Q quit
screen bash -c 'tail -f -n70 $1 | perl $tmprc.pl '
split
focus
resize 5
screen rlwrap -c bash -c 'while read -re s; do clear; echo -nE "\$s" >> '$1'; done'
EOF

screen -t "$name" -c "$tmprc"
