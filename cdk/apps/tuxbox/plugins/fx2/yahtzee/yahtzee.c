/*
** initial coding by fx2
*/


#include <fcntl.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <colors.h>
#include <draw.h>
#include <pics.h>
#include <pig.h>
#include <rcinput.h>

#include <config.h>

#define LOGO_X			600
#define LOGO_Y			30

extern	int		doexit;

extern	unsigned short	actcode;
extern	unsigned short	realcode;

typedef struct _HScore
{
	char	name[12];
	long	points;
	char	flag;
} HScore;

typedef struct _Player
{
	char	name[16];
	int		nums[17];
} Player;

static	int		numplayers=0;
static	int		actplayer=0;
static	Player	player[9];
static	char	pig_dis = 0;

static	int		pig_x[] = { 450, 450, 150, 150 };

static	int		myrand( int idx )
{
	struct timeval	tv;
	gettimeofday(&tv,0);

	return tv.tv_usec % idx;
}

static	void	DoYellow( void )
{
	if ( pig_dis )
	{
		Fx2ShowPig( pig_x[actplayer], 300, 176, 144 );
		pig_dis=0;
	}
	else
	{
		Fx2StopPig();
		pig_dis=1;
	}
}

void	EnterPlayer( void )
{
	struct timeval	tv;
	int				y;
	int				x;
	int				i;
	char			cnum[2] = { 48, 0 };
	char			txt[10];

	/* clear screen */
	for( y=0; y < 576; y+=4 )
	{
		FBFillRect( 0, y, 720, 4, BLACK );
#ifdef USEX
		FBFlushGrafic();
#endif
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		select( 0, 0, 0, 0, &tv );		/* 1ms pause */
	}
	FBDrawString( 150, 64, 48, "Yahtzee presented by", WHITE, 0 );
	FBDrawFx2Logo( 200, 112 );
	/* first enter num players */
	if ( numplayers )
	{
		x=FBDrawString( 150,200,64,"same player ? : OK / BLUE",RED,0);
#ifdef USEX
		FBFlushGrafic();
#endif
		while( realcode != 0xee )
			RcGetActCode();

		actcode=0xee;
		while( !doexit )
		{
			RcGetActCode();
			if ( actcode == RC_YELLOW )
				DoYellow();

			if (( actcode == RC_OK ) || ( actcode == RC_BLUE ))
				break;
			tv.tv_sec = 0;
			tv.tv_usec = 100000;
			select( 0, 0, 0, 0, &tv );
		}
		if ( actcode == RC_OK )
			return;
		FBFillRect( 150,200,x,64,BLACK);
	}
	x=FBDrawString( 100,232,64,"how many player (1-4): ",RED,0);
#ifdef USEX
	FBFlushGrafic();
#endif

	while( !doexit )
	{
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		select( 0, 0, 0, 0, &tv );
		actcode=0xee;
		RcGetActCode();
		if (( actcode >= 1 ) && ( actcode <= 4 ))	/* RC_1 .. RC_4 */
		{
			numplayers=actcode;
			cnum[0]=actcode+48;
			break;
		}
	}
	if ( doexit )
		return;

	FBDrawString( 100+x,232,64,cnum,WHITE,0);

	for( i=0; i < numplayers; i++ )
	{
		FBFillRect(150,300,570,64,BLACK);
		sprintf(txt,"%d. name : ",i+1);
		x=FBDrawString( 150,300,64,txt,WHITE,0);
		strcpy(player[i].name,FBEnterWord(150+x,300,64,9,WHITE));
	}
}

static	char	wu[5] = { 0, 0, 0, 0, 0 };
static	char	mwu[5] = { 0, 0, 0, 0, 0 };
static	char	ppx[] ={
29,						/* 1 */
5, 52,					/* 2 */
5, 29, 52,				/* 3 */
5, 5, 52, 52,			/* 4 */
5, 5, 29, 52, 52,		/* 5 */
5, 5, 5, 52, 52, 52,	/* 6 */ };
static	char	ppy[] ={
29,						/* 1 */
5, 52,					/* 2 */
5, 29, 52,				/* 3 */
5, 52, 5, 52,			/* 4 */
5, 52, 29, 5, 52,		/* 5 */
5, 29, 52, 5, 29, 52,	/* 6 */ };

