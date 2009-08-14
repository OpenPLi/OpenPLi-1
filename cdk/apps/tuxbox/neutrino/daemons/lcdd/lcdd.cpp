/*
	LCD-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/


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


#include <config.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>

#include "lcdd.h"
#include "newclock.h"


CLCDD::CLCDD()
	: configfile('\t')
{
}

CLCDD::~CLCDD()
{
}

CLCDD* CLCDD::getInstance()
{
	static CLCDD* lcdd = NULL;
	if(lcdd == NULL)
	{
		lcdd = new CLCDD();
	}
	return lcdd;
}

void CLCDD::saveConfig()
{
	static bool inSave=false;
	if(inSave==false)
	{
		inSave=true;
		configfile.setInt32( "lcd_brightness", lcdPainter.getBrightness() );
		configfile.setInt32( "lcd_standbybrightness", lcdPainter.getBrightnessStandby() );
		configfile.setInt32( "lcd_contrast", lcdPainter.getContrast() );
		configfile.setInt32( "lcd_power", lcdPainter.getPower() );
		configfile.setInt32( "lcd_inverse", lcdPainter.getInverse() );

		if(configfile.getModifiedFlag())
		{
			printf("[lcdd] save config\n");
			configfile.saveConfig(CONFIGDIR "/lcdd.conf");
		}
		inSave=false;
	}
}

void CLCDD::loadConfig()
{
	printf("[lcdd] load config\n");
	if(!configfile.loadConfig(CONFIGDIR "/lcdd.conf"))
	{
		lcdPainter.setBrightness(0xff);
		lcdPainter.setBrightnessStandby(0xaa);
		lcdPainter.setContrast(0x0F);
		lcdPainter.setPower(0x01);
		lcdPainter.setInverse(0x00);
		return;
	}

	lcdPainter.setBrightness( configfile.getInt32("lcd_brightness", 0xff) );
	lcdPainter.setBrightnessStandby( configfile.getInt32("lcd_standbybrightness", 0xaa) );
	lcdPainter.setContrast( configfile.getInt32("lcd_contrast", 0x0F) );
	lcdPainter.setPower( configfile.getInt32("lcd_power", 0x01) );
	lcdPainter.setInverse( configfile.getInt32("lcd_inverse", 0x00) );
}

void CLCDD::parse_command(int connfd, CLcddMsg::Header rmsg)
{
	if(rmsg.version != CLcddMsg::ACTVERSION)
	{
		printf("[lcdd] unknown version\n");
		return;
	}

	switch (rmsg.cmd)
	{
		case CLcddMsg::CMD_SHUTDOWN:
			saveConfig();
			exit(0);
			break;
		case CLcddMsg::CMD_SETSERVICENAME:
			CLcddMsg::commandServiceName msg;
			read(connfd, &msg, sizeof(msg));
			lcdPainter.show_servicename( msg.servicename);
			break;
		case CLcddMsg::CMD_SETVOLUME:
			CLcddMsg::commandVolume msg2;
			read(connfd, &msg2, sizeof(msg2));
			lcdPainter.show_volume(msg2.volume);
			break;
		case CLcddMsg::CMD_SETMUTE:
			CLcddMsg::commandMute msg3;
			read(connfd, &msg3, sizeof(msg3));
			lcdPainter.setMuted(msg3.mute);
			break;
		case CLcddMsg::CMD_SETMODE:
			CLcddMsg::commandMode msg4;
			read(connfd, &msg4, sizeof(msg4));
			lcdPainter.set_mode((CLcddTypes::mode) msg4.mode, msg4.text);
			break;
		case CLcddMsg::CMD_SETMENUTEXT:
			CLcddMsg::commandMenuText msg5;
			read(connfd, &msg5, sizeof(msg5));
			lcdPainter.show_menu(msg5.position, msg5.text, msg5.highlight);
			break;
		case CLcddMsg::CMD_SETLCDBRIGHTNESS:
			CLcddMsg::commandSetBrightness msg6;
			read(connfd, &msg6, sizeof(msg6));
			lcdPainter.setBrightness(msg6.brightness);
			break;
		case CLcddMsg::CMD_SETLCDCONTRAST:
			CLcddMsg::commandSetBrightness msg7;
			read(connfd, &msg7, sizeof(msg7));
			lcdPainter.setContrast(msg7.brightness);
			break;
		case CLcddMsg::CMD_SETSTANDBYLCDBRIGHTNESS:
			CLcddMsg::commandSetBrightness msg8;
			read(connfd, &msg8, sizeof(msg8));
			lcdPainter.setBrightnessStandby(msg8.brightness);
			break;
		case CLcddMsg::CMD_GETLCDBRIGHTNESS:
			CLcddMsg::responseGetBrightness msg9;
			msg9.brightness = lcdPainter.getBrightness();
			write(connfd, &msg9, sizeof(msg9));
			break;
		case CLcddMsg::CMD_GETLCDCONTRAST:
			CLcddMsg::responseGetBrightness msg10;
			msg10.brightness = lcdPainter.getContrast();
			write(connfd, &msg10, sizeof(msg10));
			break;
		case CLcddMsg::CMD_GETSTANDBYLCDBRIGHTNESS:
			CLcddMsg::responseGetBrightness msg11;
			msg11.brightness = lcdPainter.getBrightnessStandby();
			write(connfd, &msg11, sizeof(msg11));
			break;

		case CLcddMsg::CMD_SETLCDPOWER:
			CLcddMsg::commandPower msg12;
			read(connfd, &msg12, sizeof(msg12));
			lcdPainter.setPower(msg12.power);
			break;
		case CLcddMsg::CMD_GETLCDPOWER:
			CLcddMsg::commandPower msg13;
			msg13.power = lcdPainter.getPower();
			write(connfd, &msg13, sizeof(msg13));
			break;
		case CLcddMsg::CMD_SETLCDINVERSE:
			CLcddMsg::commandInverse msg14;
			read(connfd, &msg14, sizeof(msg14));
			lcdPainter.setInverse(msg14.inverse);
			break;
		case CLcddMsg::CMD_GETLCDINVERSE:
			CLcddMsg::commandInverse msg15;
			msg15.inverse = lcdPainter.getInverse();
			write(connfd, &msg15, sizeof(msg15));
			break;
		case CLcddMsg::CMD_UPDATE:
			lcdPainter.update();
			break;
		case CLcddMsg::CMD_PAUSE:
			lcdPainter.pause();
			break;
		case CLcddMsg::CMD_RESUME:
			lcdPainter.resume();
			break;

		default:
			printf("unknown command %i\n", rmsg.cmd);
	}
}

void* CLCDD::TimeThread(void *)
{
	while(1)
	{
		sleep(10);
		CLCDD::getInstance()->lcdPainter.show_time();
	}
	return NULL;
}

void CLCDD::sig_catch(int)
{
	 exit(0);
}

int CLCDD::main(int argc, char **argv)
{
	debugoutput = true;
	printf("Network LCD-Driver $Id: lcdd.cpp,v 1.57 2002/10/17 10:16:35 thegoodguy Exp $\n\n");

	InitNewClock();

	loadConfig();

	if(!lcdPainter.init())
	{
		//fehler beim lcd-init aufgetreten
		return -1;
	}

	//network-setup
	int listenfd, connfd;
	struct sockaddr_un servaddr;
	int clilen;

	std::string filename = LCDD_UDS_NAME;

	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, filename.c_str());
	clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
	unlink(filename.c_str());

	//network-setup
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		return -1;
	}

	if ( bind(listenfd, (struct sockaddr*) &servaddr, clilen) <0 )
	{
		perror("[lcdd] bind failed...\n");
		return -1;
	}

	if (listen(listenfd, 5) !=0)
	{
		perror("[lcdd] listen failed...\n");
		return -1;
	}

	switch (fork())
	{
		case -1: /* can't fork */
			perror("[lcdd] fork");
			return -1;

		case 0: /* child, process becomes a daemon */
			if (setsid() == -1)
			{
				perror("[lcdd] setsid");
				return -1;
			}
			break;

		default: /* parent returns to calling process */
			return 0;
	}

	signal(SIGHUP,sig_catch);
	signal(SIGINT,sig_catch);
	signal(SIGQUIT,sig_catch);
	signal(SIGTERM, sig_catch);

	/* Thread erst nach dem forken erstellen, da sonst Abbruch */
	if (pthread_create (&thrTime, NULL, TimeThread, NULL) != 0 )
	{
		perror("[lcdd]: pthread_create(TimeThread)");
		return -1;
	}

	shall_exit = false;
	while(!shall_exit)
	{
		connfd = accept(listenfd, (struct sockaddr*) &servaddr, (socklen_t*) &clilen);
		CLcddMsg::Header rmsg;
		read(connfd,&rmsg,sizeof(rmsg));
		parse_command(connfd, rmsg);
		close(connfd);
	}
	close(listenfd);
	saveConfig();
	return 0;
}


int main(int argc, char **argv)
{
	return CLCDD::getInstance()->main(argc, argv);
}

