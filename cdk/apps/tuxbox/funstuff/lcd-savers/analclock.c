/* analclock (C) 2001 by Ge0rG

   Nein, nicht das was du jetzt denkst. Das hier ist eine Analoguhr
   und nix unanständiges :-P

   Ach ja, dieses Programm unterliegt der GPL, also nix klauen ;o)
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>

#include "sinus.h"

#define SQRT 64*64+1


typedef unsigned char screen_t[LCD_BUFFER_SIZE];

int lcd_fd;

void clr() {
	if (ioctl(lcd_fd,LCD_IOCTL_CLEAR) < 0) {
		perror("clr()");
		exit(1);
	}
}

unsigned char Sqrt[SQRT];

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
	}
}


void init() {
	int i;
	if ((lcd_fd = open("/dev/dbox/lcd0",O_RDWR)) < 0) {
		perror("open(/dev/dbox/lcd0)");
		exit(1);
	}
	clr();

	i = LCD_MODE_BIN;
	if (ioctl(lcd_fd,LCD_IOCTL_ASC_MODE,&i) < 0) {
		perror("init(LCD_MODE_BIN)");
		exit(1);
	}
}

void draw_screen(screen_t s) {
	write(lcd_fd, s, LCD_BUFFER_SIZE);
}


void putpixel(int x, int y, char col, screen_t s) {
	int ofs = (y>>3)*LCD_COLS+x,
	    bit = y & 7;

	switch (col) {
	case LCD_PIXEL_ON:
		s[ofs] |= 1<<bit;
		break;
	case LCD_PIXEL_OFF:
		s[ofs] &= ~(1<<bit);
		break;
	case LCD_PIXEL_INV:
		s[ofs] ^= 1<<bit;
		break;
	}
}

void circle(int mx, int my, int r, char col, screen_t s) {
	int x, y, tr=r-1;
	for (x=0; x<=r; x++) {
		y = Sqrt[r*r - x*x];
		putpixel(mx-x, my-y, col, s);
		putpixel(mx-y, my-x, col, s);
		putpixel(mx+x, my-y, col, s);
		putpixel(mx+y, my-x, col, s);
		putpixel(mx-x, my+y, col, s);
		putpixel(mx-y, my+x, col, s);
		putpixel(mx+x, my+y, col, s);
		putpixel(mx+y, my+x, col, s);
	}
	for (x=0; x<=tr; x++) {
		y = Sqrt[tr*tr - x*x];
		putpixel(mx-x, my-y, col, s);
		putpixel(mx-y, my-x, col, s);
		putpixel(mx+x, my-y, col, s);
		putpixel(mx+y, my-x, col, s);
		putpixel(mx-x, my+y, col, s);
		putpixel(mx-y, my+x, col, s);
		putpixel(mx+x, my+y, col, s);
		putpixel(mx+y, my+x, col, s);
	}
}

#define MX 60
#define MY 32
#define RAD 27

void init_clock(screen_t s) {
	int m, x, y;
	memset(s, 0, LCD_BUFFER_SIZE);
	circle(MX, MY, 31, 1, s);
	putpixel(MX, MY, 1, s);

	for (m = 0; m < 60; m+=5) {
		x = MX - isin(-m*SIN_SIZE/30)*28/SIN_MUL;
		y = MY + isin(-SIN_SIZE/2 - m*SIN_SIZE/30)*28/SIN_MUL;
		putpixel(x, y, 1, s);
		if(m==0) {putpixel(x, y+1, 1, s);putpixel(x+1, y, 1, s);putpixel(x-1, y, 1, s);}
		else if(m==15) {putpixel(x-1, y, 1, s);putpixel(x, y+1, 1, s);putpixel(x, y-1, 1, s);}
		else if(m==30) {putpixel(x, y-1, 1, s);putpixel(x-1, y, 1, s);putpixel(x+1, y, 1, s);}
		else if(m==45) {putpixel(x+1, y, 1, s);putpixel(x, y+1, 1, s);putpixel(x, y-1, 1, s);}
	}
}

/* geklaut von Shadow's lcd.c  -- haut mich dafür! */
int sgn (int arg) {
	if(arg<0) return -1;
	if(arg>0) return 1;
	return 0;
}

