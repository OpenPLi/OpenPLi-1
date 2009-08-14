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
$Log: checker.h,v $
Revision 1.4  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.3  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.5  2001/12/19 03:23:01  tux
event mit 16:9-Umschaltung

Revision 1.3  2001/12/17 16:54:47  tux
Settings halb komplett

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef CHECKER_H
#define CHECKER_H

#include "hardware.h" 
#include "settings.h"

class checker
{
	pthread_t eventThread;
	static void* startEventChecker(void*);

public:
	int laststat;
	int laststat_mode;

	hardware *hardware_obj;
	settings *setting;
	checker(settings *s, hardware *h);
	int startEventThread();
	void fnc(int i, int mode_16_9);
	void aratioCheck();

	void set_16_9_mode(int mode);
	int get_16_9_mode();
};

#endif
