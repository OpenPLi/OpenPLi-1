/*
** initial coding by fx2
*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <rcinput.h>
#include <draw.h>
#include <pig.h>
#include <colors.h>
#include <maze.h>
#include <plugin.h>
#include <fx2math.h>

extern	int	doexit;
extern	int	debug;
extern	int	gametime;
extern	int	pices;
extern	int	score;
extern	unsigned short	actcode;
extern	unsigned short	realcode;

static	void	setup_colors( void )
{
	FBSetColor( YELLOW, 255, 255, 20 );
	FBSetColor( GREEN, 20, 255, 20 );
	FBSetColor( STEELBLUE, 80, 80, 200 );
	FBSetColor( BLUE, 30, 30, 220 );
	FBSetColor( GRAY, 130, 130, 130 );
	FBSetColor( DARK, 60, 60, 60 );

	FBSetupColors( );
}

int pacman_exec( int fdfb, int fdrc, int fdlcd, char *cfgfile )
{
	struct timeval	tv;
	int				x;
	int				jumplevel=-1;

	if ( FBInitialize( 720, 576, 8, fdfb ) < 0 )
		return -1;

	setup_colors();

	if ( RcInitialize( fdrc ) < 0 )
		return -1;

	InitLevel( 0 );

	while( doexit != 3 )
	{
		MazeInitialize();
		DrawMaze( );	/* 0 = all */
		DrawFill();
		DrawGhosts( );
		DrawPac( );
		MazePig();

		doexit=0;
		while( !doexit )
		{
			tv.tv_sec = 0;
#ifdef HAVE_DREAMBOX_HARDWARE
			tv.tv_usec = 8000;
#else
			tv.tv_usec = 1000;
#endif
			x = select( 0, 0, 0, 0, &tv );		/* 10ms pause */
	
			MovePac( );
			MoveGhosts( );
			DrawGhosts( );
			DrawPac( );
#ifdef USEX
			FBFlushGrafic();
#endif
			RcGetActCode( );
			CheckGhosts( );
		}

		if ( doexit != 3 )
		{
			actcode=0xee;
			if ( score )
				DrawScore();
			if ( !gametime )
				DrawGameOver();
#ifdef USEX
			FBFlushGrafic();
#endif
			doexit=0;
			jumplevel=-1;
			while(( actcode != RC_OK ) && !doexit )
			{
				tv.tv_sec = 0;
				tv.tv_usec = 100000;
				x = select( 0, 0, 0, 0, &tv );		/* 100ms pause */
				RcGetActCode( );
				if ( actcode == RC_HELP )
				{
					while( realcode != 0xee )
						RcGetActCode( );
					actcode=0xee;
					while(( actcode == 0xee ) && !doexit )
					{
						tv.tv_sec = 0;
						tv.tv_usec = 100000;
						x = select( 0, 0, 0, 0, &tv );		/* 100ms pause */
						RcGetActCode( );
					}
					if ( actcode <= RC_9 )
					{
						jumplevel=actcode;
						actcode=RC_OK;
					}
				}
			}
			if ( gametime )
				NextLevel();
			else
				InitLevel( jumplevel );
		}
	}

	Fx2StopPig();

/* fx2 */
/* buffer leeren, damit neutrino nicht rumspinnt */
	realcode = RC_0;
	while( realcode != 0xee )
	{
		tv.tv_sec = 0;
		tv.tv_usec = 300000;
		x = select( 0, 0, 0, 0, &tv );		/* 300ms pause */
		RcGetActCode( );
	}

	RcClose();
	FBClose();

	return 0;
}

int plugin_exec( PluginParam *par )
{
	int		fd_fb=-1;
	int		fd_rc=-1;

	for( ; par; par=par->next )
	{
		if ( !strcmp(par->id,P_ID_FBUFFER) )
			fd_fb=_atoi(par->val);
		else if ( !strcmp(par->id,P_ID_RCINPUT) )
			fd_rc=_atoi(par->val);
		else if ( !strcmp(par->id,P_ID_NOPIG) )
			fx2_use_pig=!_atoi(par->val);
	}
	return pacman_exec( fd_fb, fd_rc, -1, 0 );
}
