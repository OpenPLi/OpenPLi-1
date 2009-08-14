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
$Log: zap.h,v $
Revision 1.8.6.1  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.8  2003/01/26 00:00:20  thedoc
mv bugs /dev/null

Revision 1.7  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.6  2003/01/05 06:49:59  TheDOC
lcars should work now with the new drivers more properly

Revision 1.5  2002/06/08 20:21:09  TheDOC
adding the cam-sources with slight interface-changes

Revision 1.4  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.3  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef ZAP_H
#define ZAP_H

#include "devices.h"
#include "settings.h"
#include "osd.h"
#include "tuner.h"
#include "cam.h"
#include "lcd.h"
//#include "control.h"

class zap
{
	int vid, aud, frontend, video, audio, pcr;
	int old_frequ;
	settings setting;
	osd osdd;
	tuner tune;
	cam ca;
//	int lcd;
	int old_TS;
	bool usevideo, useaudio, usepcr;
	audio_status astatus;
	video_status vstatus;
//	lcddisplay *lcd_obj;
public:
	zap(settings &set, osd &o, tuner &t, cam &c/*, lcddisplay &l*/);
	~zap();

	void zap_allstop();
	void stop();
	void zap_to(pmt_data pmt, int VPID, int APID, int PCR, int ECM, int SID, int ONID, int TS, int PID1 = -1, int PID2 = -1);
	void zap_audio(int VPID, int APID, int ECM, int SID, int ONID);
	void close_dev();
	void dmx_start();
	void dmx_stop();
};
#endif // ZAP_H

