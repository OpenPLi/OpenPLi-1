/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
	homepage			 : www.chatville.de
	modified by			 : -
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
$Log: pig.cpp,v $
Revision 1.8.6.5  2008/08/09 16:39:14  fergy
New structure ( stoled from Neutrino )

Revision 1.8.6.4  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.8.6.3  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.8  2003/01/06 05:03:11  TheDOC
dreambox compatible

Revision 1.7  2003/01/05 20:00:11  TheDOC
hmpf

Revision 1.6  2003/01/05 19:52:47  TheDOC
forgot include

Revision 1.5  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.4  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/11/19 10:08:10  TheDOC
pig fixed

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "pig.h"

pig::pig()
{
    fd = -1;
    status = CLOSED;
}


pig::pig(int pig_nr)
{
    fd = -1;
    status = CLOSED;
    fd = pigopen (pig_nr);
}


pig::pig(int pig_nr, int x, int y, int w, int h)
{
    fd = -1;
    status = CLOSED;
    fd = pigopen (pig_nr);
    set_coord (x,y, w,h);

}

pig::~pig()
{
	pigclose ();

}

int pig::pigopen (int pig_nr)
{

 char  *pigdevs[] = {
		PIG_DEV "0"
};
		

    if ( (pig_nr>0) || (pig_nr < (int)(sizeof (pigdevs)/sizeof(char *))) ) {
    
	if (fd < 0) {
		fd = open( pigdevs[pig_nr], O_RDWR );
		if (fd >= 0) {
			status = HIDE;
			px = py = pw = ph = 0;
		}
		return fd;
	}

    }

    return -1;
}

void pig::pigclose ()
{
   if (fd >=0 ) {
	close (fd);
	fd = -1;
	status = CLOSED;
	px = py = pw = ph = 0;
   }
   return;
}

void pig::_set_window (int x, int y, int w, int h)
{
	struct v4l2_crop crop;
	struct v4l2_format coord;
	int    err;

	crop.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	err = ioctl(fd, VIDIOC_G_CROP, &crop);

	crop.c.left = 0;
	crop.c.top = 0;
	crop.c.width = 720;
	crop.c.height = 576;
	err = ioctl(fd, VIDIOC_S_CROP, &crop);
	
	coord.type = V4L2_BUF_TYPE_VIDEO_OVERLAY;
	err = ioctl(fd, VIDIOC_G_FMT, &coord);

	coord.fmt.win.w.left = x;
	coord.fmt.win.w.top = y;
	coord.fmt.win.w.width  = w;
	coord.fmt.win.w.height = h;

	err = ioctl(fd, VIDIOC_S_FMT, &coord);
}


void pig::set_coord (int x, int y, int w, int h)
{

	if (( x != px ) || ( y != py )) {
		px = x;
		py = y;
		pw = w;
		ph = h;
		_set_window (px,py,pw,ph);
	}

}

void pig::set_xy (int x, int y)
{

	if (( x != px ) || ( y != py )) {
		px = x;
		py = y;
		_set_window (px,py,pw,ph);
	}

}

void pig::set_size (int w, int h)
{

	if (( w != pw ) || ( h != ph )) {
		pw = w;
		ph = h;
		_set_window (px,py,pw,ph);
	}

}

void pig::show (int x, int y, int w, int h)
{
	set_coord (x,y, w,h);
	show ();
}

void pig::show (void)
{
	if ( fd >= 0 ) {
		int pigmode = 1;
		int err;
		err = ioctl(fd, VIDIOC_OVERLAY, &pigmode);
		status = SHOW;
	}
}

void pig::hide (void)
{
	if ( fd >= 0 ) {
		int pigmode = 0;
		int err;
		err = ioctl(fd, VIDIOC_OVERLAY, &pigmode);
		status = HIDE;
	}
}


pig::PigStatus  pig::getStatus(void)
{
	return status;

}

