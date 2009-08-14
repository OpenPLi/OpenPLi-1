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
		my $c = ord(substr($data, $y*$w+$x, 1));
		$c = $c>>3 | $c>>2<<5 | $c>>3<<11;
		printf("%5d,", $c);
	}
	print "\n";
}
