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

extern	int		doexit;

extern	unsigned short	actcode;
extern	unsigned short	realcode;
#ifdef USEX
static	unsigned long	oran=0;
#endif

static	int		houses_x[10];

static	int		player=0;
static	int		cfly=120;
static	int		last_x=120;
static	int		use_comp=1;

static	char	player2[] = "Player 2";
static	char	compi[] = "dbox";
static	char	*p2name=compi;

static	int		myrand( int idx )
{
	struct timeval tv;
	gettimeofday(&tv,0);

#ifdef USEX
	oran=tv.tv_usec;
	tv.tv_sec = 0;
	tv.tv_usec &= 0x0fff;
	select( 0, 0, 0, 0, &tv );
	return oran % idx;
#endif

	return tv.tv_usec % idx;
}

static	int		cang[]= { 45, 45 };
static	int		can_x=0;
static	int		can_y=0;

static	int		can_x_tab[] = { 12, 12, 12, 12,		// 39, 41
								11, 11, 11, 11,		// 43, 45
								10, 10, 10, 10,		// 47, 49
								10, 10, 9, 9,		// 51, 53
								9, 9, 8, 8 };		// 55, 57
static	int		can_y_tab[] = { 485, 485, 485, 485,		// 39, 41
								485, 485, 484, 484,		// 43, 45
								484, 484, 483, 483,		// 47, 49
								483, 483, 483, 483,		// 51, 53
								482, 482, 482, 482 };	// 55, 57

static	void	DrawCanon( int visible )
{
	can_x=can_x_tab[ cang[player] - 39 ];
	can_y=can_y_tab[ cang[player] - 39 ];

	if ( player )
	{
		can_x = 672 - can_x;
		FBDrawLine( 672, 496, can_x, can_y, visible?WHITE:AIR);
		FBDrawLine( 672, 496+1, can_x, can_y+1, visible?WHITE:AIR);
		FBDrawLine( 672-1, 496, can_x-1, can_y, visible?WHITE:AIR);
		FBDrawLine( 672-1, 496+1, can_x-1, can_y+1, visible?WHITE:AIR);
	}
	else
	{
		can_x += 48;
		FBDrawLine( 48, 496, can_x, can_y, visible?WHITE:AIR);
		FBDrawLine( 48, 496+1, can_x, can_y+1, visible?WHITE:AIR);
		FBDrawLine( 48+1, 496, can_x+1, can_y, visible?WHITE:AIR);
		FBDrawLine( 48+1, 496+1, can_x+1, can_y+1, visible?WHITE:AIR);
	}
}

static	void	GeneratePlayer( int pnum )
{
	int		x;
	int		i;
	int		k;
	int		p;

	for (i=0; i<5; i++ )
	{
		while(1)
		{
			x=myrand(180)+80;
			if ( pnum )
				x+=360;
			else
				x=360-x;

			x-=8;
			houses_x[pnum*5+i] = x;
			for(k=0; k<i; k++ )
			{
				p=houses_x[pnum*5+k];
				if ( x > p + 16 )
					continue;
				if ( x < p - 16 )
					continue;
				break;
			}
			if ( k==i )
				break;
		}
		FBCopyImage( x, 496, 16, 16, house_pic );
	}
}

void	TankInitialize( void )
{
	int		y;

	FBFillRect( 0, 0, 720,576, AIR );

/* earth */
	FBFillRect( 0, 512, 720, 64, BROWN );

	player=0;
	cfly=120;
	GeneratePlayer( 0 );
	GeneratePlayer( 1 );

	cang[0]=45;
	cang[1]=45;

/* label */
	FBDrawString( 500, 64, 64, "Player 1", WHITE, 0 );
	FBDrawString( 500, 128, 64, p2name, SAIR, 0 );

/* middle */
	FBFillRect( 300, 500, 120, 12, BROWN );
	FBFillRect( 330, 400, 60, 100, BROWN );
	FBFillRect( 340, 370, 40, 30, BROWN );
	FBFillRect( 340, 0, 40, 230, BROWN );

/* tank */
	FBFillRect( 32, 496, 24, 16, GREEN );
	FBFillRect( 664, 496, 24, 16, GREEN );

	FBFillRect( 0, 480, 32, 32, BROWN );
	FBFillRect( 688, 480, 32, 32, BROWN );

	for( y=0; y<7; y++ )
	{
		FBFillRect( 32, 498+y*2, (y+1)*2, 2, BROWN );
		FBFillRect( 688-(y+1)*2, 498+y*2, (y+1)*2, 2, BROWN );
	}

/* canon */
	DrawCanon(1);

	actcode=0xee;
}

