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
$Log: tdt.cpp,v $
Revision 1.8.6.2  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.8.6.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.9  2008/07/22 23:45:09  fergy
Replaced definitions for compatible building

Revision 1.8  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.7  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.6  2002/09/18 17:31:03  TheDOC
replaced O_RDONLY with O_RDWR on demux-device-open, stupid me

Revision 1.5  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.4  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.3  2002/03/03 22:56:27  TheDOC
lcars 0.20

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

#include "tdt.h"
#include "help.h"
#include "pthread.h"

#define BSIZE 10000

int tdt::start_thread()
{

	int status;

	status = pthread_create( &timeThread,
	                         NULL,
	                         start_timereader,
	                         (void *)this );
	return status;

}



void* tdt::start_timereader( void * this_ptr )
{
	while(1)
	{
		int fd, r;
		struct dmx_sct_filter_params flt;
		unsigned char buffer[BSIZE];
		time_t acttime = 0;

		while(acttime < 100000)
		{
			// Lies den TDT

			memset (&flt.filter, 0, sizeof (struct dmx_filter));
			r = BSIZE;
			flt.pid            = 0x14;
			flt.filter.filter[0] = 0x70;
			flt.filter.mask[0] = 0xFF;
			flt.timeout        = 0;
			flt.flags          = DMX_IMMEDIATE_START | DMX_ONESHOT;

			if (ioctl(fd, DMX_SET_FILTER, &flt) < 0)
				perror("TDT ioctl");
			r=read(fd, buffer, r);
			ioctl(fd,DMX_STOP,0);

			close(fd);

			if (r == 0)
				continue;

			int time = (buffer[5] << 16) | (buffer[6] << 8) | buffer[7];
			int mjd = (buffer[3] << 8) | buffer[4];
			//printf("Time: %x - MJD: %x\n", time, mjd);
			acttime = dvbtimeToLinuxTime(mjd, time);
			stime(&acttime);
			//printf("----------Aktuelle Zeit: %d\n", (int)acttime);
			//printf("----------Aktuelle Zeit: %s\n", ctime(&acttime));
		}
		sleep(1800); // alle 30 Minuten die Zeit neu setzen...
	}

}
