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

#include "lcddisplay.h"
#include "font.h"

CLCDDisplay::CLCDDisplay()
{
	//open device
	if((fd = open("/dev/dbox/lcd0",O_RDWR)) < 0)
	{
		perror("LCD (/dev/dbox/lcd0)");
		return;
	}

	//clear the display
	if ( ioctl(fd,LCD_IOCTL_CLEAR) < 0)
	{
		perror ( "clear failed");
	}
	
	//graphic (binary) mode 
	int i=LCD_MODE_BIN;
	if ( ioctl(fd,LCD_IOCTL_ASC_MODE,&i) < 0 )
		perror ("graphic mode failed");

	iconBasePath = "";
}


CLCDDisplay::~CLCDDisplay()
{
	close(fd);
}

int CLCDDisplay::invalid_col (int x)
{	
	if( x > LCD_COLS ) 
		return -1;
	if( x < 0 )
		return -1;
	return 0;
}

int CLCDDisplay::invalid_row (int y)
{
	if( y > LCD_ROWS * 8 )
		return -1;
	if( y < 0)
		return -1;
	return 0;   
}

void CLCDDisplay::convert_data ()
{
	int x,y,z;
	char tmp2[4];
	for(x=0;x < LCD_COLS;x++) {   
		for(y=0;y < LCD_ROWS;y++) {
			tmp2[y] = 0;
			for(z=0;z <= 7;z++) {
				if(raw[x][y * 8 + z] == 1){
					tmp2[y]|=1<<z;
				}
			}
			lcd[y][x] = tmp2[y];
		}
	}
}

void CLCDDisplay::update()
{
	convert_data();
	write(fd, lcd, 120*64/8);
}

int CLCDDisplay::sgn (int arg) 
{
	if(arg<0)
		return -1;
	if(arg>0)
		return 1;
	return 0;
}


void CLCDDisplay::draw_point (int x,int y, int state)
{
	if(state == LCD_PIXEL_INV)
	{
		if(raw[x][y] == LCD_PIXEL_ON) 
			raw[x][y] = LCD_PIXEL_OFF;
		else
			raw[x][y] = LCD_PIXEL_ON;
	}
	else
		raw[x][y] = state;
}


/*
 * draw_line
 * 
 * args:
 * x1    StartCol
 * y1    StartRow
 * x2    EndCol
 * y1    EndRow
 * state LCD_PIXEL_OFF/LCD_PIXEL_ON/LCD_PIXEL_INV
 * 
 */

void CLCDDisplay::draw_line (int x1, int y1, int x2, int y2, int state)  
{   
	int dx,dy,sdx,sdy,px,py,dxabs,dyabs,i;
	float slope;
   
	if(invalid_col(x1) || invalid_col(x2))
		return;

	if(invalid_row(y1) || invalid_row(y2))
		return;
	
	dx=x2-x1;      
	dy=y2-y1;      
	dxabs=abs(dx);
	dyabs=abs(dy);
	sdx=sgn(dx);
	sdy=sgn(dy);
	if (dxabs>=dyabs) /* the line is more horizontal than vertical */ {
		slope=(float)dy / (float)dx;
		for(i=0;i!=dx;i+=sdx) {	     
			px=i+x1;
			py=int( slope*i+y1 );
			draw_point(px,py,state);
		}
	}
	else /* the line is more vertical than horizontal */ {	
		slope=(float)dx / (float)dy;
		for(i=0;i!=dy;i+=sdy) {
			px=int(slope*i+x1);
			py=i+y1;
			draw_point(px,py,state);
		}
	}
}


void CLCDDisplay::draw_fill_rect (int left,int top,int right,int bottom,int state) {
	int x,y;
	for(x = left + 1;x < right;x++) {  
		for(y = top + 1;y < bottom;y++) {
			draw_point(x,y,state);
		}
	}
}


void CLCDDisplay::draw_rectangle (int left,int top, int right, int bottom, int linestate,int fillstate)
{
	
	if(invalid_col(left) || invalid_col(right))
		return;

	if(invalid_row(top) || invalid_row(bottom))
		return;

	draw_line(left,top,right,top,linestate);
	draw_line(left,top,left,bottom,linestate);
	draw_line(right,top,right,bottom,linestate);
	draw_line(left,bottom,right,bottom,linestate);
	draw_fill_rect(left,top,right,bottom,fillstate);  
}  


void CLCDDisplay::draw_polygon(int num_vertices, int *vertices, int state) 
{

	//todo: mhh i think checking some coords. would be nice

	int i;
	for(i=0;i<num_vertices-1;i++) {
		draw_line(vertices[(i<<1)+0],
			vertices[(i<<1)+1],
			vertices[(i<<1)+2],
			vertices[(i<<1)+3],
			state);
	}
   
	draw_line(vertices[0],
		vertices[1],
		vertices[(num_vertices<<1)-2],
		vertices[(num_vertices<<1)-1],
		state);
}

void CLCDDisplay::draw_char(int x, int y, char c)
{
	unsigned char *data=&font[8*c];
	for (int ay=0; ay<8; ay++)
		for (int ax=0; ax<8; ax++) 
			if ( (!invalid_col(x+ax)) && (!invalid_row(y+ay)))
				draw_point(x+ax, y+ay, data[ay]&(1<<(7-ax))?PIXEL_ON:PIXEL_OFF);
}

void CLCDDisplay::draw_string(int x, int y, char *string)
{
	while (*string)
	{
		draw_char(x, y, *string++);
		x+=8;
	}
}

void CLCDDisplay::paintIcon(string filename, int x, int y, int col)
{
	short width, height;
	unsigned char tr;

	int fd;
	filename = iconBasePath + filename;

	fd = open(filename.c_str(), O_RDONLY );
	
	if (fd==-1)
	{
		printf("\nerror while loading icon: %s\n\n", filename.c_str() );
		return;
	}

	read(fd, &width,  2 );
	read(fd, &height, 2 );
	read(fd, &tr, 1 );

	width= ((width & 0xff00) >> 8) | ((width & 0x00ff) << 8);
	height=((height & 0xff00) >> 8) | ((height & 0x00ff) << 8);

	unsigned char pixbuf[200];
	for (int count=0; count<height; count ++ )
	{
		read(fd, &pixbuf, width >> 1 );
		unsigned char *pixpos = (unsigned char*) &pixbuf;
		for (int count2=0; count2<width >> 1; count2 ++ )
		{
			unsigned char compressed = *pixpos;
			unsigned char pix1 = (compressed & 0xf0) >> 4;
			unsigned char pix2 = (compressed & 0x0f);

			if (pix1 == col)
				draw_point(x+(count2<<1),y+count, PIXEL_ON);
			else
				draw_point (x+(count2<<1),y+count, PIXEL_OFF);
			if (pix2 == col)
				draw_point(x+(count2<<1)+1,y+count, PIXEL_ON);
			else
				draw_point (x+(count2<<1)+1,y+count, PIXEL_OFF);
			pixpos++;
		}
	}
	
	close(fd);
}
