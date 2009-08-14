#!/usr/bin/perl
use strict;
use warnings;

use Getopt::Long;
use IO::File;
use Pod::Usage;

my $image;
my $operation;
my %parts = ();

GetOptions
(
  'help' => sub { pod2usage ( 1 ); },
  'man' => sub { pod2usage ( -exitstatus => 0, -verbose => 2 ); },
  'image|i=s' => \$image,
  'operation|oper|o=s' => \$operation,
  'part=s' => \%parts,
);

my %partdef =
(
  0 => [ "ppcboot", 0, 0x20000 ],
  1 => [ "root", 0x20000, 0x6c0000 ],
  2 => [ "var", 0x6e0000, 0x100000 ],
);

sub part_read
{
  my $in = shift;
  my $file = shift;
  my $begin = shift;
  my $size = shift;

  my $out = IO::File -> new ( $file, O_CREAT | O_EXCL | O_WRONLY ) or die $!;

  $in -> seek ( $begin, SEEK_SET ) or die $!;

  my $buf;

  my $temp = $size;
  while ( $temp > 4096 )
  {
    $in -> sysread ( $buf, 4096 );
    $out -> syswrite ( $buf, 4096 );
    $temp -= 4096;
  }

  $in -> sysread ( $buf, $temp );
  $out -> syswrite ( $buf, $temp );
}

sub part_write
{
  my $out = shift;
  my $file = shift;
  my $begin = shift;
  my $size = shift;

  my $in = IO::File -> new ( $file, O_RDONLY ) or die $!;

  $in -> seek ( 0, SEEK_END ) or die $!;
  my $insize = $in -> tell () or die $!;
  $in -> seek ( 0, SEEK_SET ) or die $!;
  $out -> seek ( $begin, SEEK_SET ) or die $!;

  my $buf;

  my $temp = $insize;
  while ( $temp > 4096 )
  {
    $in -> sysread ( $buf, 4096 );
    $out -> syswrite ( $buf, 4096 );
    $temp -= 4096;
  }

  $in -> sysread ( $buf, $temp );
  $out -> syswrite ( $buf, $temp );

  if ( $insize < $size )
  {
    part_write_pad ( $out, $begin + $insize, $size - $insize );
  }
}

sub part_write_pad
{
  my $out = shift;
  my $begin = shift;
  my $size = shift;

  $out -> seek ( $begin, SEEK_SET );

  my $buf = "\xff"x$size;
  $out -> syswrite ( $buf, $size );
}

if ( not defined ( $operation ) )
{
  pod2usage ( -message => "No operation given.", -exitstatus => 0, -verbose => 1 );
}
elsif ( $operation eq "build" )
{
  my $out = IO::File -> new ( $image, O_CREAT | O_EXCL | O_WRONLY ) or die $!;

  foreach ( sort ( keys ( %partdef ) ) )
  {
    if ( defined ( $parts { $partdef { $_ } -> [0] } ) )
    {
      part_write ( $out, $parts { $partdef { $_ } -> [0] }, $partdef { $_ } -> [1], $partdef { $_ } -> [2] );
    }
    else
    {
      part_write_pad ( $out, $partdef { $_ } -> [1], $partdef { $_ } -> [2] );
    }
  }
}
elsif ( $operation eq "replace" )
{
  my $out = IO::File -> new ( $image, O_WRONLY ) or die $!;

  foreach ( sort ( keys ( %partdef ) ) )
  {
    if ( defined ( $parts { $partdef { $_ } -> [0] } ) )
    {
      part_write ( $out, $parts { $partdef { $_ } -> [0] }, $partdef { $_ } -> [1], $partdef { $_ } -> [2] );
    }
  }
}
elsif ( $operation eq "extract" )
{
  my $in = IO::File -> new ( $image, O_RDONLY ) or die $!;

  foreach ( sort ( keys ( %partdef ) ) )
  {
    if ( defined ( $parts { $partdef { $_ } -> [0] } ) )
    {
      part_read ( $in, $parts { $partdef { $_ } -> [0] }, $partdef { $_ } -> [1], $partdef { $_ } -> [2] );
    }
  }
}
elsif ( $operation eq "print" )
{
  my ( $name, $begin, $end, $size );

  format STDOUT_TOP =
name       : begin    - end      (size)
.
  format STDOUT =
@<<<<<<<<<<: 0x^>>>>> - 0x^>>>>> (0x^>>>>>)
$name,         $begin,    $end,     $size
.

  foreach ( sort ( keys ( %partdef ) ) )
  {
    $name = $partdef { $_ } -> [0];
    $begin = sprintf ( "%06x", $partdef { $_ } -> [1] );
    $end = sprintf ( "%06x", $partdef { $_ } -> [1] + $partdef { $_ } -> [2] );
    $size = sprintf ( "%06x", $partdef { $_ } -> [2] );
    write;
  }
}
else
{
  pod2usage ( -message => "Unknown operation.", -exitstatus => 0, -verbose => 1 );
}

__END__

=head1 NAME

flashmanage

=head1 SYNOPSIS

flashmanage [OPTIONS]

  -i, --image FILE      image file
  -o, --operation ARG   what to do (build, extract, replace, print)
      --part NAME=FILE  partition files
      --help            brief help message
      --man             full documentation

=head2 EXAMPLES

  flashmanage.pl -i flashimage.img -o replace --part root=root.img
  flashmanage.pl -i flashimage.img -o build --part root=root.img --part var=var.img

=cut
