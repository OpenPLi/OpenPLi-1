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
$Log: timer.cpp,v $
Revision 1.10  2002/10/14 01:19:15  woglinde


mostly compiler warnings, but I got not all

Revision 1.9  2002/06/02 14:23:36  TheDOC
some fixes and changes

Revision 1.8  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.7  2002/05/20 20:08:12  TheDOC
some new timer and epg-stuff

Revision 1.6  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.5  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.3  2001/12/17 01:00:41  tux
scan.cpp fix

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include "timer.h"

timer::timer(hardware *h, channels *c, zap *z, tuner *t, osd *o, variables *v)
{
	hardware_obj = h;
	channels_obj = c;
	zap_obj = z;
	tuner_obj = t;
	osd_obj = o;
	vars = v;
}

int timer::start_thread()
{

	int status;

	//printf("1\n");
	pthread_mutex_init(&mutex, NULL);
	//printf("1\n");
	status = pthread_create( &timerThread,
	                         NULL,
	                         start_timer,
	                         (void *)this );
	return status;

}

time_t timer::getTime()
{
	return (*timer_list.begin()).second.starttime;
}

void* timer::start_timer( void * this_ptr )
{
	timer *t = (timer *) this_ptr;

	while(1)
	{
		while(t->isEmpty())
		{
			sleep(5);
		}

		t->runTimer();
		sleep(5);
	}
	return 0;
}

void timer::addTimer(time_t starttime, int type, std::string comment, int duration, int channel, std::string audio)
{
	struct timer_entry new_timer;

	memset (&new_timer, 0, sizeof(struct timer_entry));

	new_timer.starttime = starttime;
	new_timer.channel = channel;
	new_timer.duration = duration;
	new_timer.status = 0;
	new_timer.type = type;
	strcpy(new_timer.comment, comment.c_str());

	//printf("New timer: %s %d\n", ctime(&starttime), new_timer.type);

	pthread_mutex_lock( &mutex );
	timer_list.insert(std::pair<time_t, struct timer_entry>(starttime, new_timer));
	pthread_mutex_unlock( &mutex );

}

bool timer::isEmpty()
{
	pthread_mutex_lock( &mutex );
	bool empty = (timer_list.size() == 0);
	pthread_mutex_unlock( &mutex );

	return empty;
}

void timer::runTimer()
{
	pthread_mutex_lock( &mutex );
	std::multimap<time_t, struct timer_entry>::iterator it = timer_list.begin();
	struct timer_entry act_timer = (*it).second;

	if (act_timer.starttime > time(0))
	{
		pthread_mutex_unlock( &mutex );

		return;
	}

	timer_list.erase(it);
	pthread_mutex_unlock( &mutex );

	saveTimer();

	if (act_timer.type == 0)
		(*hardware_obj).shutdown();
	else if (act_timer.type == 1)
		(*hardware_obj).reboot();
	else if (act_timer.type == 2)
	{
		//int last_channel = (*channels_obj).getCurrentChannelNumber();
		(*channels_obj).setCurrentChannel(act_timer.channel);
		(*channels_obj).zapCurrentChannel();
		/*if (act_timer.duration != 0)
			addTimer(time(0) + act_timer.duration, 2, 0, last_channel);*/
		(*channels_obj).setCurrentOSDProgramInfo();
		(*hardware_obj).fnc(0);
		(*hardware_obj).fnc(2);

		(*channels_obj).receiveCurrentEIT();
		(*channels_obj).setCurrentOSDProgramEIT();
		(*channels_obj).updateCurrentOSDProgramAPIDDescr();

		vars->addEvent("VCR_START");
		//std::cout << "Starttime: " << act_timer.starttime << std::endl << "Duration: " << act_timer.duration << std::endl;
		////std::cout << (act_timer.starttime + (act_timer.duration / 60)) << std::endl;
		addTimer((act_timer.starttime + act_timer.duration ) - 1, 3, "stop-vcr");
	}
	else if (act_timer.type == 3)
	{
		vars->addEvent("VCR_STOP");
	}
}

void timer::dumpTimer()
{
	int position = 1;
	pthread_mutex_lock( &mutex );
	for (std::multimap<time_t, struct timer_entry>::iterator it = timer_list.begin(); it != timer_list.end(); ++it, position++)
	{
		time_t starttime = (*it).second.starttime;
		struct tm *t;
		t = localtime(&starttime);
		char timetxt[20];
		strftime(timetxt, sizeof timetxt, "%a, %H:%M", t);
		char text[300];
		sprintf(text, "%s Ch: %d - ", timetxt, (*it).second.channel);

		char comment[300];
		strcpy(comment, (*it).second.comment);
		if (strlen(comment) != 0)
			strcat(text, comment);
		else
			strcat(text, "Timer");
		osd_obj->addMenuEntry(position, text);

		dumped_starttimes[position] = (*it).second.starttime;
		dumped_channels[position] = (*it).second.channel;


	}
	pthread_mutex_unlock( &mutex );
}

void timer::rmTimer(int channel, time_t starttime)
{
	//printf("Start Removing Timer\n");
	pthread_mutex_lock( &mutex );
	timer_list.erase(timer_list.find(starttime));
	pthread_mutex_unlock( &mutex );
	//printf("End Removing Timer\n");
}

void timer::saveTimer()
{
	int fd;
	fd = open(CONFIGDIR "/lcars/timer.dat", O_WRONLY | O_CREAT | O_TRUNC);
	if (fd < 0)
	{
		perror("Couldn't save timer\n");
		return;
	}

	pthread_mutex_lock(&mutex);

	for (std::multimap<time_t, struct timer_entry>::iterator it = timer_list.begin(); it != timer_list.end(); ++it)
	{
		struct timer_entry tmp_timer = (*it).second;
		write(fd, &tmp_timer, sizeof(timer_entry));
	}

	pthread_mutex_unlock(&mutex);

	close(fd);
}

void timer::loadTimer()
{
	int fd = open(CONFIGDIR "/lcars/timer.dat", O_RDONLY);
	if (fd < 0)
	{
		perror("Couldn't load timer\n");
		return;
	}

	pthread_mutex_lock(&mutex);

	struct timer_entry tmp_timer;
	while(read(fd, &tmp_timer, sizeof(timer_entry)))
	{
		timer_list.insert(std::pair<time_t, struct timer_entry>(tmp_timer.starttime, tmp_timer));
	}

	pthread_mutex_unlock(&mutex);

	close(fd);
}

