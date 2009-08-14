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
$Log: tuner.h,v $
Revision 1.8.4.2  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.8.4.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.10  2008/07/22 23:45:09  fergy
Replaced definitions for compatible building

Revision 1.9  2003/03/08 17:31:18  waldi
use tuxbox and frontend infos

Revision 1.8  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.7  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.6  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.3  2001/12/07 14:10:33  rasc
Fixes for SAT tuning and Diseqc. Diseqc doesn't work properly for me (diseqc 2.0 switch).
Someone should check this please..

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef TUNER_H
#define TUNER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <memory.h>


#include "devices.h"
#include "settings.h"

class tuner
{
	settings *setting;
	int frontend;
public:
	tuner(settings *s);
	~tuner();
	fe_code_rate_t getFEC(int fec);
	bool tune(unsigned int frequ, unsigned int symbol, int polarization = -1, int fec = 0, int diseqc = 0);
};

#endif
