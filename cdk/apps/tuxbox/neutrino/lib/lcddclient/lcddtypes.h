/*
 * $Header: /cvs/tuxbox/apps/tuxbox/neutrino/lib/lcddclient/Attic/lcddtypes.h,v 1.1 2002/10/13 20:49:41 thegoodguy Exp $
 *
 * lcdd's types which are used by the clientlib - d-box2 linux project
 *
 * (C) 2002 by thegoodguy <thegoodguy@berlios.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef __lcddtypes_h__
#define __lcddtypes_h__

class CLcddTypes
{
 public:

	enum mode
		{
			MODE_TVRADIO,
			MODE_SCART,
			MODE_MENU,
			MODE_SAVER,
			MODE_SHUTDOWN,
			MODE_STANDBY,
			MODE_MENU_UTF8
		};
};

#endif /* __lcddtypes_h__ */
