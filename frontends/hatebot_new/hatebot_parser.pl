#!/usr/bin/perl
use Storable;
use utf8;
use encoding utf8;
%data = %{ retrieve('dict')} if (-e 'dict');
print "Dict loaded\n";
while (<STDIN>) {
	@words = split(/ /, clean_string($_));
	$w1 = $words[0];
	$w2 = $words[1];
	for ($i=2; $i<=$#words; $i++) {
		$w3 = $words[$i];
		push @{$data{$w1}{$w2}}, $w3;
		$w1 = $w2;
		$w2 = $w3;
	}
}

store \%data, 'dict';

sub clean_string () {
	s/\n//;
	s/(^[\s ]*|[\s ]*$)//g;
	s/["']//g;
	s/([^\.!\?])$/\1./;
	return $_;
}
