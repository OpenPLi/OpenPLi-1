#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "raw.h"

int bmp2raw(struct bmp_header bh, unsigned char *bmp, lcd_raw_buffer raw) {
	int x, y, ofs, linesize;
	linesize = ((bh.width*bh.bit_count / 8) + 3) & ~3;
	switch (bh.bit_count) {
	case 1:
		for (y=0; y<64; y++) { for (x=0; x<120; x++) {
			ofs = (bh.height - 1 - y)*linesize + (x>>3);
			raw[y][x] = 255*((bmp[ofs]>>(7-(x&7)))&1);
		} }
		break;
	case 4:
		for (y=0; y<64; y++) { for (x=0; x<60; x++) {
			ofs = (bh.height - 1 - y)*linesize + x;
			raw[y][x*2] = bmp[ofs] >> 4;
			raw[y][x*2+1] = bmp[ofs] & 0x0F;
		} }
		break;
	default:
		return -1;
	}
	return 0;
}
