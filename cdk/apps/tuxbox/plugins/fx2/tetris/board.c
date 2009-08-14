/*
** initial coding by fx2
*/


#include <stdio.h>

#include <draw.h>
#include <sys/time.h>
#include <rcinput.h>
#include <colors.h>
#include <pics.h>

#if 0
#define	STATUS_X		5
#define STATUS_Y		5
#define LOGO_X			650
#define LOGO_Y			0
#else
#define	STATUS_X		80
#define STATUS_Y		50
#define LOGO_X			500
#define LOGO_Y			30
#endif

extern	double	sqrt( double in );

extern	int		doexit;

extern	unsigned short	realcode;
extern	unsigned short	actcode;

		long	score = 0;
static	int		level = 1;
static	int		puz_x = 5;
static	int		puz_y = 0;
static	int		actpuz = 0;
static	int		nextpuz = 1;

static	int		myrand( int idx )
{
	struct timeval tv;
	gettimeofday(&tv,0);

	return tv.tv_usec % idx;
}

static	unsigned char	puz_0[] = "    X   XXX     ";
static	unsigned char	puz_1[] = "     X  XXX     ";
static	unsigned char	puz_2[] = "      X XXX     ";
static	unsigned char	puz_3[] = "    XX   XX     ";
static	unsigned char	puz_4[] = "     XX XX      ";
static	unsigned char	puz_5[] = "    XXXX        ";
static	unsigned char	puz_6[] = "     XX  XX     ";

/* magenta, blue, cyan, green, red, yellow, orange */
static	unsigned char	*puz[] = { puz_0, puz_1, puz_2, puz_3, puz_4, puz_5, puz_6 };
static	unsigned char	puzc[] = { 30, 80, 50, 60, 40, 70, 90 };

static	void	DrawScore( void )
{
	char	tscore[ 64 ];
	int		x;

	sprintf(tscore,"%ld",score);
	x=FBDrawString( LOGO_X-5, 340, 64, tscore, WHITE, BLACK );
	FBFillRect( LOGO_X+x-5, 340, 20, 64, BLACK );
}

static	void	DrawNextPuz( void )
{
	int				x;
	int				y;
	unsigned char	*p = puz[nextpuz];

	for( y=0; y<4; y++ )
	{
		for( x=0; x<4; x++, p++ )
		{
			switch( *p )
			{
			case 'X' :
				FBCopyImageCol( (x+16)*32,(y+5)*32,32,32,puzc[nextpuz],puzdata);
				break;
			default :
				FBFillRect( (x+16)*32,(y+5)*32,32,32,BLACK);
				break;
			}
		}
	}
}

void	DrawBoard( void )
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
				FBCopyImage( x*32, y*32, 32, 32, wall );
				break;
			case ' ' :
				FBFillRect( x*32, y*32, 32, 32, BLACK );
				break;
			default :
				FBFillRect( x*32, y*32, 32, 32, RED );
				break;
			}
		}
	}
	FBDrawFx2Logo(LOGO_X,LOGO_Y);
	FBDrawString(LOGO_X-5,280,64,"Score",WHITE,0);
	DrawNextPuz();
}

static	void	DrawPuz( void )
{
	int				x;
	int				y;
	unsigned char	*p;

	for( y=0; y<5; y++ )
	{
		if ( puz_y+y>16 )
			break;
		p = maze + puz_x - 1 + (puz_y+y) * MAZEW;
		for( x=-1; x<5; x++, p++ )
		{
			if ( puz_x+x > 12 )
				break;
			if ( *p == 'X' )
				FBCopyImageCol( (x+puz_x)*32, (y+puz_y)*32, 32, 32,
					puzc[actpuz], puzdata );
			if ( *p == ' ' )
				FBFillRect( (x+puz_x)*32, (y+puz_y)*32, 32, 32, BLACK );
		}
	}
}

