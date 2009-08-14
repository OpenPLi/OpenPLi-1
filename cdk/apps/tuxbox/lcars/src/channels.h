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
$Log: channels.h,v $
Revision 1.13  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.12  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.11  2002/06/12 23:30:03  TheDOC
basic NVOD should work again

Revision 1.10  2002/06/02 14:23:36  TheDOC
some fixes and changes

Revision 1.9  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.8  2002/05/31 22:33:14  TheDOC
i hate perspectives

Revision 1.7  2002/05/27 12:01:43  TheDOC
linkage-perspectives fix and stuff

Revision 1.6  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.5  2002/03/03 22:57:59  TheDOC
lcars 0.20

Revision 1.3  2001/12/17 01:00:33  tux
scan.cpp fix

Revision 1.2  2001/12/16 22:36:05  tux
IP Eingaben erweitert

Revision 1.4  2001/12/12 15:23:55  TheDOC
Segfault after Scan-Bug fixed

Revision 1.3  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#ifndef CHANNELS_H
#define CHANNELS_H

#include <string>
#include <vector>
#include <map>

#include "tuner.h"
#include "pat.h"
#include "pmt.h"
#include "settings.h"
#include "zap.h"
#include "osd.h"
#include "eit.h"
#include "cam.h"
#include "hardware.h"
#include "variables.h"

enum
{
    CHANNEL, LINKAGE, NVOD
};

struct channel
{
	int channelnumber; // equals the vector-number... don't know, if i need that, yet ;)
	int TS; // Transport-Stream
	int ONID;
	int SID;
	int PMT;
	int VPID;
	std::vector<int> APID;
	bool DD[4];
	int PCR;
	int CAID[5];
	int ECM[5];
	int TXT;
	int EIT;
	int type; // 1->video, 2->radio, 4->NVOD-reference
	int NVOD_count;
	int NVOD_TS[10]; // Nur bei type = 4 - TSID des NVOD-services
	int NVOD_ONID[10]; // Nur bei type = 4 - ONID des NVOD-services
	int NVOD_SID[10]; // Nur bei type = 4 - SID des NVOD-services
	char serviceName[100];
	char providerName[100];
	int number_perspectives;
	linkage perspective[20];
	pmt_data pmt_entry;
};

struct dvbchannel
{
	unsigned char init[4];
	unsigned short SID;
	unsigned short PMT;
	unsigned short FREQU;
	unsigned short SYMBOL;
	unsigned char FEC;
	unsigned char unknown;
	unsigned char POLARIZATION; // 0=V, 1=H
	unsigned char DISEQC;
	unsigned short VPID;
	unsigned short APID;
	unsigned short PCR;
	unsigned short AC3;
	unsigned short ECM;
	unsigned char flags;
	unsigned char type;
	unsigned short TXT;
	unsigned short TS;
	unsigned char serviceName[24];
	unsigned char AutoPIDPMT;
	unsigned char providerIndex;
	unsigned char parental;
	unsigned char countrycode;
	unsigned char linkage;
	unsigned char favourite;
	unsigned short ONID;
};

struct transponder
{
	int ONID;
	int TS;
};

struct transportstream
{
	transponder trans;
	long FREQU;
	int SYMBOL;
	int POLARIZATION; // 0->H, 1->V -- Nur Sat
	int FEC;
	int diseqc;
};

struct ltstr2
{
	bool operator()(transponder s1, transponder s2) const
	{
		if (s1.TS < s2.TS)
			return true;
		else if (s1.TS > s2.TS)
			return false;
		else
			if (s1.ONID < s2.ONID)
				return true;
			else
				return false;
	}
};

class channels
{
	std::vector<struct channel> basic_channellist; // the list of channels
	std::multimap<struct transponder, struct transportstream, ltstr2> basic_TSlist; // the list of transportstreams
	std::multimap<int, int> services_list; // services multimap pointing to basic_channellist-entries
	int cur_pos; // position for getChannel/setChannel
	std::multimap<struct transponder, struct transportstream>::iterator cur_pos_TS;
	settings *setting;
	pat *pat_obj;
	pmt *pmt_obj;
	eit *eit_obj;
	cam *cam_obj;
	osd *osd_obj;
	zap *zap_obj;
	variables *vars;
	tuner *tuner_obj;
	hardware *hardware_obj;
	event now, next;
	char audio_description[20];
	int ECM, apid;
	int video_component, component[10], number_components;
	int curr_perspective;
	int current_mode;
	int old_TS, old_ONID;
	std::queue<int> last_channels;
	std::vector<linkage> linkage_perspectives;
	linkage tmp_link;
	pmt_data NVOD_pmt;
	pmt_data linkage_pmt;
public:
	channels(settings *setting, pat *p1, pmt *p2, eit *e, cam *c, hardware *h, osd *o, zap *z, tuner *t, variables *v);
	channels(settings *setting, pat *p1, pmt *p2);

	void setStuff(eit *e, cam *c, hardware *h, osd *o, zap *z, tuner *t, variables *v);
	void setTuner(tuner *t);

