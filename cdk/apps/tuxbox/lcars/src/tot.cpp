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
$Log: tot.cpp,v $
Revision 1.9  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.8  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.7  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

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

#include "devices.h"
#include "tot.h"
#include "help.h"
#include "pthread.h"

#define BSIZE 10000

tot::tot(settings *s)
{
	setting = s;
}

int tot::start_thread()
{

	int status;

	status = pthread_create( &timeThread,
	                         NULL,
	                         start_timereader,
	                         (void *)this );
	return status;

}

void* tot::start_timereader( void * this_ptr )
{
	tot *t = (tot *) this_ptr;
	int fd, r;
	struct dmx_sct_filter_params flt;
	unsigned char buffer[BSIZE];
	time_t acttime = 0;

	while(1)
	{
		// Lies den TOT
		if ((fd=open(DEMUX_DEV, O_RDWR)) < 0)
			perror("TOT open");

		memset (&flt.filter, 0, sizeof (struct dmx_filter));
		r = BSIZE;
		flt.pid            = 0x14;
		flt.filter.filter[0] = 0x73;
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
		//printf("----------Aktuelle Zeit: %d\n", (int)acttime);
		//printf("----------Aktuelle Zeit: %s\n", ctime(&acttime));

		int descriptors_length = ((buffer[8] & 0x0f) << 8) | buffer[9];

		int position = 10;
		int offset = 0;
		while(position < descriptors_length)
		{
			int descriptor_tag = buffer[position];
			int descriptor_length = buffer[position + 1];


			if (descriptor_tag == 0x58)
			{
				int number_offsets = (int) (descriptor_length / 13);

				for (int i = 0; i < number_offsets; i++)
				{
					if (buffer[position + i * 13 + 2] == 'D' && buffer[position + i * 13 + 3] == 'E' && buffer[position + i * 13 + 4] == 'U')
					{
						offset = (((buffer[position + i * 13 + 6] & 0xf0) >> 4) * 10 + (buffer[position + i * 13 + 6] & 0xf)) * 60 + ((buffer[position + i * 13 + 7] & 0xf0) >> 4) * 10 + (buffer[position + i * 13 + 7] & 0xf);
					}
				}


			}
			position += 2 + descriptor_length;
		}


		acttime += offset * 60L;
		stime(&acttime);
		(*t->setting).setTimeOffset(offset);
		//printf("Offsetttttt: %d\n", offset);
		sleep(900);
	}
	return 0;
}