int	PutPuzIntoBoard( void )
{
	int				x;
	int				y;
	unsigned char	*p = puz[actpuz];

	for( y=0; y<4; y++ )
	{
		for( x=0; x<4; x++, p++ )
		{
			if ( *p == 'X' )
			{
				if ( maze[ (x+puz_x) + (y+puz_y)*MAZEW ] != ' ' )
					return 0;
				maze[ (x+puz_x) + (y+puz_y)*MAZEW ] = 'X';
			}
		}
	}
	DrawPuz();
	DrawNextPuz();
	return 1;
}

int	NextItem( void )
{
	unsigned char	*p = maze;
	int		x;
	int		y;

	for( y=0; y<MAZEH; y++ )
		for( x=0; x<MAZEW; x++, p++ )
			if ( *p == 'X' )
				*p = 'B';
	puz_x = 6;
	puz_y = 0;
	actpuz = nextpuz;
	nextpuz = myrand(7);
	if ( !PutPuzIntoBoard() )
		return 0;

	score += level;
	DrawScore();

	return 1;
}

static	int	MovePuz( int dist )
{
	int				x;
	int				y;
	unsigned char	*p;

	for( y=0; y<4; y++ )
	{
		p = maze + puz_x + (puz_y+y) * MAZEW;
		for( x=0; x<4; x++, p++ )
		{
			if ( *p == 'X' )
			{
				if ( *(p+dist) == 'X' )
					continue;
				if ( *(p+dist) != ' ' )
					return 0;
			}
		}
	}

	if ( dist < 0 )
	{
		for( y=0; y < 4 ; y++ )
		{
			p = maze + puz_x + (puz_y+y) * MAZEW;
	
			for( x=0; x<4; x++, p++ )
			{
				if ( *p == 'X' )
				{
					*(p+dist) = 'X';
					*p=' ';
				}
			}
		}
	}
	else
	{
		for( y=0; y < 4 ; y++ )
		{
			p = maze + puz_x + 3 + (puz_y+y) * MAZEW;
	
			for( x=0; x<4; x++, p-- )
			{
				if ( *p == 'X' )
				{
					*(p+dist) = 'X';
					*p=' ';
				}
			}
		}
	}

	DrawPuz( );
	puz_x += dist;

	return 1;
}

static	int	MoveDown( void )
{
	int				x;
	int				y;
	unsigned char	*p;

	for( y=0; y<4; y++ )
	{
		p = maze + puz_x + (puz_y+y) * MAZEW;
		for( x=0; x<4; x++, p++ )
		{
			if ( *p == 'X' )
			{
				if ( *(p+MAZEW) == 'X' )
					continue;
				if ( *(p+MAZEW) != ' ' )
					return 0;
			}
		}
	}

	for( y=0; y < 4 ; y++ )
	{
		p = maze + puz_x + (puz_y+3-y) * MAZEW;

		for( x=0; x<4; x++, p++ )
		{
			if ( *p == 'X' )
			{
				*(p+MAZEW) = 'X';
				*p=' ';
			}
		}
	}
	DrawPuz();
	puz_y++;

	return 1;
}

static	void	RotateRight( void )
{
	unsigned char	bu[16];
	unsigned char	bu2[16];
	int				x;
	int				y;
	unsigned char	*s, *t;

/* get */
	for( y=0, t=bu; y < 4; y++ )
		for( x=0, s=maze+(puz_y+y)*MAZEW+puz_x; x < 4; x++, s++, t++ )
			*t = *s == 'X' ? 'X' : ' ';
/* rotate */
	for( y=0, s=bu, t=bu2+3; y < 4; y++, t=bu2+3-y )
		for( x=0; x < 4; x++, s++, t+=4 )
			*t = *s;

/* test */
	for( y=0,s=bu2; y < 4; y++ )
		for( x=0, t=maze+(puz_y+y)*MAZEW+puz_x; x < 4; x++, s++, t++ )
			if (( *s != ' ' ) && (( *t != ' ' ) && ( *t != 'X' )))
					return;

/* insert */
	for( y=0,s=bu2; y < 4; y++ )
	{
		for( x=0, t=maze+(puz_y+y)*MAZEW+puz_x+x; x < 4; x++, s++, t++ )
		{
			if ( *s != ' ' )
				*t = *s;
			else
			{
				if ( *t == 'X' )
					*t = ' ';
			}
		}
	}
	DrawPuz();
}

