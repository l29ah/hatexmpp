#!/usr/bin/perl
use Storable;
use utf8;
use encoding 'utf8';
#binmode STDOUT, ":utf8";
%data = %{ retrieve('dict')} if (-e 'dict');
%defs = %{ retrieve('defs')} if (-e 'defs');
%top = %{ retrieve('top')} if (-e 'top');
do 'hatebot_functions.pl';
$|=1;
$mynick = $ARGV[0];
open(LOG, ">>log");
while (<STDIN>) {
	$line = $_;
	$line =~ /\d+ jid .* nick (.*) body {(.*)}/;
	$nick = $1;
	next if ($nick =~ /($mynick|\(null\)|Twilight)/ );
	print LOG "$nick: $2\n";
	$top{$nick}{words} += split /\W+/, $2;
	$top{$nick}{shit} ++ while $2 =~ /г.вн./gi;
	if ($line =~ /\d+ jid .* nick (.*) body {\#(.*?) (.*)}/) {
		($nick, $cmd, $param) = ($1, $2, $3);
		chomp($cmd);
		chomp($param);
		$param =~ s/\\n/\n/g;
		$cmd =~ s/(^\s*|\s*$)//g;
		print "cmd = '$cmd'\tparam = $param\tfunc_ref = ".$functions{$cmd};
		if ($cmd =~ /ls/) {
			print "Commands: ".join ", ", keys %functions;
		}
		if (exists $functions{$cmd}) {
			$out = &{$functions{$cmd}}($param);
			print "$nick: $out";
		}
	}
	if ($line =~ /\d+ jid .* nick (.*) body {$mynick..(.*)}/) {
		$out = hsulci($2);
		print "$1: $out" if ($out && ($1 !~ /Twilight/));
	}
}
store \%top, 'top';
sub hsulci($) {
	$param = shift;
	$w1 = randfrom(split / /, $param);
	$w1 = randfrom(keys %data) if (! exists $data{$w1} );
	$w2 = randfrom(keys %{$data{$w1}});
	$line = "$w1 $w2";
	$c = 0;
	while ($line !~ /\.$/ && $c++ < 100) {
		$w3 = randfrom(@{$data{$w1}{$w2}});
		$line .= " $w3";
		$w1 = $w2;
		$w2 = $w3;
	}
	chomp($line);
	$line = ucfirst($line);
	$line =~ s/(^[\s ]*|[\s ]*$)//g;
	$line =~ s/([^\.!\?,])$/\1./;
	return $line;
}

sub randfrom(@) {
	return $_[rand($#_+1)];
}
