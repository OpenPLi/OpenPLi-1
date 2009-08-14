/*
	NeutrinoNG  -   DBoxII-Project

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


#ifndef __neutrino_debug__
#define __neutrino_debug__

static int debug = 1;


#define DEBUG_NORMAL	1
#define DEBUG_INFO	2
#define DEBUG_DEBUG	3

void setDebugLevel( int level );

#define dprintf(debuglevel, fmt, args...) {if(debug>=debuglevel) printf( "[neutrino] " fmt, ## args);}
#define dperror(str) {perror("[neutrino] " str);}


#endif
