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
$Log: serial.cpp,v $
Revision 1.4.6.2  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.4.6.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.5  2008/07/22 23:41:40  fergy
Removed slash, as we need those commands ( or just ";" but, let leave Them... )

Revision 1.4  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.3  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <memory.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include "serial.h"
#include "channels.h"

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyS"
#define _POSIX_SOURCE 1 /* POSIX compliant source */


volatile int STOP=false;

serial::serial(container &contain) : cont(contain)
{
}

void serial::init()
{
	struct termios newtio;

	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if (fd <0)
	{
		perror(MODEMDEVICE);
		return;
	}

	tcgetattr(fd,&newtio);
	newtio.c_cflag = BAUDRATE| CS8|CREAD|CLOCAL;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag=0;
	newtio.c_lflag=0;

	newtio.c_cc[VMIN]=1;
	newtio.c_cc[VTIME]=0;

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

}

void serial::startThread()
{
	if (fd >= 0)
	{
		pthread_create(&thread, 0, &serial::startlistening, this);
	}

}

void *serial::startlistening(void *object)
{
	serial *s = (serial *) object;

	int res;
	char buf[255];
	char input[5];

	do
	{
		for (int i = 0; i < 4; i++)
		{
			input[i] = 0;
			buf[i] = 0;
		}

		do
		{
			for (int i = 1; i < 4; i++)
				input[i - 1] = input[i];
			res = read(s->fd, buf, 255);

			//printf("%c\n", buf[0]);
			input[3] = buf[0];

			input[4] = 0;
			//printf("String buf: %c%c%c%c\n", buf[0], buf[1], buf[2], buf[3]);
			//printf("String input: %c%c%c%c\n", input[0], input[1], input[2], input[3]);
			if (!strncmp(input, "CBEG", 4))
			{
				//printf("OK\n");
			}
			else
			{

			}
		} while(strncmp(input, "CBEG", 4) && strncmp(buf, "CBEG", 4));

		if (!strncmp(input, "CBEG", 4) || !strncmp(buf, "CBEG", 4))
		{
			//printf("OK\n");
			write(s->fd, "\015OK\015\012", 5);
		}
		else
			write(s->fd, "KO\n\r", 4); // If this happens, your box is seriously damaged ;)

		char buffer[255];
		char text[255];

		time_t act_time = time(0);
		bool available = false;
		while ((!(available = s->char_available())) && (time(0) - act_time < 2));
		if (!available)
			continue;

		res = read(s->fd, buffer, 255);

		//printf("Serial request: %c\n", buffer[0]);

		unsigned char crc;

		switch(buffer[0])
		{
		case 'A': // Send size of channellist
			sprintf(text, "%08x\n\r", (*s->cont.channels_obj).numberChannels() * sizeof(dvbchannel)); // Size of channellist

			//printf("%s\n", text);
			write(s->fd, text, 10);
			break;

		case 'B': // Send channellist

			crc = 0x55;
			for (int i = 0; i < (*s->cont.channels_obj).numberChannels(); i++)
			{
				dvbchannel tmp_chan = (*s->cont.channels_obj).getDVBChannel(i);

				memcpy(&buffer, &tmp_chan, sizeof(dvbchannel));
				for (int i = 0; i < (int) sizeof(dvbchannel); i++)
				{
					crc ^= buffer[i];
					write(s->fd, &buffer[i], 1);
				}
				crc ^= 0x0A;
			}
			write(s->fd, &crc, 1);
			//printf("CRC_calc: %d\n", crc);
			break;

		case 'C': // Get the channellist and save it
			{
				channels channels(s->cont.settings_obj, s->cont.pat_obj, s->cont.pmt_obj);

				bool available;

				crc = 0x55;
				while(true)
				{
					dvbchannel tmp_chan;
					int	start = 0;
					while(start < (int) sizeof(dvbchannel))
					{
						time_t act_time = time(0);
						available = false;
						while ((!(available = s->char_available())) && (time(0) - act_time < 2));
						if (!available)
							break;
						res = read(s->fd, &buffer[start], 64);

						start += res;

						if (res == 1)
							printf("%d\n", buffer[0]);

							printf("res: %d start: %d\n", res, start);


						}
					if (!available)
						break;
					for (int i = 0; i < start; i++)
					{
						crc ^= buffer[i];
					}

					//printf("Read: %d\n", res);
					memcpy(&tmp_chan, &buffer, sizeof(dvbchannel));
					//printf("done\n");
					//printf("SID: %x %d\n", tmp_chan.SID, tmp_chan.SID);
					//printf("SID: %x %d - Chan: %s\n", tmp_chan.SID, tmp_chan.SID, tmp_chan.serviceName);
					channels.addDVBChannel(tmp_chan);

				}
				//printf("CRC_calc: %d\n", crc);
				(*s->cont.osd_obj).createPerspective();

				if (crc == buffer[0])
				{
					*s->cont.channels_obj = channels;
					(*s->cont.channels_obj).saveDVBChannels();

					(*s->cont.osd_obj).setPerspectiveName("Channel Upload complete");
					(*s->cont.osd_obj).showPerspective();
					sleep(3);
					(*s->cont.osd_obj).hidePerspective();


				}
				else
				{
					(*s->cont.osd_obj).setPerspectiveName("Getting channels failed");
					(*s->cont.osd_obj).showPerspective();
					sleep(3);
					(*s->cont.osd_obj).hidePerspective();
				}

				//printf("Finished getting channels\n");
			}
			break;

		case 'D': // unsupported
			break;

		case 'E': // Send version
			write(s->fd, "LCARS                           \r", 32);
			break;

		case 'F': // Unsupported
			break;

		case 'G': // Unsupported
			break;

		case 'H': // Unsupported
			break;

		case 'I': // Unsupported
			break;

		case 'J': // Send current channel number and name
			break;

		case 'K': // Send current resolution
			break;

		case 'L': // Send current channel number
			break;

		case 'M': // Unsupported
			break;

		case 'N': // Unsupported
			break;

		case 'O': // Unsupported
			break;

		case 'P': // Get the PIDs to set
			break;

		case 'Q': // Unsupported
			break;

		case 'R': // Reset the box
			break;

		case 'S': // Start Transponderscan
			break;

		case 'T': // Get frontend-parameters
			break;

		case 'V': // Unsupported
			break;

		case 'X': // Send frontend-parameters
			break;

		case 'Y': // Write string to LCS
			break;

		case 'Z': // Zap to given channelnumber
			break;

		case '1': // Stop picture
			break;

		case '2': // Play picture
			break;

		case '3': //
			break;
		}
	} while(true);


}

bool serial::char_available()
{
	int rc;
	fd_set fds;
	struct timeval tv;

	FD_ZERO(&fds);
	FD_SET(fd,&fds);
	tv.tv_sec = tv.tv_usec = 0;

	rc = select(fd + 1, &fds, NULL, NULL, &tv);
	if (rc < 0)
		return -1;

	return FD_ISSET(fd, &fds) ? 1 : 0;
}


void serial::end()
{
	tcsetattr(fd,TCSANOW,&oldtio);

	close(fd);
}
