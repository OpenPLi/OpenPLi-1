/*
** initial coding by fx2
*/

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <linux/input.h>

#include "draw.h"
#include "rcinput.h"

#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
	static int fd_is_ext = 0;
	static int keyboard = 0;
	static int drop = 0;
#endif

#define Debug	if (debug) printf

static	int	fd = -1;
static	int	kbfd = -1;
unsigned short	realcode=0xee;
unsigned short	actcode=0xee;
int		doexit=0;
int		debug=0;

static	struct termios	tios;

void	KbInitialize( void )
{
	struct termios	ntios;

	kbfd = 0;

	if ( tcgetattr(kbfd,&tios) == -1 )
	{
		kbfd=-1;
		return;
	}
	memset(&ntios,0,sizeof(ntios));
	tcsetattr(kbfd,TCSANOW,&ntios);

	return;
}

static	unsigned short kb_translate( unsigned char c )
{
	switch(c)
	{
	case 0x41 :
		return RC_UP;
	case 0x42 :
		return RC_DOWN;
	case 0x43 :
		return RC_RIGHT;
	case 0x44 :
		return RC_LEFT;
	}
	return 0;
}

void		KbGetActCode( void )
{
	unsigned char	buf[256];
	int				x=0;
	int				left;
	unsigned short	code = 0;
	unsigned char	*p = buf;

	realcode=0xee;

	if ( kbfd != -1 )
		x = read(kbfd,buf,256);
	if ( x>0 )
	{
		for(p=buf, left=x; left; left--,p++)
		{
			switch(*p)
			{
			case 0x1b :
				if ( left >= 3 )
				{
					p+=2;
					code = kb_translate(*p);
					if ( code )
						actcode = code;
					left-=2;
				}
				else
					left=1;
				break;
			case 0x03 :
				doexit=3;
				break;
			case 0x0d :
				actcode=RC_OK;
				break;
#if 0
			case 0x1c :
				FBPrintScreen();
				break;
#endif
			case '?' :
				actcode=RC_HELP;
				break;
			case 'b' :
				actcode=RC_BLUE;
				break;
			case 'r' :
				actcode=RC_RED;
				break;
			case 'g' :
				actcode=RC_GREEN;
				break;
			case 'y' :
				actcode=RC_YELLOW;
				break;
			case '0' :
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
			case '7' :
			case '8' :
			case '9' :
				actcode=*p-48;
				break;
			case '-' :
				actcode=RC_MINUS;
				break;
			case '+' :
				actcode=RC_PLUS;
				break;
			case 'q' :
				actcode=RC_SPKR;
				FBPause();
				break;
			}
		}
		realcode=actcode;
	}
}

void	KbClose( void )
{
	if ( kbfd != -1 )
		tcsetattr(kbfd,TCSANOW,&tios);
}

int	RcInitialize( int extfd )
{
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
	char	buf[32];
	if ( extfd == -1 )
	{
		fd_is_ext = 0;
		fd = open("/dev/dbox/rc0", O_RDONLY );
		if ( fd == -1 )
			return kbfd;
		fcntl(fd, F_SETFL, O_NONBLOCK );
	}
	else
	{
		fd_is_ext = 1;
		fd = extfd;
		fcntl(fd, F_SETFL, O_NONBLOCK );
	}
/* clear rc-buffer */
	read( fd, buf, 32 );
#else
	//KbInitialize();
	fd = open( "/dev/input/event0", O_RDONLY );
	if ( fd == -1 )
	{
		return kbfd;
	}
	fcntl(fd, F_SETFL, O_NONBLOCK );
#endif
	return 0;
}

