
#define RC_0			0x00
#define RC_1			0x01
#define RC_2			0x02
#define RC_3			0x03
#define RC_4			0x04
#define RC_5			0x05
#define RC_6			0x06
#define RC_7			0x07
#define RC_8			0x08
#define RC_9			0x09
#define RC_RIGHT		0x0A
#define RC_LEFT			0x0B
#define RC_UP			0x0C
#define RC_DOWN			0x0D
#define RC_OK			0x0E
#define RC_SPKR			0x0F
#define RC_STANDBY		0x10
#define RC_GREEN		0x11
#define RC_YELLOW		0x12
#define RC_RED			0x13
#define RC_BLUE			0x14
#define RC_PLUS			0x15
#define RC_MINUS		0x16
#define RC_HELP			0x17
#define RC_SETUP		0x18
#define RC_HOME			0x1F
#define RC_PAGE_DOWN	0x53
#define RC_PAGE_UP		0x54

static	int				kbdfd = -1;
static	unsigned char	actocde = 0x0000;
static	int				autorep=2;

typedef struct _KeyAction
{
	unsigned char		press;
	unsigned char		kc;
	unsigned char		autocont;
	struct _KeyAction	*next;

} KeyAction;

static	KeyAction		*keya[ 256 ];

static	unsigned char	strg2kc( char *in )
{
	if ( !strcmp(in,"UP") )
		return RC_UP;
	if ( !strcmp(in,"DOWN") )
		return RC_DOWN;
	if ( !strcmp(in,"LEFT") )
		return RC_LEFT;
	if ( !strcmp(in,"RIGHT") )
		return RC_RIGHT;
	if ( !strcmp(in,"OK") )
		return RC_OK;
	if ( !strcmp(in,"0") )
		return RC_0;
	if ( !strcmp(in,"1") )
		return RC_1;
	if ( !strcmp(in,"2") )
		return RC_2;
	if ( !strcmp(in,"3") )
		return RC_3;
	if ( !strcmp(in,"4") )
		return RC_4;
	if ( !strcmp(in,"5") )
		return RC_5;
	if ( !strcmp(in,"6") )
		return RC_6;
	if ( !strcmp(in,"7") )
		return RC_7;
	if ( !strcmp(in,"8") )
		return RC_8;
	if ( !strcmp(in,"9") )
		return RC_9;
	if ( !strcmp(in,"SPEAKER") )
		return RC_SPKR;
	if ( !strcmp(in,"PLUS") )
		return RC_PLUS;
	if ( !strcmp(in,"MINUS") )
		return RC_MINUS;
	if ( !strcmp(in,"HELP") )
		return RC_HELP;
	if ( !strcmp(in,"SETUP") )
		return RC_SETUP;
	if ( !strcmp(in,"RED") )
		return RC_RED;
	if ( !strcmp(in,"BLUE") )
		return RC_BLUE;
	if ( !strcmp(in,"GREEN") )
		return RC_GREEN;
	if ( !strcmp(in,"YELLOW") )
		return RC_YELLOW;
	return 0xee;
}

static	KeyAction	*crAction( unsigned char press, int keycode,
						unsigned char autocont )
{
	KeyAction	*c;

	c = (KeyAction*)malloc(sizeof(KeyAction));
	c->press = press;
	c->kc = keycode;
	c->autocont = autocont;

	return c;
}

