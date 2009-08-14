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
$Log: help.cpp,v $
Revision 1.6  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.5  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.4  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.3  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <memory.h>
#include <time.h>
#include "settings.h"

time_t dvbtimeToLinuxTime(int mjd, int time)
{
	struct tm tm_time;
	time_t tim;
	int seconds, minutes, hour, day, month, year;
	int y_, m_, k;

	y_   = (int) ((mjd - 15078.2) / 365.25);

	m_   = (int) ((mjd - 14956.1 - (int) (y_ * 365.25)) / 30.6001);

	day  = mjd - 14956 - (int) (y_ * 365.25) - (int) (m_ * 30.60001);

	if ((m_ == 14) || (m_ == 15))
		k = 1;
	else
		k = 0;
	year  = y_ + k + 1900;
	month = m_ - 1 - k*12;

	hour    = (time >> 16) & 0xff;
	minutes = (time >>  8) & 0xff;
	seconds = (time) & 0xff;

	memset(&tm_time, 0, sizeof(tm_time));
	tm_time.tm_sec=(seconds&0x0f)+((seconds&0xf0)>>4)*10;
	tm_time.tm_min=(minutes&0x0f)+((minutes&0xf0)>>4)*10;
	tm_time.tm_hour=(hour&0x0f)+((hour&0xf0)>>4)*10;
	tm_time.tm_mday=day;
	tm_time.tm_mon=month-1;
	tm_time.tm_year=year-1900;
	tm_time.tm_isdst=-1;
	tim=mktime(&tm_time);
	tim=mktime(&tm_time);

	struct tm *t;
	t = localtime(&tim);
	char acttime[10];
	strftime(acttime, sizeof acttime, "%H:%M", t);
	return tim;
}
