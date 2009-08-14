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
$Log: channels.cpp,v $
Revision 1.22.6.5  2008/08/09 16:41:51  fergy
Cleaning code
Enabled some debug stuff
Enabled some disabled features

Revision 1.22.6.4  2008/08/07 20:25:30  fergy
Mostly clear of not needed lines
Added back debug messages ( just for dev. )
Enambled some disabled stuff from before
Code cleaning

Revision 1.22.6.3  2008/08/07 17:56:43  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.22  2003/01/26 00:00:19  thedoc
mv bugs /dev/null

Revision 1.21  2003/01/05 06:49:59  TheDOC
lcars should work now with the new drivers more properly

Revision 1.20  2002/11/26 20:03:14  TheDOC
some debug-output and small fixes

Revision 1.19  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.18  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.17  2002/06/13 01:35:48  TheDOC
NVOD should work now

Revision 1.16  2002/06/12 23:30:03  TheDOC
basic NVOD should work again

Revision 1.15  2002/06/12 17:46:53  TheDOC
reinsertion readded

Revision 1.14  2002/06/08 20:21:09  TheDOC
adding the cam-sources with slight interface-changes

Revision 1.13  2002/06/08 15:11:47  TheDOC
autostart in yadd added

Revision 1.12  2002/06/02 14:23:36  TheDOC
some fixes and changes

Revision 1.11  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.10  2002/05/31 22:33:14  TheDOC
i hate perspectives

Revision 1.9  2002/05/27 12:00:32  TheDOC
linkage-perspectives fix and stuff

Revision 1.8  2002/05/18 04:31:02  TheDOC
Warningelimination

Revision 1.7  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.6  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.3  2001/12/17 01:00:41  tux
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
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "channels.h"
#include "zap.h"
#include "help.h"
#include "tuner.h"
#include "settings.h"
#include "zap.h"
#include "tuner.h"
#include "pmt.h"
#include "pat.h"
#include "eit.h"

#include <config.h>

channels::channels(settings *set, pat *p1, pmt *p2)
{
	cur_pos = -1;
	pat_obj = p1;
	pmt_obj = p2;
	setting = set;
	current_mode = CHANNEL;
	linkage_perspectives.clear();
}


channels::channels(settings *set, pat *p1, pmt *p2, eit *e, cam *c, hardware *h, osd *o, zap *z, tuner *t, variables *v)
{
	setting = set;
	cur_pos = -1;
	pat_obj = p1;
	pmt_obj = p2;
	eit_obj = e;
	cam_obj = c;
	hardware_obj = h;
	osd_obj = o;
	zap_obj = z;
	tuner_obj = t;
	vars = v;
	linkage_perspectives.clear();
}

void channels::setStuff(eit *e, cam *c, hardware *h, osd*o, zap *z, tuner *t, variables *v)
{
	eit_obj = e;
	cam_obj = c;
	hardware_obj = h;
	osd_obj = o;
	zap_obj = z;
	tuner_obj = t;
	vars = v;
}

void channels::setTuner(tuner *t)
{
	tuner_obj = t;
}

bool channels::currentIsMultiPerspective()
{
	return eit_obj->isMultiPerspective();
}

int channels::currentNumberPerspectives()
{
	if (linkage_perspectives.size() == 0)
	{
		if (getCurrentNVODcount() > 0)
		{
			return getCurrentNVODcount();
		}
	}
	else
		return linkage_perspectives.size();

	return 0;
}

void channels::parsePerspectives()
{
	linkage_perspectives.clear();
	int num = eit_obj->numberPerspectives();
	for (int i = 0; i < num; i++)
	{
		basic_channellist[cur_pos].perspective[i] = eit_obj->nextLinkage();
		linkage_perspectives.insert(linkage_perspectives.end(), eit_obj->nextLinkage());
	}
	curr_perspective = 0;
	old_TS = -1;
	osd_obj->createPerspective();
	char message[100];
	sprintf(message, "Please choose perspective (%d - %d)", 1, basic_channellist[cur_pos].number_perspectives);
	osd_obj->setPerspectiveName(message);
	osd_obj->addCommand("SHOW perspective");
}

