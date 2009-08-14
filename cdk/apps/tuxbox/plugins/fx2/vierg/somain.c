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

	FBSetColor( 11, 222, 2, 2 );
	FBSetColor( 12, 248, 201, 201 );
	FBSetColor( 13, 242, 155, 156 );
	FBSetColor( 14, 235, 111, 111 );
	FBSetColor( 15, 230, 72, 72 );
	FBSetColor( 16, 226, 48, 48 );
	FBSetColor( 17, 226, 32, 32 );
	FBSetColor( 18, 254, 242, 242 );
	FBSetColor( 19, 223, 24, 24 );
	FBSetColor( 20, 244, 172, 172 );
	FBSetColor( 21, 250, 214, 214 );
	FBSetColor( 22, 222, 15, 15 );
	FBSetColor( 23, 234, 94, 94 );
	FBSetColor( 24, 238, 119, 119 );
	FBSetColor( 25, 254, 234, 234 );
	FBSetColor( 26, 246, 182, 184 );
	FBSetColor( 27, 230, 64, 64 );
	FBSetColor( 28, 238, 126, 126 );
	FBSetColor( 29, 238, 130, 130 );
	FBSetColor( 30, 250, 225, 225 );
	FBSetColor( 31, 222, 6, 6 );
	FBSetColor( 32, 234, 88, 88 );
	FBSetColor( 33, 226, 40, 40 );
	FBSetColor( 34, 230, 58, 58 );
	FBSetColor( 35, 238, 134, 134 );
	FBSetColor( 36, 254, 252, 252 );
	FBSetColor( 37, 226, 54, 54 );
	FBSetColor( 38, 230, 79, 79 );
	FBSetColor( 39, 240, 143, 143 );
	FBSetColor( 40, 246, 190, 191 );
	FBSetColor( 41, 234, 102, 102 );
	FBSetColor( 42, 222, 10, 10 );

	FBSetColor( 51, 2, 2, 158 );
	FBSetColor( 52, 193, 194, 232 );
	FBSetColor( 53, 146, 146, 214 );
	FBSetColor( 54, 111, 111, 200 );
	FBSetColor( 55, 70, 70, 184 );
	FBSetColor( 56, 48, 48, 176 );
	FBSetColor( 57, 30, 30, 170 );
	FBSetColor( 58, 230, 230, 246 );
	FBSetColor( 59, 19, 19, 166 );
	FBSetColor( 60, 170, 170, 223 );
	FBSetColor( 61, 98, 98, 195 );
	FBSetColor( 62, 218, 218, 241 );
	FBSetColor( 63, 139, 139, 211 );
	FBSetColor( 64, 14, 14, 163 );
	FBSetColor( 65, 87, 87, 191 );
	FBSetColor( 66, 206, 206, 237 );
	FBSetColor( 67, 125, 125, 206 );
	FBSetColor( 68, 62, 62, 182 );
	FBSetColor( 69, 6, 6, 162 );
	FBSetColor( 70, 36, 36, 172 );
	FBSetColor( 71, 150, 150, 214 );
	FBSetColor( 72, 238, 238, 248 );
	FBSetColor( 73, 56, 56, 180 );
	FBSetColor( 74, 26, 26, 169 );
	FBSetColor( 75, 166, 166, 222 );
	FBSetColor( 76, 79, 79, 188 );
	FBSetColor( 77, 248, 248, 253 );
	FBSetColor( 78, 181, 181, 227 );
	FBSetColor( 79, 42, 42, 174 );
	FBSetColor( 80, 10, 10, 162 );
	FBSetColor( 81, 159, 159, 219 );
	FBSetColor( 82, 226, 226, 246 );

	FBSetupColors( );
}

int vierg_exec( int fdfb, int fdrc, int fdlcd, char *cfgfile )
{
	struct timeval	tv;
	int				x;

	if ( FBInitialize( 720, 576, 8, fdfb ) < 0 )
		return -1;

	setup_colors();

	if ( RcInitialize( fdrc ) < 0 )
		return -1;


#ifdef USEX
	FBFlushGrafic();
#endif

	while( doexit != 3 )
	{
		BoardInitialize();
		DrawBoard( 0 );
		Fx2ShowPig( 420, 150, 240, 188 );

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
			actcode=0xee;
			doexit=0;
			while(( actcode != RC_OK ) && !doexit )
			{
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
	return vierg_exec( fd_fb, fd_rc, -1, 0 );
}
