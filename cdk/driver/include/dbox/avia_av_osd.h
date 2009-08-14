/*
 *   avia_av_osd.h - AViA OSD driver (dbox-II-project)
 *
 *   Homepage: http://dbox2.elxsi.de
 *
 *   Copyright (C) 2001 Gillem (htoa@gmx.net)
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
 *   $Log: avia_av_osd.h,v $
 *   Revision 1.2  2002/10/03 12:47:58  Jolt
 *   AViA AV cleanups
 *
 *   Revision 1.1  2001/03/06 21:51:35  gillem
 *   - initial release
 *
 *
 *
 *   $Revision: 1.2 $
 *
 */

typedef struct sosd_create_frame {
	int framenr;
	int x;
	int y;
	int w;
	int h;
	int gbf;
	int pel;
	int psize;
	void *palette;
	int bsize;
	void *bitmap;
} sosd_create_frame;

#define OSD_IOCTL_CREATE_FRAME	0
#define OSD_IOCTL_DESTROY_FRAME	1
