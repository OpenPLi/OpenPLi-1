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
#include <time.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>

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


static unsigned char led[2][2][16] =
  {
	{ { 0, 224, 16, 8, 4, 2, 2, 2, 2, 2, 4, 8, 16, 224, 0 },
	  { 0, 7, 8, 16, 32, 64, 64, 64, 72, 68, 34, 16, 8, 7, 0 } },

	{ { 0, 224, 240, 248, 252-32, 254-16, 254, 254, 254, 254, 252, 248, 240, 224, 0 },
	  { 0, 7, 15, 31, 63, 127, 127, 127, 127, 127, 63, 31, 15, 7, 0 } },
  };


void render_nibble(int row, int val, screen_t s) {
	int x, b, bit;
	for (b=0; b<4; b++) {
		bit = ((val & (1<<(3-b)))!=0);
		for (x=0; x<16; x++) {
			s[b*2*LCD_COLS + row + x] = led[bit][0][x];
			s[(b*2+1)*LCD_COLS + row + x] = led[bit][1][x];
		}
	}
}

void render_clock(screen_t s) {
        struct timeval tb;
        struct tm *t;
        gettimeofday(&tb, NULL);
	t = localtime(&tb.tv_sec);

	memset(s, 0, LCD_BUFFER_SIZE);

	render_nibble(  0, t->tm_hour / 10, s);
	render_nibble( 16, t->tm_hour % 10, s);

	render_nibble( 44, t->tm_min / 10, s);
	render_nibble( 60, t->tm_min % 10, s);

	render_nibble( 88, t->tm_sec / 10, s);
	render_nibble(104, t->tm_sec % 10, s);

	s[2*LCD_COLS + 32 + 5] = 24;
	s[2*LCD_COLS + 32 + 6] = 24;
	s[5*LCD_COLS + 32 + 5] = 24;
	s[5*LCD_COLS + 32 + 6] = 24;

	s[2*LCD_COLS + 76 + 5] = 24;
	s[2*LCD_COLS + 76 + 6] = 24;
	s[5*LCD_COLS + 76 + 5] = 24;
	s[5*LCD_COLS + 76 + 6] = 24;

}

int main(int argc, char *args[]) {
	screen_t screen;

	init();
	while (1) {
		render_clock(screen);
		draw_screen(screen);
		sleep(1);
	}
	clr();
	return 0;
}
