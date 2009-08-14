#!/usr/bin/perl
use strict;

my $head = "download:";
my $output;
my $wgetoptions = "-c --no-dns-cache --retry-connrefused -t3 --timeout=10";
my @mirrorsbefore = ();
my @mirrorsafter = ("http://downloads.pli-images.org/cdk-archive");

open ( RULES, $ARGV[0] ) or die;

while ( <RULES> )
{
  chomp;
  if ( ! m/^#/ and ! m/^\s*$/ )
  {
    @_ = split ( /;/, $_ );
    my $file = $_[0];
    $head .= " Archive/" . $file;
    $output .= "Archive/" . $file . ":\n\tfor i in 1 2 3 4; do (false";
    shift @_;

    my @mirrors = ();
    push(@mirrors, @mirrorsbefore);
    push(@mirrors, @_);
    foreach ( @_ )
    { 
      if ( $_ =~ m/^http:\/\/(.*?)\.sourceforge\.net\/(.*)$/ )
      {
        if ( $1 ne "dl") 
        {
          push(@mirrors, "http://dl.sourceforge.net\/$2");
          last;
        }
      }
    }
    if ( $file =~ m/gz$/ )
    {
      push(@mirrors, "ftp://ftp.berlios.de/pub/tuxbox/src");
    }
    else
    {
      push(@mirrors, "http://tuxbox.berlios.de/pub/tuxbox/cdk/src");
    }
    push(@mirrors, @mirrorsafter);

    foreach ( @mirrors )
    {
      if ( $_ =~ m#^ftp://# )
      {
        $output .= " || \\\n\twget $wgetoptions --passive-ftp -P Archive " . $_ . "/" . $file;
      }
      elsif ( $_ =~ m#^http://# )
      {
        $output .= " || \\\n\twget $wgetoptions -P Archive " . $_ . "/" . $file ;
      }
    }
    $output .= ")&& break; done\n\n";
  }
}

close ( RULES );

print $head . "\n\n" . $output . "\n";