static	void	Roll( void )
{
	int				i;
	int				x;
	int				y;
	int				n;
	int				o;
	int				delta;

	x = actplayer ? 60 + actplayer*150 : 280;
	y = 128;

#if 0
	for( i=0; i < 5; i++ )
	{
		FBFillRect( x, y, 64, 64, WHITE );
		y += 68;
	}
#endif

	while( !doexit )
	{
		for( i=0; i < 5; i++ )
		{
			if ( !wu[i] )
			{
				mwu[i] = myrand(6)+1;
				delta=myrand(60);
				for( n=0;n<delta;n++)
					myrand(mwu[i]);
			}
		}
/* draw now */
		y = 128;

		for( i=0; i < 5; i++ )
		{
			if ( !wu[i] )
			{
				for( o=1, n=0; o<mwu[i]; o++ )
					n+=o;

				FBFillRect( x, y, 64, 64, WHITE );
				for( o=0; o<mwu[i]; o++ )
					FBCopyImage( x+ppx[n+o], y+ppy[n+o], 7, 7, point );
			}
			y+=68;
		}
#ifdef USEX
		FBFlushGrafic();
#endif
		actcode=0xee;
		RcGetActCode();
		if ( actcode == RC_YELLOW )
			DoYellow();
		if (( actcode == RC_OK ))
			break;
	}
	while(( realcode != 0xee ) && !doexit )
		RcGetActCode();
	actcode=0xee;
}

static	void	SelectForRoll( void )
{
	struct timeval	tv;
	int				x;
	int				y;
	int				nr = 0;
	int				nnr = 0;
	int				rdy = 0;
	short			last;
	char			blocker=0;

	x = actplayer ? 38 + actplayer*150 : 258;
	y = 128;

	FBDrawLine( x,y+27,x+15,y+32, RED );
	FBDrawLine( x,y+28,x+13,y+32, RED );
	FBDrawLine( x,y+37,x+15,y+32, RED );
	FBDrawLine( x,y+36,x+13,y+32, RED );

	while(( realcode != 0xee ) && !doexit )
		RcGetActCode();
	last=0xee;
	blocker=0;
	while( !doexit && !rdy )
	{
		tv.tv_sec=0;
		tv.tv_usec=100000;
		select(0,0,0,0,&tv);
		actcode=0xee;
		RcGetActCode();
		if ( realcode == 0xee )
			blocker=0;
		if ( blocker && (actcode == last ))
			continue;
		last=actcode;
		blocker=1;
		switch( actcode )
		{
		case RC_YELLOW :
			DoYellow();
			break;
		case RC_UP :
			if ( nr > 0 )
				nnr=nr-1;
			break;
		case RC_DOWN :
			if ( nr < 4 )
				nnr=nr+1;
			break;
		case RC_OK :	/* select */
			if ( wu[nr] )
			{
				wu[nr] = 0;
				y=nr*68+128;
				FBDrawRect( x+22,y,63,63,RED );
				FBDrawRect( x+23,y+1,61,61,RED );
				FBDrawRect( x+24,y+2,59,59,RED );
			}
			else
			{
				wu[nr] = mwu[nr];
				y=nr*68+128;
				FBDrawRect( x+22,y,63,63,WHITE );
				FBDrawRect( x+23,y+1,61,61,WHITE );
				FBDrawRect( x+24,y+2,59,59,WHITE );
			}
			break;
		case RC_BLUE :
			rdy=1;
			break;
		}
		if ( nr != nnr )
		{
			y=nr*68+128;
			FBFillRect( x,y+27,16,12,BLACK );
			nr=nnr;
			y=nr*68+128;
			FBDrawLine( x,y+27,x+15,y+32, RED );
			FBDrawLine( x,y+28,x+13,y+32, RED );
			FBDrawLine( x,y+37,x+15,y+32, RED );
			FBDrawLine( x,y+36,x+13,y+32, RED );
		}
#ifdef USEX
		FBFlushGrafic();
#endif
	}
	y=nr*68+128;
	FBFillRect( x,y+27,16,12,BLACK );
#ifdef USEX
	FBFlushGrafic();
#endif
}

