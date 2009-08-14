/*
 *   lcd-console.c - lcd console driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2001 Gillem (htoa@gmx.net)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *
 *   $Log: lcd-console.c,v $
 *   Revision 1.10  2002/09/23 17:07:02  obi
 *   moved lcd-ks0713.h to include/dbox
 *
 *   Revision 1.9  2001/12/01 06:53:17  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.8  2001/09/17 21:25:55  TripleDES
 *   removed the "@lcd"
 *
 *   Revision 1.7  2001/01/28 19:47:12  gillem
 *   - fix setpos ...
 *
 *   Revision 1.6  2001/01/28 18:49:08  gillem
 *   add ioctl
 *   LCD_IOCTL_CLEAR
 *   LCD_IOCTL_SET_POS
 *   LCD_IOCTL_GET_POS
 *
 *   Revision 1.5  2001/01/26 23:51:33  gillem
 *   some kernel styles change
 *
 *   Revision 1.4  2001/01/06 10:06:35  gillem
 *   cvs check
 *
 *   $Revision: 1.10 $
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/smp_lock.h>
#include <linux/delay.h>

#include "lcd-console.h"
#include "lcd-font.h"
#include <dbox/lcd-ks0713.h>

#define MAX_COL 15
#define MAX_ROW 8

#define INITSTRING "@lcd:\x0a"

static int row,col;

///////////////////////////////////////////////////////////////////////////////

void lcd_init_console(void)
{
	lcd_init_font(0);
	lcd_clear();

    row = 0;
    col = 0;

	//lcd_console_put_data(INITSTRING,strlen(INITSTRING));
}

///////////////////////////////////////////////////////////////////////////////

void lcd_console_put_data( unsigned char *data, int len )
{
	int i;

	for(i=0;i<len;i++) {

		switch(data[i]) {

			case 0x0A:
                    lcd_console_new_line();
					continue;
					break;

			default:
                    break;
		}

    	if (col == MAX_COL) {
			col=0;
			row++;
		}

		if ( row == MAX_ROW ) {
			lcd_console_new_line();
			row--;
		}

		lcd_console_put_char( data[i] );
		col++;
	}
}

///////////////////////////////////////////////////////////////////////////////

void lcd_console_put_char( unsigned char data )
{
	int i;
	unsigned char b[8];

	// load font to b
	lcd_convert_to_font( b, &data, 1 );

	lcd_set_pos( row, col*8 );

	for(i=0;i<8;i++) {
		lcd_write_byte( b[i] );
	}
}

///////////////////////////////////////////////////////////////////////////////

void lcd_console_new_line()
{
	for(;col<=MAX_COL;col++) {
		lcd_console_put_char(0x20);
	}

    if ( row == MAX_ROW ) {
		row--;
		lcd_console_scroll_down( 1 );

		for(col=0;col<=MAX_COL;col++) {
			lcd_console_put_char(0x20);
		}
	}

	row++;
	col=0;
}

///////////////////////////////////////////////////////////////////////////////

void lcd_console_scroll_down( int i )
{
	static unsigned char d[LCD_BUFFER_SIZE+LCD_COLS];

	lcd_read_dram(d);
	memset( d+LCD_BUFFER_SIZE, 0x00, LCD_COLS );
	lcd_write_dram(d+LCD_COLS);
}

///////////////////////////////////////////////////////////////////////////////
