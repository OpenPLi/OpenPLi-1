/*
 *   lcd-ks0713.h - lcd driver for KS0713 and compatible (dbox-II-project)
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
 *   $Log: lcd-ks0713.h,v $
 *   Revision 1.2  2001/11/16 20:49:49  ge0rg
 *   added progress bar (lcd_status()) and text output (lcd_print()) support.
 *
 *   Revision 1.1  2001/04/23 10:01:06  Jolt
 *   LCD support
 *
 *   Revision 1.7  2001/01/28 18:49:08  gillem
 *   add ioctl
 *   LCD_IOCTL_CLEAR
 *   LCD_IOCTL_SET_POS
 *   LCD_IOCTL_GET_POS
 *
 *   Revision 1.6  2001/01/20 19:01:21  gillem
 *   - add pixel function
 *
 *   Revision 1.5  2001/01/06 10:06:35  gillem
 *   cvs check
 *
 *   $Revision: 1.2 $
 *
 */

#define LCD_STAT_BUSY			0x80
#define LCD_STAT_ADC			0x40
#define LCD_STAT_ON				0x20
#define LCD_STAT_RESETB		    0x10

#define LCD_POWERC_VC			0x04
#define LCD_POWERC_VR			0x02
#define LCD_POWERC_VF			0x01

#define	LCD_MODE_ASC			0
#define	LCD_MODE_BIN			1

#define LCDSET                  0x1000
#define LCDGET                  0x2000

#define LCD_IOCTL_STATUS		(1 |LCDGET)
#define LCD_IOCTL_ON			(2 |LCDSET)
#define	LCD_IOCTL_EON			(3 |LCDSET)
#define LCD_IOCTL_REVERSE		(4 |LCDSET)
#define	LCD_IOCTL_BIAS			(5 |LCDSET)
#define	LCD_IOCTL_ADC			(6 |LCDSET)
#define	LCD_IOCTL_SHL			(7 |LCDSET)
#define	LCD_IOCTL_RESET			(8)
#define	LCD_IOCTL_IDL			(9 |LCDSET)
#define	LCD_IOCTL_SRV			(10|LCDSET)
#define	LCD_IOCTL_SMR			(11)
#define	LCD_IOCTL_RMR			(12)
#define	LCD_IOCTL_POWERC		(13|LCDSET)
#define LCD_IOCTL_SEL_RES		(14|LCDSET)
#define LCD_IOCTL_SIR			(15|LCDSET)
#define LCD_IOCTL_SPAGE			(16|LCDSET)
#define LCD_IOCTL_SROW			(16|LCDSET)
#define LCD_IOCTL_SCOLUMN		(17|LCDSET)
#define LCD_IOCTL_SET_ADDR		(18)
#define LCD_IOCTL_READ_BYTE		(19|LCDGET)
#define LCD_IOCTL_WRITE_BYTE	(20|LCDSET)
#define LCD_IOCTL_ASC_MODE		(21|LCDSET)
#define LCD_IOCTL_SET_PIXEL     (22)
#define LCD_IOCTL_GET_POS       (23)
#define LCD_IOCTL_SET_POS       (24)
#define LCD_IOCTL_CLEAR         (25)

#define LCD_ROWS				8
#define LCD_COLS				120
#define LCD_BUFFER_SIZE			( LCD_ROWS * LCD_COLS )

#define LCD_PIXEL_OFF           0
#define LCD_PIXEL_ON            1
#define LCD_PIXEL_INV           2

typedef struct lcd_pixel {
 unsigned char x;
 unsigned char y;
 unsigned char v;   // 0 = off 1 = on 2 = inv
} lcd_pixel;

typedef struct lcd_pos {
 unsigned char row;
 unsigned char col;
} lcd_pos;


int lcd_init(void);
void lcd_set_pos( int row, int col );
void lcd_write_byte( int data );
void lcd_read_dram( unsigned char * dest );
void lcd_write_dram( unsigned char * dest );
void lcd_clear(void);
int lcd_set_pixel( struct lcd_pixel * pix );
void lcd_status(int y, unsigned char percent);