void channels::setPerspective(int number)
{
	if (linkage_perspectives.size() > 0)
	{
		if ((unsigned int) number >= linkage_perspectives.size())
			return;	
		current_mode = LINKAGE;
	}
	else if (getCurrentNVODcount() > 0)
	{
		if (number >= getCurrentNVODcount())
		return;
		current_mode = NVOD;
	}
	
	pmt_data pmt_entry;

	zap_obj->stop();
	curr_perspective = number;
	if (current_mode == LINKAGE)
	{
		tmp_link = linkage_perspectives[number];

		if (old_TS != tmp_link.TS || old_ONID != tmp_link.ONID)
		{
			tune(tmp_link.TS, tmp_link.ONID);
		}
		old_ONID = tmp_link.ONID;
		old_TS = tmp_link.TS;
	
		zap_obj->close_dev();
		pat_obj->readPAT();
		ECM = 0;
	
		memset (&pmt_entry, 0, sizeof (struct pmt_data));
		tmp_link.PMT = pat_obj->getPMT(tmp_link.SID);
		pmt_entry = pmt_obj->readPMT(tmp_link.PMT);
		linkage_pmt = pmt_entry;
		tmp_link.APIDcount = 0;
		tmp_link.PCR = pmt_entry.PCR;
		for (int i = 0; i < pmt_entry.pid_counter; i++)
		{
			if (pmt_entry.type[i] == 0x02)
				tmp_link.VPID = pmt_entry.PID[i];
			else if (pmt_entry.type[i] == 0x04 || pmt_entry.type[i] == 0x03 || pmt_entry.type[i] == 0x06)
			{
				tmp_link.APID[tmp_link.APIDcount++] = pmt_entry.PID[i];
			}
		}
	
	
		for (int i = 0; i < pmt_entry.ecm_counter; i++)
		{
			if (setting->getCAID() == pmt_entry.CAID[i])
				ECM = pmt_entry.ECM[i];
		}
			osd_obj->addCommand("HIDE perspective");
			osd_obj->createPerspective();
			osd_obj->setPerspectiveName(tmp_link.name);
			osd_obj->addCommand("SHOW perspective");
		if (tmp_link.APIDcount == 1)
			zap_obj->zap_to(pmt_entry, tmp_link.VPID, tmp_link.APID[apid], tmp_link.PCR, ECM, tmp_link.SID, tmp_link.ONID, tmp_link.TS);
		else
			zap_obj->zap_to(pmt_entry, tmp_link.VPID, tmp_link.APID[0], tmp_link.PCR, ECM, tmp_link.SID, tmp_link.ONID, tmp_link.TS, tmp_link.APID[1]);
	}
	else if (current_mode == NVOD)	
	{
		int TS = getCurrentNVOD_TS(number);
		int ONID = getCurrentNVOD_ONID(number);
		int SID = getCurrentNVOD_SID(number);
		if (old_TS != TS || old_ONID != ONID)
		{
			std::cout << "New TS selected" << std::endl;
			std::cout << "The new TS is: " << tmp_link.TS << std::endl;
			tune(TS, ONID);
		}
		old_ONID = ONID;
		old_TS = TS;
	
		zap_obj->close_dev();
		pat_obj->readPAT();
		ECM = 0;
	
		memset (&NVOD_pmt, 0, sizeof (struct pmt_data));

		int PMT = pat_obj->getPMT(SID);
		if (PMT < 1)
			return;
		NVOD_pmt = pmt_obj->readPMT(PMT);

		int APIDcount = 0;
		int PCR = NVOD_pmt.PCR;
		int VPID = 0;
		std::vector<int> APID;

		for (int i = 0; i < NVOD_pmt.pid_counter; i++)
		{
			if (NVOD_pmt.type[i] == 0x02)
				VPID = NVOD_pmt.PID[i];
			else if (NVOD_pmt.type[i] == 0x04 || NVOD_pmt.type[i] == 0x03 || NVOD_pmt.type[i] == 0x06)
			{
				printf("an APID: %04x\n", pmt_entry.PID[i]);
				APID.insert(APID.end(), NVOD_pmt.PID[i]);
				APIDcount++;
			}
			printf("type: %d - PID: %04x\n", pmt_entry.type[i], pmt_entry.PID[i]);
		}
	
		printf("ECMs: %d\n", pmt_entry.ecm_counter);
		for (int i = 0; i < NVOD_pmt.ecm_counter; i++)
		{
			if (setting->getCAID() == NVOD_pmt.CAID[i])
				ECM = NVOD_pmt.ECM[i];
			printf("CAID: %04x - ECM: %04x\n", pmt_entry.CAID[i], pmt_entry.ECM[i]);
		}
			osd_obj->addCommand("HIDE perspective");
			osd_obj->createPerspective();
			osd_obj->setPerspectiveName(tmp_link.name);
			osd_obj->addCommand("SHOW perspective");
		printf("%s\n", tmp_link.name);
		apid = 0;
		if (APIDcount == 1)
			zap_obj->zap_to(NVOD_pmt, VPID, APID[apid], PCR, ECM, SID, ONID, TS);
		else
			zap_obj->zap_to(NVOD_pmt, VPID, APID[0], PCR, ECM, SID, ONID, TS, APID[1]);
	}

}

std::string channels::getPerspectiveName(int number)
{
	if (current_mode == LINKAGE)
		return linkage_perspectives[number].name;
	else if (current_mode == NVOD)
		return "NVOD";
	
	return "N/A";
}

