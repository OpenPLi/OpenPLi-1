/*
	Control-Daemon  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean',
	              2002 dboxII-team
	
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

#include <fcntl.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <dbox/avs_core.h>
#include <dbox/fp.h>
#include <dbox/saa7126_core.h>
#include <ost/video.h>

#include <zapit/client/zapitclient.h>
#include <eventserver.h>

#include <configfile.h>
#include <controldclient/controldclient.h>
#include <controldclient/controldMsg.h>
#include <lcddclient/lcddclient.h>
#include <timerdclient/timerdclient.h>

#include "eventwatchdog.h"
#include "driver/audio.h"


#define CONF_FILE CONFIGDIR "/controld.conf"
#define SAA7126_DEVICE "/dev/dbox/saa0"


CLcddClient	lcdd;
CZapitClient	zapit;
CTimerdClient	timerd;
CEventServer	*eventServer;

/* the configuration file */
CConfigFile * config = NULL;

struct Ssettings
{
	int  volume;
	int  volume_avs;
	bool mute;
	bool mute_avs;
	int  videooutput;
	int  videoformat;

	char boxtype; // not part of the config - set by setBoxType()
} settings;

int	nokia_scart[6];
int	nokia_dvb[6];
int	sagem_scart[6];
int	sagem_dvb[6];
int	philips_scart[6];
int	philips_dvb[6];
char aspectRatio;

char BoxNames[4][10] = {"","Nokia", "Sagem", "Philips"};


void sig_catch(int);

class CControldAspectRatioNotifier : public CAspectRatioNotifier
{
	public:
		virtual void aspectRatioChanged( int newAspectRatio); //override;
};

CEventWatchDog* watchDog;
CControldAspectRatioNotifier* aspectRatioNotifier;

void saveSettings()
{
	config->saveConfig(CONF_FILE);
}

void shutdownBox()
{
	lcdd.shutdown();

	zapit.shutdown();

	timerd.shutdown();

	saveSettings();

	if (execlp("/sbin/halt", "/sbin/halt", 0)<0)
	{
		perror("exec failed - halt\n");
	}
}

void setvideooutput(int format, bool bSaveSettings = true)
{
	int fd;
	if (format < 0)
	{
		format=0;
	}
	if (format > 3)
	{
		format=3;
	}

	// 0 - COMPOSITE
	// 1 - RGB
	// 2 - SVIDEO

	if (bSaveSettings) // only set settings if we dont come from watchdog
	{
		settings.videooutput = format;
		config->setInt32("videooutput", settings.videooutput);
	}

	int	arg;

    switch ( format )
	{
	 	case 0:
	 		arg = 0;
	 		break;
		case 1:
	 		arg = 1;
	 		break;
		case 2:
	 		arg = 0;
	 		break;
    }
	if ((fd = open("/dev/dbox/avs0",O_RDWR)) < 0)
	{
		perror("open");
		return;
	}

	if (ioctl(fd, AVSIOSFBLK, &arg)< 0)
	{
		perror("AVSIOSFBLK:");
		close(fd);
		return;
	}
	close(fd);

	switch ( format )
	{
	 	case 0:
	 		arg = SAA_MODE_FBAS;
	 		break;
		case 1:
	 		arg = SAA_MODE_RGB;
	 		break;
		case 2:
	 		arg = SAA_MODE_SVIDEO;
	 		break;
    }
	if((fd = open(SAA7126_DEVICE,O_RDWR|O_NONBLOCK)) < 0)
	{
		perror("[controld] SAA DEVICE: ");
		return;
	}

	if ( (ioctl(fd, SAAIOSMODE, &arg) < 0))
	{
		perror("[controld] IOCTL: ");
	}
	close(fd);

}

