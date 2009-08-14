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
$Log: settings.cpp,v $
Revision 1.13.4.5  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.13.4.4  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.13.4.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.14  2003/03/08 17:31:18  waldi
use tuxbox and frontend infos

Revision 1.13  2003/01/26 00:00:20  thedoc
mv bugs /dev/null

Revision 1.12  2003/01/05 21:37:07  TheDOC
setting ips is now possible

Revision 1.11  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.10  2002/11/26 20:03:14  TheDOC
some debug-output and small fixes

Revision 1.9  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.8  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.7  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.6  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.5  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.6  2001/12/18 02:03:29  tux
VCR-Switch-Eventkram implementiert

Revision 1.5  2001/12/17 18:37:05  tux
Finales Settingsgedoens

Revision 1.4  2001/12/17 16:54:47  tux
Settings halb komplett

Revision 1.3  2001/12/17 01:00:41  tux
scan.cpp fix

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <stdio.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <memory.h>
#include <sstream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h> 
#include <arpa/inet.h>

#include "devices.h"
#include "settings.h"
#include "help.h"
#include "cam.h"

#define need_TUXBOX_GET
#include <tuxbox.h>

TUXBOX_GET(dbox2_gt);

settings::settings(cam *c)
{
	cam_obj = c;
	FILE *fp;
	char buffer[100];
	int type = -1;
	isGTX = false;

	printf("----------------> SETTINGS <--------------------\n");
	fp = fopen("/proc/bus/dreambox", "r");
	while (!feof(fp))
	{
		fgets(buffer, 100, fp);
		sscanf(buffer, "fe=%d", &type);
		sscanf(buffer, "mID=%d", &box);



		int gtx = 0;
		sscanf(buffer, "gtxID=%x\n", &gtx);
		if (gtx != 0)
		{
			if ((unsigned int)gtx != 0xffffffff)
			{
				isGTX = true;
			}
			else
			{
				isGTX = false;
			}
		}

	}
	fclose(fp);

	if (box == 3)
	printf ("Sagem-Box\n");
	else if (box == 1)
	printf("Nokia-Box\n");
	else if (box == 2)
	printf("Philips-Box\n");
	else
	printf("Dream Multimedia Box\n");

	isCable = (type == DBOX_FE_CABLE);

	CAID = cam_obj->getCAID();

	oldTS = -1;
	usediseqc = true;
	setting.timeoffset = 60;
	setting.ip = 0;
	setting.gwip = 0;
	setting.serverip = 0;
	setting.dnsip = 0;

	switch (tuxbox_get_vendor())
	{
	case TUXBOX_VENDOR_NOKIA:
		setting.rcRepeat = true;
		setting.supportOldRc = true;
		break;
	case TUXBOX_VENDOR_PHILIPS:
		setting.rcRepeat = true;
		setting.supportOldRc = true;
		break;
	case TUXBOX_VENDOR_DREAM_MM:
		setting.rcRepeat = true;
		setting.supportOldRc = true;
	default:
		setting.rcRepeat = false;
		setting.supportOldRc = false;
	}
	setting.output_format = 1;
	setting.video_format = 0;
	setting.switch_vcr = true;
	loadSettings();
}

int settings::getEMMpid(int TS)
{
	if (EMM < 2 || oldTS != TS || TS == -1)
	{
		EMM = find_emmpid(CAID);
		oldTS = TS;
	}
	return EMM;
}

int settings::find_emmpid(int ca_system_id) {
	char buffer[1000];
	int fd, r = 1000, count;
	struct dmx_sct_filter_params flt;

	fd=open(DEMUX_DEV, O_RDWR);
	if (fd<0)
	{
		perror(DEMUX_DEV);
		return -fd;
	}
	
	memset (&flt.filter, 0, sizeof (struct dmx_filter));
	
	flt.pid=1;
	flt.filter.filter[0]=1;
	flt.filter.mask[0]  =0xFF;
	flt.timeout=10000;
	flt.flags=DMX_ONESHOT|DMX_IMMEDIATE_START;

	if (ioctl(fd, DMX_SET_FILTER, &flt)<0)
	{
		perror("DMX_SET_FILTER");
		return 1;
	}

	ioctl(fd, DMX_START, 0);
	if ((r=read(fd, buffer, r))<=0)
	{
		perror("[settings.cpp] read (find_emmpid)");
		return 1;
	}

	close(fd);

	if (r<=0) return 0;

	r=((buffer[1]&0x0F)<<8)|buffer[2];

	count=8;
	while(count<r-1)
	{
		if ((((buffer[count+2]<<8)|buffer[count+3]) == ca_system_id) && (buffer[count+2] == ((0x18|0x27)&0xD7)))
			return (((buffer[count+4]<<8)|buffer[count+5])&0x1FFF);
		count+=buffer[count+1]+2;
	}
	return 0;
}