void channels::zapCurrentChannel()
{
	zap_obj->zap_allstop();

	linkage_perspectives.clear();

	current_mode = CHANNEL;

	zap_obj->stop();

	if (tune(getCurrentTS(), getCurrentONID()))
	{

		fprintf(stderr, "Getting pat\n");
		pat_obj->readPAT();
		fprintf(stderr, "Got it\n");
	
		ECM = 0;
	
		apid = 0;
	
		int tmp_pmt = pat_obj->getPMT(getCurrentSID());
	
		if (tmp_pmt != 0)
		{
			setCurrentPMT(pat_obj->getPMT(getCurrentSID()));
			
			fprintf(stderr, "Getting pmt\n");
			pmt_data pmt_entry = (pmt_obj->readPMT(getCurrentPMT()));
			fprintf(stderr, "Got it\n");
			
			setCurrentPMTdata(pmt_entry);
			deleteCurrentAPIDs();
			number_components = 0;
			video_component = 0;
			for (int i = 0; i < pmt_entry.pid_counter; i++)
			{
				if (pmt_entry.type[i] == 0x02)
				{
					setCurrentVPID(pmt_entry.PID[i]);
					video_component = pmt_entry.component[i];
				}
				else if (pmt_entry.type[i] == 0x04 || pmt_entry.type[i] == 0x03)
				{
					addCurrentAPID(pmt_entry.PID[i]);
					component[number_components++] = pmt_entry.component[i];
				}
				else if (pmt_entry.type[i] == 0x06 && pmt_entry.subtype[i] == 1)
				{
					setCurrentTXT(pmt_entry.PID[i]);
				}
				else if (pmt_entry.type[i] == 0x06 && pmt_entry.subtype[i] != 1)
				{
					addCurrentAPID(pmt_entry.PID[i], (bool) true);
					component[number_components++] = pmt_entry.component[i];
				}
			
				printf("type: %d - PID: %04x\n", pmt_entry.type[i], pmt_entry.PID[i]);
			}
		
				for (int i = 0; i < pmt_entry.ecm_counter; i++)
			{
				if (setting->getCAID() == pmt_entry.CAID[i])
					ECM = pmt_entry.ECM[i];
				printf("CAID: %04x - ECM: %04x\n", pmt_entry.CAID[i], pmt_entry.ECM[i]);
			}
			basic_channellist[cur_pos].PCR = pmt_entry.PCR;
		
			hardware_obj->useDD(getCurrentDD(0));
			if (getCurrentAPIDcount() == 1)
				(*zap_obj).zap_to(pmt_entry, getCurrentVPID(), getCurrentAPID(0), getCurrentPCR(), ECM, getCurrentSID(), getCurrentONID(), getCurrentTS());
			else
				(*zap_obj).zap_to(pmt_entry, getCurrentVPID(), getCurrentAPID(0), getCurrentPCR(), ECM, getCurrentSID(), getCurrentONID(), getCurrentTS(), getCurrentAPID(1));
			
			
			if (getCurrentAPIDcount() > 1)
				(*eit_obj).setAudioComponent(component[apid]);
			else
				(*eit_obj).setAudioComponent(-1);
			}
	
		if (getCurrentType() == 4)
		{
			vars->setvalue("%IS_NVOD", "true");
			osd_obj->setNVODAvailable(true);
		}
		else 
		{
			vars->setvalue("%IS_NVOD", "false");
			osd_obj->setNVODAvailable(false);
		}

		last_channels.push(cur_pos);
		int number_last_chans = 2;
		if (vars->isavailable("%NUMBERLASTCHANNELS"))
		{
			number_last_chans = atoi(vars->getvalue("%NUMBERLASTCHANNELS").c_str());
		}
		if (last_channels.size() > (unsigned int) number_last_chans)
		{
			last_channels.pop();
		}
	}
}

void channels::zapLastChannel()
{
	int numb = last_channels.front();
	last_channels.pop();
	setCurrentChannel(numb);
}

void channels::setCurrentOSDProgramInfo()
{
	osd_obj->createProgramInfo();
	
	char text[100];
	sprintf(text, "COMMAND proginfo set_service_name %s", getCurrentServiceName().c_str());
	osd_obj->addCommand(text);
	sprintf(text, "COMMAND proginfo set_service_number %d", cur_pos);
	osd_obj->addCommand(text);
}

void channels::receiveCurrentEIT()
{
	memset (&now, 0, sizeof (struct event));
	memset (&next, 0, sizeof (struct event));

	char cmd_text[100];
	eit_obj->gotNow = false;
	sprintf(cmd_text, "RECEIVE %d", getCurrentSID());
	(*eit_obj).addCommand(cmd_text);
}

void channels::setCurrentOSDProgramEIT()
{
	if (getCurrentAPIDcount() > 1)
		(*osd_obj).setLanguage(audio_description);
	if (now.par_rating != 0)
		(*osd_obj).setParentalRating(now.par_rating);

	(*osd_obj).setNowDescription(now.event_name);
	(*osd_obj).setNextDescription(next.event_name);
	(*osd_obj).setNowTime(now.starttime);
	(*osd_obj).setNextTime(next.starttime);
}

void channels::updateCurrentOSDProgramEIT()
{
	if (next.starttime <= time(0))
	{
		getCurrentEIT();
		setCurrentOSDProgramEIT();
	}
}

void channels::setCurrentOSDEvent()
{
	(*osd_obj).setEPGEventName(now.event_name);
	(*osd_obj).setEPGEventShortText(now.event_short_text);
	(*osd_obj).setEPGEventExtendedText(now.event_extended_text);
	(*osd_obj).setEPGProgramName(getCurrentServiceName());
	(*osd_obj).setEPGstarttime(now.starttime);
	(*osd_obj).setEPGduration(now.duration);
}

void channels::zapCurrentAudio(int pid)
{
	apid = pid;

	hardware_obj->useDD(true);

	if (current_mode == CHANNEL)
	{
		if (basic_channellist[cur_pos].DD[pid])
		printf("Dolby Digital ON ......................................\n");
		else
		printf("Dolby Digital OFF ......................................\n");
		hardware_obj->useDD(getCurrentDD(apid));
		zap_obj->zap_audio(getCurrentVPID(), getCurrentAPID(apid), ECM, getCurrentSID(), getCurrentONID());

		eit_obj->setAudioComponent(component[apid]);
	}
	else if (current_mode == LINKAGE)
	{
		zap_obj->zap_audio(tmp_link.VPID, tmp_link.APID[apid] , ECM, tmp_link.SID, tmp_link.ONID);
		zap_obj->zap_audio(getCurrentVPID(), getCurrentAPID(apid), ECM, getCurrentSID(), getCurrentONID());

		event now = eit_obj->getNow();
				for (int i = 0; i < now.number_components; i++)
				{
					if (now.component_tag[i] == component[apid])
					{
						strcpy(audio_description, now.audio_description[i]);
					}
				}
	}
	else if (current_mode == NVOD)
	{
		int VPID = 0;
		std::vector<int> APID;

		for (int i = 0; i < NVOD_pmt.pid_counter; i++)
		{
			if (NVOD_pmt.type[i] == 0x02)
				VPID = NVOD_pmt.PID[i];
			else if (NVOD_pmt.type[i] == 0x04 || NVOD_pmt.type[i] == 0x03 || NVOD_pmt.type[i] == 0x06)
			{
				APID.insert(APID.end(), NVOD_pmt.PID[i]);
			}
		}
		zap_obj->zap_audio(VPID, APID[apid] , ECM, getCurrentNVOD_SID(curr_perspective), getCurrentNVOD_ONID(curr_perspective));
	}


}