#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
static unsigned short translate( unsigned short code )
{
	if ((code&0xFF00)==0x5C00)
	{
		switch (code&0xFF)
		{
		case 0x0C: return RC_STANDBY;
		case 0x20: return RC_HOME;
		case 0x27: return RC_SETUP;
		case 0x00: return RC_0;
		case 0x01: return RC_1;
		case 0x02: return RC_2;
		case 0x03: return RC_3;
		case 0x04: return RC_4;
		case 0x05: return RC_5;
		case 0x06: return RC_6;
		case 0x07: return RC_7;
		case 0x08: return RC_8;
		case 0x09: return RC_9;
		case 0x3B: return RC_BLUE;
		case 0x52: return RC_YELLOW;
		case 0x55: return RC_GREEN;
		case 0x2D: return RC_RED;
		case 0x54: return RC_PAGE_UP;
		case 0x53: return RC_PAGE_DOWN;
		case 0x0E: return RC_UP;
		case 0x0F: return RC_DOWN;
		case 0x2F: return RC_LEFT;
		case 0x2E: return RC_RIGHT;
		case 0x30: return RC_OK;
		case 0x16: return RC_PLUS;
		case 0x17: return RC_MINUS;
		case 0x28: return RC_SPKR;
		case 0x82: return RC_HELP;
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
		case 0xFE: keyboard=0;return 0xee;
		case 0xFF: keyboard=1;return 0xee;
#endif
		default:
			//perror("unknown old rc code");
			return 0xee;
		}
	} else if (!(code&0x00))
		return code&0x3F;
	return 0xee;
}
#endif


void		RcGetActCode( void )
{
	int				x=0;
	unsigned short	code = 0;
	static  unsigned short cw=0;
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
	char buf[32];
	if ( fd != -1 )
		x = read( fd, buf, 32 );
	if ( x < 2 )
	{
		realcode=0xee;
		if  ( cw == 1 )
			cw = 0;
		return;
	}
	x -= 2;
	memcpy(&code,buf+x,2);
	if (!drop && !keyboard)
	{
		drop = 1;
		return;
	}
	code=translate(code);
	realcode=code;
	if ( code == 0xee )
	{
		drop = 0;
		return;
	}
#else
	struct input_event ev;

	if ( fd != -1 ) {

		do {

			x = read(fd, &ev, sizeof(struct input_event));

			if ((x == sizeof(struct input_event)) && ((ev.value == 1)||(ev.value == 2)))
				break;

		} while (x == sizeof(struct input_event));

	}

	if ( x % sizeof(struct input_event) )
	{
		//KbGetActCode();
		realcode=0xee;
		return;
	}

	Debug("%d bytes from FB received ...\n",x);

	realcode=code=ev.code;

	Debug("code=%04x\n",code);
#endif

	if ( cw == 2 )
	{
		actcode=code;
		return;
	}

	switch(code)
	{
#if 0
	case RC_HELP:
		if ( !cw )
			FBPrintScreen();
		cw=1;
		break;
#endif
	case RC_SPKR:
		if ( !cw )
		{
			cw=2;
			FBPause();
			cw=0;
		}
		break;
	case RC_HOME:
		doexit=3;
		break;
#ifdef HAVE_DBOX_HARDWARE
	case KEY_1:
		actcode = 1;
		break;
	case KEY_2:
		actcode = 2;
		break;
	case KEY_3:
		actcode = 3;
		break;
	case KEY_4:
		actcode = 4;
		break;
	case KEY_5:
		actcode = 5;
		break;
	case KEY_6:
		actcode = 6;
		break;
	case KEY_7:
		actcode = 7;
		break;
	case KEY_8:
		actcode = 8;
		break;
	case KEY_9:
		actcode = 9;
		break;
	case KEY_0:
		actcode = 0;
		break;
#endif
#if 0
	case RC_UP:
	case RC_DOWN:
	case RC_RIGHT:
	case RC_LEFT:
	case RC_OK:
#endif
	default :
		cw=0;
		actcode=code;
		break;
	}

	return;
}

void	RcClose( void )
{
	KbClose();
	if ( fd == -1 )
		return;
#if defined HAVE_DREAMBOX_HARDWARE || defined HAVE_IPBOX_HARDWARE
	if ( !fd_is_ext )
#endif
	close(fd);
}
