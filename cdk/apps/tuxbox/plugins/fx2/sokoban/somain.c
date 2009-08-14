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


extern	int	doexit;
extern	int	debug;
extern	unsigned short	actcode;
extern	unsigned short	realcode;	/* akt.taste : 0xee = zZ keine taste */


extern	void 	ladeVerzeichnis (void);
extern	void	MoveMouse(void);
extern	void 	freeMem(void);

static	void	setup_colors( void )
{
	FBSetColor( BLACK,   0,   0,   0);
	FBSetColor( R,     255,   0,   0);
	FBSetColor( G,       0, 255,   0);
	FBSetColor( B,       0,   0, 255);
	FBSetColor( GRAY,   80,  80,  80);
	FBSetColor( WHITE, 255, 255, 255);

FBSetColor(11,0,0,0);
FBSetColor(12,91,91,91);
FBSetColor(13,141,141,141);
FBSetColor(14,115,115,115);
FBSetColor(15,27,27,27);
FBSetColor(16,81,81,81);
FBSetColor(17,255,255,255);
FBSetColor(18,62,62,62);
FBSetColor(19,89,151,89);
FBSetColor(20,141,206,141);
FBSetColor(21,116,184,116);
FBSetColor(22,85,85,85);
FBSetColor(23,72,72,72);
FBSetColor(24,48,48,48);
FBSetColor(25,36,36,36);
FBSetColor(26,24,24,24);
FBSetColor(27,12,12,12);
FBSetColor(28,60,60,60);
FBSetColor(29,145,145,145);
FBSetColor(30,97,97,97);
FBSetColor(31,121,121,121);
FBSetColor(32,109,109,109);
FBSetColor(33,133,133,133);
FBSetColor(34,157,157,157);
FBSetColor(35,170,170,170);
FBSetColor(36,218,218,218);
FBSetColor(37,194,194,194);
FBSetColor(38,206,206,206);
FBSetColor(39,60,60,85);
FBSetColor(40,242,242,242);
FBSetColor(41,242,242,255);
FBSetColor(42,121,121,97);
FBSetColor(43,157,157,133);
FBSetColor(44,242,255,255);
FBSetColor(45,218,218,242);
FBSetColor(46,242,242,133);
FBSetColor(47,255,255,60);
FBSetColor(48,242,242,48);
FBSetColor(49,255,242,48);
FBSetColor(50,255,218,60);
FBSetColor(51,255,206,85);
FBSetColor(52,242,218,206);
FBSetColor(53,206,206,218);
FBSetColor(54,218,218,170);
FBSetColor(55,255,255,0);
FBSetColor(56,255,242,0);
FBSetColor(57,255,206,0);
FBSetColor(58,255,194,60);
FBSetColor(59,242,242,218);
FBSetColor(60,255,255,206);
FBSetColor(61,255,218,0);
FBSetColor(62,255,206,48);
FBSetColor(63,255,242,242);
FBSetColor(64,255,255,170);
FBSetColor(65,255,218,48);
FBSetColor(66,255,242,218);
FBSetColor(67,255,255,242);
FBSetColor(68,157,157,170);
FBSetColor(69,194,194,206);
FBSetColor(70,170,170,206);
FBSetColor(71,85,85,97);
FBSetColor(72,170,170,194);
FBSetColor(73,218,242,255);
FBSetColor(74,157,170,206);
FBSetColor(75,97,97,121);
FBSetColor(76,48,48,36);
FBSetColor(77,133,133,97);
FBSetColor(78,242,242,170);
FBSetColor(79,218,206,194);
FBSetColor(80,194,206,242);
FBSetColor(81,170,170,133);
FBSetColor(82,194,194,121);
FBSetColor(83,157,157,97);
FBSetColor(84,36,36,24);
FBSetColor(85,60,48,48);
FBSetColor(86,85,85,24);
FBSetColor(87,157,157,36);
FBSetColor(88,218,218,48);
FBSetColor(89,206,206,48);
FBSetColor(90,255,242,24);
FBSetColor(91,255,218,24);
FBSetColor(92,242,206,60);
FBSetColor(93,206,194,133);
FBSetColor(94,218,218,121);
FBSetColor(95,242,242,97);
FBSetColor(96,242,242,85);
FBSetColor(97,255,242,60);
FBSetColor(98,242,218,12);
FBSetColor(99,206,170,0);
FBSetColor(100,145,121,12);
FBSetColor(101,72,60,36);
FBSetColor(102,97,97,36);
FBSetColor(103,255,255,12);
FBSetColor(104,255,255,85);
FBSetColor(105,255,242,12);
FBSetColor(106,255,194,0);
FBSetColor(107,242,170,12);
FBSetColor(108,206,170,157);
FBSetColor(109,255,255,24);
FBSetColor(110,218,133,0);
FBSetColor(111,60,60,36);
FBSetColor(112,218,218,0);
FBSetColor(113,255,157,0);
FBSetColor(114,194,97,0);
FBSetColor(115,72,72,48);
FBSetColor(116,72,72,85);
FBSetColor(117,255,170,0);
FBSetColor(118,157,85,0);
FBSetColor(119,97,85,24);
FBSetColor(120,133,121,0);
FBSetColor(121,157,121,0);
FBSetColor(122,194,157,0);
FBSetColor(123,194,133,0);
FBSetColor(124,133,85,0);
FBSetColor(125,121,60,0);
FBSetColor(126,85,48,24);
FBSetColor(127,60,60,48);
FBSetColor(128,109,85,12);
FBSetColor(129,170,121,0);
FBSetColor(130,97,72,0);
FBSetColor(131,72,48,36);
FBSetColor(132,85,145,85);
FBSetColor(133,72,133,72);
FBSetColor(134,48,72,48);
FBSetColor(135,36,48,36);
FBSetColor(136,24,48,24);
FBSetColor(137,60,97,60);
FBSetColor(138,145,206,145);
FBSetColor(139,60,85,60);
FBSetColor(140,0,12,0);
FBSetColor(141,109,182,109);
FBSetColor(142,85,133,85);
FBSetColor(143,97,109,97);
FBSetColor(144,109,170,109);
FBSetColor(145,72,109,72);
FBSetColor(146,12,36,12);
FBSetColor(147,72,121,72);
FBSetColor(148,36,60,36);
FBSetColor(149,97,170,97);
FBSetColor(150,97,157,97);
FBSetColor(151,48,85,48);
FBSetColor(152,60,109,60);
FBSetColor(153,12,24,12);
FBSetColor(154,85,121,85);
FBSetColor(155,48,97,48);
FBSetColor(156,36,72,36);
FBSetColor(157,121,182,121);
FBSetColor(158,133,206,133);
FBSetColor(159,24,36,24);
FBSetColor(160,97,145,97);
FBSetColor(161,60,72,60);
FBSetColor(162,72,97,72);
FBSetColor(163,85,97,109);
FBSetColor(164,48,72,36);
FBSetColor(165,36,48,24);
FBSetColor(166,60,85,48);
FBSetColor(167,85,109,24);
FBSetColor(168,157,170,36);
FBSetColor(169,145,133,12);
FBSetColor(170,72,85,36);
FBSetColor(171,97,109,36);
FBSetColor(172,60,85,36);
FBSetColor(173,97,97,24);
FBSetColor(174,85,60,24);
FBSetColor(175,109,157,109);
FBSetColor(176,109,97,12);
FBSetColor(177,133,194,133);
FBSetColor(178,121,170,121);
FBSetColor(179,209,68,157);
FBSetColor(180,150,35,105);
FBSetColor(181,182,49,132);
FBSetColor(182,68,91,209);
FBSetColor(183,35,51,150);
FBSetColor(184,49,70,182);


	FBSetupColors();
}