void channels::updateCurrentOSDProgramAPIDDescr()
{
	if (getCurrentAPIDcount() > 1)
		(*osd_obj).setLanguage(audio_description);
}

dvbchannel channels::getDVBChannel(int number)
{
	channel tmp_chan = basic_channellist[number];
	dvbchannel chan;

	memset (&chan, 0, sizeof(struct dvbchannel));

	transponder trans;
	trans.TS = tmp_chan.TS;
	trans.ONID = tmp_chan.ONID;
	std::multimap<struct transponder, struct transportstream>::iterator ts;
	// = basic_TSlist.find(trans);

	chan.init[0] = 'D';
	chan.init[1] = 'V';
	chan.init[2] = 'S';
	chan.init[3] = 'O';
	chan.SID = tmp_chan.SID;
	chan.PMT = tmp_chan.PMT;
	chan.FREQU = (*ts).second.FREQU;
	chan.SYMBOL = (*ts).second.SYMBOL;
	chan.POLARIZATION = (*ts).second.POLARIZATION;
	chan.VPID = tmp_chan.VPID;
	chan.APID = tmp_chan.APID[0];
	chan.PCR = tmp_chan.PCR;
	chan.ECM = tmp_chan.ECM[0];
	chan.type = tmp_chan.type;
	chan.TS = tmp_chan.TS;
	for (int i = 0; i < 24; i++)
		chan.serviceName[i] = tmp_chan.serviceName[i];
	chan.AutoPIDPMT = 3;
	chan.ONID = tmp_chan.ONID;

	return chan;

}

void channels::addChannel()
{
	struct channel new_channel;
	printf("New Channel number %d\n", numberChannels());
	memset (&new_channel, 0, sizeof(struct channel));

	new_channel.channelnumber = numberChannels();
	basic_channellist.insert(basic_channellist.end(), new_channel);
	cur_pos = numberChannels() - 1;
}

void channels::addChannel(channel new_channel)
{
	new_channel.channelnumber = numberChannels();
	basic_channellist.insert(basic_channellist.end(), new_channel);
	services_list.insert(std::pair<int, int>(new_channel.SID, basic_channellist.size() - 1));
	cur_pos = numberChannels() - 1;
}

void channels::addDVBChannel(dvbchannel chan)
{
	struct channel tmp_chan;
	memset (&tmp_chan, 0, sizeof(struct channel));
	tmp_chan.APID.clear();

	tmp_chan.SID = chan.SID;
	tmp_chan.PMT = chan.PMT;
	tmp_chan.VPID = chan.VPID;
	tmp_chan.APID[0] = chan.APID;
	tmp_chan.PCR = chan.PCR;
	tmp_chan.ECM[0] = chan.ECM;
	tmp_chan.CAID[0] = setting->getCAID();
	tmp_chan.type = chan.type;
	tmp_chan.TS = chan.TS;
	for (int i = 0; i < 24; i++)
		tmp_chan.serviceName[i] = chan.serviceName[i];
	tmp_chan.ONID = chan.ONID;

	basic_channellist.insert(basic_channellist.end(), tmp_chan);
	services_list.insert(std::pair<int, int>(chan.SID, basic_channellist.size() - 1));

	struct transportstream tmp_TS;
	memset (&tmp_TS, 0, sizeof(struct transportstream));

	tmp_TS.trans.TS = chan.TS;
	tmp_TS.trans.ONID = chan.ONID;
	tmp_TS.FREQU = chan.FREQU;
	tmp_TS.SYMBOL = chan.SYMBOL;
	tmp_TS.POLARIZATION = chan.POLARIZATION & 0x1;
	tmp_TS.FEC = chan.FEC;

	transponder trans;
	trans.TS = chan.TS;
	trans.ONID = chan.ONID;
	if (basic_TSlist.count(trans) == 0)
	{
		basic_TSlist.insert(std::pair<struct transponder, struct transportstream>(trans, tmp_TS));
	}


}

channel channels::getChannelByNumber(int number)
{
	return basic_channellist[number];
}

void channels::updateChannel(int number, channel channel_data)
{
	basic_channellist[number] = channel_data;
}

int channels::getChannelNumber(int TS, int ONID, int SID)
{
	printf("Wanted: TS: %x\n ONID: %x\n SID: %x\n", TS, ONID, SID);


	printf ("Found: %d\n",  services_list.count(SID));

	std::pair<std::multimap<int, int>::iterator, std::multimap<int, int>::iterator> ip = services_list.equal_range(SID);
	for (std::multimap<int, int>::iterator it = ip.first; it != ip.second; ++it)
	{
		int pos = (*it).second;
		setCurrentChannel(pos);
		printf("Checking Position %d\n TS: %x\n ONID: %x\n", pos, getCurrentTS(), getCurrentONID());
		if (getCurrentTS() == TS && getCurrentONID() == ONID)
			return pos;

	}

	return -1;
}

bool channels::setCurrentChannel(int channelnumber)
{
	printf("SetCurrentChannel to %d\n", channelnumber);
	if ((channelnumber > numberChannels() - 1) || (channelnumber < 0))
		return false;
	cur_pos = channelnumber;
	transponder trans;
	trans.TS = basic_channellist[channelnumber].TS;
	trans.ONID = basic_channellist[channelnumber].ONID;
	cur_pos_TS = basic_TSlist.find(trans);
	return true;
}