void setVideoFormat(int format, bool bSaveFormat = true )
{
	int fd;
	int videoDisplayFormat;
	int avsiosfncFormat;
	int wss;

	/*
		16:9 : fnc 1
		4:3  : fnc 2
	*/


	if (bSaveFormat) // only set settings if we dont come from watchdog or video_off
	{
		if (format < 0)
			format=0;
		if (format > 3)
			format=3;

		settings.videoformat = format;
		config->setInt32("videoformat", settings.videoformat);
	}

	if (format==0) // automatic switch
	{
		printf("[controld] setting VideoFormat to auto \n");

		switch ( aspectRatio )
		{
			case 2 :	// 4:3
				format= 2;
				break;
			case 3 :	// 16:9
			case 4 :	// 21,1:1
				format= 1;
				break;
			default:
				format= 2;
				// damits nicht ausgeht beim starten :)
		}
	}

	if ((fd = open("/dev/dbox/avs0",O_RDWR)) < 0)
	{
		perror("open");
		return;
	}

	if (format< 0)
		format= 0;

	avsiosfncFormat = format;
	if (settings.boxtype == CControldClient::BOXTYPE_PHILIPS) // Philips
	{
		switch (format)
		{
			case 1 :
				avsiosfncFormat=2;
				break;
			case 2 :
				avsiosfncFormat=3;
				break;
		}
	}
	if (ioctl(fd,AVSIOSFNC,&avsiosfncFormat)< 0)
	{
		perror("AVSIOSFNC");
		close(fd);
		return;
	}
	close(fd);

	switch( format )
	{
		//	?	case AVS_FNCOUT_INTTV	: videoDisplayFormat = VIDEO_PAN_SCAN;
		case AVS_FNCOUT_EXT169	:
			videoDisplayFormat = VIDEO_CENTER_CUT_OUT;
			wss = SAA_WSS_169F;
			break;
		case AVS_FNCOUT_EXT43	:
			videoDisplayFormat = VIDEO_LETTER_BOX;
			wss = SAA_WSS_43F;
			break;
		default:
			videoDisplayFormat = VIDEO_LETTER_BOX;
			wss = SAA_WSS_43F;
			break;
			//	?	case AVS_FNCOUT_EXT43_1	: videoDisplayFormat = VIDEO_PAN_SCAN;
	}

	zapit.setDisplayFormat((int)videoDisplayFormat);

	if ( (fd = open(SAA7126_DEVICE,O_RDWR)) < 0)
	{
		perror("open " SAA7126_DEVICE);
		return;
	}

	ioctl(fd,SAAIOSWSS,&wss);
	close(fd);
}