	// multiperspective-stuff

	bool currentIsMultiPerspective();
	int currentNumberPerspectives();
	void parsePerspectives();
	void setPerspective(int number);
	std::string getPerspectiveName(int number);

	// end multiperspective-stuff

	void zapCurrentChannel();
	void zapLastChannel();
	void setCurrentOSDProgramInfo();
	void receiveCurrentEIT();
	void setCurrentOSDProgramEIT();
	void setCurrentOSDEvent();
	void updateCurrentOSDProgramEIT();
	void zapCurrentAudio(int apid);
	int getCurrentAudio() { return apid; }
	void updateCurrentOSDProgramAPIDDescr();

	event getCurrentNow() { return now; }
	event getCurrentNext() { return next; }

	void clearChannels() { basic_channellist.clear(); basic_TSlist.clear(); }

	void addChannel(); // Adds a channel at the end and sets current position to new channel
	void addChannel(channel new_channel);
	void addDVBChannel(dvbchannel tmp_channel);

	void setCurrentTS(int TS);
	void setCurrentPMTdata(pmt_data pmt);
	void setCurrentONID(int ONID);
	void setCurrentSID(int SID);
	void setCurrentPMT(int PMT);
	void setCurrentVPID(int VPID);
	void addCurrentAPID(int APID, int number = -1);
	void addCurrentAPID(int APID, bool DD);
	void deleteCurrentAPIDs();
	void setCurrentPCR(int PCR);
	void setCurrentTXT(int TXT);
	void addCurrentCA(int CAID, int ECM, int number = -1);
	void setCurrentEIT(int EIT);
	void setCurrentType(int type);
	void clearCurrentNVODs() { basic_channellist[cur_pos].NVOD_count = 0; }
	void addCurrentNVOD(int NVOD_TS, int NVOD_ONID, int NVOD_SID, int number = -1);
	void setCurrentNVODCount(int count);
	void setCurrentServiceName(std::string serviceName);
	void setCurrentProviderName(std::string serviceName);

	int numberChannels() { return basic_channellist.size(); } // Returns number of Channels
	int numberTransponders() { return basic_TSlist.size(); }
	bool setCurrentChannel(int channelnumber); // false if setChannel failed
	void setCurrentChannelViaSID(int SID);
	int getCurrentChannelNumber() { return cur_pos; } // returns the currently set channelnumber / -1 if not set

	int getChannelNumber(int TS, int ONID, int SID);
	channel getChannelByNumber(int number);
	void updateChannel(int number, channel channel_data);

	dvbchannel getDVBChannel(int number);

	std::string getServiceName(int channelnumber);
	std::string getShortServiceName(int channelnumber);

	int getCurrentTS();
	pmt_data getCurrentPMTdata();
	int getCurrentONID();
	int getCurrentSID();
	int getCurrentPMT();
	int getCurrentVPID();
	int getCurrentAPIDcount();
	int getCurrentAPID(int number);
	int getCurrentAPID();
	bool getCurrentDD(int number = 0);
	int getCurrentPCR();
	int getCurrentCAcount();
	int getCurrentCAID(int number = 0);
	int getCurrentECM(int number = 0);
	int getCurrentEIT();
	int getCurrentTXT();
	int getCurrentType();
	int getType(int number) { return basic_channellist[number].type; };
	int getCurrentNVODcount();
	int getCurrentNVOD_TS(int number);
	int getCurrentNVOD_ONID(int number);
	int getCurrentNVOD_SID(int number);
	std::string getCurrentServiceName();
	std::string getCurrentProviderName();

	bool addTS(int TS, int ONID, int FREQU, int SYMBOL, int POLARIZATION = -1, int FEC = -1, int diseqc = 1);

	int getFrequency(int TS, int ONID);
	int getSymbolrate(int TS, int ONID);
	int getPolarization(int TS, int ONID);
	int getFEC(int TS, int ONID);
	int getDiseqc(int TS, int ONID);
	transportstream getTS(int TS, int ONID);

	bool tune(int TS, int ONID);
	bool tuneCurrentTS();

	void setBeginTS() { cur_pos_TS = basic_TSlist.begin(); }
	bool setNextTS();
	void clearTS() { basic_TSlist.clear(); }
	int getCurrentSelectedTS() { return (*cur_pos_TS).second.trans.TS; }
	int getCurrentSelectedONID() { return (*cur_pos_TS).second.trans.ONID; }
	int getCurrentFrequency() { return (*cur_pos_TS).second.FREQU; }
	int getCurrentSymbolrate() { return (*cur_pos_TS).second.SYMBOL; }
	int getCurrentPolarization() { return (*cur_pos_TS).second.POLARIZATION; }
	int getCurrentFEC() { return (*cur_pos_TS).second.FEC; }
	int getCurrentDiseqc() { return (*cur_pos_TS).second.diseqc; }


	void dumpTS();
	void dumpChannels();

	void saveDVBChannels();
	void loadDVBChannels();

	void saveTS();
	void loadTS();
};

#endif
