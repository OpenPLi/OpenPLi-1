/* bcdclock (C) 2001 by Ge0rG

   das ist eine BCD-Uhr. Die Uhr zeigt HH:MM:SS in Binärdarstellung an,
   jede Ziffer entspricht einer Spalte am Bildschirm. Viel Spass beim
   ablesen...

   Ach ja, dieses Programm unterliegt der GPL, also nix klauen ;o)
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>


#define DOT_SIZE 1
#define X_DOTS 120
#define Y_DOTS 64


typedef unsigned char field[Y_DOTS][X_DOTS];

typedef unsigned char screen_t[LCD_BUFFER_SIZE];

int lcd_fd;

void clr() {
	if (ioctl(lcd_fd,LCD_IOCTL_CLEAR) < 0) {
		perror("clr()");
		exit(1);
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


void rnd_gol(field f) {
	int x, y;

	for (y=0; y<Y_DOTS; y++)
		for (x=0; x<X_DOTS; x++) {
			f[y][x] = random() & 1;
		}
}

void gol(field from, field to) {
	int x, y, count, x1, x2, y1, y2;

	y = random() % Y_DOTS;
	x = random() % X_DOTS;
	//printf("x=%i y=%i n=%i\n", x, y, from[y][x]);
	from[y][x] = 1;
	for (y=0; y<Y_DOTS; y++)
		for (x=0; x<X_DOTS; x++) {
			//     x1 x  x2
			// y1 [X][X][X]
			// y  [X]>·<[X]
			// y2 [X][X][X]
			x1 = (x+X_DOTS-1) % X_DOTS;
			x2 = (x+1) % X_DOTS;
			y1 = (y+Y_DOTS-1) % Y_DOTS;
			y2 = (y+1) % Y_DOTS;
			count = from[y1][x1] + from[y1][x] + from[y1][x2] +
			        from[y][x1]  +               from[y][x2]  + 
			        from[y2][x1] + from[y2][x] + from[y2][x2];
			switch(count) {
			case 1:
				to[y][x] = 0;
				break;
			case 2:
				to[y][x] = from[y][x];
				break;
			case 3:
				to[y][x] = 1;
				break;
			default:
				to[y][x] = 0;
				break;

			}
		}
}


void draw_gol(field f, screen_t s) {
	int x, y, yy, pix;

	for (y=0; y<LCD_ROWS; y++) {
		for (x=0; x<LCD_COLS; x++) {
			pix = 0;
			for (yy=7; yy>=0; yy--) {
				pix = (pix<<1) + f[8*y+yy][x];
			}
			s[y*LCD_COLS+x] = pix;
		}
	}
}

int main(int argc, char *args[]) {
	screen_t screen;
	field *f1, *f2, *tmp;

	f1 = malloc(sizeof(field));
	f2 = malloc(sizeof(field));
	init();
	rnd_gol(*f1);
	while (1) {
		gol(*f1, *f2);
		draw_gol(*f2, screen);
		draw_screen(screen);
		tmp = f1; f1 = f2; f2 = tmp;
		usleep(1000*100);
	}
	clr();
	return 0;
}