void LoadScart_Settings()
{
	// scart
	sagem_scart[0]= 2;
	sagem_scart[1]= 1;
	sagem_scart[2]= 0;
	sagem_scart[3]= 0;
	sagem_scart[4]= 0;
	sagem_scart[5]= 0;

	nokia_scart[0]= 3;
	nokia_scart[1]= 2;
	nokia_scart[2]= 1;
	nokia_scart[3]= 0;
	nokia_scart[4]= 1;
	nokia_scart[5]= 1;

	philips_scart[0]= 2;
	philips_scart[1]= 2;
	philips_scart[2]= 3;
	philips_scart[3]= 0;
	philips_scart[4]= 3;
	philips_scart[5]= 0;

	// dvb
	sagem_dvb[0]= 0;
	sagem_dvb[1]= 0;
	sagem_dvb[2]= 0;
	sagem_dvb[3]= 0;
	sagem_dvb[4]= 0;
	sagem_dvb[5]= 0;

	nokia_dvb[0]= 5;
	nokia_dvb[1]= 1;
	nokia_dvb[2]= 1;
	nokia_dvb[3]= 0;
	nokia_dvb[4]= 1;
	nokia_dvb[5]= 0;

	philips_dvb[0]= 1;
	philips_dvb[1]= 1;
	philips_dvb[2]= 1;
	philips_dvb[3]= 0;
	philips_dvb[4]= 1;
	philips_dvb[5]= 0;

	FILE* fd = fopen(CONFIGDIR"/scart.conf", "r");
	if(fd)
	{
		printf("[controld]: loading scart-config (scart.conf)\n");

		char buf[1000];
		fgets(buf,sizeof(buf),fd);

		if(fgets(buf,sizeof(buf),fd)!=NULL)
		{
			sscanf( buf, "nokia_scart: %d %d %d %d %d %d\n", &nokia_scart[0], &nokia_scart[1], &nokia_scart[2], &nokia_scart[3], &nokia_scart[4], &nokia_scart[5] );
			//printf( buf );
		}
		if(fgets(buf,sizeof(buf),fd)!=NULL)
		{
			sscanf( buf, "nokia_dvb: %d %d %d %d %d %d\n", &nokia_dvb[0], &nokia_dvb[1], &nokia_dvb[2], &nokia_dvb[3], &nokia_dvb[4], &nokia_dvb[5] );
			//printf( buf );
		}
		if(fgets(buf,sizeof(buf),fd)!=NULL)
		{
			sscanf( buf, "sagem_scart: %d %d %d %d %d %d\n", &sagem_scart[0], &sagem_scart[1], &sagem_scart[2], &sagem_scart[3], &sagem_scart[4], &sagem_scart[5] );
			//printf( buf );
		}
		if(fgets(buf,sizeof(buf),fd)!=NULL)
		{
			sscanf( buf, "sagem_dvb: %d %d %d %d %d %d\n", &sagem_dvb[0], &sagem_dvb[1], &sagem_dvb[2], &sagem_dvb[3], &sagem_dvb[4], &sagem_dvb[5] );
			//printf( buf );
		}
		if(fgets(buf,sizeof(buf),fd)!=NULL)
		{
			sscanf( buf, "philips_scart: %d %d %d %d %d %d\n", &philips_scart[0], &philips_scart[1], &philips_scart[2], &philips_scart[3], &philips_scart[4], &philips_scart[5] );
			//printf( buf );
		}
		if(fgets(buf,sizeof(buf),fd)!=NULL)
		{
			sscanf( buf, "philips_dvb: %d %d %d %d %d %d\n", &philips_dvb[0], &philips_dvb[1], &philips_dvb[2], &philips_dvb[3], &philips_dvb[4], &philips_dvb[5] );
			//printf( buf );
		}
		fclose(fd);
	}
	else
	{
		printf("[controld]: failed to load scart-config (scart.conf), using standard-values\n");
	}
}


void routeVideo(int v1, int a1, int v2, int a2, int v3, int a3, int fblk)
{
	int fd;

	if ((fd = open("/dev/dbox/avs0",O_RDWR)) < 0)
	{
		perror("open");
		return;
	}

	if (ioctl(fd, AVSIOSFBLK, &fblk)< 0)
	{
		perror("AVSIOSFBLK:");
		close(fd);
		return;
	}

	if (ioctl(fd,AVSIOSVSW1,&v1)< 0)
	{
		perror("AVSIOSVSW1:");
		close(fd);
		return;
	}

	if (ioctl(fd,AVSIOSASW1,&a1)< 0)
	{
		perror("AVSIOSASW1:");
		close(fd);
		return;
	}

	if (ioctl(fd,AVSIOSVSW2,&v2)< 0)
	{
		perror("AVSIOSVSW2:");
		close(fd);
		return;
	}

	if (ioctl(fd,AVSIOSASW2,&a2)< 0)
	{
		perror("AVSIOSASW2:");
		close(fd);
		return;
	}

	if (ioctl(fd,AVSIOSVSW3,&v3)< 0)
	{
		perror("AVSIOSVSW3:");
		close(fd);
		return;
	}

	if (ioctl(fd,AVSIOSASW3,&a3)< 0)
	{
		perror("AVSIOSASW3:");
	}

	close(fd);
}