void channels::setCurrentChannelViaSID(int SID)
{
	cur_pos = (*services_list.find(SID)).second;
}

void channels::setCurrentTS(int TS)
{
	printf("setCurrentTS to %d\n", TS);
	basic_channellist[cur_pos].TS = TS;
}

void channels::setCurrentPMTdata(pmt_data pmt)
{
	basic_channellist[cur_pos].pmt_entry = pmt;
}

void channels::setCurrentONID(int ONID)
{
	printf("setCurrentONID to %d\n", ONID);
	basic_channellist[cur_pos].ONID = ONID;
}

void channels::setCurrentSID(int SID)
{
	printf("setCurrentSID to %d\n", SID);
	basic_channellist[cur_pos].SID = SID;
	services_list.insert(std::pair<int, int>(SID, cur_pos));
}

void channels::setCurrentPMT(int PMT)
{
	printf("setCurrentPMT to %d\n", PMT);
	basic_channellist[cur_pos].PMT = PMT;
}

void channels::setCurrentVPID(int VPID)
{
	printf("setCurrentVPID to %d \n", VPID);
	basic_channellist[cur_pos].VPID = VPID;
	printf("end setCurrentVPID to %d \n", VPID);
}

void channels::addCurrentAPID(int APID, int number)
{
	if (number == -1)
		number = getCurrentAPIDcount();
	printf("addCurrentAPID %d to %04x\n", number, APID);
	basic_channellist[cur_pos].APID.insert(basic_channellist[cur_pos].APID.end(), APID);// = APID;
	basic_channellist[cur_pos].DD[number] = false;
}

void channels::addCurrentAPID(int APID, bool DD)
{
	addCurrentAPID(APID);
	basic_channellist[cur_pos].DD[getCurrentAPIDcount() - 1] = DD;
}

bool channels::getCurrentDD(int number)
{
	return basic_channellist[cur_pos].DD[number];
}

void channels::setCurrentPCR(int PCR)
{
	basic_channellist[cur_pos].PCR = PCR;
}

void channels::setCurrentTXT(int TXT)
{
	basic_channellist[cur_pos].TXT = TXT;
}

void channels::addCurrentCA(int CAID, int ECM, int number)
{
	printf("addCurrentCA to %d - %d\n", CAID, ECM);
	if (number == -1)
		number = getCurrentCAcount();
	basic_channellist[cur_pos].CAID[number] = CAID;
	basic_channellist[cur_pos].ECM[number] = ECM;
}

void channels::setCurrentEIT(int EIT)
{
	basic_channellist[cur_pos].EIT = EIT;
}

void channels::setCurrentType(int type)
{
	printf("setCurrentType to %d \n", type);
	basic_channellist[cur_pos].type = type;
}

void channels::addCurrentNVOD(int NVOD_TS, int NVOD_ONID, int NVOD_SID, int number)
{
	printf("addCurrentNVOD to %d - %d\n", NVOD_TS, NVOD_SID);
	if (number == -1)
		number = basic_channellist[cur_pos].NVOD_count;
	basic_channellist[cur_pos].NVOD_TS[number] = NVOD_TS;
	basic_channellist[cur_pos].NVOD_SID[number] = NVOD_SID;
	basic_channellist[cur_pos].NVOD_ONID[number] = NVOD_ONID;
}

void channels::setCurrentNVODCount(int count)
{
	printf("setCurrentNVODCount to %d\n", count);
	basic_channellist[cur_pos].NVOD_count = count;
}


void channels::setCurrentServiceName(std::string serviceName)
{
	printf("setCurrentServiceName\n");
	strcpy(basic_channellist[cur_pos].serviceName, serviceName.c_str());
}

void channels::setCurrentProviderName(std::string providerName)
{
	printf("setCurrentProviderName\n");
	strcpy(basic_channellist[cur_pos].providerName, providerName.c_str());
}

std::string channels::getServiceName(int channelnumber)
{
	if (channelnumber < 0 || channelnumber >= numberChannels())
		return "";
	int position = 0;
	char name[100];
	for (int i = 0; i < (int)strlen(basic_channellist[channelnumber].serviceName); i++)
	{
		if (basic_channellist[channelnumber].serviceName[i] != 135 && basic_channellist[channelnumber].serviceName[i] != 134)
			name[position++] = basic_channellist[channelnumber].serviceName[i];
	}
	name[position] = '\0';
	return name;
}

std::string channels::getShortServiceName(int channelnumber)
{
	int position = 0;
	char name[100];
	bool started = false;
	for (int i = 0; i < (int)strlen(basic_channellist[channelnumber].serviceName); i++)
	{
		if (started == true)
		{
			if (basic_channellist[channelnumber].serviceName[i] != 135)
				name[position++] = basic_channellist[channelnumber].serviceName[i];
			else
				started = false;
		}
		else if (basic_channellist[channelnumber].serviceName[i] == 134)
			started = true;
	}
	name[position] = '\0';

	if (position == 0)
		strcpy(name, basic_channellist[channelnumber].serviceName);

	// remove ugly characters:
	char clean_name[100];
	position = 0;
	for (int i = 0; i <= (int)strlen(name); i++)
	{
		if (name[i] != 5 && name[i] != 135 && name[i] != 134)
		{
			clean_name[position++] = name[i];
		}

	}

	return clean_name;
}

int channels::getCurrentTS()
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].TS;
	else if (current_mode == LINKAGE)
		return tmp_link.TS;
	else if (current_mode == NVOD)
		return getCurrentNVOD_TS(curr_perspective);
	else
		return 0;
}

