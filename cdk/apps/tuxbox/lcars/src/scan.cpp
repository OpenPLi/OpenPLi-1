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
$Log: scan.cpp,v $
Revision 1.18.4.3  2008/08/09 16:41:51  fergy
Cleaning code
Enabled some debug stuff
Enabled some disabled features

Revision 1.18.4.2  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.18.4.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.19  2003/03/08 17:31:18  waldi
use tuxbox and frontend infos

Revision 1.18  2002/11/26 20:03:14  TheDOC
some debug-output and small fixes

Revision 1.17  2002/10/31 19:53:37  TheDOC
Cablescan should work now with the current (buggy?) drivers (it actually
works for me)

Revision 1.16  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.15  2002/06/15 02:33:03  TheDOC
some changes + bruteforce-channelscan for cable

Revision 1.14  2002/06/04 20:39:12  TheDOC
old version worked better :)

Revision 1.13  2002/06/03 19:30:48  TheDOC
scanlist.dat missing -> show error

Revision 1.12  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.11  2002/05/18 04:31:02  TheDOC
Warningelimination

Revision 1.10  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.9  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.3  2001/12/17 01:00:41  tux
scan.cpp fix

Revision 1.7  2001/12/17 00:18:18  obi
readded revision 1.5

Revision 1.5  2001/12/07 23:12:31  rasc
scanfile.dat reorganized to handle more transponders,
some small fixes

Revision 1.3  2001/12/07 14:10:33  rasc
Fixes for SAT tuning and Diseqc. Diseqc doesn't work properly for me (diseqc 2.0 switch).
Someone should check this please..

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/
#include <unistd.h>
#include <stdio.h>
#include <stdio.h>
#include "scan.h"

scan::scan(settings *s, pat *p1, pmt *p2, nit *n, sdt *s1, osd *o, tuner *t, channels *c)
{
	setting = s;
	pat_obj = p1;
	pmt_obj = p2;
	nit_obj = n;
	sdt_obj = s1;
	osd_obj = o;
	tuner_obj = t;
	channels_obj = c;
}

void scan::readUpdates()
{
	channels tmp_channels(setting, pat_obj, pmt_obj);
	sdt_obj->getChannels(&tmp_channels);

	bool changed = false;
	bool new_channels = false;

	int old_number = channels_obj->getCurrentChannelNumber();
	for (int i = 0; i < tmp_channels.numberChannels(); i++)
	{
		channel tmp_channel = tmp_channels.getChannelByNumber(i);
		int channelnumber = channels_obj->getChannelNumber(tmp_channel.TS, tmp_channel.ONID, tmp_channel.SID);
		if (channelnumber == -1)
		{
			channels_obj->addChannel(tmp_channel);
			changed = true;
			new_channels = true;
		}
		else
		{
			channel tmp_channel_old = channels_obj->getChannelByNumber(channelnumber);
			if (strcmp(tmp_channel.serviceName, tmp_channel_old.serviceName))
			{
				channels_obj->updateChannel(channelnumber, tmp_channel);
				if (channelnumber == old_number)
					osd_obj->setServiceName(tmp_channel.serviceName);
				changed = true;
			}
		}
	}
	channels_obj->setCurrentChannel(old_number);
	if (changed)
		channels_obj->saveDVBChannels();
	if (new_channels)
	{
		osd_obj->setPerspectiveName("New Channels found and added!!!!");
		osd_obj->addCommand("SHOW perspective");
		sleep(5);
		osd_obj->addCommand("HIDE perspective");
	}
}


