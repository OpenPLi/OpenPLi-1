#include "lcd-ks0713.h"
#include "font_6x11.h"
#include "lcd-font.h"


void lcd_print(int x, int y, char *text, unsigned char c0, unsigned char c1) {
	int c;
	unsigned char color;
	struct lcd_pixel pix;

	for (c = 0; c<strlen(text); c++) {
		int xx, yy;

		for (yy=0; yy < FONT_Y; yy++) {
			for (xx=0; xx < FONT_X; xx++) {
				if ((fontdata[text[c]*11+yy]<<xx) & 0x80) {
					// zeichen da
					pix.v = c1;
				} else {
					// zeichen nicht da
					pix.v = c0;
				}
				if (pix.v != LCD_PIXEL_KEEP) {
					pix.x = x+c*FONT_X+xx;
					pix.y = y+yy;
					lcd_set_pixel(&pix);
				}
			}
		}
	}
}
