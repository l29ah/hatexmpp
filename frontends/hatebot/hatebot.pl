#!/usr/bin/perl
$| = 1;
print "!$ARGC $ARGV[0]";
$fn = $ARGV[0];
while (<STDIN>) {
	$line = $_;
	if ($line =~ /\d+ (.*?): \^(.*?) (.*)/) {
		($user, $cmd, $param) = ($1, $2, $3);
		print "user=$user cmd=$cmd param=$param\n";
		
		chomp($cmd);
		if ($cmd eq "test") {
			$out = "I am working! $param";
		}
		
		if ($cmd eq "google") {
			$out = `./google.sh $param`;
		}
		#elsif ($cmd eq "dejap") {
		#	$out = `bash -c 'echo -n "$param" | iconv -t EUC-JP | kakasi -Ha -Ka -Ja -Ea -ka -s -p -rkunrei -c -C'`;
		#}
		
		open(OUTFILE, ">$fn");
		print OUTFILE "$user: $out";
		close(OUTFILE);
	}
}
