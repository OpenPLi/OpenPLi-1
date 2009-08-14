/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <global.h>

#include "actionlog.h"



#ifdef USEACTIONLOG


CActionLog::CActionLog()
{
	if(!opendevice(SERIALDEVICE))
	{
		if( opendevice(SERIALDEVICEALTERNATE) )
		{
			printf("actionlog started (sec)\n");
		}
	}
	else
	{
		printf("actionlog started (std)\n");
	}
}

CActionLog::~CActionLog()
{
	if(fd!=-1)
	{
		tcsetattr(fd,TCSANOW,&oldtio);
		close(fd);
		fd=-1;
	}
}

bool CActionLog::opendevice(string devicename)
{
	if((fd = open(devicename.c_str(), O_RDWR | O_NOCTTY )) < 0)
	{
		fd=-1;
		perror(SERIALDEVICE);
		return false;
	}

	/* save current port settings */
	tcgetattr(fd,&oldtio);

	bzero(&newtio, sizeof(newtio));
	newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

	return true;
}

void CActionLog::print(string text)
{
	if(fd!=-1)
	{
		write(fd, text.c_str(), text.size() );
	}
}

void CActionLog::println(string text)
{
	print(text + "\r\n");
}


#endif