pmt_data channels::getCurrentPMTdata()
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].pmt_entry;
	else if (current_mode == LINKAGE)
		return linkage_pmt;
	else if (current_mode == NVOD)
		return NVOD_pmt;
	pmt_data null_data;
	return null_data;
}

int channels::getCurrentONID()
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].ONID;
	else if (current_mode == LINKAGE)
		return tmp_link.ONID;
	else if (current_mode == NVOD)
		return getCurrentNVOD_ONID(curr_perspective);
	return 0;
}

int channels::getCurrentSID()
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].SID;
	else if (current_mode == LINKAGE)
		return tmp_link.SID;
	else if (current_mode == NVOD)
		return getCurrentNVOD_SID(curr_perspective);
	else
		return 0;

}

int channels::getCurrentPMT()
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].PMT;
	else if (current_mode == LINKAGE)
		return tmp_link.PMT;
	else if (current_mode == NVOD)
		return NVOD_pmt.pmt;
	else
		return 0;
}

int channels::getCurrentVPID()
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].VPID;
	else if (current_mode == LINKAGE)
		return tmp_link.VPID;
	else if (current_mode == NVOD)
	{
		for (int i = 0; i < NVOD_pmt.pid_counter; i++)
			if (NVOD_pmt.type[i] == 0x02)
				return NVOD_pmt.PID[i];
	}
	return 0;
}

int channels::getCurrentAPIDcount()
{
	int count = 0;
	if (current_mode == CHANNEL)
	{
		//std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++APID-count: " << basic_channellist[cur_pos].APID.size() << std::endl;
		count = basic_channellist[cur_pos].APID.size();
	}
	else if (current_mode == LINKAGE)
	{
		count = tmp_link.APIDcount;
	}
	else if (current_mode == NVOD)
	{
		for (int i = 0; i < NVOD_pmt.pid_counter; i++)
			if (NVOD_pmt.type[i] == 0x04 || NVOD_pmt.type[i] == 0x03 || NVOD_pmt.type[i] == 0x06)
				count++;
	}
	return count;
}

void channels::deleteCurrentAPIDs()
{
	basic_channellist[cur_pos].APID.clear();

}

int channels::getCurrentAPID(int number)
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].APID[number];
	else if (current_mode == LINKAGE)
		return tmp_link.APID[number];
	else if (current_mode == NVOD)
	{
		int count = 0;
		for (int i = 0; i < NVOD_pmt.pid_counter; i++)
			if (NVOD_pmt.type[i] == 0x04 || NVOD_pmt.type[i] == 0x03 || NVOD_pmt.type[i] == 0x06)
			{
				if (count == number)
					return NVOD_pmt.PID[i];
				count++;
			}
	}
	else
		return 0;
	return 0;
}

int channels::getCurrentAPID()
{
	return getCurrentAPID(apid);
}

int channels::getCurrentPCR()
{
	if (current_mode == CHANNEL)
		return basic_channellist[cur_pos].PCR;
	else if (current_mode == LINKAGE)
		return tmp_link.PCR;
	else
		return 0;
}

int channels::getCurrentTXT()
{
	return basic_channellist[cur_pos].TXT;
}

int channels::getCurrentCAcount()
{
	int count = 0;
	while(basic_channellist[cur_pos].CAID[count++] != 0);
	return count;
}

int channels::getCurrentCAID(int number)
{
	return basic_channellist[cur_pos].CAID[number];
}

int channels::getCurrentECM(int number)
{
	return basic_channellist[cur_pos].ECM[number];
}

int channels::getCurrentEIT()
{
	return basic_channellist[cur_pos].EIT;
}

int channels::getCurrentType()
{
	return basic_channellist[cur_pos].type;
}

int channels::getCurrentNVODcount()
{
	return basic_channellist[cur_pos].NVOD_count;
}

int channels::getCurrentNVOD_TS(int number)
{
	return basic_channellist[cur_pos].NVOD_TS[number];
}

int channels::getCurrentNVOD_ONID(int number)
{
	return basic_channellist[cur_pos].NVOD_ONID[number];
}

int channels::getCurrentNVOD_SID(int number)
{
	return basic_channellist[cur_pos].NVOD_SID[number];
}

std::string channels::getCurrentServiceName()
{
	int position = 0;
	char name[100];
	for (int i = 0; i < (int)strlen(basic_channellist[cur_pos].serviceName); i++)
	{
		if (basic_channellist[cur_pos].serviceName[i] != 135 && basic_channellist[cur_pos].serviceName[i] != 134)
			name[position++] = basic_channellist[cur_pos].serviceName[i];
	}
	name[position] = '\0';
	return name;
}

std::string channels::getCurrentProviderName()
{
	std::string pname(basic_channellist[cur_pos].providerName);
	return pname;
}

bool channels::addTS(int TS, int ONID, int FREQU, int SYMBOL, int POLARIZATION, int FEC, int diseqc)
{
	struct transportstream new_transportstream;

	memset (&new_transportstream, 0, sizeof(struct transportstream));
	transponder trans;
	trans.TS = TS;
	trans.ONID = ONID;
	if ((*basic_TSlist.find(trans)).second.FREQU == FREQU)
		return false;

	new_transportstream.trans.TS = TS;
	new_transportstream.trans.ONID = ONID;
	new_transportstream.FREQU = FREQU;
	new_transportstream.SYMBOL = SYMBOL;
	new_transportstream.POLARIZATION = POLARIZATION;
	new_transportstream.FEC = FEC;
	new_transportstream.diseqc = diseqc;

	basic_TSlist.insert(std::pair<struct transponder, struct transportstream>(new_transportstream.trans, new_transportstream));

	return true;
}

