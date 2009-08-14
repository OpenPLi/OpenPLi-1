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
$Log: timer.h,v $
Revision 1.7  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.6  2002/05/20 20:08:12  TheDOC
some new timer and epg-stuff

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.3  2001/12/17 01:00:34  tux
scan.cpp fix

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef	TIMER_H
#define TIMER_H

#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <map>
#include <string>
#include <unistd.h>

#include "hardware.h"
#include "channels.h"
#include "zap.h"
#include "tuner.h"
#include "osd.h"
#include "variables.h"

#include <config.h>

struct timer_entry
{
	time_t starttime;
	int duration;
	int status; // 0 - active, 1 - done, 2 - disabled, 3 - failed, 4 - running
	int type; // 0 - shutdown, 1 - reboot, 2 - zap, 3 - record
	int channel;
	char comment[100];
};

class timer
{
	pthread_t timerThread;
	pthread_mutex_t mutex;

	static void* start_timer( void * );

	std::multimap<time_t, struct timer_entry> timer_list;

	hardware *hardware_obj;
	channels *channels_obj;
	zap *zap_obj;
	tuner *tuner_obj;
	osd *osd_obj;
	variables *vars;

	time_t dumped_starttimes[20];
	int dumped_channels[20];
public:
	timer(hardware *h, channels *c, zap *z, tuner *t, osd *o, variables *v);
	int start_thread();

	void runTimer();
	bool isEmpty();

	void addTimer(time_t starttime, int type, std::string comment, int duration = 0, int channel = -1, std::string audio = "");
	time_t getTime();
	int getNumberTimer() { return timer_list.size(); }
	void dumpTimer();
	int getDumpedChannel(int i) { return dumped_channels[i]; }
	time_t getDumpedStarttime(int i) { return dumped_starttimes[i]; }
	void rmTimer(int channel, time_t starttime);
	void saveTimer();
	void loadTimer();
};

#endif
