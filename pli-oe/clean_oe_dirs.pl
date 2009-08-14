#!/usr/bin/perl -w

# This script removes builds of old modules. It looks at the r part (revision) of packages.
# Run the script from the build directory

# Clean up work directory
# Make inventory of directories that can be deleted, store in tobedel list.

%package = ();
@tobedel = ();
opendir( WORK, "tmp/work" ) || die "Something wrong with the directory: $!";

while ($dir = readdir( WORK ))
{
	if ($dir =~ /-r([r\d]+)$/)
	{
		$rev = $1;
		$dir =~ s/-r[r\d]+$/-r/;
		
		if (exists( $package{$dir} ))
		{
			if ( $rev ne $package{$dir} )
			{
				if ( $rev gt $package{$dir} )
				{
					push( @tobedel, $dir . $package{$dir} );
					$package{$dir} = $rev;
				}
				else
				{
					push( @tobedel, $dir . $rev );
				}
			}
		}
		else
		{
			$package{$dir} = $rev;
		}
	}

}
closedir( WORK );

# Delete found directories
if ( @tobedel > 0 )
{
	$fls = "";
	foreach $dir ( @tobedel )
	{
		$fls .= " tmp/work/" . $dir;
	}
	$cmd = "rm -rf " . $fls . " &> /dev/null";
	system( $cmd ) && die $!;
}

# Clean up stamps directory
# Make inventory of files that can be deleted, store in tobedel list.

%package = ();
@tobedel = ();
opendir( STAMPS, "tmp/stamps" ) || die "Something wrong with the directory: $!";

while ($fil = readdir( STAMPS ))
{
	if ($fil =~ /-r([r\d]+)\.do_(fetch|patch|configure|compile|build|install|populate_staging|package)$/)
	{
		$rev = $1;
		$fil =~ s/-r[r\d]+\.do_(fetch|patch|configure|compile|build|install|populate_staging|package)$/-r/;
		
		if (exists( $package{$fil} ))
		{
			if ( $rev gt $package{$fil} )
			{
				if (grep($_ eq $fil . $package{$fil}, @tobedel) == 0)
				{
					push( @tobedel, $fil . $package{$fil} );
				}
				$package{$fil} = $rev;
			}
			elsif ( $rev lt $package{$fil} )
			{
				if (grep($_ eq $fil . $rev, @tobedel) == 0)
				{
					push( @tobedel, $fil . $rev );
				}
			}
		}
		else
		{
			$package{$fil} = $rev;
		}
	}

}
closedir( STAMPS );

# Delete found files
if ( @tobedel > 0 )
{
	$fls = "";
	foreach $fil ( @tobedel )
	{
		$fls .= " tmp/stamps/" . $fil . "*";
	}
	$cmd = "rm -f " . $fls . " &> /dev/null";
	system( $cmd ) && die $!;
}

# Clean up sources directory
# Make inventory of files that can be deleted, store in tobedel list.

%package = ();
@tobedel = ();
opendir( SOURCES, "../sources" ) || die "Something wrong with the directory: $!";

while ($fil = readdir( SOURCES ))
{
	if ($fil =~ /_(\d+)_now\.tar\.gz$/)
	{
		$rev = $1;
		$fil =~ s/\d+_now\.tar\.gz$//;
		
		if (exists( $package{$fil} ))
		{
			if ( $rev > $package{$fil} )
			{
				push( @tobedel, $fil . $package{$fil} );
				$package{$fil} = $rev;
			}
			else
			{
				push( @tobedel, $fil . $rev );
			}
		}
		else
		{
			$package{$fil} = $rev;
		}
	}

}
closedir( SOURCES );

# Delete found files
if ( @tobedel > 0 )
{
	$fls = "";
	foreach $fil ( @tobedel )
	{
		$fls .= " ../sources/" . $fil . "_now.tar.gz";
	}
	$cmd = "rm -f " . $fls . " &> /dev/null";
	system( $cmd ) && die $!;
}
