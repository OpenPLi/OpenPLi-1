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

#define	BOMBS			16

extern	double	sqrt( double in );

extern	int		doexit;

extern	unsigned short	actcode;

static	int		numflags=0;
static	int		failflags=0;
static	long	score = 0;
static	int		mouse_x = 3;
static	int		mouse_y = 3;

static	struct timeval starttv;

static	int		ww2[10] = { 0, 5, 10, 9, 10, 9, 9, 9, 9, 9 };

static	int		myrand( int idx )
{
	struct timeval tv;
	gettimeofday(&tv,0);

	return tv.tv_usec % idx;
}

void	DrawScore( void )
{
	char			tscore[ 64 ];
	struct timeval tv;
	gettimeofday(&tv,0);

	score = tv.tv_sec - starttv.tv_sec;

	if ( score > 36000 )
		score=36000;

	score = 37000 - score;

	sprintf(tscore,"%ld",score);
	FBDrawString( 190, 210, 64, "Score", WHITE, 0 );
	FBDrawString( 190, 290, 64, tscore, WHITE, 0 );
}

static	void	DrawMouse( void )
{
//	FBDrawVLine( mouse_x * 32 + 15, mouse_y * 32 + 4, 24, GREEN );
//	FBDrawHLine( mouse_x * 32 + 4, mouse_y * 32 + 15, 24, GREEN );
	FBFillRect( mouse_x * 32 + 4, mouse_y * 32 + 15, 24, 2, GREEN );
	FBFillRect( mouse_x * 32 + 15, mouse_y * 32 + 4, 2, 24, GREEN );
}

static	int	DrawField( int x, int y )
{
	unsigned char	*p = maze + MAZEW * y + x;
	unsigned char	*p2;
	int				vx;
	int				vy;
	int				num;
	int				r;
	char			cnum[2] = { 48, 0 };

	switch ( *p )
	{
	case 'N' :
	case 'B' :
		FBCopyImage( x*32, y*32, 32, 32, dflag );
		break;
	case 'n' :
	case 'b' :
		FBCopyImage( x*32, y*32, 32, 32, dout );
		break;
	case 'p' :
		p2 = p - MAZEW - 1;
		for( num=0, vy=y-1; vy < y+2; vy++ )
		{
			for( vx=x-1; vx < x+2; vx++, p2++ )
			{
				if (( *p2 == 'b' ) || ( *p2 == 'B' ))
					num++;
			}
			p2 += MAZEW - 3;
		}
		FBFillRect( x*32+1, y*32+1, 31, 31, GRAY );
		FBDrawHLine( x*32, y*32, 32, WHITE );
		FBDrawVLine( x*32, y*32, 32, WHITE );
		if ( num )
		{
			r = 16 - (ww2[num] / 2);
			if ( r < 0 )
				r=0;
			cnum[0]=num+48;
			FBDrawString( x*32+r, y*32, 32, cnum, BLACK, 0 );
		}
		return num;
	}
	return 0;
}

void	DrawBoard( int rbomb )
{
	int				x;
	int				y;
	unsigned char	*p = maze;

	for( y = 0; y < MAZEH; y++ )
	{
		for( x = 0; x < MAZEW; x++, p++ )
		{
			switch ( *p )
			{
			case '#' :
				FBFillRect( x*32, y*32, 32, 32, STEELBLUE );
				break;
			case ' ' :
				FBFillRect( x*32, y*32, 32, 32, BLACK );
				break;
			case 'n' :
				FBCopyImage( x*32, y*32, 32, 32, dout );
				break;
			case 'b' :
				if ( rbomb )
					FBOverlayImage( x*32, y*32, 32, 32, 0, 0, WHITE, dbomb, dout, dout, dout );
				else
					FBCopyImage( x*32, y*32, 32, 32, dout );
				break;
			case 'B' :
				if ( rbomb )
					FBOverlayImage( x*32, y*32, 32, 32, 0, 0, WHITE, dbomb, dflag, dflag, dflag );
				else
					FBCopyImage( x*32, y*32, 32, 32, dout );
				break;
			case 'N' :
				FBCopyImage( x*32, y*32, 32, 32, dflag );
				break;
			}
		}
	}
	FBDrawRect( 3*32-3, 3*32-3, 10*32+5, 10*32+5, WHITE );
	FBDrawRect( 3*32-4, 3*32-4, 10*32+7, 10*32+7, WHITE );
	FBDrawFx2Logo( LOGO_X, LOGO_Y );
	gettimeofday(&starttv,0);

	DrawMouse();
}

