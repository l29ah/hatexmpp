
%functions = {};

$functions{dfn} = sub ($) {
	$param = shift;
	$param =~ /(.*)=(.*)/;
	($word, $def) = ($1, $2);
	$word =~ s/(^\s*|\s*$)//g;
	$def =~ s/(^\s*|\s*$)//g;
	push @{$defs{$word}}, $def;
	store \%defs, 'defs';
	return "Ууу! Записала!";
};


$functions{wtf} = sub ($) {
	$param = shift;
	if (exists $defs{$param}) {
		return "$param - ".randfrom(@{$defs{$param}});
	}
	else {
		return "Ууу! Не нашла";
	}
};

$functions{wtfall} = sub ($) {
	$param = shift;
	if (exists $defs{$param}) {
		return "$param:\n - ".join("\n - ",@{$defs{$param}});
	}
	else {
		return "Ууу! Не нашла";
	}
};

$functions{randfrom} = sub ($) {
	return randfrom(split / /, shift);
};

$functions{top} = sub ($) {
	$param = shift;
	my $out = "Говна топ:\n";
	$out .= sprintf("%-30s %-6s %-6s\n", "*Ник*", "*Говна*", "*Слов*";
	foreach $val (sort {$top{$b}{$param} <=> $top{$a}{$param}} keys %top) {
		$out .= sprintf("%-30s %-6s %-6s\n", $val, $top{$val}{shit}, $top{$val}{words}) if $top{$val}{$param};
	}
	return chomp($out);
};

$functions{sync} = sub {
	store \%top, 'top';
	return "Готово";
};

$functions{givemesources} = sub {
	return `cat hatebot.pl`;
}

print join(", ", keys %functions);