static int scode2c64(unsigned scancode)
{
	switch (scancode)
	{
	case ',' : return MATRIX(5,7);
	case '.': return MATRIX(5,4);
	case 'a': case 'A' : return MATRIX(1,2);
	case 'b': case 'B' : return MATRIX(3,4);
	case 'c': case 'C' : return MATRIX(2,4);
	case 'd': case 'D' : return MATRIX(2,2);
	case 'e': case 'E' : return MATRIX(1,6);
	case 'f': case 'F' : return MATRIX(2,5);
	case 'g': case 'G' : return MATRIX(3,2);
	case 'h': case 'H' : return MATRIX(3,5);
	case 'i': case 'I' : return MATRIX(4,1);
	case 'j': case 'J' : return MATRIX(4,2);
	case 'k': case 'K' : return MATRIX(4,5);
	case 'l': case 'L' : return MATRIX(5,2);
	case 'm': case 'M' : return MATRIX(4,4);
	case 'n': case 'N' : return MATRIX(4,7);
	case 'o': case 'O' : return MATRIX(4,6);
	case 'p': case 'P' : return MATRIX(5,1);
	case 'q': case 'Q' : return MATRIX(7,6);
	case 'r': case 'R' : return MATRIX(2,1);
	case 's': case 'S' : return MATRIX(1,5);
	case 't': case 'T' : return MATRIX(2,6);
	case 'u': case 'U' : return MATRIX(3,6);
	case 'v': case 'V' : return MATRIX(3,7);
	case 'w': case 'W' : return MATRIX(1,1);
	case 'x': case 'X' : return MATRIX(2,7);
	case 'y': case 'Y' : return MATRIX(3,1);
	case 'z': case 'Z' : return MATRIX(1,4);

	case 0x7f: return MATRIX(0,0);
	case 0x08: return MATRIX(0,0);
	case 0x07: return MATRIX(7,1);
	case 0x0d: return MATRIX(0,1);
	case ' ': return MATRIX(7,4);
	case 0x1b: return MATRIX(7,7);

	case '0': return MATRIX(4,3);
	case '1': return MATRIX(7,0);
	case '2': return MATRIX(7,3);
	case '"': return MATRIX(7,3) | 0x80;
	case '3': return MATRIX(1,0);
	case '4': return MATRIX(1,3);
	case '5': return MATRIX(2,0);
	case '6': return MATRIX(2,3);
	case '7': return MATRIX(3,0);
	case '8': return MATRIX(3,3);
	case '9': return MATRIX(4,0);

	case '[': return MATRIX(5,6);
	case '*': return MATRIX(6,1);
	case '$': return MATRIX(1,3) | 0x80;
	case ']': return MATRIX(6,1) | 0x80;
	case '/': return MATRIX(6,7);
	case ':': return MATRIX(5,5);
	case '~': return MATRIX(6,2);
	case '#': return MATRIX(6,5) | 0x80;
	case '<': return MATRIX(6,6);
	case '>': return MATRIX(6,7);
	case '-': return MATRIX(5,3);
	case '+': return MATRIX(5,0);
	case '=': return MATRIX(6,5);
	default :
//		printf("unused code %02x\n",scancode);
		break;
	}
	return 0;
}

static	KeyAction	*buildAction( char *in )
{
	KeyAction	n1;
	KeyAction	*l=&n1;
	KeyAction	*c=0;
	char		*p;
	char		*t;
	int			kc;
	int			pl;

	n1.next = 0;

	for( p=in; *p; p++ )
	{
		if ( *p == '<' )
		{
			p++;
			t=strchr(p,'>');
			if ( !t )
				break;
			pl=(t-p);
			*t=0;
			if ( !strcmp(p,"F1") )
			{
				c=crAction( 3, MATRIX(0,4), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"F2") )
			{
				c=crAction( 3, MATRIX(0,4) | 0x80, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"F3") )
			{
				c=crAction( 3, MATRIX(0,5), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"F4") )
			{
				c=crAction( 3, MATRIX(0,5) | 0x80, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"F5") )
			{
				c=crAction( 3, MATRIX(0,6), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"F6") )
			{
				c=crAction( 3, MATRIX(0,6) | 0x80, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"RET") )
			{
				c=crAction( 1, MATRIX(0,1), 1 );
				l->next = c;
				l=c;
				c=crAction( 0, MATRIX(0,1), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"DEL") )
			{
				c=crAction( 1, MATRIX(0,0), 1 );
				l->next = c;
				l=c;
				c=crAction( 0, MATRIX(0,0), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"SHL") )
			{
				c=crAction( 3, MATRIX(1,0), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"SHR") )
			{
				c=crAction( 3, MATRIX(6,4), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"HOM") )
			{
				c=crAction( 3, MATRIX(6,3), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"R/S") )
			{
				c=crAction( 3, MATRIX(7,7), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"C=") )
			{
				c=crAction( 3, MATRIX(7,5), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"CTL") )
			{
				c=crAction( 3, MATRIX(7,2), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"CUP") )
			{
				c=crAction( 3, MATRIX(0,7) | 0x80, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"CDOWN") )
			{
				c=crAction( 3, MATRIX(0,7), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"CLEFT") )
			{
				c=crAction( 3, MATRIX(0,2) | 0x80, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"CRIGHT") )
			{
				c=crAction( 3, MATRIX(0,2), 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"UP") )
			{
				c=crAction( 4, 0x01, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"DOWN") )
			{
				c=crAction( 4, 0x02, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"LEFT") )
			{
				c=crAction( 4, 0x04, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"RIGHT") )
			{
				c=crAction( 4, 0x08, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"UP/LEFT") )
			{
				c=crAction( 4, 0x05, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"UP/RIGHT") )
			{
				c=crAction( 4, 0x09, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"DOWN/LEFT") )
			{
				c=crAction( 4, 0x03, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"DOWN/RIGHT") )
			{
				c=crAction( 4, 0x0a, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"FIRE") )
			{
				c=crAction( 4, 0x10, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"FIRE2") )
			{
				c=crAction( 4, 0x20, 1 );
				l->next = c;
				l=c;
			}
			else if ( !strcmp(p,"PRINT") )
			{
				c=crAction( 5, 0x1, 0 );
				l->next = c;
				l=c;
			}
			p+=pl;
			continue;
		}
		kc=scode2c64( *p );
		c=crAction( 1, kc, 1 );
		l->next = c;
		l=c;
		c=crAction( 0, kc, 1 );
		l->next = c;
		l=c;
	}
	l->autocont=0;

	return n1.next;
}

