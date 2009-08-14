/* das vielleicht sinnloseste Programm für die d-box (C) 2001 by Ge0rG

   INSTALLATION:
     1. lcdcirc.c und sinus.h downloaden
     2. #include für lcd-ks0713.h an eigenen Pfad anpassen
     3. powerpc-linux-gcc lcdcirc.c -o lcdcirc
     4. auf d-box kopieren
     5. lcdd beenden
     6. lcdstars ausführen
     7. gespannt zukucken (nur wenn man direkt von vorn draufkuckt, sieht
        es auch wirklich schick aus)

   Ach ja, dieses Programm unterliegt der GPL, also nix klauen ;o)
*/

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>

#include "sinus.h"
#include <rcinput.h>

#include <plugin.h>

#include <fx2math.h>

extern	int	doexit;
extern	unsigned short	actcode;

typedef unsigned char screen_t[LCD_BUFFER_SIZE];

#define FIELD_X 240
#define FIELD_Y 128
//#define xx(x) (x-FIELD_X>>1)*(x-FIELD_X>>1)
//#define yy(y) (y-FIELD_Y>>1)*(y-FIELD_Y>>1)
#define xx(x) (x-FIELD_X/2)*(x-FIELD_X/2)
#define yy(y) (y-FIELD_Y/2)*(y-FIELD_Y/2)

#define SQRT (FIELD_X-1)*(FIELD_X-1) + (FIELD_Y-1)*(FIELD_Y-1) + 1

int lcd_fd;

void clr() {
	if (ioctl(lcd_fd,LCD_IOCTL_CLEAR) < 0) {
		perror("clr()");
		exit(1);
	}
}

void init() {
/*	int i;
	if ((lcd_fd = open("/dev/dbox/lcd0",O_RDWR)) < 0) {
		perror("open(/dev/dbox/lcd0)");
		exit(1);
	}*/
	clr();

/*	i = LCD_MODE_BIN;
	if (ioctl(lcd_fd,LCD_IOCTL_ASC_MODE,&i) < 0) {
		perror("init(LCD_MODE_BIN)");
		exit(1);
	}*/
}

void draw_screen(screen_t s) {
	write(lcd_fd, s, LCD_BUFFER_SIZE);
}


unsigned char Sqrt[SQRT];
unsigned char field[FIELD_Y>>3][FIELD_X];


void init_sqrt() {
	int pos, i, ii;
	i=0;
	ii=0;
	for (pos=0; pos<SQRT; pos++) {
		Sqrt[pos] = i;
		if (pos>ii) {
			i++;
			ii = i*i;
		}
		//printf("%i %i\n", pos, Sqrt[pos]);
	}
	//printf("%i\n", SQRT);
}

void init_field() {
	unsigned int x, y, bla;

	for (y=0; y<FIELD_Y; y++) {
		for (x=0; x<FIELD_X; x++) {
			bla = Sqrt[xx(x) + yy(y)];
			if ((bla % 32) < 16) field[y>>3][x] |= (1 << (y&7));
		}
	}
}


void blt_field(int ofsx, int ofsy, screen_t s) {
	int x, y, w, bit_y, byte_y;
	byte_y = ofsy >> 3;
	bit_y = ofsy & 7;
	for (y=0; y<LCD_ROWS; y++) {
		for (x=0; x<LCD_COLS; x++) {
			w = field[byte_y+y][x+ofsx] +
			   (field[byte_y+y+1][x+ofsx]<<8);
			s[y*LCD_COLS+x] = (w >> bit_y) & 0xff;
		}
	}
}

#define GL 12
static unsigned char gl[GL] = {62, 65, 73, 73, 121, 0, 127, 64, 64, 64, 64, 0};
//#define GL 8
//static unsigned char gl[GL] = { 30, 33, 37, 125, 64, 64, 96, 0};

void blt_two(int ofsx1, int ofsy1, int ofsx2, int ofsy2, screen_t s) {
	int x, y, w1, w2, bit_y1, byte_y1, bit_y2, byte_y2;
	byte_y1 = ofsy1 >> 3;
	bit_y1 = ofsy1 & 7;
	byte_y2 = ofsy2 >> 3;
	bit_y2 = ofsy2 & 7;
	for (y=0; y<LCD_ROWS; y++) {
		for (x=0; x<LCD_COLS; x++) {
			w1 = field[byte_y1+y][x+ofsx1] +
			   (field[byte_y1+y+1][x+ofsx1]<<8);
			w2 = field[byte_y2+y][x+ofsx2] +
			   (field[byte_y2+y+1][x+ofsx2]<<8);
			s[y*LCD_COLS+x] = ((w1 >> bit_y1) & 0xff) ^ 
			                  ~((w2 >> bit_y2) & 0xff);
		}
	}
	for (x=0; x<GL; x++) s[LCD_BUFFER_SIZE-GL+x]^=gl[x];
}

int lcdcirc_exec() {
	screen_t screen;
	int t, x1, y1, x2, y2;

	init_sqrt();
	init();
	init_field();
	t = random();
	doexit=0;
	while (!doexit) {
		//printf("isin(%i)=%i\n", x,
		x1 = 60 + isin(1400+9*t)*59/SIN_MUL;
		y1 = 32 + isin(100+7*t)*31/SIN_MUL;
		x2 = 60 + isin(6*t)*59/SIN_MUL;
		y2 = 32 + isin(2000+5*t)*31/SIN_MUL;

		blt_two(x1, y1, x2, y2, screen);
		draw_screen(screen);
		usleep(80000);
		t++;
		
		RcGetActCode();
		if( doexit==3)
			doexit=1;
		else
			doexit=0;
	}
	clr();
	return 0;
}

int plugin_exec( PluginParam *par )
{
	int a, fd_rc=-1;

	for( ; par; par=par->next )
	{
		if ( !strcmp(par->id,P_ID_RCINPUT) )
			fd_rc=_atoi(par->val);
		else if(!strcmp(par->id, P_ID_LCD))
			lcd_fd=_atoi(par->val);
	}
	
	if ( RcInitialize( fd_rc ) < 0 )
		return -1;

	if ( lcd_fd <= 0 )
		if( (lcd_fd=open("/dev/dbox/lcd0", O_RDWR)) < 0 )
		{
			perror("can't open(/dev/dbox/lcd0)");
			return -1;
		}

	a=lcdcirc_exec();
	
  	RcClose();
	return a;
}