int channels::getFrequency(int TS, int ONID)
{
	transponder trans;
	trans.TS = TS;
	trans.ONID = ONID;
	return basic_TSlist.find(trans)->second.FREQU;

	/*std::pair<std::multimap<int, struct transportstream>::iterator, std::multimap<struct transponder, struct transportstream>::iterator> ip = basic_TSlist.equal_range(TS);
	for (std::multimap<int, struct transportstream>::iterator it = ip.first; it != ip.second; ++it)
	{
		if ((*it).second.ONID == ONID)
			return (*it).second.FREQU;
	}

	return -1;*/
}

int channels::getSymbolrate(int TS, int ONID)
{
	transponder trans;
	trans.TS = TS;
	trans.ONID = ONID;
	return basic_TSlist.find(trans)->second.SYMBOL;

	/*std::pair<std::multimap<int, struct transportstream>::iterator, std::multimap<int, struct transportstream>::iterator> ip = basic_TSlist.equal_range(TS);
		for (std::multimap<int, struct transportstream>::iterator it = ip.first; it != ip.second; ++it)
		{
			if ((*it).second.ONID == ONID)
				return (*it).second.SYMBOL;
		}
		
		return -1;*/
}

int channels::getPolarization(int TS, int ONID)
{
	transponder trans;
	trans.TS = TS;
	trans.ONID = ONID;
	return basic_TSlist.find(trans)->second.POLARIZATION;

	/*std::pair<std::multimap<int, struct transportstream>::iterator, std::multimap<int, struct transportstream>::iterator> ip = basic_TSlist.equal_range(TS);
	for (std::multimap<int, struct transportstream>::iterator it = ip.first; it != ip.second; ++it)
	{
		if ((*it).second.ONID == ONID)
			return (*it).second.POLARIZATION;
	}

	return -1;*/
}

int channels::getFEC(int TS, int ONID)
{
	transponder trans;
	trans.TS = TS;
	trans.ONID = ONID;
	return basic_TSlist.find(trans)->second.FEC;

	/*std::pair<std::multimap<int, struct transportstream>::iterator, std::multimap<int, struct transportstream>::iterator> ip = basic_TSlist.equal_range(TS);
		for (std::multimap<int, struct transportstream>::iterator it = ip.first; it != ip.second; ++it)
		{
			if ((*it).second.ONID == ONID)
				return (*it).second.FEC;
		}
		
		return -1;*/
}

int channels::getDiseqc(int TS, int ONID)
{
	transponder trans;
	trans.TS = TS;
	trans.ONID = ONID;
	return basic_TSlist.find(trans)->second.diseqc;

	/*std::pair<std::multimap<int, struct transportstream>::iterator, std::multimap<int, struct transportstream>::iterator> ip = basic_TSlist.equal_range(TS);
		for (std::multimap<int, struct transportstream>::iterator it = ip.first; it != ip.second; ++it)
		{
			if ((*it).second.ONID == ONID)
				return (*it).second.diseqc;
		}
		
		return -1;*/
}

/*transportstream channels::getTS(int TS, int ONID)
{
	std::pair<std::multimap<int, struct transportstream>::iterator, std::multimap<int, struct transportstream>::iterator> ip = basic_TSlist.equal_range(TS);
	for (std::multimap<int, struct transportstream>::iterator it = ip.first; it != ip.second; ++it)
	{
		if ((*it).second.ONID == ONID)
			return (*it).second;
	}
	
	transportstream tmp_ts;

	tmp_ts.TS = -1;
	return tmp_ts;
}*/


void channels::dumpTS()
{
	for (std::multimap<struct transponder, struct transportstream>::iterator it = basic_TSlist.begin(); it != basic_TSlist.end(); ++it)
	{
		printf("ONID: %d - TS: %d - FREQU: %ld - SYMBOL: %d - POL: %d - FEC: %d\n", (*it).second.trans.ONID, (*it).second.trans.TS, (*it).second.FREQU, (*it).second.SYMBOL, (*it).second.POLARIZATION, (*it).second.FEC);
	}
}


void channels::dumpChannels()
{
	for (std::vector<struct channel>::iterator it = basic_channellist.begin(); it != basic_channellist.end(); ++it)
	{
		printf("#%d TS: %04x - SID: %04x - Name: %s\n", (*it).channelnumber, (*it).TS, (*it).SID, (*it).serviceName);
	}
	printf("Das sind %d Kan„le\n", basic_channellist.size());
}

bool channels::tune(int TS, int ONID)
{
	transponder trans;
	trans.TS = TS;
	trans.ONID = ONID;
	std::multimap<struct transponder, struct transportstream>::iterator it = basic_TSlist.find(trans);

	//std::cout << basic_TSlist.size() << std::endl;
	std::cout << "ONID: " << trans.ONID << std::endl;
	std::cout << "TS: " << trans.TS << std::endl;
	std::cout << "Tuning (channel) to " << (*it).second.FREQU << " - " << (*it).second.SYMBOL << std::endl;
	return tuner_obj->tune((*it).second.FREQU, (*it).second.SYMBOL, (*it).second.POLARIZATION, (*it).second.FEC, (*cur_pos_TS).second.diseqc);
}

bool channels::setNextTS()
{
	if (++cur_pos_TS != basic_TSlist.end())
	{
		return true;
	}
	return false;
}

bool channels::tuneCurrentTS()
{
	return tuner_obj->tune((*cur_pos_TS).second.FREQU, (*cur_pos_TS).second.SYMBOL, (*cur_pos_TS).second.POLARIZATION, (*cur_pos_TS).second.FEC, (*cur_pos_TS).second.diseqc);

}

