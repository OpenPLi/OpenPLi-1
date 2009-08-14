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
$Log: pat.cpp,v $
Revision 1.11.6.1  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.11  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.10  2002/11/26 20:03:14  TheDOC
some debug-output and small fixes

Revision 1.9  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.8  2002/09/18 17:31:03  TheDOC
replaced O_RDONLY with O_RDWR on demux-device-open, stupid me

Revision 1.7  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.6  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.5  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.4  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

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
#include <iostream>

#include <map>

#include "devices.h"
#include "pat.h"

#define BSIZE 10000

bool pat::readPAT()
{
	int fd, r;
	struct dmx_sct_filter_params flt;
	unsigned char buffer[BSIZE];

	fd=open(DEMUX_DEV, O_RDWR);
	if (fd < 0)
	{
		perror("open readPAT-open");
	}
	ioctl(fd,DMX_STOP,0);
	memset (&flt.filter, 0, sizeof (struct dmx_filter));
	r = BSIZE;
	flt.pid            = 0x0;
	flt.filter.filter[0] = 0x0;
	flt.filter.mask[0] = 0xff;
	flt.timeout        = 1000;
	flt.flags          = DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_FILTER, &flt)<0)
	{
		perror("DMX_SET_FILTER readPAT");
	}
	r=read(fd, buffer, r);

	close(fd);

	int transport_stream_id = (buffer[3] << 8) | buffer[4];
	if (oldpatTS != transport_stream_id)
	{
		oldpatTS = transport_stream_id;

		pat_list.clear();

		for (int i = 8; i < r - 5; i += 4)
		{
			if (((buffer[i] << 8) | buffer[i + 1]) != 0)
			{
				pat_entry temp_pat;
				temp_pat.TS = transport_stream_id;
				temp_pat.SID = (buffer[i] << 8) | buffer[i + 1];
				temp_pat.PMT = ((buffer[i + 2] & 0x1f) << 8 | buffer[i + 3]);
				pat_list.insert(std::pair<int, struct pat_entry>(temp_pat.SID, temp_pat));
			}
		}
	}

	return (r > 0);
}

int pat::getPMT(int SID)
{
	if (pat_list.count(SID) == 0)
		return 0;
	std::multimap<int, struct pat_entry>::iterator it = pat_list.find(SID);

	return (*it).second.PMT;
}

