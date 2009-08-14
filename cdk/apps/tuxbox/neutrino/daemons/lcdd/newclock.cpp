/*      
        LCD-Daemon  -   DBoxII-Project

        Copyright (C) 2001 Steffen Hehn 'McClean'
        Homepage: http://dbox.cyberphoria.org/



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


#include <config.h>
#include "newclock.h"

static bool	time_digits[24*32 * 10];
static bool	days[24*16 * 7];
static bool	date_digits[16*16 * 10];
static bool	months[32*16 * 12];

static bool	signs[] = {1, 1, 1, 1,
					   1, 1, 1, 1,
					   1, 1, 1, 1,
					   1, 1, 1, 1,

					   1, 1, 1, 0,
					   1, 1, 1, 0,
					   0, 1, 1, 0,
					   1, 1, 0, 0,

					   1, 1, 1, 0,
					   1, 1, 1, 0,
					   1, 1, 1, 0,
					   0, 0, 0, 0};

void InitNewClock()
{
	FILE *fd;
	char filename_usr[] = CONFIGDIR "/lcdd/clock/t_a.bmp";
	char filename_std[] = DATADIR   "/lcdd/clock/t_a.bmp";
	char *filename = &filename_usr[0];
	int	 digit_pos = sizeof(filename_usr)-6;
	char line_buffer[4];
	int	 row, byte, bit;
	unsigned char BMPWidth;
	unsigned char BMPHeight;

	//time

	while(filename[digit_pos] <= 'j')
	{
		if((fd = fopen(filename, "rb")) == 0)
		{
			printf("[lcdd] time-skin not found -> using default...\n");
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos] = 'a';
			continue;
		}

		fseek(fd, 0x12, SEEK_SET);
		fread(&BMPWidth, 1, 1, fd);
		fseek(fd, 0x16, SEEK_SET);
		fread(&BMPHeight, 1, 1, fd);

		if(BMPWidth > 24 || BMPHeight > 32)
		{
			printf("[lcdd] time-skin not supported -> using default...\n");
			fclose(fd);
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos] = 'a';
			continue;
		}

		fseek(fd, 0x3E, SEEK_SET);

		for(row = 32-1; row >= 0; row--)
		{
			fread(&line_buffer, 1, sizeof(line_buffer), fd);

			for(byte = 0; byte < 24/8; byte++)
			{
				for(bit = 7; bit >= 0; bit--)
				{
					if(line_buffer[byte] & 1<<bit)	time_digits[7-bit + byte*8 + row*24 + (filename[digit_pos] - 'a')*sizeof(time_digits)/10] = 1;
					else							time_digits[7-bit + byte*8 + row*24 + (filename[digit_pos] - 'a')*sizeof(time_digits)/10] = 0;
				}
			}
		}

		fclose(fd);

		filename[digit_pos]++;
	}

	//weekday

	filename = &filename_usr[0];
	digit_pos = sizeof(filename_usr)-6;
	filename[digit_pos-2] = 'w';
	filename[digit_pos]   = 'a';

	while(filename[digit_pos] <= 'g')
	{
		if((fd = fopen(filename, "rb")) == 0)
		{
			printf("[lcdd] weekday-skin not found -> using default...\n");
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos-2] = 'w';
			filename[digit_pos]   = 'a';
			continue;
		}

		fseek(fd, 0x12, SEEK_SET);
		fread(&BMPWidth, 1, 1, fd);
		fseek(fd, 0x16, SEEK_SET);
		fread(&BMPHeight, 1, 1, fd);

		if(BMPWidth > 24 || BMPHeight > 16)
		{
			printf("[lcdd] weekday-skin not supported -> using default...\n");
			fclose(fd);
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos-2] = 'w';
			filename[digit_pos]   = 'a';
			continue;
		}

		fseek(fd, 0x3E, SEEK_SET);

		for(row = 16-1; row >= 0; row--)
		{
			fread(&line_buffer, 1, sizeof(line_buffer), fd);

			for(byte = 0; byte < 24/8; byte++)
			{
				for(bit = 7; bit >= 0; bit--)
				{
					if(line_buffer[byte] & 1<<bit)	days[7-bit + byte*8 + row*24 + (filename[digit_pos] - 'a')*sizeof(days)/7] = 1;
					else							days[7-bit + byte*8 + row*24 + (filename[digit_pos] - 'a')*sizeof(days)/7] = 0;
				}
			}
		}

		fclose(fd);

		filename[digit_pos]++;
	}

	//date

	filename = &filename_usr[0];
	digit_pos = sizeof(filename_usr)-6;
	filename[digit_pos-2] = 'd';
	filename[digit_pos]   = 'a';

	while(filename[digit_pos] <= 'j')
	{
		if((fd = fopen(filename, "rb")) == 0)
		{
			printf("[lcdd] date-skin not found -> using default...\n");
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos-2] = 'd';
			filename[digit_pos]   = 'a';
			continue;
		}

		fseek(fd, 0x12, SEEK_SET);
		fread(&BMPWidth, 1, 1, fd);
		fseek(fd, 0x16, SEEK_SET);
		fread(&BMPHeight, 1, 1, fd);

		if(BMPWidth > 16 || BMPHeight > 16)
		{
			printf("[lcdd] date-skin not supported -> using default...\n");
			fclose(fd);
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos-1] = 'd';
			filename[digit_pos]   = 'a';
			continue;
		}

		fseek(fd, 0x3E, SEEK_SET);

		for(row = 16-1; row >= 0; row--)
		{
			fread(&line_buffer, 1, sizeof(line_buffer), fd);

			for(byte = 0; byte < 16/8; byte++)
			{
				for(bit = 7; bit >= 0; bit--)
				{
					if(line_buffer[byte] & 1<<bit)	date_digits[7-bit + byte*8 + row*16 + (filename[digit_pos] - 'a')*sizeof(date_digits)/10] = 1;
					else							date_digits[7-bit + byte*8 + row*16 + (filename[digit_pos] - 'a')*sizeof(date_digits)/10] = 0;
				}
			}
		}

		fclose(fd);

		filename[digit_pos]++;
	}

	//month

	filename = &filename_usr[0];
	digit_pos = sizeof(filename_usr)-6;
	filename[digit_pos-2] = 'm';
	filename[digit_pos]   = 'a';

	while(filename[digit_pos] <= 'l')
	{
		if((fd = fopen(filename, "rb")) == 0)
		{
			printf("[lcdd] month-skin not found -> using default...\n");
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos-2] = 'm';
			filename[digit_pos]   = 'a';
			continue;
		}

		fseek(fd, 0x12, SEEK_SET);
		fread(&BMPWidth, 1, 1, fd);
		fseek(fd, 0x16, SEEK_SET);
		fread(&BMPHeight, 1, 1, fd);

		if(BMPWidth > 32 || BMPHeight > 16)
		{
			printf("[lcdd] month-skin not supported -> using default...\n");
			fclose(fd);
			filename = &filename_std[0];
			digit_pos = sizeof(filename_std)-6;
			filename[digit_pos-2] = 'm';
			filename[digit_pos]   = 'a';
			continue;
		}

		fseek(fd, 0x3E, SEEK_SET);

		for(row = 16-1; row >= 0; row--)
		{
			fread(&line_buffer, 1, sizeof(line_buffer), fd);

			for(byte = 0; byte < 32/8; byte++)
			{
				for(bit = 7; bit >= 0; bit--)
				{
					if(line_buffer[byte] & 1<<bit)	months[7-bit + byte*8 + row*32 + (filename[digit_pos] - 'a')*sizeof(months)/12] = 1;
					else							months[7-bit + byte*8 + row*32 + (filename[digit_pos] - 'a')*sizeof(months)/12] = 0;
				}
			}
		}

		fclose(fd);

		filename[digit_pos]++;
	}
}

void RenderSign(CLCDDisplay* display, int sign, int x_position, int y_position)
{
	int x, y;

	for(y = 0; y < 4; y++)
	{
		for(x = 0; x < 4; x++)
		{
			if(signs[x + y*4 + sign*sizeof(signs)/3])	display->draw_point(x_position + x, y_position + y, CLCDDisplay::PIXEL_ON);
			else										display->draw_point(x_position + x, y_position + y, CLCDDisplay::PIXEL_OFF);
		}
	}
}

void RenderTimeDigit(CLCDDisplay* display, int digit, int position)
{
	int x, y;

	for(y = 0; y < 32; y++)
	{
		for(x = 0; x < 24; x++)
		{
			if(time_digits[x + y*24 + digit*sizeof(time_digits)/10])	display->draw_point(position + x, 5 + y, CLCDDisplay::PIXEL_ON);
			else														display->draw_point(position + x, 5 + y, CLCDDisplay::PIXEL_OFF);
		}
	}
}

void RenderDay(CLCDDisplay* display, int day)
{
	int x, y;

	for(y = 0; y < 16; y++)
	{
		for(x = 0; x < 24; x++)
		{
			if(days[x + y*24 + day*sizeof(days)/7])	display->draw_point(5 + x, 43 + y, CLCDDisplay::PIXEL_ON);
			else									display->draw_point(5 + x, 43 + y, CLCDDisplay::PIXEL_OFF);
		}
	}
}

void RenderDateDigit(CLCDDisplay* display, int digit, int position)
{
	int x, y;

	for(y = 0; y < 16; y++)
	{
		for(x = 0; x < 16; x++)
		{
			if(date_digits[x + y*16 + digit*sizeof(date_digits)/10])	display->draw_point(position + x, 43 + y, CLCDDisplay::PIXEL_ON);
			else														display->draw_point(position + x, 43 + y, CLCDDisplay::PIXEL_OFF);
		}
	}
}

void RenderMonth(CLCDDisplay* display, int month)
{
	int x, y;

	for(y = 0; y < 16; y++)
	{
		for(x = 0; x < 32; x++)
		{
			if(months[x + y*32 + month*sizeof(months)/12])	display->draw_point(83 + x, 43 + y, CLCDDisplay::PIXEL_ON);
			else											display->draw_point(83 + x, 43 + y, CLCDDisplay::PIXEL_OFF);
		}
	}
}

void ShowNewClock(CLCDDisplay* display, int hour, int minute, int day, int date, int month)
{
	RenderTimeDigit(display, hour/10, 5);
	RenderTimeDigit(display, hour%10, 32);
	RenderTimeDigit(display, minute/10, 64);
	RenderTimeDigit(display, minute%10, 91);

	RenderDay(display, day);

	RenderDateDigit(display, date/10, 43);
	RenderDateDigit(display, date%10, 60);

	RenderMonth(display, month);

	RenderSign(display, 0, 58, 15);
	RenderSign(display, 0, 58, 23);
	RenderSign(display, 1, 31, 57);
	RenderSign(display, 2, 78, 56);
}