int soko_exec( int fdfb, int fdrc, int fdlcd, char *cfgfile )
{
	struct timeval	tv;
	int				x;

	if ( FBInitialize( 720, 576, 8, fdfb ) < 0 )
		return -1;

	setup_colors();

	if ( RcInitialize( fdrc ) < 0 )
		return -1;

	Fx2ShowPig( 540, 450, 120, 90 );

	ladeVerzeichnis();
	Startbildschirm();

	while( doexit != 3 )
	{
		doexit=0;
		BoardInitialize();
#ifdef USEX
		FBFlushGrafic();
#endif

		if ( doexit == 4 )	/* fx2 - install error: no level found */
		{
			doexit=0;
			while(( actcode != RC_OK ) && !doexit )
			{
				tv.tv_sec = 0;
				tv.tv_usec = 100000;
				x = select( 0, 0, 0, 0, &tv );		/* 100ms pause */
				RcGetActCode( );
			}
			break;
		}

		while( !doexit )
		{
			tv.tv_sec = 0;
			tv.tv_usec = 10000;
			x = select( 0, 0, 0, 0, &tv );		/* 100ms pause */
	
			RcGetActCode( );
			MoveMouse();
		}

		if ( doexit != 3 )
		{
			actcode=0xee;
			if ( doexit ==2 )
				DrawScore();
			else
				DrawGameOver();
				if (level+1 < max_level)
					{
					level++;
					}
				else
					{
					level = 0;
					}
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

	freeMem();

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
	return soko_exec( fd_fb, fd_rc, -1, 0 );
}