void switch_vcr( bool vcr_on)
{
	LoadScart_Settings();

	if (vcr_on)
	{
		//turn to scart-input
		printf("[controld]: switch to scart-input... (%s)\n", BoxNames[settings.boxtype]);
		if (settings.boxtype == 2) // Sagem
		{
			routeVideo(sagem_scart[0], sagem_scart[1], sagem_scart[2], sagem_scart[3], sagem_scart[4], sagem_scart[5], 0);
		}
		else if (settings.boxtype == 1) // Nokia
		{
			routeVideo(nokia_scart[0], nokia_scart[1], nokia_scart[2], nokia_scart[3], nokia_scart[4], nokia_scart[5], 2);
		}
		else if (settings.boxtype == 3) // Philips
		{
			routeVideo(philips_scart[0], philips_scart[1], philips_scart[2], philips_scart[3], philips_scart[4], philips_scart[5], 2);
		}
	}
	else
	{	//turn to dvb...
		printf("[controld]: switch to dvb-input... (%s)\n", BoxNames[settings.boxtype]);
		if (settings.boxtype == 2) // Sagem
		{
			routeVideo( sagem_dvb[0], sagem_dvb[1], sagem_dvb[2], sagem_dvb[3], sagem_dvb[4], sagem_dvb[5], settings.videooutput);
		}
		else if (settings.boxtype == 1) // Nokia
		{
			routeVideo( nokia_dvb[0], nokia_dvb[1], nokia_dvb[2], nokia_dvb[3], nokia_dvb[4], nokia_dvb[5], settings.videooutput);
		}
		else if (settings.boxtype == 3) // Philips
		{
			routeVideo( philips_dvb[0], philips_dvb[1], philips_dvb[2], philips_dvb[3], philips_dvb[4], philips_dvb[5], settings.videooutput);
		}
	}
}

void setScartMode(bool onoff)
{
	if(onoff)
	{
		lcdd.setMode(CLcddTypes::MODE_SCART);
	}
	else
	{
		lcdd.setMode(CLcddTypes::MODE_TVRADIO);
	}
	switch_vcr( onoff );
}

void disableVideoOutput(bool disable)
{
	int arg=disable?1:0;
	int fd;
	printf("[controld] videoOutput %s\n", disable?"off":"on");

	if((fd = open(SAA7126_DEVICE,O_RDWR|O_NONBLOCK)) < 0)
	{
		perror("[controld] SAA DEVICE: ");
		return;
	}

	if ( (ioctl(fd,SAAIOSPOWERSAVE,&arg) < 0))
	{
		perror("[controld] IOCTL: ");
		close(fd);
		return;
	}
	close(fd);
	/*
		arg=disable?0:0xf;
		if((fd = open("/dev/dbox/fp0",O_RDWR|O_NONBLOCK)) < 0)
		{
			perror("[controld] FP DEVICE: ");
			return;
		}

		if ( (ioctl(fd,FP_IOCTL_LCD_DIMM,&arg) < 0))
		{
			perror("[controld] IOCTL: ");
			close(fd);
			return;
		}
		close(fd);
	*/
	if(!disable)
	{
		zapit.startPlayBack();
		setvideooutput(settings.videooutput, false);
		setVideoFormat(settings.videoformat, false);
	}
	else
	{
		setvideooutput(0, false);
		setVideoFormat(-1, false);
		zapit.stopPlayBack();
	}
}

void setBoxType()
{
	char strmID[40];
	strcpy( strmID, getenv("mID") );
	int mID = atoi(strmID);

	switch ( mID )
	{
		case 3:	settings.boxtype= CControldClient::BOXTYPE_SAGEM;
				break;
		case 2:	settings.boxtype= CControldClient::BOXTYPE_PHILIPS;
				break;
		default:
			settings.boxtype= CControldClient::BOXTYPE_NOKIA;
	}

	printf("[controld] Boxtype detected: (%s, %d, %d, %s)\n", strmID, mID, settings.boxtype, BoxNames[settings.boxtype]);
}


