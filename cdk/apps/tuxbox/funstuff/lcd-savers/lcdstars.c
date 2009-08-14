/* das vielleicht sinnloseste Programm für die d-box (C) 2001 by Ge0rG

   INSTALLATION:
     1. #include für lcd-ks0713.h an eigenen Pfad anpassen
     2. powerpc-linux-gcc lcdstars.c -o lcdstars
     3. auf d-box kopieren
     4. lcdd beenden
     5. lcdstars ausführen
     6. gespannt zukucken (nur wenn man direkt von vorn draufkuckt, sieht
        es auch wirklich schick aus)

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

#define STARS 40

typedef unsigned char screen_t[LCD_BUFFER_SIZE];

int lcd_fd;

void clr() {
	if (ioctl(lcd_fd,LCD_IOCTL_CLEAR) < 0) {
		perror("clr()");
		exit(1);
	}
}

int s_x[STARS], s_y[STARS], s_z[STARS];

void s_nova(int idx) {
	s_x[idx] = random() / (RAND_MAX/40000) - 20000;
	s_y[idx] = random() / (RAND_MAX/20000) - 10000;
	s_z[idx] = 0+(random() / (RAND_MAX/800));
	//printf("s_nova(%i): %i %i %i\n", idx, s_x[idx], s_y[idx], s_z[idx]);
}

void s_update(screen_t scr) {
	int s, x, y;
	memset(scr, 0, LCD_BUFFER_SIZE);
	for (s = 0; s < STARS; s++) {
		x = 60 + s_x[s] / s_z[s];
		y = 32 - s_y[s] / s_z[s];
		if ((x<0) || (x>=120) || (y<0) || (y>=64) || (s_z[s]<1))
			s_nova(s);
		else {
			scr[x + (y>>3)*LCD_COLS] |= (1<<(y&7));
			s_z[s]--;
			x = 60 + s_x[s] / s_z[s];
			y = 32 - s_y[s] / s_z[s];
			if ((x<0) || (x>=120) || (y<0) || (y>=64) || (s_z[s]<1))
				s_nova(s);
			else {
				s_z[s]-=9;
				scr[x + (y>>3)*LCD_COLS] |= (1<<(y&7));
			}
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
	for (i=0; i<STARS; i++) {
		s_x[i] = random() / (RAND_MAX/40000) - 20000;
		s_y[i] = random() / (RAND_MAX/20000) - 10000;
		s_z[i] = random() / (RAND_MAX/1000);
	}
}

void draw_screen(screen_t s) {
	write(lcd_fd, s, LCD_BUFFER_SIZE);
}


int main(int argc, char *args[]) {
	screen_t screen;

	init();
	while (1) {
		s_update(screen);
		draw_screen(screen);
		usleep(80000);
	}
	clr();
	return 0;
}