static	void	Flame( int x )
{
	struct timeval	tv;
	int				i;
	int				k;
	unsigned char	*flame[] = { flame1_pic, flame2_pic, flame3_pic };

	for( i=0; i<10; i++ )
	{
		k=myrand(3);
		FBCopyImage( x, 496, 16, 16, flame[k] );
#ifdef USEX
		FBFlushGrafic();
#endif
		tv.tv_usec = myrand(100000)+50000;
		tv.tv_sec=0;
		select(0,0,0,0,&tv);
	}
	FBFillRect( x, 496, 16, 16, AIR );
}

static	int	Fly( float ang_x, float ang_y )
{
	struct timeval	tv;
	int				x=0;
	int				y=0;
	int				xo=0;
	int				yo=0;
	int				xo2=0;
	int				yo2=0;
	int				i;
	float			speedy=(ang_y+ang_x)/80;
	float			speedx=(ang_y+ang_x)/160;
	unsigned char	px;
	int				inair=0;

	for( i=1; !doexit; i++ )
	{
		if ( ( can_x+x >= 720 ) || ( can_x+x<0 ))
			break;
		if ( xo2 && yo2 )
		{
			FBFillRect( can_x+xo2, can_y-yo2, 2, 2, AIR );
#ifdef USEX
		FBFlushGrafic();
#endif
		}
		if ( xo && yo )
		{
			FBFillRect( can_x+xo, can_y-yo, 2, 2, WHITE );
#ifdef USEX
		FBFlushGrafic();
#endif
			xo2=xo;
			yo2=yo;
		}
		if (( y < 480 ) && ( y > -32 ))
		{
			px=FBGetPixel( can_x+x, can_y-y );
			if ( !inair )
			{
				if ( px == AIR )
					inair=1;
			}
			else
			{
				if (( px != AIR ) && ((x!=xo) || (y!=yo)) &&
					((x!=xo+1) || (y!=yo+1)) &&
					((x!=xo-1) || (y!=yo-1)) )
				{
					FBFillRect( can_x+x, can_y-y, 2, 2, AIR );
#ifdef USEX
					FBFlushGrafic();
#endif
					break;
				}
			}

			if ( y > -32 )
				FBFillRect( can_x+x, can_y-y, 2, 2, RED );
#ifdef USEX
			FBFlushGrafic();
#endif
			xo=x;
			yo=y;
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			select( 0, 0, 0, 0, &tv );
		}
		else
		{
			xo=0;
			yo=0;
		}
		if ( y <= -32 )
			break;
		if ( player )
			x-=ang_x;
		else
			x+=ang_x;
		y+=ang_y;
		ang_y-=speedy;
		ang_x-=speedx;
		if ( ang_y < 0 )
			speedx*=1.04;
		if ( ang_x < 0 )
			ang_x=0;
		RcGetActCode( );
	}
	if ( xo2 && yo2 )
	{
		FBFillRect( can_x+xo2, can_y-yo2, 2, 2, AIR );
#ifdef USEX
		FBFlushGrafic();
#endif
	}
	if ( xo && yo )
	{
		FBFillRect( can_x+xo, can_y-yo, 2, 2, AIR );
#ifdef USEX
		FBFlushGrafic();
#endif
	}
	last_x=x+can_x;
	if ( y <= 0 )
	{
		for( i=0; i<10; i++ )
		{
			if ( !houses_x[i] )
				continue;
			if ((houses_x[i] <= x+can_x ) &&
				(houses_x[i]+16 > x+can_x ))
			{
				Flame( houses_x[i] );
				houses_x[i]=0;
				return 1;
			}
		}
	}
	return 0;
}

static	void	RunToFly( int speed )
{
	int				o=0;
	int				y=0;
	char			won[64];

	if ( speed < 18 )
		speed=18;
	o = speed/4;
	y = o*cang[player]/90;
	if ( Fly( o-y , y ) )
	{
		for( o=0; o<5; o++ )
			if ( houses_x[o] )
				break;
		if ( o==5 )
		{
			sprintf(won,"%s won the game",p2name);
			FBDrawString( 200, 360, 64, won, RED, 0 );
			doexit=4;
		}
		else
		{
			for( o=5; o<10; o++ )
				if ( houses_x[o] )
					break;
			if ( o==10 )
			{
				FBDrawString( 200, 360, 64, "Player 1 won the game", RED, 0 );
				doexit=4;
			}
		}
	}

	while( realcode != 0xee )
		RcGetActCode( );
	actcode=0xee;
}

