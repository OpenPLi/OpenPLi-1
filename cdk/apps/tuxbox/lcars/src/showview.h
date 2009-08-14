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
$Log: showview.h,v $
Revision 1.3  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef SHOWVIEW_H
#define SHOWVIEW_H

#include <time.h>
#include <string>

class showview
{
	std::string acSender[207];
	unsigned short anStart[1024];
	unsigned short anLength[1024];

	int mod(int num, int div);
	int getTimeIndex(time_t begin, int duration);
	int nd(int num);
public:
	showview();
	int generateCode(time_t begin, int duration);
};

#endif
