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
#include <board.h>
#include <colors.h>
#include <pig.h>
#include <plugin.h>
#include <fx2math.h>

extern	int	doexit;
extern	int	debug;
extern	unsigned short	actcode;
extern	unsigned short	realcode;

static	void	setup_colors( void )
{
	FBSetColor( YELLOW, 255, 255, 30 );
	FBSetColor( GREEN, 30, 255, 30 );
	FBSetColor( STEELBLUE, 30, 30, 180 );
	FBSetColor( BLUE, 130, 130, 255 );
	FBSetColor( GRAY, 130, 130, 130 );
	FBSetColor( DARK, 30, 30, 30 );
	FBSetColor( MAGENTA, 220, 30, 220 );
	FBSetColor( CYAN, 30, 220, 220 );

	FBSetupColors( );
}

int master_exec( int fdfb, int fdrc, int fdlcd, char *cfgfile )
{
	struct timeval	tv;
	int				x;

	if ( FBInitialize( 720, 576, 8, fdfb ) < 0 )
		return -1;

	setup_colors();

	if ( RcInitialize( fdrc ) < 0 )
		return -1;


	while( doexit != 3 )
	{
		MasterInitialize();

#ifdef USEX
		FBFlushGrafic();
#endif
		Fx2ShowPig( 430, 355, 176, 144 );

		doexit=0;
		while( !doexit )
		{
			tv.tv_sec = 0;
			tv.tv_usec = 200000;
			select( 0, 0, 0, 0, &tv );

			actcode=0xee;
			RcGetActCode( );
			Play();
#ifdef USEX
			FBFlushGrafic();
#endif
			while( realcode != 0xee )
				RcGetActCode( );
		}
		if ( doexit == 4 )		// level changed
		{
			doexit=0;
			continue;
		}

		if ( doexit != 3 )
		{
			actcode=0xee;
#ifdef USEX
			FBFlushGrafic();
#endif
			doexit=0;
			while(( actcode != RC_OK ) && !doexit )
			{
				tv.tv_sec = 0;
				tv.tv_usec = 100000;
				x = select( 0, 0, 0, 0, &tv );		/* 100ms pause */
				RcGetActCode( );
				if (( actcode > 0 ) && (actcode < 5 ))
				{
					Play();
					doexit=1;
				}
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
	return master_exec( fd_fb, fd_rc, -1, 0 );
}
