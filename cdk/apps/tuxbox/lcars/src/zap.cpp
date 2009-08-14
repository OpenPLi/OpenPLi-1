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
$Log: zap.cpp,v $
Revision 1.15.2.1.2.3  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.15.2.1.2.2  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.15.2.1.2.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.16  2003/10/07 20:39:28  thedoc

someone added premiere-ca support, which was not supposed to be supported in lcars, sorry...

Revision 1.15  2003/01/28 00:25:24  thedoc
why does it always show such strange error messages?

Revision 1.14  2003/01/26 00:00:20  thedoc
mv bugs /dev/null

Revision 1.13  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.12  2003/01/05 06:49:59  TheDOC
lcars should work now with the new drivers more properly

Revision 1.11  2002/11/26 20:03:14  TheDOC
some debug-output and small fixes

Revision 1.10  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.9  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.8  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.7  2002/09/17 12:20:24  TheDOC
start/stop devices

Revision 1.6  2002/06/08 20:21:09  TheDOC
adding the cam-sources with slight interface-changes

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
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <memory.h>

#include <set>
#include <algorithm>
#include <string>

#include "zap.h"
#include "help.h"
#include "settings.h"
#include "nit.h"
#include "sdt.h"
#include "osd.h"
#include "tuner.h"
#include "pat.h"
#include "pmt.h"
#include "lcd.h"

#define BSIZE 10000

zap::zap(settings &set, osd &o, tuner &t, cam &c/*, lcddisplay &l*/) : setting(set), osdd(o), tune(t), ca(c)/*, lcd(l)*/
{
//	lcd_obj = l;
	printf("Initializing zapper...\n");
//	lcd_obj->setTextSize(12);
//	lcd_obj->putText(80, 58, 0, "Initializing...");
//	vcaps.cap = DVB_VIDEO_CAP_SOURCE_FORMATS;
//	ioctl(fd, DVB_VIDEO_GET_CAPS, &vcaps);

	vid = open(VIDEO_DEV, O_RDWR);
	if((video = open(DEMUX_DEV, O_RDWR)) < 0) {
		printf("Cannot open demux device \n");
		exit(1);
	}

	if((audio = open(DEMUX_DEV, O_RDWR)) < 0) {
		printf("Cannot open demux device\n");
		exit(1);
	}
	aud = open(AUDIO_DEV, O_RDWR);
	ioctl(vid, VIDEO_SELECT_SOURCE, (video_stream_source_t)VIDEO_SOURCE_DEMUX);
	old_frequ = 0;
	old_TS = -1;
	usevideo = false, useaudio = false, usepcr = false;
}

zap::~zap()
{
	close(vid);
	close(video);
	close(audio);
	close(frontend);
}


void zap::close_dev()
{
	close(vid);
	close(aud);
	close(video);
	close(audio);
	close(pcr);
}

void zap::zap_allstop()
{
	if (usepcr)
		if (ioctl(pcr,DMX_STOP) < 0)
			perror("[zap.cpp]DMX_STOP pcr zap_allstop");

	do
	{
		if (ioctl(aud, AUDIO_GET_STATUS, &astatus) < 0)
			perror ("[zap.cpp]AUDIO_GET_STATUS");
#ifdef HAVE_LINUX_DVB_VERSION_H
		if (astatus.play_state != AUDIO_STOPPED)
#elif HAVE_OST_DMX_H
		if (astatus.playState != AUDIO_STOPPED)
#endif
		{
			std::cout << "[zap.cpp]Stopping audio-device" << std::endl;
			if (ioctl(audio, DMX_STOP) < 0)
				perror("[zap.cpp]DMX_STOP audio");
			if (ioctl(aud, AUDIO_STOP) < 0)
				perror("[zap.cpp]AUDIO_STOP");
		}
#ifdef HAVE_LINUX_DVB_VERSION_H
	} while (astatus.play_state != AUDIO_STOPPED);
#elif HAVE_OST_DMX_H
	} while (astatus.playState != AUDIO_STOPPED);
#endif
	
	do
	{
		if (ioctl(vid, VIDEO_GET_STATUS, &vstatus) < 0)
			perror ("[zap.cpp]VIDEO_GET_STATUS");
#ifdef HAVE_LINUX_DVB_VERSION_H
		if (vstatus.play_state != VIDEO_STOPPED)
#elif HAVE_OST_DMX_H
		if (vstatus.playState != VIDEO_STOPPED)
#endif
		{
			std::cout << "[zap.cpp]Stopping video-device" << std::endl;
			if (ioctl(video, DMX_STOP) < 0)
				perror("[zap.cpp]DMX_STOP video");
			if (ioctl(vid, VIDEO_STOP) < 0)
				perror("[zap.cpp]VIDEO_STOP");
		}
#ifdef HAVE_LINUX_DVB_VERSION_H
	} while (vstatus.play_state != VIDEO_STOPPED);
