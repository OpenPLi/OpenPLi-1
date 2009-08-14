#!/usr/bin/perl

my @hdr;
while(1) {
	my $line = <>;
	next if $line =~ /^#/;
	push @hdr, $line;
	last if @hdr == 3;
}

my $data = <>;

my ($w, $h) = split(/ /, $hdr[1]);

#print STDERR "h=$h w=$w";

for (my $y=0; $y<$h; $y++) {
	for (my $x=0; $x<$w; $x++) {
		my $r = ord(substr($data, ($y*$w+$x)*3  , 1));
		my $g = ord(substr($data, ($y*$w+$x)*3+1, 1));
		my $b = ord(substr($data, ($y*$w+$x)*3+2, 1));
		$c = $b>>3 | $g>>2<<5 | $r>>3<<11;
		printf("%5d,", $c);
	}
	print "\n";
}