static	void	Flag( int x, int y )
{
	unsigned char	*p = maze + MAZEW * y + x;

	switch( *p )
	{
	case 'p' :
		break;
	case 'N' :
		failflags--;
	case 'B' :
		numflags--;
		*p |= 32;
		DrawField( x, y );
		DrawMouse();
		break;
	case 'n' :		// nothing
		if ( numflags == BOMBS )
			return;
		failflags++;
	case 'b' :		// a bomb
		if ( numflags == BOMBS )
			return;
		numflags++;
		*p &= ~32;
		DrawField( x, y );
		DrawMouse();

		if ( !failflags && (numflags == BOMBS ))
			doexit=2;
		break;
	}
}

void	BoardInitialize( void )
{
	int				x;
	int				y;
	int				i;
	int				n;
	unsigned char	*p = maze;

	for( y = 0; y < MAZEH; y++ )
	{
		for( x = 0; x < MAZEW; x++, p++ )
		{
			if ( *p == 'b' )
				*p = 'n';
			if ( *p == 'p' )
				*p = 'n';
			if ( *p == 'B' )
				*p = 'n';
			if ( *p == 'N' )
				*p = 'n';
		}
	}
	actcode=0xee;
	score=0;
	mouse_x=3;
	mouse_y=3;
	numflags=0;

	// put in bombs
	for( i=0; i < BOMBS; i++ )
	{
		x = myrand(10)+3;
		n = myrand(10);
		for( y = myrand(10)+3; n>0; n-- )
			y=myrand(10)+3;
		if ( maze[ y*MAZEW + x ] != 'n' )
			i--;
		maze[ y * MAZEW + x ] = 'b';
	}
}

void	DrawGameOver( void )
{
	DrawBoard(1);
	FBDrawString( 500, 210, 64, "Game Over", RED, 0 );
}

static	void	rekSel( int x, int y )
{
	unsigned char	*p = maze + MAZEW * y + x;
	int				num;

	if (( x < 3 ) || ( x > 12 ) || ( y < 3 ) || ( y > 12 ))
		return;

	*p = 'p';
	num=DrawField( x, y );

	if ( num )
		return;

#if 0
	p -= (MAZEW+1);
	if ( *p == 'n' )
		rekSel( x-1,y-1 );
	p++;
	if ( *p == 'n' )
		rekSel( x,y-1 );
	p++;
	if ( *p == 'n' )
		rekSel( x+1,y-1 );
	p+=(MAZEW-2);
	if ( *p == 'n' )
		rekSel( x-1,y );
	p++;
	p++;
	if ( *p == 'n' )
		rekSel( x+1,y );
	p+=(MAZEW-2);
	if ( *p == 'n' )
		rekSel( x-1,y+1 );
	p++;
	if ( *p == 'n' )
		rekSel( x,y+1 );
	p++;
	if ( *p == 'n' )
		rekSel( x+1,y+1 );
#else
	p -= (MAZEW);
	if ( *p == 'n' )
		rekSel( x,y-1 );
	p+=(MAZEW-1);
	if ( *p == 'n' )
		rekSel( x-1,y );
	p+=2;
	if ( *p == 'n' )
		rekSel( x+1,y );
	p+=(MAZEW-1);
	if ( *p == 'n' )
		rekSel( x,y+1 );
#endif
}

static	void	SelectField( int x, int y )
{
	unsigned char	*p = maze + MAZEW * y + x;

	switch( *p )
	{
	case 'p' :		// is pressed
		break;
	case 'b' :		// a bomb
		doexit=1;
		break;
	case 'n' :		// nothing
		rekSel( x, y );
		DrawMouse();
		break;
	}
}

void	MoveMouse( void )
{
static	int	locked = 0;

	if ( locked )
	{
		locked--;
		actcode=0xee;
		return;
	}
	switch( actcode )
	{
	case RC_RIGHT :
		if ( mouse_x < 12 )
		{
			DrawField( mouse_x, mouse_y );
			mouse_x++;
			DrawMouse();
			locked=1;
		}
		break;
	case RC_LEFT :
		if ( mouse_x > 3 )
		{
			DrawField( mouse_x, mouse_y );
			mouse_x--;
			DrawMouse();
			locked=1;
		}
		break;
	case RC_DOWN :
		if ( mouse_y < 12 )
		{
			DrawField( mouse_x, mouse_y );
			mouse_y++;
			DrawMouse();
			locked=1;
		}
		break;
	case RC_UP :
		if ( mouse_y > 3 )
		{
			DrawField( mouse_x, mouse_y );
			mouse_y--;
			DrawMouse();
			locked=1;
		}
		break;
	case RC_OK :
		SelectField( mouse_x, mouse_y );
		locked=1;
		break;
	case RC_BLUE :
		Flag( mouse_x, mouse_y );
		locked=1;
		break;
	}
}