void render_line( int xa, int ya, int xb, int yb, int farbe, int width, screen_t s )
{
	int 	dx = abs (xa - xb);
	int	dy = abs (ya - yb);
	int	x;
	int	y;
	int	End;
	int	step;
	int 	first=1;

	if ( dx > dy )
	{
		if(first)
		{
			ya-=width/2;
			yb-=width/2;
			first=0;
		}
		while((width--)>0)	
		{
			int	p = 2 * dy - dx;
			int	twoDy = 2 * dy;
			int	twoDyDx = 2 * (dy-dx);

			if ( xa > xb )
			{
				x = xb;
				y = yb;
				End = xa;
				step = ya < yb ? -1 : 1;
			}
			else
			{
				x = xa;
				y = ya;
				End = xb;
				step = yb < ya ? -1 : 1;
			}

			putpixel(x, y, farbe, s);

			while( x < End )
			{
				x++;
				if ( p < 0 )
					p += twoDy;
				else
				{
					y += step;
					p += twoDyDx;
				}
				putpixel(x, y, farbe, s);
			}
			ya++;
			yb++;
		}
	}
	else
	{
		if(first)
		{
			xa-=width/2;
			xb-=width/2;
			first=0;
		}
		while((width--)>0)
		{
			int	p = 2 * dx - dy;
			int	twoDx = 2 * dx;
			int	twoDxDy = 2 * (dx-dy);

			if ( ya > yb )
			{
				x = xb;
				y = yb;
				End = ya;
				step = xa < xb ? -1 : 1;
			}
			else
			{
				x = xa;
				y = ya;
				End = yb;
				step = xb < xa ? -1 : 1;
			}

			putpixel(x, y, farbe, s);

			while( y < End )
			{
				y++;
				if ( p < 0 )
					p += twoDx;
				else
				{
					x += step;
					p += twoDxDy;
				}
				putpixel(x, y, farbe, s);
			}
			xa++;
			xb++;
		}
	}
}

void render_clock(screen_t back, screen_t s) {
        struct timeval tb;
        struct tm *t;
	int x, y;
        gettimeofday(&tb, NULL);
	t = localtime(&tb.tv_sec);

	memcpy(s, back, LCD_BUFFER_SIZE);

	x = MX - isin(-((t->tm_hour % 12)*60 + t->tm_min)*SIN_SIZE/360)*RAD/SIN_MUL*2/3;
	y = MY + isin(-SIN_SIZE/2 - ((t->tm_hour % 12)*60 + t->tm_min)*SIN_SIZE/360)*RAD/SIN_MUL*2/3;
	render_line(MX, MY, x, y, LCD_PIXEL_ON, 3, s);

	x = MX - isin(-(t->tm_min*60+t->tm_sec)*SIN_SIZE/1800)*RAD/SIN_MUL;
	y = MY + isin(-SIN_SIZE/2 - (t->tm_min*60+t->tm_sec)*SIN_SIZE/1800)*RAD/SIN_MUL;
	render_line(MX, MY, x, y, LCD_PIXEL_ON, 2, s);

	x = MX - isin(-t->tm_sec*SIN_SIZE/30)*31/SIN_MUL;
	y = MY + isin(-SIN_SIZE/2 - t->tm_sec*SIN_SIZE/30)*31/SIN_MUL;
	render_line(MX, MY, x, y, LCD_PIXEL_ON, 1, s);
}

int main(int argc, char *args[]) {
	screen_t screen, back;

	init_sqrt();
	init();
	init_clock(back);
	while (1) {
		render_clock(back, screen);
		draw_screen(screen);
		sleep(1);
	}
	clr();
	return 0;
}
