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
$Log: pig.h,v $
Revision 1.7.6.1  2008/08/09 16:39:14  fergy
New structure ( stoled from Neutrino )

Revision 1.7  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.6  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.3  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef PIG_H
#define PIG_H

#include <linux/videodev.h>

#define PIG_DEV "/dev/v4l/video"

class pig
{
public:
	pig ();
	pig (int pig_nr);
	pig (int pig_nr, int x, int y, int w, int h);
	~pig ();
	int  pigopen  (int pig_nr);
	void pigclose (void);
	void set_coord (int x, int y, int w, int h);
	void set_xy    (int x, int y);
	void set_size  (int w, int h);
	void show (void);
	void show (int x, int y, int w, int h);
	void hide (void);

	enum PigStatus { CLOSED, HIDE, SHOW };
	PigStatus getStatus(void);

private:
	void _set_window  (int x, int y, int w, int h);

	int fd;
	int px, py, pw, ph;
	int stackpos;
	PigStatus  status;
};

#endif
