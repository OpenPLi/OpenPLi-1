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
$Log: settings.h,v $
Revision 1.6.4.4  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.6.4.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.7  2003/03/08 17:31:18  waldi
use tuxbox and frontend infos

Revision 1.6  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.5  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.4  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.5  2001/12/18 02:03:29  tux
VCR-Switch-Eventkram implementiert

Revision 1.4  2001/12/17 18:37:05  tux
Finales Settingsgedoens

Revision 1.3  2001/12/17 16:54:47  tux
Settings halb komplett

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#include <config.h>

#include "cam.h"

#include <tuxbox.h>
#include <tuxbox/info_dbox2.h>

tuxbox_dbox2_gt_t tuxbox_get_dbox2_gt (void);

struct setting_s
{
	int timeoffset;
	unsigned int ip;
	unsigned int gwip;
	unsigned int dnsip;
	unsigned int serverip;
	bool txtreinsertion;
	bool rcRepeat;
	bool supportOldRc;
	int video_format;
	int output_format;
	bool switch_vcr;
	std::string proxy_server;
	int proxy_port;
	int inversion;
};

class settings
{
	bool isCable;
	bool isGTX;
	int CAID;
	int EMM;
	int find_emmpid(int ca_system_id);
	int box; // 1= nokia 2=sagem
	int oldTS;
	bool usediseqc;
	cam *cam_obj;
	std::string version;
	setting_s setting;
public:
	settings(cam *c);
	void initme();
	bool boxIsCable();
	bool boxIsSat();
	int getCAID();
	int getTransparentColor();
	int getEMMpid(int TS = -1);

	void setIP(char n1, char n2, char n3, char n4);
	char getIP(char number);

	void setgwIP(char n1, char n2, char n3, char n4);
	char getgwIP(char number);

	void setdnsIP(char n1, char n2, char n3, char n4);
	char getdnsIP(char number);

	void setserverIP(char n1, char n2, char n3, char n4);
	char getserverIP(char number);

	void setRcRepeat(bool repeat) { setting.rcRepeat = repeat; }
	bool getRCRepeat() { return setting.rcRepeat; }

	void setSupportOldRc(bool old) { setting.supportOldRc = old; }
	bool getSupportOldRc() { return setting.supportOldRc; }

	void setVideoFormat(int format) { setting.video_format = format; }
	int getVideoFormat() { return setting.video_format; }

	void setOutputFormat(int format) { setting.output_format = format; }
	int getOutputFormat() { return setting.output_format; }

	void setSwitchVCR(bool swit) { setting.switch_vcr = swit; }
	bool getSwitchVCR() { return setting.switch_vcr; }

	void setInversion(int inversion) { setting.inversion = inversion; }
	int getInversion() { return setting.inversion; }

	void setProxyServer(std::string proxy) { setting.proxy_server = proxy; }
	std::string getProxyServer() { return setting.proxy_server; }

	void setProxyPort(int port) { setting.proxy_port = port; }
	int getProxyPort() { return setting.proxy_port; }

	int getBox() { return box; }
	bool boxIsGTX() { return isGTX; }
	void setDiseqc(bool use) { usediseqc = use; }
	bool useDiseqc() { return usediseqc; }

	void setTXTReinsertion(bool insert) { setting.txtreinsertion = insert; }
	bool getTXTReinsertion() { return setting.txtreinsertion; }

	void setVersion(std::string ver) { version = ver; }
	std::string getVersion() { return "LCARS V" + version; }
	std::string getSmallVersion() { return version; }
	void setTimeOffset(int offset) { setting.timeoffset = offset; }
	int getTimeOffset() { return setting.timeoffset; }
	void saveSettings();
	void loadSettings();
};

#endif // ZAP_H
