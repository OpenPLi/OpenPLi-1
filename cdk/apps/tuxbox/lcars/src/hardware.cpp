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
$Log: hardware.cpp,v $
Revision 1.11.4.6  2008/08/09 16:41:51  fergy
Cleaning code
Enabled some debug stuff
Enabled some disabled features

Revision 1.11.4.5  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.11.4.4  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.11.4.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.12  2003/03/08 17:31:18  waldi
use tuxbox and frontend infos

Revision 1.11  2003/01/26 04:08:16  thedoc
correct sagem vcr-scart-routing

Revision 1.10  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.9  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.8  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.7  2002/06/02 15:39:40  TheDOC
video data viewable

Revision 1.6  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.4  2001/12/18 02:03:29  tux
VCR-Switch-Eventkram implementiert

Revision 1.3  2001/12/17 16:54:47  tux
Settings halb komplett

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include "hardware.h"

#include "devices.h"

hardware::hardware(settings *s, variables *v)
{
	setting = s;
	vars = v;
	vcr_on = false;
	old_DD_state = true;
	old_fblk = OUTPUT_RGB;
}

void hardware::fnc(int i)
{
	avs = open("/dev/dbox/avs0",O_RDWR);
	ioctl(avs, AVSIOSFNC, &i);

	close(avs);
}

int hardware::getAvsStatus()
{
	avs = open("/dev/dbox/avs0",O_RDWR);
	int val;
	ioctl(avs, AVSIOGSTATUS, &val);

	close(avs);
	return val;
}

int hardware::getVCRStatus()
{
	int val;
	int fp = open("/dev/dbox/fp0",O_RDWR);
	ioctl(fp, FP_IOCTL_GET_VCR, &val);
	close(fp);
	return val;
}

int hardware::getARatio()
{
	int check = 0;
	char buffer[100];
	FILE* fp = fopen("/proc/bus/bitstream", "r");
	while (!feof(fp))
	{
		fgets(buffer, 100, fp);
		sscanf(buffer, "A_RATIO: %d", &check);
	}
	fclose(fp);
	return check;
}

int hardware::getVideoSizeX()
{
	int check = 0;
	char buffer[100];
	FILE* fp = fopen("/proc/bus/bitstream", "r");
	while (!feof(fp))
	{
		fgets(buffer, 100, fp);
		sscanf(buffer, "H_SIZE: %d", &check);
	}
	fclose(fp);
	return check;
}

int hardware::getVideoSizeY()
{
	int check = 0;
	char buffer[100];
	FILE* fp = fopen("/proc/bus/bitstream", "r");
	while (!feof(fp))
	{
		fgets(buffer, 100, fp);
		sscanf(buffer, "V_SIZE: %d", &check);
	}
	fclose(fp);
	return check;
}

int hardware::getAudioType()
{
	int check = 0;
	char buffer[100];
	FILE* fp = fopen("/proc/bus/bitstream", "rt");
	while (!feof(fp))
	{
		fgets(buffer, 100, fp);
		sscanf(buffer, "A_TYPE: %d", &check);
	}
	fclose(fp);
	return check;
}

bool hardware::switch_vcr()
{
	int i = 0, j = 0, nothing, nothinga, fblk = 2;

	if (!vcr_on)
	{
		old_fblk = getfblk();
		avs = open("/dev/dbox/avs0",O_RDWR);

		switch (tuxbox_get_vendor())
		{
		case TUXBOX_VENDOR_SAGEM:
			i = 0;
			j = 1;
			nothing = 7;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			ioctl(avs,AVSIOSASW2,&nothinga);
			break;

		case TUXBOX_VENDOR_NOKIA:
			i = 3;
			j = 2;
			nothing = 1;
			nothinga = 2;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			ioctl(avs,AVSIOSASW2,&nothinga);
			break;

		case TUXBOX_VENDOR_PHILIPS:
			i = 2;
			j = 2;
			nothing = 3;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW3,&nothing);
			ioctl(avs,AVSIOSVSW2,&i);
			ioctl(avs,AVSIOSASW2,&j);
			break;
/*		FIX ME!!!
		case TUXBOX_VENDOR_DREAM_MM:
			i = 0;
			j = 1;
			nothing = 7;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			ioctl(avs,AVSIOSASW2,&nothinga);
			break;

		case TUXBOX_VENDOR_TECHNOTREND:
			i = 0;
			j = 1;
			nothing = 7;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			ioctl(avs,AVSIOSASW2,&nothinga);
			break;
*/
		default:
			break;
		}
	}
	else
	{
		setOutputMode(old_fblk);
		avs = open("/dev/dbox/avs0",O_RDWR);

		switch (tuxbox_get_vendor())
		{
		case TUXBOX_VENDOR_SAGEM:
			i = 0;
			j = 0;
			nothing = 0;

			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			break;

		case TUXBOX_VENDOR_NOKIA:
			i = 5;
			j = 1;
			nothing = 1;
			nothinga = 2;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			ioctl(avs,AVSIOSASW2,&nothinga);
			break;

		case TUXBOX_VENDOR_PHILIPS:
			i = 1;
			j = 1;
			nothing = 1;

			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW3,&nothing);
			ioctl(avs,AVSIOSVSW2,&i);
			ioctl(avs,AVSIOSASW2,&j);
			break;
/*		FIX ME!!!!		
		case TUXBOX_VENDOR_DREAM_MM:
			i = 5;
			j = 1;
			nothing = 1;
			nothinga = 2;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			ioctl(avs,AVSIOSASW2,&nothinga);
			break;

		case TUXBOX_VENDOR_TECHNOTREND:
			i = 5;
			j = 1;
			nothing = 1;
			nothinga = 2;
			ioctl(avs,AVSIOSFBLK,&fblk);
			ioctl(avs,AVSIOSVSW2,&nothing);
			ioctl(avs,AVSIOSVSW1,&i);
			ioctl(avs,AVSIOSASW1,&j);
			ioctl(avs,AVSIOSASW2,&nothinga);
			break;
*/
		default:
			break;
		}
	}

	vcr_on = !vcr_on;
	close(avs);
	return vcr_on;

}