static	int		ScoreOf( int l )
{
	int		i;
	int		val = 0;
	int		max = 0;
	int		cnt[6];

	if ( l < 6 )
	{
		for( i=0; i < 5; i++ )
			if ( mwu[i] == (l+1) )
				val += (l+1);
		return val;
	}
	memset(cnt,0,6*sizeof(int));
	for( i=0; i < 5; i++ )
	{
		cnt[ mwu[i]-1 ]++;
		if ( cnt[ mwu[i]-1 ] > max )
			max = cnt[ mwu[i]-1 ];
		val += mwu[i];
	}
	if ( max==5 && ( player[actplayer].nums[13] > 0 ))
		return 50;
	switch( l )
	{
	case 8 :	/* 3 gl. */
		if ( max >= 3 )
			return val;
		break;
	case 9 :	/* 4 gl. */
		if ( max >= 4 )
			return val;
		break;
	case 10 :	/* full house */
		if ( max == 3 )
		{
			for( i=0; i < 6; i++ )
				if ( cnt[i] == 2 )
					return 25;
		}
		break;
	case 11 :	/* kl.str */
		if (( cnt[0] && cnt[1] && cnt[2] && cnt[3] ) ||
			( cnt[1] && cnt[2] && cnt[3] && cnt[4] ) ||
			( cnt[2] && cnt[3] && cnt[4] && cnt[5] ))
				return 30;
		break;
	case 12 :	/* gr.str */
		if (( cnt[0] && cnt[1] && cnt[2] && cnt[3] && cnt[4] ) ||
			( cnt[1] && cnt[2] && cnt[3] && cnt[4] && cnt[5] ))
				return 40;
		break;
	case 13 :	/* yahtzee */
		if ( max == 5 )
			return 50;
		break;
	case 14 :	/* chance */
		return val;
	}
	return 0;
}

static	void	SelectInBoard( void )
{
	struct timeval	tv;
	time_t			t1;
	time_t			t2=0;
	short			last=0;
	int				nr=0;
	int				nnr=0;
	int				i = actplayer;
	char			cnum[ 64 ];
	char			rdy=0;
	char			blocker;
	int				val=-1;

	t1 = time(0);

	for( nnr=0; nnr<15; nnr++ )
		if ( player[i].nums[nnr] == -1 )
			break;
	if ( nnr == 15 )
		nnr=nr;

	nr = -1;

	while( ! (t2 > t1) )
	{
		while(( realcode != 0xee ) && !doexit )
			RcGetActCode();
		last=0xee;
		t2 = time(0);
	}
	blocker=0;
	while( !doexit && !rdy )
	{
		tv.tv_sec=0;
		tv.tv_usec=100000;
		select(0,0,0,0,&tv);
		actcode=0xee;
		RcGetActCode();
		if ( realcode == 0xee )
			blocker=0;
		if ( blocker && (actcode == last ))
			continue;
		last=actcode;
		blocker=1;
		switch( actcode )
		{
		case RC_YELLOW :
			DoYellow();
			break;
		case RC_UP :
			for( nnr=nr-1; nnr>-1; nnr-- )
				if ( player[i].nums[nnr] == -1 )
					break;
			if ( nnr == -1 )
				nnr=nr;
			break;
		case RC_DOWN :
			for( nnr=nr+1; nnr<15; nnr++ )
				if ( player[i].nums[nnr] == -1 )
					break;
			if ( nnr == 15 )
				nnr=nr;
			break;
		case RC_OK :
			player[actplayer].nums[nr] = val;
			if ( nr < 6 )
			{
				player[actplayer].nums[6] = 0;
				for( i=0; i<6; i++ )
					if ( player[actplayer].nums[i] != -1 )
						player[actplayer].nums[6] += player[actplayer].nums[i];
				if ( player[actplayer].nums[6] > 62 )
					player[actplayer].nums[7] = 35;
			}
			player[actplayer].nums[15] = 0;
			for( i=8; i<15; i++ )
				if ( player[actplayer].nums[i] != -1 )
					player[actplayer].nums[15] += player[actplayer].nums[i];
			player[actplayer].nums[16] = player[actplayer].nums[15]+
										player[actplayer].nums[6]+
										player[actplayer].nums[7];

			rdy=1;
			break;
		}
		if ( nnr != nr )
		{
			if ( nr != -1 )
				FBFillRect( 126+30+i*150, 62+28*nr, 100, 28, BLACK );

			nr=nnr;

			val=ScoreOf(nr);
			if ( val )
				sprintf(cnum," %d ",val);
			else
				sprintf(cnum," --- ");
			FBFillRect( 126+30+i*150, 62+28*nr, 100, 28, WHITE );
			FBDrawString( 126+30+i*150, 62+28*nr, 32, cnum, RED, 0 );
		}
#ifdef USEX
		FBFlushGrafic();
#endif
	}
	actplayer++;
	if ( actplayer == numplayers )
		actplayer=0;
}