bool settings::boxIsCable()
{
	return isCable;
}

bool settings::boxIsSat()
{
	return !isCable;
}

int settings::getCAID()
{
	return CAID;
}
/*	FIX ME!!!!	*/
int settings::getTransparentColor()
{
	if (tuxbox_get_dbox2_gt() == TUXBOX_DBOX2_GT_GTX)
		return 0xFC0F;
	else
		return 0;
}

void settings::setIP(char n1, char n2, char n3, char n4)
{
	std::stringstream ostr;
	ostr << "ifconfig eth0 " << (int)n1 << "." << (int)n2 << "." << (int)n3 << "." << (int)n4 << " &" << std::ends;
	std::string command = ostr.str();
	std::cout << command << std::endl;

	setting.ip = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;

	system(command.c_str());
	struct sockaddr_in sin;
	int sk;
	unsigned char *ptr;
	struct ifreq ifr;

	memset(&ifr, 0x00, sizeof ifr);
	memset(&sin, 0x00, sizeof sin);

	sin.sin_family = AF_INET;
	char test[] = "192.168.1.1";
	if (inet_aton(test, &sin.sin_addr)==0) {
	}

	strcpy(ifr.ifr_name, "eth0");
	memcpy(&ifr.ifr_addr, &sin, sizeof(ifr.ifr_addr));
	if ((sk = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	}

	if (ioctl(sk, SIOCSIFADDR, &ifr)==-1) {
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	close (sk);

}

char settings::getIP(char number)
{
	return (setting.ip >> ((3 - number) * 8)) & 0xff;
}

void settings::setgwIP(char n1, char n2, char n3, char n4)
{
	std::stringstream ostr;
	ostr << "route add default gw " << (int)n1 << "." << (int)n2 << "." << (int)n3 << "." << (int)n4 << std::ends;
	std::string command = ostr.str();
	std::cout << command << std::endl;
	system(command.c_str());

	setting.gwip = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;
}

char settings::getgwIP(char number)
{
	return (setting.gwip >> ((3 - number) * 8)) & 0xff;
}

void settings::setdnsIP(char n1, char n2, char n3, char n4)
{
	std::stringstream ostr;
	ostr << "echo \"nameserver " << (int)n1 << "." << (int)n2 << "." << (int)n3 << "." << (int)n4 << "\" > /etc/resolv.conf" << std::ends;
	std::string command = ostr.str();
	std::cout << command << std::endl;
	system(command.c_str());

	setting.dnsip = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;
}

char settings::getdnsIP(char number)
{
	return (setting.dnsip >> ((3 - number) * 8)) & 0xff;
}

void settings::setserverIP(char n1, char n2, char n3, char n4)
{
	setting.serverip = (n1 << 24) | (n2 << 16) | (n3 << 8) | n4;
}

char settings::getserverIP(char number)
{
	return (setting.serverip >> ((3 - number) * 8)) & 0xff;
}

void settings::saveSettings()
{
	std::stringstream ostr;
	ostr << "TimeOffset=" << setting.timeoffset << std::endl;
	ostr << "BoxIP=" << (int)getIP(0) << "." << (int)getIP(1) << "." << (int)getIP(2) << "." << (int)getIP(3) << std::endl;
	ostr << "GatewayIP=" << (int)getgwIP(0) << "." << (int)getgwIP(1) << "." << (int)getgwIP(2) << "." << (int)getgwIP(3) << std::endl;
	ostr << "DNSIP=" << (int)getdnsIP(0) << "." << (int)getdnsIP(1) << "." << (int)getdnsIP(2) << "." << (int)getdnsIP(3) << std::endl;
	if (setting.serverip != 0)
		ostr << "ServerIP=" << (int)getserverIP(0) << "." << (int)getserverIP(1) << "." << (int)getserverIP(2) << "." << (int)getserverIP(3) << std::endl;

	ostr << "SupportOldRC=";
	if (setting.supportOldRc)
		ostr << "true" << std::endl;
	else
		ostr << "false" << std::endl;

	ostr << "RCRepeat=";
	if (setting.rcRepeat)
		ostr << "true" << std::endl;
	else
		ostr << "false" << std::endl;

	ostr << "SwitchVCR=";
	if (setting.switch_vcr)
		ostr << "true" << std::endl;
	else
		ostr << "false" << std::endl;

	ostr << "ProxyServer=" << setting.proxy_server << std::endl;
	ostr << "ProxyPort=" << setting.proxy_port << std::endl;

	ostr << "OutputFormat=" << setting.output_format << std::endl;
	ostr << "VideoFormat=" << setting.video_format << std::endl;
	ostr << "Inversion=" << setting.inversion << std::endl;

	ostr << std::ends;
	std::string configfile = ostr.str();
	int fd = open(CONFIGDIR "/lcars/lcars.conf", O_WRONLY|O_TRUNC|O_CREAT, 0666);
	write(fd, configfile.c_str(), configfile.length());
	close (fd);
}

void settings::loadSettings()
{
	std::ifstream inFile;
	std::string line[20];
	int linecount = 0;

	inFile.open(CONFIGDIR "/lcars/lcars.conf");
	printf("----------------> OPENING Lcars.conf <--------------------\n");

	while(linecount < 20 && getline(inFile, line[linecount++]));

	for (int i = 0; i < linecount; i++)
	{
		std::istringstream iss(line[i]);
		std::string cmd;
		std::string parm;

		getline(iss, cmd, '=');
		getline(iss, parm, '=');

		if (cmd == "TimeOffset")
		{
			setting.timeoffset = atoi(parm.c_str());
		}
		else if (cmd == "BoxIP" || cmd == "GatewayIP" || cmd == "DNSIP" || cmd == "ServerIP")
		{
			unsigned char ip[4];
			int ipcount = 0;
			std::istringstream iss2(parm);
			std::string ippart;
			while(getline(iss2, ippart, '.'))
			{
				ip[ipcount++] = atoi(ippart.c_str());
			}
			if (ipcount != 4)
			{
				std::cout << "Error in Config-File on load settings\n" << cmd << std::endl;
	printf("----------------> IP SETTINGS <--------------------\n");
				continue;
			}
			else
			{
				bool isvalid = false;
				for (int j = 0; j < 4; j++)
				{
					if (ip[j] != 0)
						isvalid = true;
				}
				if (!isvalid)
					continue;
				if (cmd == "BoxIP")
				{
					setIP(ip[0], ip[1], ip[2], ip[3]);
				}
				else if (cmd == "GatewayIP")
				{
					setgwIP(ip[0], ip[1], ip[2], ip[3]);
				}
				else if (cmd == "ServerIP")
				{
					setserverIP(ip[0], ip[1], ip[2], ip[3]);
				}
				else if (cmd == "DNSIP")
				{
					setdnsIP(ip[0], ip[1], ip[2], ip[3]);
				}
			}
		}
		else if (cmd == "SupportOldRC")
		{
			if (parm == "true")
				setting.supportOldRc = true;
			else if (parm == "false")
				setting.supportOldRc = false;
			else
				std::cout << "Error in Config-File on load settings\n" << cmd << std::endl;
	printf("----------------> OLD RC SUPPORT <--------------------\n");
		}
		else if (cmd == "RCRepeat")
		{
			if (parm == "true")
				setting.rcRepeat = true;
			else if (parm == "false")
				setting.rcRepeat = false;
			else
				std::cout << "Error in Config-File on load settings\n" << cmd << std::endl;
	printf("----------------> RC REPEAT <--------------------\n");
		}
		else if (cmd == "SwitchVCR")
		{
			if (parm == "true")
				setting.switch_vcr = true;
			else if (parm == "false")
				setting.switch_vcr = false;
			else
				std::cout << "Error in Config-File on load settings\n" << cmd << std::endl;
	printf("----------------> SWITCH VCR <--------------------\n");
		}
		else if (cmd == "ProxyServer")
		{
			setProxyServer(parm);
		}
		else if (cmd == "Inversion")
		{
			setInversion(atoi(parm.c_str()));
		}
		else if (cmd == "ProxyPort")
		{
			setProxyPort(atoi(parm.c_str()));
		}
		else if (cmd == "OutputFormat")
		{
			setting.output_format = atoi(parm.c_str());
		}
		else if (cmd == "VideoFormat")
		{
			setting.video_format = atoi(parm.c_str());
		}
		else
		{
			std::cout << "Error in Config-File on load settings\n" << cmd << std::endl;
	printf("----------------> GLOBAL SETTINGS ERROR <--------------------\n");
		}
	}

	inFile.close();
}

