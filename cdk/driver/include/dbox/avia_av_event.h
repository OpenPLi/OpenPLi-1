/*
 *   avia_av_event.h - event driver for AViA (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2002 Florian Schirmer (jolt@tuxbox.org)
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
 */

#ifndef AVIA_AV_EVENT_H
#define AVIA_AV_EVENT_H

#define AVIA_AV_EVENT_TIMER 100	/* max. ~100Hz (not realtime...) */

struct avia_av_event_reg {

	u16 hsize;
	u16 vsize;
	u16 aratio;
	u16 frate;
	u16 brate;
	u16 vbsize;
	u16 atype;
	
};

extern int avia_av_event_init(void);
extern void avia_av_event_exit(void);
	    
#endif
