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
 *   $Log: lcd-font.h,v $
 *   Revision 1.3  2001/12/01 06:53:17  gillem
 *   - malloc.h -> slab.h
 *
 *   Revision 1.2  2001/01/06 10:06:35  gillem
 *   cvs check
 *
 *   $Revision: 1.3 $
 *
 */

void lcd_init_font( unsigned char *fontdata );
void lcd_convert_to_font( unsigned char *dest, unsigned char *source, int slen );
