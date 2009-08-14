/*
 *   stv6412.h - audio/video switch driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2000-2002 Gillem gillem@berlios.de
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
 *   $Log: stv6412.h,v $
 *   Revision 1.3  2002/02/28 20:42:45  gillem
 *   - some changes
 *   - add vcr/tv slow blanking event
 *
 *   Revision 1.2  2002/01/01 14:16:28  gillem
 *   - update
 *
 *   Revision 1.1  2001/05/26 09:19:50  gillem
 *   - initial release
 *
 *
 *   $Revision: 1.3 $
 *
 */

#ifdef __KERNEL__
int stv6412_init(struct i2c_client *client);
int stv6412_command(struct i2c_client *client, unsigned int cmd, void *arg );
int stv6412_set_volume( struct i2c_client *client, int vol );
int stv6412_get_volume(void);
int stv6412_get_status(struct i2c_client *client);
#endif