#elif HAVE_OST_DMX_H
	} while (vstatus.playState != VIDEO_STOPPED);
#endif
}

void zap::zap_to(pmt_data pmt, int VPID, int APID, int PCR, int ECM, int SID, int ONID, int TS, int PID1, int PID2)
{
	if (VPID == 0)
		VPID = 0x1fff;
	if (APID == 0)
		APID = 0x1fff;
	if (PCR == 0)
		PCR = 0x1fff;

	if (ECM != 0 && ONID != 0x85)
	{
		std::cout << "Doing CA" << std::endl;
		ca.initialize();
		if (VPID != 0x1fff)
		{
			ca.addPID(VPID);
		}
		ca.addPID(APID);
		ca.setECM(ECM);
		ca.setSID(SID);
		ca.setONID(ONID);
		ca.setPMTentry(pmt);
		if (PID1 != -1)
		{
			ca.addPID(PID1);
		}
		if (PID2 != -1)
			ca.addPID(PID2);

		if (old_TS != TS)
		{
			ca.reset();
			ca.setEMM(setting.getEMMpid());
			ca.startEMM();
		}
		ca.descramble();
	}

	usevideo = ((VPID >= 0x20) && (VPID <= 0x1FFB));		
	
	usepcr = ((PCR >= 0x20) && (PCR <= 0x1FFB));

	useaudio = ((APID >= 0x20) && (APID <= 0x1FFB));

	std::cout << "Start Zapping" << std::endl;

	if (usevideo)
	{
		std::cout << "[zap.cpp]Open video" << std::endl;
		if ((vid = open(VIDEO_DEV, O_RDWR)) < 0)
		{
			perror(VIDEO_DEV);
			exit(1);
		}

		std::cout << "[zap.cpp]Open video demux" << std::endl;
		if((video = open(DEMUX_DEV, O_RDWR)) < 0) {
			perror(DEMUX_DEV);
			exit(1);
		}
	}

	if (usepcr)
	{
		std::cout << "[zap.cpp]Open pcr demux" << std::endl;
		if((pcr = open(DEMUX_DEV, O_RDWR)) < 0) {
			perror(DEMUX_DEV);
			exit(1);
		}
	}

	if (useaudio)
	{
		std::cout << "[zap.cpp]Open audio" << std::endl;
		if ((aud = open(AUDIO_DEV, O_RDWR)) < 0)
		{	
			perror(AUDIO_DEV);
			exit(1);
		}

		std::cout << "[zap.cpp]Open audio demux" << std::endl;
		if((audio = open(DEMUX_DEV, O_RDWR)) < 0) {
			perror(DEMUX_DEV);
			exit(1);
		}
	}

	struct dmx_pes_filter_params pes_filter;

	printf("Zap to\nSID: %04x\nVPID: %04x\nAPID: %04x\nECM: %04x\nONID: %04x\n\n", SID, VPID, APID, ECM, ONID);

	
	ioctl(audio,AUDIO_SET_BYPASS_MODE, 0);
	if (usevideo)
	{
#ifdef HAVE_LINUX_DVB_VERSION_H
		if (vstatus.stream_source != VIDEO_SOURCE_DEMUX)
#elif HAVE_OST_DMX_H
		if (vstatus.streamSource != VIDEO_SOURCE_DEMUX)
#endif
		{
			if (ioctl(vid, VIDEO_SELECT_SOURCE, (video_stream_source_t)VIDEO_SOURCE_DEMUX) < 0)
				perror ("[zap.cpp]VIDEO_SELECT_SOURCE");
		}

		/* vpid */
		pes_filter.pid     = VPID;
		pes_filter.input   = DMX_IN_FRONTEND;
		pes_filter.output  = DMX_OUT_DECODER;
#ifdef HAVE_LINUX_DVB_VERSION_H
		pes_filter.pes_type = DMX_PES_VIDEO;
#elif HAVE_OST_DMX_H
		pes_filter.pesType = DMX_PES_VIDEO;
#endif
		
		pes_filter.flags   = 0;
		if (ioctl(video,DMX_SET_PES_FILTER,&pes_filter) < 0)
			perror("[zap.cpp]DMX_SET_PES_FILTER video");
		usevideo = true;
	}

	/* pcr */
	if (usepcr)
	{
		pes_filter.pid     = PCR;
		pes_filter.input   = DMX_IN_FRONTEND;
		pes_filter.output  = DMX_OUT_DECODER;
#ifdef HAVE_LINUX_DVB_VERSION_H
		pes_filter.pes_type = DMX_PES_PCR;
#elif HAVE_OST_DMX_H
		pes_filter.pesType = DMX_PES_PCR;
#endif
		pes_filter.flags   = 0;
		if (ioctl(pcr,DMX_SET_PES_FILTER,&pes_filter)< 0 )
			perror("[zap.cpp]DMX_SET_PES_FILTER pcr");
		usepcr = true;
	}

	/* apid */
	if (useaudio)
	{
#ifdef HAVE_LINUX_DVB_VERSION_H
		if (astatus.stream_source != AUDIO_SOURCE_DEMUX)
#elif HAVE_OST_DMX_H
		if (astatus.streamSource != AUDIO_SOURCE_DEMUX)
#endif
		{
			if (ioctl(aud, AUDIO_SELECT_SOURCE, (audio_stream_source_t)AUDIO_SOURCE_DEMUX) < 0)
				perror("[zap.cpp]AUDIO_SELECT_SOURCE");
		}

		pes_filter.pid     = APID;
		pes_filter.input   = DMX_IN_FRONTEND;
		pes_filter.output  = DMX_OUT_DECODER;
#ifdef HAVE_LINUX_DVB_VERSION_H
		pes_filter.pes_type = DMX_PES_AUDIO;
#elif HAVE_OST_DMX_H
		pes_filter.pesType = DMX_PES_AUDIO;
#endif
		pes_filter.flags   = 0;
		if (ioctl(audio,DMX_SET_PES_FILTER,&pes_filter) < 0)
			perror("[zap.cpp]DMX_SET_PES_FILTER audio");
		useaudio = true;
	}
	
	if (usepcr)
	{
		if (ioctl(pcr, DMX_START) < 0)
			perror("[zap.cpp]DMX_START pcr");
	}

	if (useaudio)
	{
		if (ioctl(audio, DMX_START) < 0)
			perror("[zap.cpp]DMX_START audio");
	}

	if (usevideo)
	{	
		if (ioctl(video, DMX_START) < 0)
			perror("[zap.cpp]DMX_START video");
	}

	if (usevideo)
	{	
		if (ioctl(vid, VIDEO_PLAY) < 0)
			perror("[zap.cpp]VIDEO_PLAY");
	}

	if (useaudio)
	{	
		if (ioctl(aud, AUDIO_PLAY) < 0)
			perror("[zap.cpp]AUDIO_PLAY");
	}

	ioctl(audio,AUDIO_SET_BYPASS_MODE, 1);

	printf("Zapping...\n");
	old_TS = TS;
	std::cout << "Zapping done" << std::endl;
}

