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
$Log: nit.cpp,v $
Revision 1.9.6.3  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.9  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.8  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.7  2002/09/18 17:31:03  TheDOC
replaced O_RDONLY with O_RDWR on demux-device-open, stupid me

Revision 1.6  2002/09/18 10:48:37  obi
use devfs devices

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

#include "devices.h"
#include "nit.h"

#define BSIZE 10000

int nit::getTransportStreams(channels *channels, int diseqc)
{

	long fd, r;
	struct dmx_sct_filter_params flt;
	unsigned char buffer[BSIZE];
	int countTS = 0;

	fd=open(DEMUX_DEV, O_RDWR);

	memset (&flt.filter, 0, sizeof (struct dmx_filter));

	flt.pid            = 0x10;
	flt.filter.filter[0] = 0x41;
	flt.filter.mask[0] = 0xF0;
	flt.timeout        = 10000;
	flt.flags          = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

	ioctl(fd, DMX_SET_FILTER, &flt);

	int sec_counter = 0;
	do
	{
		r = BSIZE;
		r=read(fd, buffer, r);
		if (r < 1)
			return 0;
		int descriptor_length = (((buffer[8] & 0x0f) << 8) | buffer[9]);
		int start = descriptor_length + 10;

		int counter = 0;
		int startbyte = start + 2;


		while (startbyte + counter < r - 5)
		{
			int transport_stream_id = (buffer[startbyte + counter] << 8) | buffer[startbyte + 1 + counter];
			int original_network_id = (buffer[startbyte + 2 + counter] << 8) | buffer[startbyte + 3 + counter];
			long frequ = 0;
			int symbol = 0;
			int polarization = -1;
			int fec = -1;

			int descriptors_length = ((buffer[startbyte + 4 + counter] & 0x0f) << 8) | buffer[startbyte + 5 + counter];
			int start = startbyte + counter + 6;
			while (start < startbyte + counter + 6 + descriptors_length)
			{

				if (buffer[start] == 0x44) // cable
				{
					frequ = (((buffer[start + 2] & 0xf0) >> 4) * 1000) + ((buffer[start + 2] & 0xf) * 100) + (((buffer[start + 3] & 0xf0) >> 4) * 10) + ((buffer[start + 3] & 0xf));
					frequ *= 10;
					symbol = (((buffer[start + 9] & 0xf0) >> 4) * 100000) + ((buffer[start + 9] & 0xf) * 10000) + (((buffer[start + 10] & 0xf0) >> 4) * 1000) + ((buffer[start + 10] & 0xf) * 100) + (((buffer[start + 11] & 0xf0) >> 4) * 10) + ((buffer[start + 11] & 0xf));
					countTS++;
				}
				else if (buffer[start] == 0x43) // sat
				{
					frequ = (((buffer[start + 2] & 0xf0) >> 4) * 100000) + ((buffer[start + 2] & 0xf) * 10000) + (((buffer[start + 3] & 0xf0) >> 4) * 1000) + ((buffer[start + 3] & 0xf) * 100) + (((buffer[start + 4] & 0xf0) >> 4) * 10) + ((buffer[start + 4] & 0xf));
					symbol = (((buffer[start + 9] & 0xf0) >> 4) * 100000) + ((buffer[start + 9] & 0xf) * 10000) + (((buffer[start + 10] & 0xf0) >> 4) * 1000) + ((buffer[start + 10] & 0xf) * 100) + (((buffer[start + 11] & 0xf0) >> 4) * 10) + ((buffer[start + 11] & 0xf));
					polarization = ((buffer[start + 8] & 0x60) >> 5);
					fec = buffer[start + 12] & 0x0f;
					countTS++;
				}
/*	Here goes terrestrial stuff in the future	*/
/*				else if (buffer[start] == 0x42) // terrestrial
				{
					frequ = (((buffer[start + 2] & 0xf0) >> 4) * 1000) + ((buffer[start + 2] & 0xf) * 100) + (((buffer[start + 3] & 0xf0) >> 4) * 10) + ((buffer[start + 3] & 0xf));
					frequ *= 10;
					symbol = (((buffer[start + 9] & 0xf0) >> 4) * 100000) + ((buffer[start + 9] & 0xf) * 10000) + (((buffer[start + 10] & 0xf0) >> 4) * 1000) + ((buffer[start + 10] & 0xf) * 100) + (((buffer[start + 11] & 0xf0) >> 4) * 10) + ((buffer[start + 11] & 0xf));
					countTS++;
				}*/
				start += buffer[start + 1] + 2;

			}
			(*channels).addTS(transport_stream_id, original_network_id, frequ, symbol, polarization, fec, diseqc);

			counter += 6 + descriptors_length;
		}

	} while( buffer[7] != sec_counter++);
	ioctl(fd,DMX_STOP,0);
	close (fd);
	printf("Found Transponders: %d\n", countTS);

	return countTS;
}