static	void	Bomb( void )
{
	struct timeval	tv;
	int				speed=0;

	while( realcode != 0xee )
		RcGetActCode( );
	actcode=0xee;

	FBFillRect( 300, 515, 121, 19, BLACK );
	FBDrawRect( 299, 514, 122, 20, WHITE );
	FBDrawRect( 298, 513, 124, 22, WHITE );

/* 25 % */
	FBDrawVLine( 301+118-30, 515, 4, WHITE );
	FBDrawVLine( 301+118-29, 515, 4, WHITE );
	FBDrawVLine( 301+118-30, 529, 4, WHITE );
	FBDrawVLine( 301+118-29, 529, 4, WHITE );

/* 50 % */
	FBDrawVLine( 301+118-60, 515, 4, WHITE );
	FBDrawVLine( 301+118-59, 515, 4, WHITE );
	FBDrawVLine( 301+118-60, 529, 4, WHITE );
	FBDrawVLine( 301+118-59, 529, 4, WHITE );

/* 75 % */
	FBDrawVLine( 301+118-90, 515, 4, WHITE );
	FBDrawVLine( 301+118-89, 515, 4, WHITE );
	FBDrawVLine( 301+118-90, 529, 4, WHITE );
	FBDrawVLine( 301+118-89, 529, 4, WHITE );

	actcode=0xee;
	while( speed < 118 && (actcode != RC_OK) && !doexit )
	{
		FBDrawVLine( 301+118-speed, 516, 16, GREEN );
		FBDrawVLine( 301+117-speed, 516, 16, GREEN );

		if ( speed == 30 )
		{
/* 25 % */
			FBDrawVLine( 301+118-30, 516, 3, BLACK );
			FBDrawVLine( 301+118-29, 516, 3, BLACK );
			FBDrawVLine( 301+118-30, 529, 3, BLACK );
			FBDrawVLine( 301+118-29, 529, 3, BLACK );
		}
		if ( speed == 60 )
		{
/* 50 % */
			FBDrawVLine( 301+118-60, 516, 3, BLACK );
			FBDrawVLine( 301+118-59, 516, 3, BLACK );
			FBDrawVLine( 301+118-60, 529, 3, BLACK );
			FBDrawVLine( 301+118-59, 529, 3, BLACK );
		}
		if ( speed == 90 )
		{
/* 50 % */
			FBDrawVLine( 301+118-90, 516, 3, BLACK );
			FBDrawVLine( 301+118-89, 516, 3, BLACK );
			FBDrawVLine( 301+118-90, 529, 3, BLACK );
			FBDrawVLine( 301+118-89, 529, 3, BLACK );
		}

#ifdef USEX
		FBFlushGrafic();
#endif
		actcode=0xee;
		RcGetActCode( );

		if ( actcode == 0xee )
		{
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			select( 0, 0, 0, 0, &tv );
			speed+=2;
		}
	}
	RunToFly( speed );
}

void	Play( void )
{
	int		i;

	if ( use_comp )
	{
		if ( !player && (actcode == 0xee ))
			return;
	}
	else
	{
		if ( actcode == 0xee )
			return;
	}
	switch( actcode )
	{
	case RC_RED :
		FBDrawString( 500, 128, 64, p2name, AIR, 0 );
		if ( use_comp )
		{
			use_comp=0;
			p2name=player2;
		}
		else
		{
			use_comp=1;
			p2name=compi;
		}
		FBDrawString( 500, 128, 64, p2name, player ? WHITE: SAIR, 0 );
		break;
	case RC_UP :
		if ( cang[player] < 55 )
		{
			DrawCanon(0);
			cang[player]+=2;
			DrawCanon(1);
		}
		break;
	case RC_DOWN :
		if ( cang[player] > 40 )
		{
			DrawCanon(0);
			cang[player]-=2;
			DrawCanon(1);
		}
		break;
	case RC_OK :
	case 0xee :
		if ( use_comp && player ) // computer
		{
			RunToFly( cfly );
			if ( last_x < 340 )
			{
				for( i=0; i<5; i++ )
					if (houses_x[i] > last_x )
						break;
			}
			else
				i=0;
			cfly -= 4;
			if (( cfly < 65 ) || ( i==5 ))
			{
				cfly=120;
				DrawCanon(0);
				if ( cang[player] > 40 )
					cang[player]-=2;
				else
					cang[player] = 55;
				DrawCanon(1);
			}
		}
		else
		{
			Bomb();
		}
		if ( !doexit )
		{
			DrawCanon(0);
			FBDrawString( 500, player?128:64, 64,
					player ? p2name : "Player 1", SAIR, 0 );
			player=!player;
			DrawCanon(1);
			FBDrawString( 500, player?128:64, 64,
					player ? p2name : "Player 1", WHITE, 0 );
		}
		break;
	}
	return;
}
