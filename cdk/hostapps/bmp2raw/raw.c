#include "raw.h"

void packed2raw(lcd_packed_buffer source, lcd_raw_buffer dest) {
	int x, y, pix;
	for (y=0; y<64; y++) {
		for (x=0; x<120; x++) {
			pix = (source[y>>3][x] >> (y&7)) & 1;
			dest[y][x] = pix*255;
		}
	}
}
void raw2packed(lcd_raw_buffer source, lcd_packed_buffer dest) {
	int x, y, y_sub, pix;

	for (y=0; y<8; y++) {
		for (x=0; x<120; x++) {
			pix = 0;
			for (y_sub=7; y_sub>=0; y_sub--) {
				pix = pix<<1;
				if (source[y*8+y_sub][x]) pix++;
			}
			dest[y][x] = pix;
		}
	}
}

void raw2raw4bit(lcd_raw_buffer source, lcd_raw4bit_buffer dest) {
	int x, y;

	for (y=0; y<64; y++) {
		for (x=0; x<60; x++) {
			dest[y][x] = ((source[y][x*2]<<4) & 0xf0) +
			             (source[y][x*2+1] & 0x0f);
		}
	}
}