static	void	RotateLeft( void )
{
	unsigned char	bu[16];
	unsigned char	bu2[16];
	int				x;
	int				y;
	unsigned char	*s, *t;

/* get */
	for( y=0, t=bu; y < 4; y++ )
		for( x=0, s=maze+(puz_y+y)*MAZEW+puz_x; x < 4; x++, s++, t++ )
			*t = *s == 'X' ? 'X' : ' ';

/* rotate */
	for( y=0, s=bu, t=bu2+12; y < 4; y++, t=bu2+12+y )
		for( x=0; x < 4; x++, s++, t-=4 )
			*t = *s;

/* test */
	for( y=0,s=bu2; y < 4; y++ )
		for( x=0, t=maze+(puz_y+y)*MAZEW+puz_x; x < 4; x++, s++, t++ )
			if (( *s != ' ' ) && (( *t != ' ' ) && ( *t != 'X' )))
					return;

/* insert */
	for( y=0,s=bu2; y < 4; y++ )
	{
		for( x=0, t=maze+(puz_y+y)*MAZEW+puz_x+x; x < 4; x++, s++, t++ )
		{
			if ( *s != ' ' )
				*t = *s;
			else
			{
				if ( *t == 'X' )
					*t = ' ';
			}
		}
	}
	DrawPuz();
}

void	MoveSide( void )
{
static int blocker = 0;
static	short last = 0;

	if ( realcode == 0xee )
		blocker=0;

	if ( blocker && ( actcode == last ))
		return;

	last=actcode;

	switch( actcode )
	{
	case RC_LEFT :
		MovePuz(-1);
		break;
	case RC_RIGHT :
		MovePuz(+1);
		break;
	case RC_DOWN :
		MoveDown();
		break;
	case RC_UP :
		RotateLeft();
		blocker =1;
		break;
	case RC_OK :
		RotateRight();
		blocker =1;
		break;
	}

	actcode = 0xee;
}

int	FallDown( void )
{
static int blocker = 0;

	blocker++;
	if ( blocker < 30 )
		return 1;
	blocker=0;

	return MoveDown();
}

static	void	DelLines( int li1, int li2 )
{
	int				lines = li2 ? li1 - li2 + 1 : 1;
	int				ny = li2 ? li2 : li1;
	int				x;
	int				y;
	unsigned char	*s;
	unsigned char	*t;

	FBBlink( 3*32, ny*32, 10*32, lines*32, 2 );

	for( y=ny-1; y>0; y-- )
		for( x=3, s=maze+y*MAZEW+x, t=s+(lines*MAZEW); x<13; x++, s++, t++ )
			*t = *s;

	FBMove( 3*32, 2*32, 3*32, (2+lines)*32, 10*32, (ny-2)*32 );
}

void	RemoveCompl( void )
{
	int				y;
	int				x;
	unsigned char	*s;
	int				li1=0;
	int				li2=0;
	int				lines = 0;

	for( y=15; y>1; y-- )
	{
		for( x=3, s = maze + y*MAZEW + x; x < 13; x++, s++ )
		{
			if ( *s == ' ' )
				break;
		}
		if ( x != 13 )
		{
			if ( li1 )
			{
				DelLines( li1, li2 );
				y=li1+1;
			}
			li1=0;
			li2=0;
			continue;
		}
		lines++;
		if ( li1 )
			li2=y;
		else
			li1=y;
	}
	if ( li1 )
		DelLines( li1, li2 );

	if ( lines )
	{
		/* points for lines */
		score += ((level+1)*lines);

		level+=(lines-1);
	}
}

void	BoardInitialize( void )
{
	int				x;
	int				y;
	unsigned char	*p = maze;

	for( y = 0; y < MAZEH; y++ )
	{
		for( x = 0; x < MAZEW; x++, p++ )
		{
			if ( *p != '#' )
				*p = ' ';
		}
	}
	actcode=0xee;
	score=0;
	puz_x = 6;
	puz_y = 0;
	level = 1;
}

void	DrawGameOver( void )
{
	FBDrawString( 190, 290, 64, "Game Over", RED, 0 );
}