channels scan::scanChannels(int type, int start_frequency, int start_symbol, int start_polarization, int start_fec)
{
	int number;
	channels tmp_channels(setting, pat_obj, pmt_obj);
	bool badcable = false;

	//settings settings;
	osd_obj->createScan();
	osd_obj->addCommand("SHOW scan");
	sleep(1);

	osd_obj->setScanProgress(0);
	osd_obj->setScanChannelNumber(0);


	if (setting->boxIsCable())
	{
		start_symbol = 6900;
		osd_obj->createPerspective();

		if (type == NORMAL || type == FULL)
		{
			for (int i = 0; (i < 3) && (tmp_channels.numberTransponders() < 1); i++)
			{
				start_frequency = 3460;
				if (i == 1)
				{
					std::cout << "Inversion off " << std::endl;
					setting->setInversion(INVERSION_OFF);
				}
				else if (i == 0)
				{
					std::cout << "Inversion auto" << std::endl;
					setting->setInversion(INVERSION_AUTO);
				}
				else if (i == 2)
				{
					std::cout << "Inversion on" << std::endl;
					setting->setInversion(INVERSION_ON);
				}


				while(tmp_channels.numberTransponders() < 1)
				{
					char message[100];
					sprintf(message, "Searching NIT on %d - %d", start_frequency, start_symbol);
					osd_obj->setPerspectiveName(message);
					osd_obj->addCommand("SHOW perspective");
	
					if (tuner_obj->tune(start_frequency, start_symbol))
					{
						std::cout << "Checking frequency: " << start_frequency << " with symbol: " << start_symbol << std::endl;
						
						number = nit_obj->getTransportStreams(&tmp_channels);
						if (tmp_channels.numberTransponders() > 0)
						{
							osd_obj->setScanTSNumber(tmp_channels.numberTransponders());
							tmp_channels.dumpTS();
							break;
						}
					}
					start_frequency += 80;
					if (start_frequency > 4000)
					break;
				}
			}
		}

		if (type == NORMAL || type == FULL)
		{
			if (tmp_channels.numberTransponders() < 1)
			{
				osd_obj->setPerspectiveName("Sorry, no NIT found! Check cables!!!");
				osd_obj->addCommand("SHOW perspective");

				exit(-1);
			}
		}

		int test_frequ = 20000;
		if (tmp_channels.numberTransponders() > 0)
		{
			tmp_channels.setBeginTS();
			test_frequ = tmp_channels.getCurrentFrequency();
		}
		
		if (test_frequ > 10000 || type == BRUTEFORCE)
		{
			osd_obj->setPerspectiveName("Manually searching... (That takes time :)");
			badcable = true;
			osd_obj->addCommand("SHOW perspective");

			tmp_channels.clearTS();

			sleep(5);

			for (int i = 500; i < 8900; i += 80)
			{
				char message[100];
				sprintf(message, "Checking %d\n %d\n", i, 6900);
				osd_obj->setPerspectiveName(message);
				osd_obj->addCommand("SHOW perspective");

				if (tuner_obj->tune(i, 6900))
				{
					if (pat_obj->readPAT())
					{
						channels tmp_channels2(setting, pat_obj, pmt_obj);
						sdt_obj->getChannels(&tmp_channels2);
						tmp_channels.addTS(pat_obj->getTS(), sdt_obj->getONID(), i, 6900);
						std::cout << "Found Transponders: " << pat_obj->getTS() << " " << sdt_obj->getONID() << " " << i << " " << 6900 << std::endl;
						osd_obj->setScanTSNumber(tmp_channels.numberTransponders());
					}
				}
			}
		}
	}
	else if (setting->boxIsSat())
	{
		int max_chans = 2;

		int start_frq[20];	// see: tune
		int start_sym[20];
		int start_pol[20];
		int start_fe[20];
		int start_dis[20];	// start diseq 0..3, -1 = auto
		FILE *fp;
		int dis_start, dis_end;
		char tmp[100];


		fp = fopen(CONFIGDIR "/lcars/scanlist.dat", "r");

		int co = 0;
		while(!feof(fp))
		{
			char text[256];

			printf("Starting at position: %d\n", co);

			fgets(text, 255, fp);
			if (!isdigit(*text)) continue;
			sscanf (text,"%i,%i,%i,%i,%i %s\n",
			        &start_frq[co], &start_sym[co],
			        &start_pol[co], &start_fe[co],
			        &start_dis[co], tmp);
			printf ("Scandat file successfuly loaded:\n Freq:%d\n, SymR:%d\n, Pol:%d\n, FEC:%d\n DiSeqc:%d\n",
			start_frq[co], start_sym[co],
			start_pol[co], start_fe[co],
			start_dis[co]);
			co++;
			if ((unsigned int) co >= (sizeof(start_fe)/sizeof(start_fe[0])) )
				break;
		}
		max_chans = co;


		////printf("Diseqc: %d\n", dis);
		int i = 0;

		//int old = channels.numberTransponders();
		while (i < max_chans)
		{
			printf("Default start: %d\n", i);

			if (start_dis[i]  < 0) {
				dis_start = 0;
				dis_end   = 3;
			} else {
				dis_start = dis_end = start_dis[i];
			}

			for (int dis = dis_start; dis <= dis_end; dis++)
			{
				char message[255];
				sprintf(message, "Searching on %d\n %d\n %d\n %d\n %d\n",
				        start_frq[i], start_sym[i],
				        start_pol[i], start_fe[i], dis);
				osd_obj->setPerspectiveName(message);
				osd_obj->addCommand("SHOW perspective");

				printf ("Start tuning\n");

				if (tuner_obj->tune(start_frq[i], start_sym[i], start_pol[i], start_fe[i], dis))
				{

					printf("Finished tuning\n");

					printf ("Start NIT\n");
					number = nit_obj->getTransportStreams(&tmp_channels, dis);
					printf ("End NIT\n");
				}
			}

			i++;
		}

		if (tmp_channels.numberTransponders() < 1)
		{
			osd_obj->setPerspectiveName("Sorry, no NIT found! Check cables!!!");
			osd_obj->addCommand("SHOW perspective");
			exit(-1);
		}
	}

	osd_obj->setScanTSNumber(tmp_channels.numberTransponders());
	printf("Found %d transponders: \n", tmp_channels.numberTransponders());
	tmp_channels.dumpTS();
	sleep(5);
	int count = 0;
	int numberTS = tmp_channels.numberTransponders();
	int numberChannels = 0;

	tmp_channels.setBeginTS();

	char message[100];
	sprintf(message, "Scanning Channels");
	osd_obj->setPerspectiveName(message);
	osd_obj->addCommand("SHOW perspective");

	tmp_channels.setTuner(tuner_obj);
	do
	{
		if(tmp_channels.tuneCurrentTS())
		{
			printf("Get Channels - Start\n");
			sdt_obj->getChannels(&tmp_channels);

			if (type == FULL)
			{
				std::cout << "Full Channel Scan" << std::endl;
				pat_obj->readPAT();
				for (int i = numberChannels; i < tmp_channels.numberChannels(); i++)
				{
					tmp_channels.setCurrentChannel(i);
					tmp_channels.setCurrentPMT(pat_obj->getPMT(tmp_channels.getCurrentSID()));

					pmt_data pmt_entry;
					if (tmp_channels.getCurrentPMT() != 0)
					{
						pmt_entry = pmt_obj->readPMT(tmp_channels.getCurrentPMT());

						tmp_channels.setCurrentPCR(pmt_entry.PCR);

						tmp_channels.deleteCurrentAPIDs();
						for (int j = 0; j < pmt_entry.pid_counter; j++)
						{
							if (pmt_entry.type[j] == 0x02)
							{
								tmp_channels.setCurrentVPID(pmt_entry.PID[j]);
							}
							else if (pmt_entry.type[j] == 0x04 || pmt_entry.type[j] == 0x03)
							{
								tmp_channels.addCurrentAPID(pmt_entry.PID[j]);
							}
						}

						for (int j = 0; j < pmt_entry.ecm_counter; j++)
						{
							if (setting->getCAID() == pmt_entry.CAID[j])
								tmp_channels.addCurrentCA(pmt_entry.CAID[j], pmt_entry.ECM[j]);
						}
					}
					else
					{
						tmp_channels.deleteCurrentAPIDs();
						tmp_channels.setCurrentVPID(0x1fff);
						tmp_channels.addCurrentAPID(0x1fff);
					}


					numberChannels = tmp_channels.numberChannels();

				}
			}

		}
		osd_obj->setScanChannelNumber(tmp_channels.numberChannels());
		printf("Get Channels - Finish\n");
		count++;
		osd_obj->setScanProgress((int)(((float)count / numberTS) * 100));
	} while(tmp_channels.setNextTS());

	{
		char message[100];
		sprintf(message, "Found %d channels on %d Transponders.", tmp_channels.numberChannels(), tmp_channels.numberTransponders());
		osd_obj->setPerspectiveName(message);
		osd_obj->addCommand("SHOW perspective");
		sleep(5);
	}

	osd_obj->addCommand("HIDE scan");
	osd_obj->addCommand("HIDE perspective");
	osd_obj->hidePerspective();

	//printf("Found channels: %d\n", channels.numberChannels());
	//channels.saveDVBChannels();

	return tmp_channels;
}