void zap::zap_audio(int VPID, int APID, int ECM, int SID, int ONID)
{
	struct dmx_pes_filter_params pes_filter;

	int i = AVS_MUTE;
	int avs = open(AVS_DEV, O_RDWR);
	printf("----------------> OPENING AVS <--------------------\n");
	ioctl(avs,AVSIOSMUTE,&i);
	close(avs);
	ioctl(aud, AUDIO_STOP, true);
	ioctl(audio, DMX_STOP, 0);

	printf("Zapp on\nSID: %04x\nVPID: %04x\nAPID: %04x\nECM: %04x\nONID: %04x\n\n", SID, VPID, APID, ECM, ONID);

	/* apid */
	pes_filter.pid     = APID;
	pes_filter.input   = DMX_IN_FRONTEND;
	pes_filter.output  = DMX_OUT_DECODER;
#ifdef HAVE_LINUX_DVB_VERSION_H
	pes_filter.pes_type = DMX_PES_AUDIO;
#elif HAVE_OST_DMX_H
	pes_filter.pesType = DMX_PES_AUDIO;
#endif
	pes_filter.flags   = 0;
	ioctl(audio, DMX_SET_PES_FILTER, &pes_filter);

	ioctl(audio, DMX_START, true);
	ioctl(aud, AUDIO_PLAY, true);
	usleep(300000);
	i = AVS_UNMUTE;
	avs = open(AVS_DEV,O_RDWR);
	printf("----------------> AVS_MUTE <--------------------\n");
	ioctl(avs,AVSIOSMUTE,&i);
	close(avs);
}


void zap::dmx_start()
{
}

void zap::dmx_stop()
{
	ioctl(video,DMX_STOP,0);
	ioctl(audio,DMX_STOP,0);
	ioctl(vid, VIDEO_STOP);
}

void zap::stop()
{
	zap_allstop();

	if (usevideo)
	{
		close(vid);
		close(video);
	}
	if (useaudio)
	{
		close(aud);
		close(audio);
	}
	if (usepcr)
		close(pcr);
}


