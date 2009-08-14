/*
 *   cxa2092.h - audio/video switch driver (dbox-II-project)
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
 *   $Log: cxa2092.h,v $
 *   Revision 1.12  2002/02/28 20:42:45  gillem
 *   - some changes
 *   - add vcr/tv slow blanking event
 *
 *   Revision 1.11  2002/01/01 14:16:28  gillem
 *   - update
 *
 *   Revision 1.10  2001/03/16 20:49:21  gillem
 *   - fix errors
 *
 *   Revision 1.9  2001/03/15 22:20:23  Hunz
 *   nothing important...
 *
 *   Revision 1.8  2001/03/03 11:02:57  gillem
 *   - cleanup
 *
 *   Revision 1.7  2001/01/28 09:05:42  gillem
 *   some fixes
 *
 *   Revision 1.6  2001/01/20 19:18:11  gillem
 *   - add AVSIOGSTATUS
 *
 *   Revision 1.5  2001/01/16 19:39:15  gillem
 *   some new ioctls
 *
 *   Revision 1.4  2001/01/15 19:50:08  gillem
 *   - bug fix
 *   - add test appl.
 *
 *   Revision 1.3  2001/01/15 17:02:32  gillem
 *   rewriten
 *
 *   Revision 1.2  2001/01/06 10:05:43  gillem
 *   cvs check
 *
 *   $Revision: 1.12 $
 *
 */

#ifdef __KERNEL__
int cxa2092_init(struct i2c_client *client);
int cxa2092_command(struct i2c_client *client, unsigned int cmd, void *arg );
int cxa2092_set_volume( struct i2c_client *client, int vol );
int cxa2092_get_volume(void);
int cxa2092_get_status(struct i2c_client *client);
#endif

