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
#include <solboard.h>
#include <colors.h>
#include <pig.h>
#include <plugin.h>
#include <fx2math.h>

//#define SOLBOARD_DEBUG

extern	int	doexit;
extern	int	debug;
extern	unsigned short	actcode;
extern	unsigned short	realcode;

/**
 * Sets framebuffer colors according to definitions in colors.h
 */
static	void	setup_colors( void )
{
	FBSetColor( YELLOW, 255, 255, 30 );
	FBSetColor( GREEN, 30, 255, 30 );
	FBSetColor( STEELBLUE, 30, 30, 180 );
	FBSetColor( BLUE, 130, 130, 255 );
	FBSetColor( GRAY, 130, 130, 130 );
	FBSetColor( DARK, 30, 30, 30 );
	FBSetColor( ORANGE, 255, 199, 15);

	FBSetupColors( );
}

/**
 * The main loop
 * @param fdfb 1 if framebuffer should be used, 0 otherwise
 * @param fdrc 1 if remote control should be used, 0 otherwise
 * @param fdlcd 1 if lcd should be used, 0 otherwise
 * @param cfgfile the config file
 * @return 0
 */
int sol_exec( int fdfb, int fdrc, int fdlcd, char *cfgfile )
{
	struct timeval tv;
	int x;

	if ( FBInitialize( 720, 576, 8, fdfb ) < 0 )
		return -1;

	setup_colors();

	if ( RcInitialize( fdrc ) < 0 )
		return -1;
	// 228, 187
	Fx2ShowPig( 420, 300, 228, 187 );

	while( doexit != 3 )
	{
#ifdef SOLBOARD_DEBUG
	  printf("somain: board init\n");
#endif
		BoardInitialize();
		DrawBoard();

		doexit=0;
		while( !doexit )
		{
		  tv.tv_sec = 0;
		  tv.tv_usec = 10000;
		  x = select( 0, 0, 0, 0, &tv );		/* 100ms pause */
		  
		  RcGetActCode( );
		  MoveMouse();
#ifdef USEX
		  FBFlushGrafic();
#endif
		}
		
		if ( doexit != 3 )
		  {
#ifdef SOLBOARD_DEBUG
		    printf("somain: actcode=0xee\n");
#endif
		    
		    actcode=0xee;
		    if ( doexit ==2 ) {
#ifdef SOLBOARD_DEBUG
		      printf("somain: before DrawScore\n");
#endif
		      DrawScore();
		    }
		    else {
#ifdef SOLBOARD_DEBUG
		      printf("somain: //DrawGameOver()\n");
#endif
		      
				//DrawGameOver();
		    }
#ifdef USEX
#ifdef SOLBOARD_DEBUG
		    printf("somain: FBFlushGrafic()\n");
#endif
		    
		    FBFlushGrafic();
#endif
		    doexit=0;
		    while(( actcode != RC_GREEN ) && !doexit )
		      {
#ifdef SOLBOARD_DEBUG
			printf("somain: RC_loop\n");
#endif
			
			tv.tv_sec = 0;
				tv.tv_usec = 100000;
				x = select( 0, 0, 0, 0, &tv );		/* 100ms pause */
				RcGetActCode( );
			}
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

/**
 * This function is called by neutrino/src/gui/gamelist.cpp
 * @param PluginParam the parameters of the plugin
 */
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
	return sol_exec( fd_fb, fd_rc, -1, 0 );
}