// input:   0 (min volume) <=     volume           <= 100 (max volume)
// output: 63 (min volume) >= map_volume(., true)  >=   0 (max volume)
// output:  0 (min volume) <= map_volume(., false) <= 255 (max volume)
const unsigned char map_volume(const unsigned char volume, const bool to_AVS)
{
	int res = 0;
	if( to_AVS )
	{
		res = lrint(64 - 32 * log(volume/13.5)) & 0xFFFFFFFF;
		if (res < 0)
		{
			res = 0;
		}
		else if (res > 63)
		{
			res = 63;
		}

	}
	else
		res = (int) (volume * 2.55);

	return res;
}


void parse_command(int connfd, CControld::Header * rmessage)
{
	
	if(rmessage->version != CControld::ACTVERSION)
	{
		perror("[controld] wrong version\n");
		return;
	}

	switch (rmessage->cmd)
	{
	case CControld::CMD_SHUTDOWN:
		shutdownBox();
		break;

	case CControld::CMD_SAVECONFIG:
		saveSettings();
		break;

	case CControld::CMD_SETVOLUME:
	case CControld::CMD_SETVOLUME_AVS:
		CControld::commandVolume msg_commandVolume;
		read(connfd, &msg_commandVolume, sizeof(msg_commandVolume));

		if (rmessage->cmd == CControld::CMD_SETVOLUME)
		{
			settings.volume = msg_commandVolume.volume;
			config->setInt32("volume", settings.volume);
			zapit.setVolume(map_volume(msg_commandVolume.volume, false), map_volume(msg_commandVolume.volume, false));
		}
		else
		{
			settings.volume_avs = msg_commandVolume.volume;
			config->setInt32("volume_avs", settings.volume_avs);
			audioControl::setVolume(map_volume(msg_commandVolume.volume, true));
		}
		lcdd.setVolume(msg_commandVolume.volume);
		eventServer->sendEvent(CControldClient::EVT_VOLUMECHANGED, CEventServer::INITID_CONTROLD, &msg_commandVolume.volume, sizeof(msg_commandVolume.volume));
		break;

	case CControld::CMD_MUTE:
		settings.mute = true;
		config->setBool("mute", settings.mute);
		zapit.muteAudio(true);
		lcdd.setMute(true);
		eventServer->sendEvent(CControldClient::EVT_MUTECHANGED, CEventServer::INITID_CONTROLD, &settings.mute, sizeof(settings.mute));
		break;
	case CControld::CMD_MUTE_AVS:
		settings.mute_avs = true;
		config->setBool("mute_avs", settings.mute_avs);
		audioControl::setMute(true);
		lcdd.setMute(true);
		eventServer->sendEvent(CControldClient::EVT_MUTECHANGED, CEventServer::INITID_CONTROLD, &settings.mute_avs, sizeof(settings.mute_avs));
		break;
	case CControld::CMD_UNMUTE:
		settings.mute = false;
		config->setBool("mute", settings.mute);
		zapit.muteAudio(false);
		lcdd.setMute(settings.mute_avs);
		eventServer->sendEvent(CControldClient::EVT_MUTECHANGED, CEventServer::INITID_CONTROLD, &settings.mute_avs, sizeof(settings.mute_avs));
		break;
	case CControld::CMD_UNMUTE_AVS:
		settings.mute_avs = false;
		config->setBool("mute_avs", settings.mute_avs);
		audioControl::setMute(false);
		lcdd.setMute(settings.mute);
		eventServer->sendEvent(CControldClient::EVT_MUTECHANGED, CEventServer::INITID_CONTROLD, &settings.mute, sizeof(settings.mute));
		break;

		case CControld::CMD_SETANALOGMODE:
			CControld::commandAnalogMode msgmd;
			read(connfd, &msgmd, sizeof(msgmd));
			zapit.setAudioMode(msgmd.mode);
			break;
		case CControld::CMD_SETVIDEOFORMAT:
			//printf("[controld] set videoformat\n");
			CControld::commandVideoFormat msg2;
			read(connfd, &msg2, sizeof(msg2));
			setVideoFormat(msg2.format);
			break;
		case CControld::CMD_SETVIDEOOUTPUT:
			//printf("[controld] set videooutput\n");
			CControld::commandVideoOutput msg3;
			read(connfd, &msg3, sizeof(msg3));
			setvideooutput(msg3.output);
			break;
		case CControld::CMD_SETBOXTYPE:
			//printf("[controld] set boxtype\n");    //-------------------dummy!!!!!!!!!!
			CControld::commandBoxType msg4;
			read(connfd, &msg4, sizeof(msg4));
			setBoxType();
			break;
		case CControld::CMD_SETSCARTMODE:
			//printf("[controld] set scartmode\n");
			CControld::commandScartMode msg5;
			read(connfd, &msg5, sizeof(msg5));
			setScartMode(msg5.mode);
			break;
		case CControld::CMD_SETVIDEOPOWERDOWN:
			//printf("[controld] set scartmode\n");
			CControld::commandVideoPowerSave msg10;
			read(connfd, &msg10, sizeof(msg10));
			disableVideoOutput(msg10.powerdown);
			break;

	case CControld::CMD_GETVOLUME:
	case CControld::CMD_GETVOLUME_AVS:
		CControld::responseVolume msg_responseVolume;
		msg_responseVolume.volume = (rmessage->cmd == CControld::CMD_GETVOLUME) ? settings.volume : settings.volume_avs;
		write(connfd, &msg_responseVolume, sizeof(msg_responseVolume));
		break;

	case CControld::CMD_GETMUTESTATUS:
	case CControld::CMD_GETMUTESTATUS_AVS:
		CControld::responseMute msg_responseMute;
		msg_responseMute.mute = (rmessage->cmd == CControld::CMD_GETMUTESTATUS) ? settings.mute : settings.mute_avs;
		write(connfd, &msg_responseMute, sizeof(msg_responseMute));
		break;

		case CControld::CMD_GETVIDEOFORMAT:
			//printf("[controld] get videoformat (fnc)\n");
			CControld::responseVideoFormat msg8;
			msg8.format = settings.videoformat;
			write(connfd,&msg8,sizeof(msg8));
			break;
		case CControld::CMD_GETASPECTRATIO:
			//printf("[controld] get videoformat (fnc)\n");
			CControld::responseAspectRatio msga;
			msga.aspectRatio = aspectRatio;
			write(connfd,&msga,sizeof(msga));
			break;
		case CControld::CMD_GETVIDEOOUTPUT:
			//printf("[controld] get videooutput (fblk)\n");
			CControld::responseVideoOutput msg9;
			msg9.output = settings.videooutput;
			write(connfd,&msg9,sizeof(msg9));
			break;
		case CControld::CMD_GETBOXTYPE:
			//printf("[controld] get boxtype\n");
			CControld::responseBoxType msg0;
			msg0.boxtype = settings.boxtype;
			write(connfd,&msg0,sizeof(msg0));
			break;

		case CControld::CMD_REGISTEREVENT:
			eventServer->registerEvent(connfd);
			break;
		case CControld::CMD_UNREGISTEREVENT:
			eventServer->unRegisterEvent(connfd);
			break;

		default:
			printf("[controld] unknown command\n");
	}
}


