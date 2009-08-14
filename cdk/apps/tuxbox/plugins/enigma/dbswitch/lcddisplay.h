#ifndef __lcddisplay__
#define __lcddisplay__

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#include <string>

#include <dbox/lcd-ks0713.h>

using namespace std;

class CLCDDisplay
{
	private:
		unsigned char raw[132][64];
		unsigned char lcd[LCD_ROWS][LCD_COLS];
		int fd;
	public:
		
		enum { PIXEL_ON = LCD_PIXEL_ON, PIXEL_OFF = LCD_PIXEL_OFF, PIXEL_INV = LCD_PIXEL_INV };

		CLCDDisplay();
		~CLCDDisplay();

		int invalid_col(int x);
		int invalid_row(int y);
		void convert_data();
		void update();
		void draw_point (int x,int y, int state);
		void draw_char(int x, int y, char c);
		void draw_string(int x, int y, char *string);
};

#endif