void	RunYahtzee( void )
{
	struct timeval	tv;
	int				y;
	int				i;
	int				n;
	char			cnum[ 64 ];

	/* clear screen */
	for( y=0; y < 576; y+=4 )
	{
		FBFillRect( 0, y, 720, 4, BLACK );
#ifdef USEX
		FBFlushGrafic();
#endif
		tv.tv_sec = 0;
		tv.tv_usec = 1000;
		select( 0, 0, 0, 0, &tv );		/* 1ms pause */
	}

	/* init values */
	for( i=0; i < numplayers; i++ )
	{
		for( n=0; n<15; n++ )
			player[i].nums[n]=-1;
		player[i].nums[6] = 0;
		player[i].nums[7] = 0;
		player[i].nums[15] = 0;
		player[i].nums[16] = 0;
	}

	/* draw board */
	FBDrawString( 36, 62, 32, "1",WHITE,0);
	FBDrawString( 36, 90, 32, "2",WHITE,0);
	FBDrawString( 36, 118, 32, "3",WHITE,0);
	FBDrawString( 36, 146, 32, "4",WHITE,0);
	FBDrawString( 36, 174, 32, "5",WHITE,0);
	FBDrawString( 36, 202, 32, "6",WHITE,0);
	FBDrawString( 36, 230, 32, "total",WHITE,0);
	FBDrawString( 36, 258, 32, "Bonus",WHITE,0);
	FBDrawString( 36, 286, 32, "3of a kind",WHITE,0);
	FBDrawString( 36, 314, 32, "4of a kind",WHITE,0);
	FBDrawString( 36, 342, 32, "FullHouse",WHITE,0);
	FBDrawString( 36, 370, 32, "Sm. street",WHITE,0);
	FBDrawString( 36, 398, 32, "Lg. street",WHITE,0);
	FBDrawString( 36, 426, 32, "Yahtzee",WHITE,0);
	FBDrawString( 36, 454, 32, "Chance",WHITE,0);
	FBDrawString( 36, 482, 32, "total",WHITE,0);
	FBDrawString( 36, 510, 32, "Summary",WHITE,0);

	actplayer=0;

	doexit=0;
	while( !doexit )
	{
		FBFillRect( 126, 0, 720-126, 576, BLACK );

		for( i=0; i < numplayers; i++ )
		{
			FBFillRect( 126+i*150, 30, 2, 512, WHITE );
			FBDrawString( 136+i*150, 30, 32, player[i].name,
				WHITE,
				i==actplayer?RED:BLACK );
			for( n=0; n<17; n++ )
			{
				if ( player[i].nums[n] != -1 )
				{
					if ( player[i].nums[n] ||
						( n==6 ) || ( n==15 ) || (n==16))
						sprintf(cnum,"%d",player[i].nums[n]);
					else
						strcpy(cnum," - ");
					FBDrawString( 126+30+i*150, 62+28*n, 32, cnum, WHITE, 0 );
				}
			}
		}
		for( n=0; n<17; n++ )
		{
			if ( player[actplayer].nums[n] == -1 )
				break;
		}
		if ( n==17 )
			return;

		if ( !pig_dis )
			Fx2ShowPig( pig_x[actplayer], 300, 176, 144 );

		memset(wu,0,5);		/* mark all wuerfel for roll */
		memset(mwu,0,5);	/* mark all wuerfel for roll */
		for( n=0; n < 3 && !doexit; n++ )
		{
			Roll();
			memcpy(wu,mwu,5);
			if (( n < 2 ) && !doexit )
				SelectForRoll();
			for( i=0; i < 5 ; i++ )
				if ( wu[i] == 0 )
					break;
			if ( i==5 )
				break;
		}
		if ( doexit )
			return;
		SelectInBoard();

#ifdef USEX
		FBFlushGrafic();
#endif
		while(( realcode != 0xee ) && !doexit )
			RcGetActCode( );
		actcode=0xee;
	}
}