static	void		init_keyfile(void)
{
	FILE			*fp;
	char			buffer[256];
	char			*p;
	char			*c;
	KeyAction		*k;
	KeyAction		*e;
	unsigned char	kc;

	memset(keya,0,sizeof(KeyAction*)*256);

	if ( *kbdname )
		strcpy(buffer,kbdname);
	else
	{
		strcpy(buffer,d64image);
		p=strrchr(buffer,'.');
		if ( !p )
			return;
		strcpy(p,".kbd");
	}
	if ( ! *buffer )
		return;

	fp = fopen( buffer, "r" );
	if ( !fp )
		return;
	while( fgets( buffer, 256, fp ) )
	{
		if ( *buffer == '#' )
			continue;
		p=strchr(buffer,'\n');
		if ( p )
			*p=0;
		p=strchr(buffer,':');
		if ( !p )
			continue;
		*p=0;
		p++;
		if ( !strcmp(buffer,"autorep") )
		{
			autorep=atoi(p);
			if (( autorep < 0 ) || ( autorep > 100 ))
				autorep=2;
			continue;
		}
		kc=strg2kc(buffer);
		if ( kc == 0xee )
			continue;
		k=buildAction(p);
		if ( k )
		{
			if ( !keya[kc] )
				keya[kc] = k;
			else
			{
				for( e=keya[kc]; e->next; e=e->next );
				e->next = k;
			}
		}
	}
	fclose(fp);
}

static void	c64setkey( int kc )
{
	int				c64_byte, c64_bit, shifted;

	if (keystate[kc])
		return;
	keystate[kc] = 1;
	c64_byte = kc >> 3;
	c64_bit = kc & 7;
	shifted = kc & 128;
	c64_byte &= 7;
	if (shifted) {
		key_matrix[6] &= 0xef;
		rev_matrix[4] &= 0xbf;
	}
	key_matrix[c64_byte] &= ~(1 << c64_bit);
	rev_matrix[c64_bit] &= ~(1 << c64_byte);
}

static	void	rec64setkey( int kc )
{
	int				c64_byte, c64_bit, shifted;

	keystate[kc] = 0;
	c64_byte = kc >> 3;
	c64_bit = kc & 7;
	shifted = kc & 128;
	c64_byte &= 7;
	if (shifted) {
		key_matrix[6] |= 0x10;
		rev_matrix[4] |= 0x40;
	}
	key_matrix[c64_byte] |= (1 << c64_bit);
	rev_matrix[c64_bit] |= (1 << c64_byte);
}

