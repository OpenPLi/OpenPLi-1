/*
 *	event.h - global event driver (dbox-II-project)
 *
 *	Homepage: http://dbox2.elxsi.de
 *
 *	Copyright (C) 2001 Gillem (gillem@berlios.de)
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *	$Log: event.h,v $
 *	Revision 1.5.4.1  2003/06/19 14:31:28  tmbinc
 *	added frame-rate-changed event
 *	
 *	Revision 1.5  2002/03/02 18:33:13  tmbinc
 *	changed VCR_ON/OFF to _CHANGED, added IOCTL to get status
 *	
 *	Revision 1.4  2002/02/28 20:41:46  gillem
 *	- add vcr/tv slow blanking event
 *	
 *	Revision 1.3  2001/12/19 19:46:47  gillem
 *	- some work on event-filter
 *	
 *	Revision 1.2  2001/12/18 17:58:52  gillem
 *	- add events
 *	- TODO: filter events
 *	
 *	Revision 1.1  2001/12/08 15:12:33  gillem
 *	- initial release
 *	
 *
 *	$Revision: 1.5.4.1 $
 *
 */

#ifndef EVENT_H
#define EVENT_H

/* global event defines
 */
#define EVENT_NOP		0
#define EVENT_VCR_CHANGED		1
#define EVENT_VHSIZE_CHANGE	4
#define EVENT_ARATIO_CHANGE	8
#define EVENT_SBTV_CHANGE	16	/* avs event pin 8 tv */
#define EVENT_SBVCR_CHANGE	32	/* avs event pin 8 vcr */
#define EVENT_FRATE_CHANGE  64  /* framerate has changed */

#define EVENT_SET_FILTER	1

/* other data
 */

struct event_t {
	unsigned int event;
} event_t;

#ifdef __KERNEL__
extern int event_write_message( struct event_t * buf, size_t count );
#endif

#endif
