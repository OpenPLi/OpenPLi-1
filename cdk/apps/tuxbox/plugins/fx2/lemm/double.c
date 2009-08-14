/*
** initial coding by fx2
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <draw.h>
#include <sys/time.h>
#include <rcinput.h>
#include <colors.h>
#include <sprite.h>

#define max(a,b)	((a)>(b)?(a):(b))

static	unsigned char	*dblImage=0;
#define	stride	328
#define	lfb		dblImage

static	unsigned long	lineflags[32];

#define MarkFlag(a,b)	lineflags[(b)/5]|=(1L<<((a)/16))

void	dblCopyImage( int x1, int y1, int dx, int dy, unsigned char *src )
{
	int			x;
	int			y;

	if ( x1+dx < 0 )
		return;
	if ( x1 >= stride )
		return;
	if ( y1+dy < 0 )
		return;
	if ( y1 >= 160 )
		return;

	for( y=0; (y<dy) && (y+y1<160); y++ )
	{
		if ( y1+y < 0 )
			continue;
		for( x=0; (x<dx) && (x+x1<stride); x++ )
		{
			if (( x1+x >= 0 ) && *(src+dx*y+x) )
			{
				if ( *(lfb+(y1+y)*stride+x1+x) != *(src+dx*y+x) )
				{
					*(lfb+(y1+y)*stride+x1+x) = *(src+dx*y+x);
					MarkFlag(x+x1,y+y1);
				}
			}
		}
	}
}

void	dblXMove( int x1, int x2 )
{
	int				y;
	int				i;
	int				dx=0;

	if ( x1 < x2 )
	{
		dx=x2-x1;

		for( y=0; y < 160; y++ )
			memmove(lfb+y*stride+x2,lfb+y*stride+x1,stride-dx);
		*lineflags=0;
		for( i=x1/16; i<(x2+1)/16; i++ )
			*lineflags|=(1L<<i);
		for( i=1; i<32; i++ )
			lineflags[i]=*lineflags;
	}
	if ( x1 > x2 )
	{
		dx=x1-x2;

		for( y=0; y < 160; y++ )
			memcpy(lfb+y*stride+x2,lfb+y*stride+x1,stride-dx);
		*lineflags=0;
		for( i=(stride-dx)/16; i<(stride)/16; i++ )
			*lineflags|=(1L<<i);
		for( i=1; i<32; i++ )
			lineflags[i]=*lineflags;
	}
	if ( !dx )
		return;

/* move screen also */
	FBMove( x1+x1+32, 32, x2+x2+32, 32, 656-dx-dx, 320 );
}

static	void	_drawLine( int x, int y, int dx )
{
	unsigned char	*p;
	int				i;

	y*=5;
	x<<=4;	// *16
	dx<<=4;	// *16
	p = lfb+y*stride+x;

	if ( x+dx>stride)
		dx=stride-x;

	for( i=0; i<10; i+=2, p+=stride )
		FB2CopyImage( x+x+32, y+y+i+32, dx, 1, p, 1 );
}

void	dblDrawFrame( int all )
{
	int				k;
	int				i;
	unsigned long	f;
	int				x=0;
	int				dx=0;

	for( k=0; k<32; k++ )
	{
		dx=0;
		x=0;
		for( i=0; i<21; i++ )
		{
			if ( !all && !lineflags[k] )
				break;
			f= (1L<<i);
			if ( !all && !(lineflags[k] & f ) )
			{
				if ( dx )
					_drawLine( x, k, dx );
				dx=0;
				x=0;
				continue;
			}
			lineflags[k]&=~f;			// remove flag because drawed
			if ( !dx )
				x=i;
			dx++ ;
		}
		if ( dx )
			_drawLine( x, k, dx );
	}
}

int	dblInit( void )
{
	memset(lineflags,0,sizeof(long)*32);

	dblImage=malloc(160*stride);
	if ( !dblImage )
		return -1;
	return 0;
}

void	dblFree( void )
{
	if ( dblImage )
		free( dblImage );
}