static	int		last_key = 0xee;
static	int		isjoy;
static	int		prblocked=0;

static	void	autodo( int key )
{
	KeyAction	*k;
	int			frk=0;

	isjoy = 0;

	if ( key != 0xee )
		last_key = key;
	if ( last_key == 0xee )
		return;

	k = keya[ last_key ];
	if ( !k )
		return;

	switch ( k->press )
	{
	case 1:
		c64setkey(k->kc);
		break;
	case 0:
		rec64setkey(k->kc);
		break;
	case 3:
		if ( !k->next && ( key == 0xee ))
			rec64setkey(k->kc);
		else
			c64setkey(k->kc);
		break;
	case 4:
		if ( !k->next && ( key == 0xee ))
		{
			joystate |= k->kc;
		}
		else
		{
			joystate &= ~k->kc;
			if ( k->kc & 0x0f )
				isjoy=1;
		}
		break;
	case 5:
		if ( prblocked > 0 )
			break;
		if ( !k->next && ( key != 0xee ))
		{
			prblocked=10;
			vga_printscreen( "/var/tmp/screen.xpm" );
		}
		break;
	}

	if ( k->next )
	{
		keya[last_key] = k->next;
		frk=1;
	}
	if ( !k->autocont )
	{
		switch( k->press )
		{
		case 1:
		case 0:
			last_key = 0xee;
			break;
		case 3:
		case 4:
		case 5:
			last_key = key;
			break;
		}
	}

	if ( frk )
		free(k);
}

#ifdef i386

#include <termios.h>

static	struct termios	tios;

static	int	keyboard_init( void )
{
	struct termios	ntios;

	kbdfd=0;

	init_keyfile();

	tcgetattr(kbdfd,&tios);
	memset(&ntios,0,sizeof(ntios));
	tcsetattr(kbdfd,TCSANOW,&ntios);

	return 0;
}

#if 0
static	void	keyboard_update( void )
{
	unsigned char	buf[ 256 ];
	int				x;
	int				left;
	unsigned short	code = 0;
	unsigned char	*p = buf;
	int				kc;

	if ( kbdfd != 0 )
		return;

	x = read( kbdfd,buf,256);
	if ( x>0 )
	{
		for( p=buf,left=x; left; left--, p++ )
		{
			switch( *p )
			{
			case 0x1b :
				if ( left >= 3 )
				{
					p+=2;
					switch( *p )
					{
					case 0x41 :
						joystate &= ~0x1;
						break;
					case 0x42 :
						joystate &= ~0x2;
						break;
					case 0x43 :
						joystate &= ~0x8;
						break;
					case 0x44 :
						joystate &= ~0x4;
						break;
					case 0x5b :
						p++;
						left--;
						switch( *p )
						{
						case 0x41 :
							c64setkey( MATRIX(0,4) );
							break;
						case 0x42 :
							c64setkey( MATRIX(0,4)|0x80 );
							break;
						case 0x43 :
							c64setkey( MATRIX(0,5) );
							break;
						case 0x44 :
							c64setkey( MATRIX(0,5)|0x80 );
							break;
						case 0x45 :
							c64setkey( MATRIX(0,6) );
							break;
						}
						break;
					case 0x31 :
						p++;
						left--;
						switch( *p )
						{
						case 0x37 :
							c64setkey( MATRIX(0,6)|0x80 );
							break;
						case 0x38 :
							c64setkey( MATRIX(0,3) );
							break;
						case 0x39 :
							c64setkey( MATRIX(0,3)|0x80 );
							break;
						default :
							printf("31,%02x\n\r",*p);
							break;
						}
						break;
					case 0x32 :
						p++;
						left--;
						switch( *p )
						{
						case 0x30 :
							c64setkey( MATRIX(7,7) );
							break;
						case 0x31 :
							c64setkey( MATRIX(7,5) );
							break;
						case 0x33 :
							joystate=0;
							c64setkey( MATRIX(0,7) );
							break;
						case 0x34 :
							f12pressed=1;
							break;
						case 0x7e :
							joystate &= ~0x10;
							break;
						default :
							printf("32,%02x\n\r",*p);
							break;
						}
						break;
					default :
printf("do 3. key %02x\n\r",*p);
						break;
					}
					left-=2;
				}
				else
					left=1;
				break;
			case 0x03 :
				tcsetattr(kbdfd,TCSANOW,&tios);
				close(kbdfd);
				kbdfd=-1;
				quit=1;
				break;
			default :
				kc = scode2c64(*p);
				c64setkey(kc);
				break;
			}
		}
	}
	else
	{
		joystate=0xff;
		for( kc=0; kc<256; kc++ )
		{
			if (!keystate[kc])
				continue;
			keystate[kc]--;
			if (keystate[kc])
				continue;
			rec64setkey( kc );
		}
	}
}
#endif

