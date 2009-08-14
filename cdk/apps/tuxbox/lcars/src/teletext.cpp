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
$Log: teletext.cpp,v $
Revision 1.16  2003/01/05 22:39:56  alexw
removed headerfile

Revision 1.15  2003/01/05 20:59:28  TheDOC
old vbi possible now... just add -DOLD_VBI in Makefile.am to the INCLUDES line

Revision 1.14  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.13  2003/01/05 06:49:59  TheDOC
lcars should work now with the new drivers more properly

Revision 1.12  2002/11/26 20:03:14  TheDOC
some debug-output and small fixes

Revision 1.11  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.10  2002/10/05 23:20:33  obi
*hust*

Revision 1.9  2002/10/05 23:10:01  obi
"das ist ja c++" ;)

Revision 1.8  2002/10/05 20:31:10  obi
napi style teletext

Revision 1.7  2002/06/12 17:46:53  TheDOC
reinsertion readded

Revision 1.6  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.4  2001/12/20 00:31:38  tux
Plugins korrigiert

Revision 1.3  2001/12/17 14:00:41  tux
Another commit

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <stdio.h>

#include "devices.h"

#define BSIZE 10000

#include "teletext.h"

#ifdef OLD_VBI
#define AVIA_VBI_START_VTXT	1
#define AVIA_VBI_STOP_VTXT	2 
#endif

void teletext::getTXT(int PID)
{
}

void teletext::startReinsertion(int PID)
{
#ifndef OLD_VBI
	dmx_pes_filter_params pesFilterParams;

	pesFilterParams.pid = PID;
	pesFilterParams.input = DMX_IN_FRONTEND;
	pesFilterParams.output = DMX_OUT_DECODER;
#ifdef HAVE_LINUX_DVB_VERSION_H
	pesFilterParams.pes_type = DMX_PES_TELETEXT;
#elif HAVE_OST_DMX_H
	pesFilterParams.pesType = DMX_PES_TELETEXT;
#endif
	pesFilterParams.flags  = DMX_IMMEDIATE_START;

	std::cout << "Start reinsertion on PID " << PID << std::endl;
	
	if (txtfd == -1)
		txtfd = open(DEMUX_DEV, O_RDWR);
	
	if (ioctl(txtfd, DMX_SET_PES_FILTER, &pesFilterParams) < 0)
		perror("[teletext.cpp]DMX_SET_PES_FILTER");
#else
	std::cout << "Start reinsertion on PID " << PID << std::endl;
	int txtfd = open("/dev/dbox/vbi0", O_RDWR);
	ioctl(txtfd, AVIA_VBI_START_VTXT, PID);

	close(txtfd);

#endif
}

void teletext::stopReinsertion()
{
#ifndef OLD_VBI
	if (txtfd == -1)
		return;

	std::cout << "Stop reinsertion" << std::endl;
	
	if (ioctl(txtfd, DMX_STOP, 0) < 0);

	close(txtfd);
	txtfd = -1;
#else
	std::cout << "Stop reinsertion" << std::endl;
	int txtfd = open("/dev/dbox/vbi0", O_RDWR);
	ioctl(txtfd, AVIA_VBI_STOP_VTXT, true);

	close(txtfd);

#endif
}