void	DrawWinner( void )
{
	struct timeval	tv;
	char			text[ 64 ];
	int				w=0;
	int				i;
	int				n;
	HScore			hsc[8];
	int				fd;

	Fx2StopPig();

	if ( numplayers == 1 )
	{
		FBFillRect( 180, 180, 400, 180, WHITE );

		sprintf(text,"%d points !",player[0].nums[16] );
		FBDrawString( 300, 232, 64, text, RED, 0 );
	}
	else
	{
		FBFillRect( 180, 180, 450, 180, WHITE );

		for( i=1; i<numplayers; i++ )
			if ( player[i].nums[16] > player[w].nums[16] )
				w=i;
		sprintf(text,"Winner is %s",player[w].name);
		FBDrawString( 200, 200, 64, text, RED, 0 );
		sprintf(text,"with %d points",player[w].nums[16] );
		FBDrawString( 200, 264, 64, text, RED, 0 );
	}
#ifdef USEX
	FBFlushGrafic();
#endif

/* load HScore */
	fd = open( GAMESDIR "/yahtzee.hscore", O_RDONLY );
	if ( fd == -1 )
	{
		mkdir( GAMESDIR, 567 );
		for( i=0;i<8;i++)
		{
			strcpy(hsc[i].name,"-");
			hsc[i].points=0;
			hsc[i].flag=0;
		}
	}
	else
	{
		read( fd, hsc, sizeof(hsc) );
		close(fd);
		for( i=0;i<8;i++)
			hsc[i].flag=0;
	}

/* insert into hscore */
	for( n=0; n < numplayers; n++ )
	{
		for( i=0; i<8; i++ )
			if ( player[n].nums[16] > hsc[i].points )
				break;
		if ( i==8 )
			continue;
		if ( i < 7 )
			memmove( hsc+i+1,hsc+i,sizeof(HScore)*(7-i) );
		strcpy(hsc[i].name,player[n].name);
		hsc[i].points=player[n].nums[16];
		hsc[i].flag=1;
	}
/* save hscore */
	fd = open( GAMESDIR "/yahtzee.hscore", O_CREAT|O_WRONLY, 438 );
	if ( fd != -1 )
	{
		write( fd, hsc, sizeof(hsc) );
		close(fd);
	}

	while( realcode != 0xee )
		RcGetActCode();

	actcode=0xee;
	i=50;
	while( !doexit && ( i>0 ))
	{
		RcGetActCode();
		if ( actcode == RC_OK )
			break;
		tv.tv_sec = 0;
		tv.tv_usec = 200000;
		select( 0, 0, 0, 0, &tv );
		i--;
	}

/* show hscore */
	FBFillRect( 0, 0, 720, 576, BLACK );

	FBDrawString( 220, 32, 64, "HighScore", RED, BLACK );
	for( i=0; i<8; i++ )
	{
		if ( hsc[i].flag )
			FBFillRect( 88, 120+i*48, 8, 8, YELLOW );
		FBDrawString( 100, 100+i*48, 48, hsc[i].name, WHITE, 0 );
		sprintf(text,"%ld",hsc[i].points);
		n = FBDrawString( 400, 100+i*48, 48, text, BLACK, BLACK );
		FBDrawString( 500-n, 100+i*48, 48, text, WHITE, BLACK );
	}
#ifdef USEX
	FBFlushGrafic();
#endif

	while( realcode != 0xee )
		RcGetActCode();

	actcode=0xee;
	i=0;
	while( !doexit )
	{
		RcGetActCode();
		if ( actcode == RC_OK )
			break;
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		select( 0, 0, 0, 0, &tv );
		i++;
		if ( i==50 )
		{
			FBDrawString( 190, 480, 48, "press OK for new game",GRAY,0);
#ifdef USEX
			FBFlushGrafic();
#endif
		}
	}
}