static	void	keyboard_update( void )
{
	unsigned char	buf[ 256 ];
	int				x;
	int				left;
	unsigned short	code = 0;
	unsigned char	*p = buf;
	int				kc;
static int			slowdown=0;

	if ( kbdfd != 0 )
		return;

	if ( prblocked )
		prblocked--;

	x = read( kbdfd,buf,256);
	if ( x>0 )
	{
		if ( slowdown )
			autodo(0xee);
		slowdown=0;
		for( p=buf,left=x; left; left--, p++ )
		{
			switch( *p )
			{
			case 0x1b :
				if ( left >= 3 )
				{
					p+=2;
					switch( *p )
					{
					case 0x41 :
						autodo( RC_UP );
						break;
					case 0x42 :
						autodo( RC_DOWN );
						break;
					case 0x43 :
						autodo( RC_LEFT );
						break;
					case 0x44 :
						autodo( RC_RIGHT );
						break;
					case 0x32 :
						p++;
						left--;
						switch( *p )
						{
						case 0x7e :
							autodo( RC_OK );
							break;
						}
						break;
					}
					left-=2;
				}
				else
					left=1;
				break;
			case 0x03 :
				tcsetattr(kbdfd,TCSANOW,&tios);
				close(kbdfd);
				kbdfd=-1;
				quit=1;
				break;
			case 'r' :
				autodo( RC_RED );
				break;
			case 'g' :
				autodo( RC_GREEN );
				break;
			case 'b' :
				autodo( RC_BLUE );
				break;
			case 'y' :
				autodo( RC_YELLOW );
				break;
			}
		}
		if ( isjoy )
			slowdown+=autorep;
	}
	else
	{
		if ( slowdown )
			slowdown--;
		if ( slowdown )
			return;
		autodo(0xee);
	}
}

#else

#include <dbox/fp.h>

static	int	keyboard_init( void )
{
	char	buf[32];

	kbdfd=open("/dev/dbox/rc0", O_RDONLY);
	if ( kbdfd == -1 )
		return -1;
	fcntl(kbdfd,F_SETFL,O_NONBLOCK);
	ioctl(kbdfd,RC_IOCTL_BCODES, 1 );

	read( kbdfd, buf, 32 );		/* make it empty */

	init_keyfile();

	return 0;
}

static	unsigned short rctranslate( unsigned short code )
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
		default:
			//perror("unknown old rc code");
			return 0xee;
		}
	} else if (!(code&0x00))
		return code&0x3F;
	return 0xee;
}

static	void	keyboard_update( void )
{
	char			buf[32];
	int				x;
	unsigned short	code = 0xee;
static int			slowdown=0;
static int			firstkey=1;

	if ( prblocked )
		prblocked--;

	x = read( kbdfd,buf,32);
	if ( x < 2 )
	{
		if ( slowdown )
			slowdown--;
		if ( slowdown )
			return;
		autodo(0xee);
		return;
	}

	x-=2;

	memcpy(&code,buf+x,2);

	code=rctranslate(code);

	if ( code == 0xee )
		return;

	if (firstkey)
		vga_drawlogo();
	firstkey=0;

	if ( slowdown )
	{
		if ( code != last_key )
			autodo(0xee);
		slowdown=0;
	}

	if( code == RC_HOME )
	{
		quit=1;
		joystate = 0xff;
	}
	else if( code == RC_STANDBY )
	{
		f11pressed=1;
		joystate = 0xff;
	}
	else
		autodo(code);

	if ( isjoy )
		slowdown+=autorep;
}

#endif
