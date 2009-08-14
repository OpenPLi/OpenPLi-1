/*
 *   i2c-8xx.h - ppc i2c driver (dbox-II-project)
 *
 *   Copyright (C) 2000-2001 Tmbinc, Gillem (htoa@gmx.net)
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
 *   $Log: i2c-8xx.h,v $
 *   Revision 1.4  2001/02/20 18:37:05  gillem
 *   - remove polling some drivers not work now !
 *
 *   Revision 1.3  2001/01/06 10:06:01  gillem
 *   cvs check
 *
 *   $Revision: 1.4 $
 *
 */

#ifndef _I2C_H_
#define _I2C_H_

void i2c_init(int speed);

void i2c_send( unsigned char address,
				unsigned char secondary_address,
				int enable_secondary,
				unsigned short size, unsigned char dataout[] );

void i2c_receive(unsigned char address,
				unsigned char secondary_address,
				int enable_secondary,
                unsigned short size_to_expect, unsigned char datain[] );

#endif
