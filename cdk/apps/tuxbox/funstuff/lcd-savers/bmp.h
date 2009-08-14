#ifndef __bmp__
#define __bmp__

#include "raw.h"

struct bmp_header {
	unsigned char _B;	// = 'B'
	unsigned char _M;	// = 'M'
	long int file_size;	// file size
	long int reserved;	// = 0
	long int data_offset;	// start of raw data
	// bitmap info header starts here
	long int header_size;	// = 40
	long int width;		// = 120
	long int height;	// = 64
	short int planes;	// = 1
	short int bit_count;	// 1..24
	long int compression;	// = 0
	long int image_size;	// 120*64*bitcount/8
	long int x_ppm;		// X pixels/meter
	long int y_ppm;		// Y pixels/meter
	long int colors_used;	// 
	long int colors_vip;	// important colors (all=0)
}__attribute((packed));

struct bmp_color {
	unsigned char r, g, b, reserved;
}__attribute((packed));

int bmp2raw(struct bmp_header bh, unsigned char *bmp, lcd_raw_buffer raw);

#endif // __bmp__
