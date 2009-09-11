#!/usr/bin/perl
use Storable;
use utf8;
use encoding utf8;
%data = %{ retrieve('dict')};

for $k1 ( sort keys %data ) {
	print "$k1\n";
	for $k2 ( sort keys %{$data{$k1}} ) {
		print "\t$k2\n";
		for $k3 (@{$data{$k1}{$k2}}) {
			print "\t\t$k3\n";
		}
	}
}


