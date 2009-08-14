/*
	LCD-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
		baseroutines by Shadow_
	Homepage: http://mcclean.cyberphoria.org/



	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
 
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

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
		string	iconBasePath;
	public:
		
		enum
		{
			PIXEL_ON = LCD_PIXEL_ON,
			PIXEL_OFF = LCD_PIXEL_OFF,
			PIXEL_INV =	LCD_PIXEL_INV
		};
		
		CLCDDisplay();
		~CLCDDisplay();

		int invalid_col(int x);
		int invalid_row(int y);
		void convert_data();
		int sgn(int arg);
		void setIconBasePath(string bp){iconBasePath=bp;};


		void update();

		void draw_point (int x,int y, int state);
		void draw_line (int x1, int y1, int x2, int y2, int state);
		void draw_fill_rect (int left,int top,int right,int bottom,int state);
		void draw_rectangle (int left,int top, int right, int bottom, int linestate,int fillstate);
		void draw_polygon(int num_vertices, int *vertices, int state);
		void draw_char(int x, int y, char c);
		void draw_string(int x, int y, char *string);

		void paintIcon(string filename, int x, int y, int col);
};

#endif
