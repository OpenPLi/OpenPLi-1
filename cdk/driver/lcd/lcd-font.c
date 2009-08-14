/*
 *   lcd-font.c - lcd font driver (dbox-II-project)
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
 *   $Log: lcd-font.c,v $
 *   Revision 1.5  2002/08/12 17:16:27  obi
 *   removed compiler warnings
 *
 *   Revision 1.4  2001/12/01 06:53:17  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.3  2001/06/03 20:45:50  kwon
 *   indent
 *
 *   Revision 1.2  2001/01/06 10:06:35  gillem
 *   cvs check
 *
 *   $Revision: 1.5 $
 *
 */

#include <linux/slab.h>
#include "lcd-font.h"
#include "font_acorn_8x8.h"
/*
#include "font_8x8.h"
#include "font_pearl_8x8.h"
*/

unsigned char *lcd_font;

///////////////////////////////////////////////////////////////////////////////

void
lcd_init_font (unsigned char *fontdata)
{
        int i, x, y;

	if (fontdata == 0)
	{
/*                fontdata = fontdata_pearl8x8;
                fontdata = fontdata_8x8;*/
                fontdata = acorndata_8x8;

                for (i = 0; i < 256; i++)
                {
                        char r[8];
                        memcpy (r, fontdata+i*8, 8);
                        memset (fontdata+i*8, 0, 8);
                        for (x = 0; x < 8; x++)
                                for (y = 0; y < 8; y++)
                                        if (r[x] & (1 << (7 - y)))
                                                fontdata[i*8+y] |= 1 << x;
                }
        }

	lcd_font = fontdata;
}

///////////////////////////////////////////////////////////////////////////////

void
lcd_convert_to_font (unsigned char *dest, unsigned char *source, int slen)
{
	int i;

	for (i = 0;i < slen; i++)
		memcpy (dest + (i*8), lcd_font + (source[i]*8), 8);
}

///////////////////////////////////////////////////////////////////////////////
