#ifndef __raw__
#define __raw__

struct raw_header {
	short int width;
	short int height;
	unsigned char trans;
}__attribute((packed));

typedef unsigned char lcd_raw_buffer[64][120];
typedef unsigned char lcd_packed_buffer[8][120];
typedef unsigned char lcd_raw4bit_buffer[64][60];

void packed2raw(lcd_packed_buffer, lcd_raw_buffer);
void raw2packed(lcd_raw_buffer, lcd_packed_buffer);
void raw2raw4bit(lcd_raw_buffer, lcd_raw4bit_buffer);

#endif // __raw__