void channels::saveDVBChannels()
{
	FILE *fp;

	printf("Save File\n");
	fp = fopen(CONFIGDIR "/lcars/lcars.dvb", "wb");
	for (std::vector<struct channel>::iterator it = basic_channellist.begin(); it != basic_channellist.end(); ++it)
	{
		dvbchannel chan;

		memset (&chan, 0, sizeof(struct dvbchannel));

		transponder trans;
		trans.TS = it->TS;
		trans.ONID = it->ONID;
		std::multimap<struct transponder, struct transportstream>::iterator ts = basic_TSlist.find(trans);

		printf("SID: %x\n", (*it).SID);
		chan.init[0] = 'D';
		chan.init[1] = 'V';
		chan.init[2] = 'S';
		chan.init[3] = 'O';
		chan.SID = (*it).SID;
		chan.PMT = (*it).PMT;
		chan.TXT = (*it).TXT;
		chan.FREQU = (*ts).second.FREQU;
		chan.SYMBOL = (*ts).second.SYMBOL;
		chan.POLARIZATION = (*ts).second.POLARIZATION;
		chan.DISEQC = (*ts).second.diseqc;


		chan.FEC = (*ts).second.FEC;
		chan.VPID = (*it).VPID;
		chan.APID = (*it).APID[0];
		chan.PCR = (*it).PCR;
		for (int i = 0; i < 5; i++)
		{
			if (setting->getCAID() == (*it).CAID[i])
				chan.ECM = (*it).ECM[i];
		}
		chan.type = (*it).type;
		chan.TS = (*it).TS;
		for (int i = 0; i < 24; i++)
			chan.serviceName[i] = (*it).serviceName[i];

		chan.AutoPIDPMT = 3;
		chan.ONID = (*it).ONID;

		fwrite(&chan, sizeof(dvbchannel), 1, fp);
		printf("Size: %d\n", sizeof(dvbchannel));
	}
	fclose(fp);
}

void channels::loadDVBChannels()
{
	int fd;

	printf("Loading Channels from file\n");
	if ((fd = open(CONFIGDIR "/lcars/lcars.dvb", O_RDONLY)) < 0)
	{
		printf("No channels available!\n");
		return;
	}
	dvbchannel chan;
	int count = 0;
	while(read(fd, &chan, sizeof(dvbchannel)) > 0)
	{
		struct channel tmp_chan;
		memset (&tmp_chan, 0, sizeof(struct channel));
		tmp_chan.APID.clear();

		tmp_chan.SID = chan.SID;
		tmp_chan.PMT = chan.PMT;
		tmp_chan.VPID = chan.VPID;
		tmp_chan.APID.insert(tmp_chan.APID.end(), chan.APID);
		tmp_chan.PCR = chan.PCR;
		tmp_chan.ECM[0] = chan.ECM;
		tmp_chan.CAID[0] = setting->getCAID();
		tmp_chan.type = chan.type;
		tmp_chan.TS = chan.TS;
		tmp_chan.TXT = chan.TXT;
		for (int i = 0; i < 24; i++)
			tmp_chan.serviceName[i] = chan.serviceName[i];
		tmp_chan.ONID = chan.ONID;

		basic_channellist.insert(basic_channellist.end(), tmp_chan);
		services_list.insert(std::pair<int, int>(chan.SID, count++));

		struct transportstream tmp_TS;
		memset (&tmp_TS, 0, sizeof(struct transportstream));

		tmp_TS.trans.TS = chan.TS;
		tmp_TS.trans.ONID = chan.ONID;
		tmp_TS.FREQU = chan.FREQU;
		tmp_TS.SYMBOL = chan.SYMBOL;
		tmp_TS.POLARIZATION = chan.POLARIZATION & 0x1;
		tmp_TS.FEC = chan.FEC;
		tmp_TS.diseqc = chan.DISEQC;

		transponder trans;
		trans.TS = chan.TS;
		trans.ONID = chan.ONID;
		if (basic_TSlist.count(trans) == 0)
		{
			basic_TSlist.insert(std::pair<struct transponder, struct transportstream>(trans, tmp_TS));
		}
	}
	close(fd);
	printf("Channels loaded\n");
}

void channels::saveTS()
{
	FILE *fp;

	printf("Save TS File\n");
	fp = fopen(CONFIGDIR "/lcars/transponders.dvb2", "wb");

	for (std::multimap<struct transponder, struct transportstream>::iterator it = basic_TSlist.begin(); it != basic_TSlist.end(); ++it)
	{
		transportstream tmp_ts;
		tmp_ts = it->second;
		fwrite(&tmp_ts, sizeof(transportstream), 1, fp);

		printf("TS: %d\n FREQU: %ld\n SYMBOL: %d\n POL: %d\n FEC: %d\n", (*it).second.trans.TS, (*it).second.FREQU, (*it).second.SYMBOL, (*it).second.POLARIZATION, (*it).second.FEC);
	}

	fclose(fp);
}

void channels::loadTS()
{
	basic_TSlist.clear();
	int fd;

	printf("Loading TS\n");
	if ((fd = open(CONFIGDIR "/lcars/transponders.dvb2", O_RDONLY)) < 0)
	{
		printf("No TS available!\n");
		return;
	}
	transportstream tmp_ts;

	while(read(fd, &tmp_ts, sizeof(transportstream)) > 0)
	{
		if (basic_TSlist.count(tmp_ts.trans) == 0)
		{
			basic_TSlist.insert(std::pair<struct transponder, struct transportstream>(tmp_ts.trans, tmp_ts));
		}
	}
	close(fd);
}

