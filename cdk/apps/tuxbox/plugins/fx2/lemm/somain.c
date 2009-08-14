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
#include <plugin.h>
#include <fx2math.h>
#include <sprite.h>

extern	int	doexit;
extern	int	debug;
extern	int	gametime;
extern	int	pices;
extern	int	score;
extern	unsigned short	actcode;
extern	unsigned short	realcode;

extern	void	RemovePics( void );
extern	int		InitLemm( void );
extern	void	InitLevel( void );
extern	void	PicSetupColors( void );

/* special */
extern	void	AnimateDeko( void );
extern	void	UnanimatedDeko( void );
extern	void	RunKey( void );
extern	void	RunLemm( void );
extern	void	RemoveBg( void );
extern	void	SoundPlay( int pnr );
extern	int		dblInit( void );
extern	void	dblFree( void );
extern	void	dblDrawFrame( int all );

static	void	setup_colors( void )
{
	FBSetColor( YELLOW, 255, 255, 20 );
	FBSetColor( GREEN, 20, 255, 20 );
	FBSetColor( STEELBLUE, 0, 0, 51 );
	FBSetColor( BLUE, 30, 30, 220 );
	FBSetColor( GRAY, 130, 130, 130 );
	FBSetColor( DARK, 60, 60, 60 );
	FBSetColor( 150, 0, 0, 51 );		// blocker

	PicSetupColors();

	FBSetupColors( );
}

int lemmings_exec( int fdfb, int fdrc, int fdlcd, char *cfgfile )
{
	struct timeval	tv;
	int				x;
	int				rc=0;

	if ( FBInitialize( 720, 576, 8, fdfb ) < 0 )
		return -1;

	setup_colors();

	if ( dblInit() < 0 )
		return -1;

	if ( RcInitialize( fdrc ) < 0 )
		return -1;

	while( doexit != 3 )
	{
		if ( InitLemm() != 0 )
			break;

		InitLevel();

		dblDrawFrame( 1 );

		Fx2ShowPig( 430, 358, 240, 188 );

		doexit=0;
		while( !doexit )
		{
			tv.tv_sec = 0;
			tv.tv_usec = 80000;
			x = select( 0, 0, 0, 0, &tv );		/* 50ms pause */
			RcGetActCode( );
			RunKey();
			UnanimatedDeko();
			AnimateDeko();
			RunLemm();
			dblDrawFrame( 0 );
#ifdef USEX
			FBFlushGrafic();
#endif
		}

		rc=doexit;

		FreeSprites();

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
			}
		}

		if ( rc == 2 )
			break;
	}

	SoundPlay(-2);	// stop thread

	dblFree();
	RemoveBg();
	RemovePics();
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
	return lemmings_exec( fd_fb, fd_rc, -1, 0 );
}