void sig_catch(int signal)
{
	switch (signal)
	{
	case SIGHUP:
		saveSettings();
		break;
	default:
		saveSettings();
		exit(0);
	}
}

int main(int argc, char **argv)
{
	int listenfd, connfd;
	printf("Controld  $Id: controld.cpp,v 1.76.2.1 2002/10/24 20:34:32 thegoodguy Exp $\n\n");
	
	switch (fork())
	{
	case -1:
		perror("[controld] fork");
		return -1;
	case 0:
		break;
	default:
		return 0;
	}
	
	if (setsid() == -1)
	{
		perror("[controld] setsid");
		return -1;
	}

	eventServer = new CEventServer;

	struct sockaddr_un servaddr;
	int clilen;
	memset(&servaddr, 0, sizeof(struct sockaddr_un));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, CONTROLD_UDS_NAME);
	clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
	unlink(CONTROLD_UDS_NAME);

	//network-setup
	if ((listenfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
	}

	if ( bind(listenfd, (struct sockaddr*) &servaddr, clilen) <0 )
	{
		perror("[controld] bind failed...\n");
		exit(-1);
	}


	if (listen(listenfd, 5) !=0)
	{
		perror("[controld] listen failed...\n");
		exit( -1 );
	}

	//busyBox
	signal(SIGHUP,sig_catch);
	signal(SIGINT,sig_catch);
	signal(SIGQUIT,sig_catch);
	signal(SIGTERM,sig_catch);

	/* load configuration */
	config = new CConfigFile(',');

	if (!config->loadConfig(CONF_FILE))
	{
		/* set defaults if no configuration file exists */
		printf("[controld] %s not found\n", CONF_FILE);
	}


	settings.volume      = config->getInt32("volume", 100);
	settings.volume_avs  = config->getInt32("volume_avs", 100);
	settings.mute        = config->getBool("mute", false);
	settings.mute_avs    = config->getBool("mute_avs", false);
	settings.videooutput = config->getInt32("videooutput", 1); // fblk1 - rgb
	settings.videoformat = config->getInt32("videoformat", 2); // fnc2 - 4:3

	setBoxType(); // dummy set - liest den aktuellen Wert aus!

	watchDog = new CEventWatchDog();
	aspectRatioNotifier = new CControldAspectRatioNotifier();
	watchDog->registerNotifier(WDE_VIDEOMODE, aspectRatioNotifier);

	//init
	audioControl::setVolume(map_volume(settings.volume_avs, true));
	zapit.setVolume(map_volume(settings.volume, false), map_volume(settings.volume, false));
	lcdd.setVolume(settings.volume_avs);    // we could also display settings.volume at startup

	audioControl::setMute(settings.mute_avs);
	zapit.muteAudio(settings.mute);
	lcdd.setMute(settings.mute || settings.mute_avs);

	setvideooutput(settings.videooutput);
	setVideoFormat(settings.videoformat, false);


	try
	{
		struct CControld::Header rmessage;
		while(true)
		{
			connfd = accept(listenfd, (struct sockaddr*) &servaddr, (socklen_t*) &clilen);
			memset(&rmessage, 0, sizeof(rmessage));
			read(connfd,&rmessage,sizeof(rmessage));

			parse_command(connfd, &rmessage);
			close(connfd);
		}
	}
	catch (std::exception& e)
	{
		printf("[controld] caught std-exception in main-thread %s!\n", e.what());
	}
	catch (...)
	{
	    printf("[controld] caught exception in main-thread!\n");
  	}
}

void CControldAspectRatioNotifier::aspectRatioChanged( int newAspectRatio )
{
	//printf("[controld] CControldAspectRatioNotifier::aspectRatioChanged( %d ) \n", newAspectRatio);
	aspectRatio= newAspectRatio;

	if ( settings.videoformat == 0 )
	{
		switch (newAspectRatio)
		{
			case 2 :	// 4:3
				setVideoFormat( 2, false );
				break;
			case 3 :	// 16:9
			case 4 :	// 2,21:1
				setVideoFormat( 1, false );
				break;
			default:
				printf("[controld] Unknown aspectRatio: %d", newAspectRatio);
		}
	}
}
