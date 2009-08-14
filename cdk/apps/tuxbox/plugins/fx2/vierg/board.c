/*
** initial coding by fx2
*/


#include <stdio.h>

#include <draw.h>
#include <sys/time.h>
#include <rcinput.h>
#include <colors.h>
#include <pics.h>

#define	STATUS_X		80
#define STATUS_Y		50
#define LOGO_X			500
#define LOGO_Y			30

extern	double	sqrt( double in );

extern	int		doexit;
static	int		ipos=3;

extern	unsigned short	actcode;

static	char	maze[42];
static	int		tst[7];

static	struct timeval starttv;

static	int		myrand( int idx )
{
	struct timeval tv;
	gettimeofday(&tv,0);

	return tv.tv_usec % idx;
}

static	void	msleep( int msec )
{
	struct timeval tv;
	tv.tv_sec = msec/1000;
	tv.tv_usec = 1000*(msec%1000);
	select( 0, 0, 0, 0, &tv );
}

void	DrawBoard( void )
{
	int				x;
	int				y;

	FBFillRect( 0, 0, 720, 576, BLACK );

	for( y = 0; y < 6; y++ )
	{
		for( x = 0; x < 7; x++ )
		{
			FBCopyImage( x*48+64, y*48+96, 48, 48, dout );
		}
	}
	FBOverlayImage( ipos*48+64+6, 48+4, 36, 40, 0, 0, WHITE, dred, 0,0,0);
	FBDrawFx2Logo( LOGO_X, LOGO_Y );
	gettimeofday(&starttv,0);
}

void	BoardInitialize( void )
{
	memset(maze,0,sizeof(maze));

	actcode=0xee;
	ipos=3;
}

static	void	Fall( int x, unsigned char *dr, char v )
{
	int		y;

	for( y=0; y<6; y++ )
	{
		if ( maze[ (5-y)*7 + x ] )
			break;
		if(y)
		{
			msleep(100);
			FBCopyImage( x*48+64+6, y*48+48+4, 36, 40, dgray );
			maze[ (6-y)*7 + x ] = 0;
		}
		else
			FBFillRect( x*48+64+6, y*48+48+4, 36, 40, BLACK );
		maze[ (5-y)*7 + x ] = v;
		FBOverlayImage( x*48+64+6, y*48+96+4, 36, 40, 0, 0, WHITE,
				dr, dgray, dgray, dgray);
	}
}

static	int	vFall( int x, char v )
{
	int		y;
	int		idx=-1;

	idx=x;
	for( y=0; y<6; y++, idx+=7 )
	{
		if ( !maze[ idx ] )
		{
			maze[ idx ] = v;
			return idx;
		}
	}
	return -1;
}

static	int	TestGameOver( int mask )
{
	int		x;
	int		y;
	int		idx;

	for( y=0, idx=0; y<3; y++ )
	{
		for( x=0; x<7; x++, idx++ )
		{
			if ( maze[ idx ] & mask )	// start-point
			{
				if ( x < 4 )
				{
					// vertikal nach rechts testen
					if (( maze[ idx +1 ] & mask ) &&
						( maze[ idx +2 ] & mask ) &&
						( maze[ idx +3 ] & mask ))
					{
						return 1;		// game over
					}
					// diagonale nach rechts testen
					if (( maze[ idx +8 ] & mask ) &&
						( maze[ idx +16 ] & mask ) &&
						( maze[ idx +24 ] & mask ))
					{
						return 1;		// game over
					}
				}
				if ( x > 2 )
				{
					// diagonale nach links testen
					if (( maze[ idx +6 ] & mask ) &&
						( maze[ idx +12 ] & mask ) &&
						( maze[ idx +18 ] & mask ))
					{
						return 1;		// game over
					}
				}
				// nach oben testen
				if (( maze[ idx +7 ] & mask ) &&
					( maze[ idx +14 ] & mask ) &&
					( maze[ idx +21 ] & mask ))
				{
					return 1;		// game over
				}
			}
		}
	}
// der rest wird nur auf waagerecht untersucht
	for( ; y<6; y++ )
	{
		for( x=0; x<7; x++, idx++ )
		{
			if (( x < 4 ) && maze[ idx ] & mask )	// start-point
			{
				// vertikal nach rechts testen
				if (( maze[ idx +1 ] & mask ) &&
					( maze[ idx +2 ] & mask ) &&
					( maze[ idx +3 ] & mask ))
				{
					return 1;		// game over
				}
			}
		}
	}
// test auf patt
	for( idx=35, x=0; x<7; x++, idx++ )
		if ( !maze[idx] )
			break;
	return x==7 ? 2:0;
}

static	void	CPlay( int x )
{
	FBOverlayImage( x*48+64+6, 48+4, 36, 40, 0, 0, WHITE, dblue, 0,0,0);
	msleep(700);
	Fall( x, dblue, 2 );
}

static	void	outmaze()
{
	int		x;
	int		y;
	int		idx;

	printf("+-+-+-+-+-+-+-+%c\n",0x0d);
	for( y=0, idx=35; y<6; y++ )
	{
		for( x=0; x<7; x++, idx++ )
			printf("|%c",maze[idx]?((maze[idx]&1)?'o':'*'):' ');
		idx-=14;
		printf("|%c\n",0x0d);
	}
	printf("+-+-+-+-+-+-+-+%c\n\n",0x0d);
}