void scan::updateChannels(channels *chan)
{
	channels newchannels = scanChannels();

	int newsort[(*chan).numberChannels() + newchannels.numberChannels()];
	int notfound[newchannels.numberChannels()];
	int numbernotfoundchannels = 0;

	printf("Starting update-Compare\n");
	//printf("Old: %d - New: %d\n", (*chan).numberChannels(), newchannels.numberChannels());
	for (int i = 0; i < (*chan).numberChannels(); i++)
		newsort[i] = -1;

	printf("----> 1\n");
	for (int i = 0; i < newchannels.numberChannels(); i++)
	{
		printf("%d\n", i);
		channel tmp_chan = newchannels.getChannelByNumber(i);

		int chan_num = (*chan).getChannelNumber(tmp_chan.TS, tmp_chan.ONID, tmp_chan.SID);

		printf("Channel number: %d\n", chan_num);
		if (chan_num != -1)
			newsort[chan_num] = i;
		else
		{
			notfound[numbernotfoundchannels++] = i;
		}
	}
	printf("----> 2\n");


	for (int i = 0; i < numbernotfoundchannels; i++)
	{
		newsort[(*chan).numberChannels() + i] = notfound[i];
	}

	printf("----> 3\n");
	channel empty_chan;
	memset (&empty_chan, 0, sizeof(struct channel));
	strcpy(empty_chan.serviceName, "--> Deleted");

	printf("Complete: %d\n", (*chan).numberChannels() + numbernotfoundchannels);
	printf("----> 4\n");
	int numbercomplete = (*chan).numberChannels() + numbernotfoundchannels;

	(*chan).clearChannels();

	for (int i = 0; i < numbercomplete; i++)
	{
		channel tmp_chan;

		if (newsort[i] != -1)
			tmp_chan = newchannels.getChannelByNumber(newsort[i]);
		else
			tmp_chan = empty_chan;

		(*chan).addChannel(tmp_chan);
	}

	newchannels.setBeginTS();
	for (int i = 0; i < newchannels.numberTransponders(); i++)
	{
		printf("Adding Transponder %x\n ONID %x\n Frequency %d\n", newchannels.getCurrentSelectedTS(), newchannels.getCurrentSelectedONID(), newchannels.getCurrentFrequency());
		(*chan).addTS(newchannels.getCurrentSelectedTS(), newchannels.getCurrentSelectedONID(), newchannels.getCurrentFrequency(), newchannels.getCurrentSymbolrate(), newchannels.getCurrentPolarization(), newchannels.getCurrentFEC());
		newchannels.setNextTS();
	}

}
