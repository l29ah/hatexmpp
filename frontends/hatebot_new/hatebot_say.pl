#!/usr/bin/perl
use Storable;
use utf8;
use encoding 'utf8';
%data = %{ retrieve('dict')};
while (<STDIN>) {
	print hsulci($_)."\n";
}

sub hsulci {
	$w1 = randfrom(keys %data);
	$w2 = randfrom(keys %{$data{$w1}});
	$line = "$w1 $w2";
	for ($i=0; $i<10+rand(30); $i++) {
		$w3 = randfrom(@{$data{$w1}{$w2}});
		$line .= " $w3";
		$w1 = $w2;
		$w2 = $w3;
	}
	chomp($line);
	$line = ucfirst($line);
	$line =~ s/(^[\s ]*|[\s ]*$)//g;
	$line =~ s/,$//;
	$line =~ s/([^\.!\?])$/\1./;
	return $line;
}

sub randfrom(@) {
	return $_[rand($#_+1)];
}
