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
$Log: checker.cpp,v $
Revision 1.12.4.4  2008/08/07 17:56:43  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.12.4.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.14  2008/22/07/23:35:00 fergy
Lcars back to live's :-)

Revision 1.13  2003/10/16 00:33:23  obi
bugfix

Revision 1.12  2003/01/06 05:03:11  TheDOC
dreambox compatible

Revision 1.11  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.10  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.9  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.8  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.7  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.6  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.5  2002/05/18 04:31:02  TheDOC
Warningelimination

Revision 1.4  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.3  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.4  2001/12/19 03:23:01  tux
event mit 16:9-Umschaltung

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <stdio.h>
#include <iostream>
#include <dbox/event.h>

#include "devices.h"
#include "checker.h"
#include <pthread.h>

#define BSIZE 10000

static int mode_16_9; // hell, this IS damn ugly

checker::checker(settings *s, hardware *h)
{
	setting = s;
	hardware_obj = h;
	laststat = 0;
	laststat_mode = 0;
}

void checker::set_16_9_mode(int mode)
{
	mode_16_9 = mode;
}

int checker::startEventThread()
{
	int status;

	status = pthread_create( &eventThread,
	                         NULL,
	                         startEventChecker,
	                         (void *)this );
	return status;
}

void checker::fnc(int i, int mode_16_9)
{
	int	avs = open("/dev/dbox/avs0",O_RDWR);
	int vid = open(VIDEO_DEV, O_RDWR);
	if (!vid)
		std::cout << "Couldn't open video-device for 16:9-change" << std::endl;
	ioctl(avs, AVSIOSFNC, &i);
	int format = 0;

	if (i == 1)
		format = VIDEO_CENTER_CUT_OUT;
	if (i == 0)
		if (mode_16_9 == 2)
			format = VIDEO_LETTER_BOX;
		else
			format = VIDEO_PAN_SCAN;

	if (!ioctl(vid, VIDEO_SET_DISPLAY_FORMAT, &format))
		std::cout << "Couldn't set display format" << std::endl;
	close(avs);
	close(vid);
}


int checker::get_16_9_mode()
{
	return mode_16_9;
}

void* checker::startEventChecker(void* object)
{
	checker *c = (checker *) object;
	struct event_t event;
	int fd;


	if((fd = open("/dev/dbox/event0", O_RDWR)) < 0)
	{
		perror("open");
		pthread_exit(NULL);
	}

	int old_vcr_mode = c->get_16_9_mode();

	while(1)
	{
		read(fd, &event, sizeof(event_t));

		if (event.event == EVENT_VCR_CHANGED)
		{
			switch (c->hardware_obj->getVCRStatus())
			{
			case VCR_STATUS_ON:
				if (c->hardware_obj->vcrIsOn())
				{
					c->hardware_obj->fnc(2);
				}
				else
					if (c->setting->getSwitchVCR())
						if (!c->hardware_obj->vcrIsOn())
						{
							c->hardware_obj->switch_vcr();
							old_vcr_mode = c->get_16_9_mode();
							c->set_16_9_mode(1);
						}
				break;
			case VCR_STATUS_OFF:

				if (!c->hardware_obj->vcrIsOn())
				{
					c->hardware_obj->fnc(0);
				}
				else
					if (c->setting->getSwitchVCR())
						if (c->hardware_obj->vcrIsOn())
						{
							c->hardware_obj->switch_vcr();
							c->set_16_9_mode(old_vcr_mode);
						}
				break;
			case VCR_STATUS_16_9:

				if (c->hardware_obj->vcrIsOn())
				{
					c->hardware_obj->fnc(1);
				}
				else
					break;
			}
		}
		else if (event.event == EVENT_ARATIO_CHANGE)
		{

			c->aratioCheck();
		}
		else if (event.event == EVENT_VCR_CHANGED)
		{

		if (c->setting->getSwitchVCR())
			if (c->hardware_obj->vcrIsOn())
			{
				c->hardware_obj->switch_vcr();
				c->set_16_9_mode(old_vcr_mode);
			}
		}
	}
	close(fd);
	pthread_exit(NULL);
}

void checker::aratioCheck()
{
	int check = hardware_obj->getARatio();

	if (check == 3) // 16:9
	{
		if (laststat != 1 || laststat_mode != mode_16_9)
		{
			if (mode_16_9 == 0)
				fnc(1, mode_16_9);
			else
				fnc(0, mode_16_9);
			laststat_mode = mode_16_9;
			laststat = 1;
		}

	}
	if (check != 3)
	{
		if (laststat != 0)
		{
			fnc(0, mode_16_9);
			laststat = 0;
		}
	}
}
