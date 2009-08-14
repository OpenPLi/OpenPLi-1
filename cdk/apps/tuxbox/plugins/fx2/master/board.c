/*
** initial coding by fx2
*/


#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include <colors.h>
#include <draw.h>
#include <rcinput.h>

#define	STATUS_X		80
#define STATUS_Y		50
#define LOGO_X			500
#define LOGO_Y			30

#define	BOMBS			16

extern	int		doexit;

extern	unsigned short	actcode;
extern	unsigned short	realcode;

static	int			bx=0;
static	int			by=0;
static	int			f[5];
static	int			of[5];

static	char		bvis=0;

static	char	cnum[] = {
					RED, GREEN, YELLOW, BLUE,
					MAGENTA, CYAN, DARK, WHITE };

static	char	*levnam[] = {
					"(1) very easy",
					"(2) easy",
					"(3) normal",
					"(4) strong" };

static	int		level=2;
/* 0 - no brain - 5 colors, no doubles */
/* 1 - pound brain - 8 colors, no doubles */
/* 2 - normal brain - 6 colors, doubles */
/* 3 - mega brain - 8 colors, doubles */

static	char	num_colors[] = { 5, 8, 6, 8 };
static	char	use_double[] = { 0, 0, 1, 1 };
static	int		seq[5];

static	int		myrand( int idx )
{
	struct timeval tv;
	gettimeofday(&tv,0);

	return tv.tv_usec % idx;
}

static	void	GenCode( void )
{
	int				i;
	int				j;
	struct timeval	tv;

	for( i=0; i<5; i++ )
	{
		f[i]=-1;
		of[i]=-1;
		tv.tv_sec=0;
		tv.tv_usec = myrand(120)*100;
		select(0,0,0,0,&tv);
		seq[i] = myrand( num_colors[level] );
		if ( seq[i] >= num_colors[level] )
			seq[i]=myrand(num_colors[level]);

		if ( !use_double[level] )
		{
			for( j=0; j<i; j++ )
			{
				if ( seq[j] == seq[i] )
				{
					i--;
					break;
				}
			}
		}
	}
}

void	MasterInitialize( void )
{
	int				x;
	int				y;
	int				i;

	FBFillRect( 0, 0, 720,576, BLACK );

	GenCode();

	x=FBDrawString( 400, 100, 48, "Level : ",WHITE, 0 );
	FBDrawString( 400+x, 100, 48, levnam[level],WHITE, 0 );
	x=FBDrawString( 400, 150, 32, "available colors : ", WHITE, 0 );
	FBFillRect( 400+x,158, num_colors[level]*12+4, 16, GRAY );
	for( i=0; i<num_colors[level]; i++ )
		FBFillRect( 404+x+i*12, 162, 8, 8, cnum[i]);
	if ( use_double[level] )
		FBDrawString( 400, 180, 32, "same colors possible", WHITE, 0 );
	FBDrawString( 400, 210, 32, "1,2,3,4 - change level", WHITE, 0 );
	FBFillRect( 400, 240+12, 8, 8, RED);
	FBDrawString( 416, 240, 32, "copy last line", WHITE, 0 );
	FBFillRect( 400, 270+12, 8, 8, YELLOW);
	FBDrawString( 416, 270, 32, "switch right", WHITE, 0 );
	FBFillRect( 400, 300+12, 8, 8, GREEN);
	FBDrawString( 416, 300, 32, "switch left", WHITE, 0 );

/* board */
	FBFillRect( 60, 100, 300, 400, GRAY );
	FBFillRect( 58, 98, 302, 2, WHITE );
	FBFillRect( 58, 98, 2, 402, WHITE );
	FBFillRect( 360, 98, 2, 402, DARK );
	FBFillRect( 58, 500, 304, 2, DARK );

	for( y=0; y < 8; y++ )
	{
		for( x=0; x < 5; x++ )
		{
			FBDrawRect( 65+(x*34), 110+(y*50), 30, 30, BLACK );
			FBDrawRect( 66+(x*34), 111+(y*50), 28, 28, BLACK );
		}
		FBDrawVLine( 245, 102, 396, BLACK );
		FBDrawVLine( 246, 102, 396, BLACK );
	}

	actcode=0xee;
	bvis=0;
	bx=0;
	by=0;
}