static	void	MyPlay( void )
{
	int		x;
	int		idx;
	int		k;
	int		vidx[7];
	int		max=0;

	for( x=0; x<7; x++ )
		tst[x]=0;
/* test: eigener sieg in 1nem zug */
	for( x=0; x<7; x++ )
	{
		idx=vFall( x, 6 );
//printf("test %d (pos=%d)%c\n",x,idx,0x0d);
//outmaze();
		if ( idx != -1 )
		{
			if ( TestGameOver(2) )	// great ! - choose it
			{
				maze[idx]=0;	// remove virt. chip
				CPlay( x );
				return;
			}
			k=vFall( x, 5 ); // put playerchip over me
			if ( k != -1 )
			{
				if ( TestGameOver(1) )	// fault - this field is ugly
					tst[x] -= 50;
				else
					tst[x]++;
				maze[k]=0;	// remove virt. chip
			}
			else
				tst[x]++;
			maze[idx]=0;	// remove virt. chip
		}
		else
			tst[x]=-999999;	// neg val
	}
/* test: player sieg in 1-2 zuegen */
	for( x=0; x<7; x++ )
	{
		idx=vFall( x, 5 );
		if ( idx != -1 )
		{
			if ( TestGameOver(1) )	// great ! - choose it
				tst[x] += 50;
			else
			{
				int		idx2;

				for( k=0;k<7;k++)
				{
					if ( k==x )
						continue;
					idx2=vFall(k,5);
					if ( idx2 != -1 )
					{
						if ( TestGameOver(1) )	// great ! - choose it
							tst[x] += 10;
						maze[idx2]=0;	// remove virt. chip
					}
				}
			}
			maze[idx]=0;	// remove virt. chip
		}
	}

// search highest val
	for( x=1; x<7; x++ )
		if ( tst[x] > tst[max] )
			max=x;
	idx=0;
	for( x=0; x<7; x++ )
	{
		if (( tst[x] == tst[max] ) && !maze[35+x] )
		{
			vidx[idx] = x;
			idx++;
		}
	}

	if ( !idx )	// never reached
		return;

	if ( idx > 1 )
	{
		int		i1;
		int		i2;

		for( k=0;k<idx;k++)
		{
			i1=vFall( vidx[k], 5 );
			if ( i1 == -1 )
				continue;
			for( x=0;x<7;x++)
			{
				i2=vFall(x,5);
				if ( i2 == -1 )
					continue;
				if ( TestGameOver(2) )	// great ! - choose it
					tst[vidx[k]] += 5;
				maze[i2]=0;				// remove virt. chip
			}
			maze[i1]=0;				// remove virt. chip
		}
	}
// search highest val again
	max=0;
	for( x=1; x<7; x++ )
		if ( tst[x] > tst[max] )
			max=x;
	idx=0;
	for( x=0; x<7; x++ )
	{
		if (( tst[x] == tst[max] ) && !maze[35+x] )
		{
			vidx[idx] = x;
			idx++;
		}
	}

	if ( !idx )	// never reached
		return;

	idx=myrand(idx);
	CPlay(vidx[idx]);
}

static	int	GameOver( int mask )
{
	int		k;
	k=TestGameOver( mask );
	if ( !k )
		return 0;
	if ( k == 2 )		// patt
	{
		FBDrawString( 190, 410, 64, "good game !", WHITE, 0 );
	}
	else if ( mask == 1 )
	{
		FBDrawString( 190, 410, 64, "You won !", WHITE, 0 );
	}
	else
	{
		FBDrawString( 190, 410, 64, "Iam the Winner !", WHITE, 0 );
	}
	doexit=1;
	return 1;
}

void	MoveMouse( void )
{
static	int	locked = 0;
	int		k;

	if ( locked )
	{
		locked--;
		actcode=0xee;
		return;
	}
	k=0;
	switch( actcode )
	{
	case RC_7 : k++;
	case RC_6 : k++;
	case RC_5 : k++;
	case RC_4 : k++;
	case RC_3 : k++;
	case RC_2 : k++;
	case RC_1 : k++;
		break;
	}
	if ( k )
		ipos=k-1;
	switch( actcode )
	{
	case RC_RIGHT :
		if ( ipos < 6 )
		{
			FBFillRect( ipos*48+64+6, 48+4, 36, 40, BLACK );
			ipos++;
			FBOverlayImage( ipos*48+64+6, 48+4, 36, 40, 0, 0, WHITE,
				dred, 0,0,0);
			locked=1;
		}
		break;
	case RC_LEFT :
		if ( ipos > 0 )
		{
			FBFillRect( ipos*48+64+6, 48+4, 36, 40, BLACK );
			ipos--;
			FBOverlayImage( ipos*48+64+6, 48+4, 36, 40, 0, 0, WHITE,
				dred, 0,0,0);
			locked=1;
		}
		break;
	case RC_7 :
	case RC_6 :
	case RC_5 :
	case RC_4 :
	case RC_3 :
	case RC_2 :
	case RC_1 :

	case RC_OK :
		locked=1;
		if ( maze[ipos+35] )
			break;
		Fall( ipos, dred, 1 );
		k=TestGameOver( 1 );
		if ( GameOver(1) )
			return;
		MyPlay();
		if ( GameOver(2) )
			return;
		ipos=3;
		FBOverlayImage( ipos*48+64+6, 48+4, 36, 40, 0, 0, WHITE,
				dred, 0,0,0);
		break;
	}
}