void hardware::switch_mute()
{
	int i;

	if (muted)
	{
		i = AVS_UNMUTE;
		vars->setvalue("%IS_MUTED", "false");
	}
	else
	{
		i = AVS_MUTE;
		vars->setvalue("%IS_MUTED", "true");
	}
	avs = open("/dev/dbox/avs0",O_RDWR);
	ioctl(avs, AVSIOSMUTE, &i);
	close(avs);
	muted = !muted;


}

int hardware::vol_minus(int value)
{
	int i;
	avs = open("/dev/dbox/avs0",O_RDWR);
	ioctl(avs, AVSIOGVOL, &i);
	if (i < 63)
		i += value;
	if (i > 63)
		i = 63;
	ioctl(avs, AVSIOSVOL, &i);
	close(avs);
	vars->setvalue("%VOLUME", 63 - i);
	return i;
}

int hardware::vol_plus(int value)
{
	int i;
	avs = open("/dev/dbox/avs0",O_RDWR);
	ioctl(avs, AVSIOGVOL, &i);
	if (i > 0)
		i -= value;
	if (i < 0)
		i = 0;
	ioctl(avs, AVSIOSVOL, &i);
	close(avs);
	vars->setvalue("%VOLUME", 63 - i);
	return i;
}

void hardware::setOutputMode(int i)
{
	int setmode = 0;

	switch (tuxbox_get_vendor())
	{
	case TUXBOX_VENDOR_NOKIA:
		if (i == OUTPUT_FBAS)
			setmode = 0;
		else
			setmode = 3;
		break;

	case TUXBOX_VENDOR_PHILIPS:
		if (i == OUTPUT_FBAS)
			setmode = 0;
		else
			setmode = 1;
		break;

	default:
		break;
	}

	setfblk(setmode);
}

void hardware::setfblk(int i)
{
	avs = open("/dev/dbox/avs0",O_RDWR);
	fblk = i;
	ioctl(avs,AVSIOSFBLK,&fblk);
	close(avs);
}

int hardware::getfblk()
{
	avs = open("/dev/dbox/avs0",O_RDWR);
	if (ioctl(avs, AVSIOGFBLK, &fblk)< 0) {
		perror("AVSIOGFBLK:");
		exit(0);
	}
	close(avs);

	int outputtype = 0;

	switch (tuxbox_get_vendor())
	{
	case TUXBOX_VENDOR_NOKIA:
		if (fblk == 3)
			outputtype = OUTPUT_RGB;
		else if (fblk == 0)
			outputtype = OUTPUT_FBAS;
		break;

	case TUXBOX_VENDOR_PHILIPS:
		if (fblk == 1)
			outputtype = OUTPUT_RGB;
		else if (fblk == 0)
			outputtype = OUTPUT_FBAS;
		break;

	default:
		break;
	}

	return outputtype;
}


void hardware::shutdown()
{
	system("/sbin/halt &");
}

void hardware::reboot()
{
	system("/sbin/reboot &");
}

void hardware::useDD(bool use)
{
	if (old_DD_state == use)
		return;
	int fd = open(AUDIO_DEV, O_RDWR);
	if (use)
	{
		ioctl(fd, AUDIO_SET_BYPASS_MODE, 0);
	}
	else
	{
		ioctl(fd, AUDIO_SET_BYPASS_MODE, 1);
	}
	close(fd);
	old_DD_state = use;
}