void	Play( void )
{
	int				x;
	int				i;
	int				j;
	int				mseq[5];
	int				xof[5];
static	int	lcount=0;

	lcount++;
	if ( !(lcount%5) )
	{
		bvis=!bvis;
		if ( bvis )
		{
			FBDrawRect( 65+(bx*34), 110+(by*50), 30, 30, RED );
			FBDrawRect( 66+(bx*34), 111+(by*50), 28, 28, RED );
		}
		else
		{
			FBDrawRect( 65+(bx*34), 110+(by*50), 30, 30, BLACK );
			FBDrawRect( 66+(bx*34), 111+(by*50), 28, 28, BLACK );
		}
	}
	switch( actcode )
	{
	case RC_1 :
	case RC_2 :
	case RC_3 :
	case RC_4 :
		if ( level == actcode -1 )
			return;
		level=actcode-1;
		doexit=4;
		return;
	case RC_RED :
		memcpy(f,of,sizeof(int)*5);
		if ( bvis )
		{
			FBDrawRect( 65+(bx*34), 110+(by*50), 30, 30, BLACK );
			FBDrawRect( 66+(bx*34), 111+(by*50), 28, 28, BLACK );
		}
		bvis=0;
		lcount=4;
		for( i=0; i<5; i++ )
		{
			FBFillRect( 67+(i*34), 112+(by*50), 27, 27,
				f[i] == -1 ? GRAY : cnum[f[i]]);
		}
		return;
	case RC_YELLOW :
		if ( bx < 4 )
		{
			int	o;
			o = f[bx+1];
			f[bx+1] = f[bx];
			f[bx] = o;
			FBFillRect( 67+((bx+1)*34), 112+(by*50), 27, 27,
				f[bx+1] == -1 ? GRAY : cnum[f[bx+1]]);
			FBFillRect( 67+(bx*34), 112+(by*50), 27, 27,
				f[bx] == -1 ? GRAY : cnum[f[bx]]);
		}
		return;
	case RC_GREEN :
		if ( bx > 0 )
		{
			int	o;
			o = f[bx-1];
			f[bx-1] = f[bx];
			f[bx] = o;
			FBFillRect( 67+((bx-1)*34), 112+(by*50), 27, 27,
				f[bx-1] == -1 ? GRAY : cnum[f[bx-1]]);
			FBFillRect( 67+(bx*34), 112+(by*50), 27, 27,
				f[bx] == -1 ? GRAY : cnum[f[bx]]);
		}
		return;
	case RC_UP :
		f[bx]--;
		if ( f[bx] < 0 )
			f[bx] = num_colors[ level ]-1;
		break;
	case RC_DOWN :
		f[bx]++;
		if ( f[bx] == num_colors[ level ] )
			f[bx] = 0;
		break;
	case RC_LEFT :
		if ( bx == 0 )
			return;
		if ( bvis )
		{
			FBDrawRect( 65+(bx*34), 110+(by*50), 30, 30, BLACK );
			FBDrawRect( 66+(bx*34), 111+(by*50), 28, 28, BLACK );
		}
		lcount=4;
		bvis=0;
		bx--;
		return;
	case RC_RIGHT :
		if ( bx == 4 )
			return;
		if ( bvis )
		{
			FBDrawRect( 65+(bx*34), 110+(by*50), 30, 30, BLACK );
			FBDrawRect( 66+(bx*34), 111+(by*50), 28, 28, BLACK );
		}
		lcount=4;
		bvis=0;
		bx++;
		return;
	case RC_OK :
		memcpy(xof,f,sizeof(int)*5);
		for( i=0; i < 5; i++ )
			if ( f[i] == -1 )
				return;
		if ( !use_double[ level ] )
			for( i=1; i < 5; i++ )
				for( j=0; j<i; j++ )
					if ( f[i] == f[j] )
						return;
		if ( bvis )
		{
			FBDrawRect( 65+(bx*34), 110+(by*50), 30, 30, BLACK );
			FBDrawRect( 66+(bx*34), 111+(by*50), 28, 28, BLACK );
		}
		bvis=0;
		lcount=4;
		/* test now */
		x=0;
		memcpy(mseq,seq,sizeof(int)*5);
		for( i=0; i < 5; i++ )
		{
			if ( f[i] == mseq[i] )
			{
				f[i]=-1;
				mseq[i]=-1;
				FBFillRect( 260+(x*12), 121+(by*50), 8, 8, DARK );
				x++;
			}
		}
		if ( x == 5 )
		{
			doexit=1;
			FBDrawString( 260, 37, 64, " great ", RED, 0 );
			return;
		}
		for( i=0; i < 5; i++ )
		{
			if ( f[i] == -1 )
				continue;
			for( j=0; j < 5; j++ )
			{
				if ( f[i] == mseq[j] )
				{
					f[i]=-1;
					mseq[j]=-1;
					FBFillRect( 260+(x*12), 121+(by*50), 8, 8, WHITE );
					x++;
					break;
				}
			}
		}
		if ( by == 7 )
		{
			doexit=2;
			FBDrawString( 260, 37, 64, " you lost ", RED, 0 );
			for( i=0; i < 5; i++ )
			{
				FBDrawRect( 65+(i*34), 60, 30, 30, GRAY );
				FBDrawRect( 66+(i*34), 61, 28, 28, GRAY );
				FBFillRect( 67+(i*34), 62, 27, 27, cnum[seq[i]]);
			}
			return;
		}
		by++;
		memcpy(of,xof,sizeof(int)*5);

		bx=0;
		for( i=0; i < 5; i++ )
			f[i] = -1;

		return;
	default :
		return;
	}
	FBFillRect( 67+(bx*34), 112+(by*50), 27, 27, cnum[f[bx]]);
}
