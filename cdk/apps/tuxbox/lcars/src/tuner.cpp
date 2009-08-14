/***************************************************************************
    copyright            : (C) 2001 by TheDOC
    email                : thedoc@chatville.de
    homepage		 : www.chatville.de
    modified by		 : -
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

$Log: tuner.cpp,v $
Revision 1.25.4.6  2008/08/09 16:41:51  fergy
Cleaning code
Enabled some debug stuff
Enabled some disabled features

Revision 1.25.4.5  2008/08/07 21:07:47  fergy
Revert changes as Dreambox tuner don't wanna get channels.
Now is ok

Revision 1.25.4.4  2008/08/07 17:56:44  fergy
Reverting last changes, as on this way it boot and scan, but NOT show main screen ( on Dreambox )
Added some debug lines back to find out what/where is problem on opening channel after completed scan.

Revision 1.25.4.3  2008/07/30 18:49:18  fergy
Mostly removed debug messages
Tuned-up lcd.cpp & lcd.h code
Globaly removed trash from code
Added stuff for future progress of Lcars

Revision 1.25.4.1  2008/07/22 22:05:44  fergy
Lcars is live again :-)
Again can be builded with Dreambox branch.
I don't know if Dbox can use it for real, but let give it a try on Dreambox again

Revision 1.27  2008/07/22 23:49:36  fergy
Replaced definitions for compatible building

Revision 1.26  2003/03/08 17:31:18  waldi
use tuxbox and frontend infos

Revision 1.25  2003/02/07 14:07:23  alexw
for those with new api

Revision 1.24  2003/02/07 13:55:16  alexw
frontend needs to be opened only once

Revision 1.23  2003/01/26 00:00:20  thedoc
mv bugs /dev/null

Revision 1.22  2003/01/05 22:22:09  TheDOC
stupid drivers

Revision 1.21  2003/01/05 19:28:45  TheDOC
lcars should be old-api-compatible again

Revision 1.20  2002/11/26 20:03:14  TheDOC
some debug-output and small fixes

Revision 1.19  2002/11/16 02:35:13  obi
clear event before ioctl

Revision 1.18  2002/11/16 00:20:05  obi
fe events

Revision 1.17  2002/11/12 19:09:02  obi
ported to dvb api v3

Revision 1.16  2002/10/20 02:03:37  TheDOC
Some fixes and stuff

Revision 1.15  2002/10/13 01:30:34  obi
build fix

Revision 1.14  2002/09/18 10:48:37  obi
use devfs devices

Revision 1.13  2002/06/02 12:18:47  TheDOC
source reformatted, linkage-pids correct, xmlrpc removed, all debug-messages removed - 110k smaller lcars with -Os :)

Revision 1.12  2002/05/18 04:31:02  TheDOC
Warningelimination

Revision 1.11  2002/05/18 02:55:24  TheDOC
LCARS 0.21TP7

Revision 1.10  2002/03/03 22:56:27  TheDOC
lcars 0.20

Revision 1.9  2001/12/20 19:20:49  obi
defined OLD_TUNER_API in Makefile.am instead of tuner.cpp

Revision 1.8  2001/12/17 01:30:02  obi
use /dev/dvb/adapter0/frontend0 for new tuner api.
code for the new tuner api is still disabled by default.

Revision 1.7  2001/12/16 16:09:16  rasc
new dvb tuner API  FE_SET_FRONTEND.
Not tested (drivers have to be changed first), only compile test!

Revision 1.6  2001/12/11 13:38:44  TheDOC
new cdk-path-variables, about 10 new features and stuff

Revision 1.5  2001/12/07 19:36:10  rasc
fix to diseqc Step 2.

Revision 1.4  2001/12/07 14:12:51  rasc
minor fix.

Revision 1.3  2001/12/07 14:10:33  rasc
Fixes for SAT tuning and Diseqc. Diseqc doesn't work properly for me (diseqc 2.0 switch).
Someone should check this please..

Revision 1.2  2001/11/15 00:43:45  TheDOC
 added

*/

#include "tuner.h"

tuner::tuner(settings *s)
{
	setting = s;
	
#ifdef HAVE_LINUX_DVB_VERSION_H
	if ((frontend = open(FRONTEND_DEV, O_RDWR|O_NONBLOCK)) < 0)
	{
		perror("OPEN FRONTEND DEVICE");
		exit(1);
	}
/*
	dvb_frontend_info info;
	if (ioctl(frontend, FE_GET_INFO, &info))
	{
		perror("ioctl");
		exit(1);
	}

	type = info.type; */
#endif
}

tuner::~tuner()
{
	close(frontend);
}

fe_code_rate_t tuner::getFEC(int fec)
{
	switch (fec)
	{
	case -1:
	case 15:
		return FEC_NONE;
	case 0:
		return FEC_AUTO;
	case 1:
		return FEC_1_2;
	case 2:
		return FEC_2_3;
	case 3:
		return FEC_3_4;
	case 4:
		return FEC_5_6;
	case 5:
		return FEC_7_8;
	default:
		return FEC_AUTO;
	}
}

// -- New Tuning API
// -- 2001-12-16 rasc
// polarization = 0 -> H, polarization = 1 -> V
bool tuner::tune(unsigned int frequ, unsigned int symbol, int polarization, int fec, int dis)
{
	#ifdef HAVE_LINUX_DVB_VERSION_H
	struct dvb_frontend_parameters frontp;
	struct dvb_frontend_event event;
	struct dvb_diseqc_master_cmd cmd;
	fe_sec_mini_cmd_t mini_cmd;
	fe_sec_tone_mode_t tone_mode;
	fe_sec_voltage_t voltage;

	if (setting->boxIsSat())
	{
		// $$$ rasc
		// Das Verhalten von Sectone (22KHz) sollte konfigurierbar sein.
		// Ebenso die ZF fuer die LNBs (1 + 2) fuer jeweils Hi und Lo - Band
		// die Werte hier sind Standard fuer das Ku-Band, allerdings waere es
		// interessant auch andere Werte zu haben (z.B. 10 GHz ZF, oder 4 GHz ZF)
		// Dies ist sinnvoll, wenn man die dbox fuer den Sat-DX Empfang, oder
		// aeltere LNBs (naja) nutzen moechte.

		if (frequ > 11700)
		{
			frontp.frequency = (frequ * 1000)-10600000;
			tone_mode = SEC_TONE_ON;
		}
		else
		{
			frontp.frequency = (frequ * 1000)-9750000;
			tone_mode = SEC_TONE_OFF;
		}

		if (polarization == 0)
			voltage = SEC_VOLTAGE_18;
		else
			voltage = SEC_VOLTAGE_13;

		frontp.u.qpsk.fec_inner = getFEC(fec);

		// -- symbol rate (SAT)
		frontp.u.qpsk.symbol_rate = symbol * 1000;

		// diseqc
		cmd.msg_len = 4;
		cmd.msg[0] = 0xe0;
		cmd.msg[1] = 0x10;
		cmd.msg[2] = 0x38;
		cmd.msg[3] = 0xf0
				| ((dis * 4) & 0x0f)
				| ((voltage == SEC_VOLTAGE_18) ? 2 : 0)
				| ((tone_mode == SEC_TONE_ON)  ? 1 : 0);

		mini_cmd = (dis / 4) % 2 ? SEC_MINI_B : SEC_MINI_A;

		if (ioctl(frontend, FE_SET_TONE, SEC_TONE_OFF) < 0)
			perror("FE_SET_TONE");

		if (ioctl(frontend, FE_SET_VOLTAGE, voltage) < 0)
			perror("FE_SET_VOLTAGE");
		
		usleep(15 * 1000);
		
		if (ioctl(frontend, FE_DISEQC_SEND_MASTER_CMD, &cmd) < 0)
			perror("FE_DISEQC_SEND_MASTER_CMD");
		
		usleep(15 * 1000);
		
		if (ioctl(frontend, FE_DISEQC_SEND_BURST, mini_cmd) < 0)
			perror("FE_DISEQC_SEND_BURST");
		
		usleep(15 * 1000);
		
		if (ioctl(frontend, FE_SET_TONE, tone_mode) < 0)
			perror("FE_SET_TONE");
	}

	if (setting->boxIsCable())
	{
		frontp.frequency = frequ * 100000;
		frontp.u.qam.symbol_rate = symbol * 1000;
		frontp.u.qam.fec_inner = getFEC(fec);
		frontp.u.qam.modulation = QAM_64;
	}

	// -- Spektrum Inversion
	// -- should be configurable, fixed for now (rasc)
	frontp.inversion = (fe_spectral_inversion_t) setting->getInversion();

	while (1) {
		if (ioctl(frontend, FE_GET_EVENT, &event) == -1)
			break;
	}

	if (ioctl(frontend, FE_SET_FRONTEND, &frontp) < 0) {
		perror("FE_SET_FRONTEND");
		return 0;
	}

	do {
		memset(&event, 0, sizeof(event));
		ioctl(frontend, FE_GET_EVENT, &event);
	}
	while (!(event.status & (FE_HAS_LOCK | FE_TIMEDOUT)));

	printf ("Freq: %u\n ifreq: %u\n  Pol: %d\n  FEC: %d\n  Sym: %u\n  dis: %d\n",
	        frequ, frontp.frequency, (int)polarization , (int)fec,
	        symbol, (int)dis);

	printf ("Lock Status: %d\n",event.status);

	uint16_t state1, state2;

	if (ioctl(frontend, FE_READ_SNR, &state1) < 0)
		perror("FE_READ_SNR");

	if (ioctl(frontend, FE_READ_SIGNAL_STRENGTH, &state2) < 0)
		perror("FE_READ_SIGNAL_STRENGTH");

	printf ("S/N: %d\n  Strength: %d\n",state1,state2);
	if (event.status & FE_HAS_LOCK)
		std::cout << "Has lock" << std::endl;
	else
		std::cout << "Doesn't have lock" << std::endl;

	return (event.status & FE_HAS_LOCK);
#elif HAVE_OST_DMX_H

	int device;
	int frontend;
	struct secCmdSequence seq;
	struct secCommand cmd;
	FrontendParameters frontp;
	int status;

	if (setting->boxIsSat())
	{

		// $$$ rasc
		// Das Verhalten von Sectone (22KHz) sollte konfigurierbar sein.
		// Ebenso die ZF fuer die LNBs (1 + 2) fuer jeweils Hi und Lo - Band
		// die Werte hier sind Standard fuer das Ku-Band, allerdings waere es
		// interessant auch andere Werte zu haben (z.B. 10 GHz ZF, oder 4 GHz ZF)
		// Dies ist sinnvoll, wenn man die dbox fuer den Sat-DX Empfang, oder
		// aeltere LNBs (naja) nutzen moechte.

		if (frequ > 11700)
		{
			frontp.Frequency = (frequ * 1000)-10600000;
			seq.continuousTone = SEC_TONE_ON;
		}
		else
		{
			frontp.Frequency = (frequ * 1000)-9750000;
			seq.continuousTone = SEC_TONE_OFF;
		}

		if (polarization == 0)
		{
			seq.voltage=SEC_VOLTAGE_18;
		}
		else
		{
			seq.voltage=SEC_VOLTAGE_13;
		}

		frontp.u.qpsk.FEC_inner = getFEC(fec);

		// -- symbol rate (SAT)
		frontp.u.qpsk.SymbolRate = symbol * 1000;

		// diseqc Group Byte  (dis: 0..3)
		// 2001-12-06 rasc

		cmd.type=SEC_CMDTYPE_DISEQC;
		cmd.u.diseqc.addr=0x10;
		cmd.u.diseqc.cmd=0x38;
		cmd.u.diseqc.numParams=1;
		cmd.u.diseqc.params[0]=0xF0
								| ((dis*4) & 0x0F)
								| ((seq.voltage == SEC_VOLTAGE_13)     ? 3 : 0)
								| ((seq.voltage == SEC_VOLTAGE_18)     ? 2 : 0)
								| ((seq.continuousTone == SEC_TONE_ON) ? 1 : 0);

		seq.miniCommand=SEC_MINI_NONE;
		seq.numCommands=1;
		seq.commands=&cmd;

		if((device = open("/dev/dvb/card0/sec0", O_RDWR)) < 0)
		{
			perror("OPEN SEC DEVICE");
			exit(1);
		}

		if (ioctl(device,SEC_SEND_SEQUENCE,&seq) < 0)
		{
			perror("SEC_SEND_SEQUENCE");
		}

		close(device);
	}

	if (setting->boxIsCable())
	{
		if (frequ < 1500) // sorry, but the old drivers are buggy :(
			return false;
		frontp.Frequency = frequ * 100;
		frontp.u.qam.SymbolRate = symbol * 1000;
		frontp.u.qam.FEC_inner = getFEC(fec);
		// frontp.u.qam.FEC_outer = FEC_AUTO;
		frontp.u.qam.QAM = QAM_64;
	}

	// -- Spektrum Inversion
	// -- should be configurable, fixed for now (rasc)
	if (setting->getInversion() == INVERSION_ON)
	{
		frontp.Inversion = INVERSION_ON;
	}
	else if (setting->getInversion() == INVERSION_OFF)
	{
		frontp.Inversion = INVERSION_OFF;
	}
	else if (setting->getInversion() == INVERSION_AUTO)
	{
		frontp.Inversion = INVERSION_AUTO;
	}
	if ((frontend = open(FRONTEND_DEV, O_RDWR|O_NONBLOCK)) < 0)
	{
		perror("OPEN FRONTEND DEVICE");
		exit(1);
	}

	if (ioctl(frontend, FE_SET_FRONTEND, &frontp) < 0)
	{
		perror("FE_SET_FRONTEND");
	}

	if (ioctl(frontend, FE_READ_STATUS, &status) < 0)
	{
		perror("FE_READ_STATUS");
	}

	printf (" Freq: %ld\n   ifreq: %ld\n  Pol: %d\n  FEC: %d\n  Sym: %ld\n  dis: %d\n  (param: 0x%02x)\n",
	        (long)frequ,(long)frontp.Frequency,(int)polarization ,(int)fec,
	        (long)symbol, (int)dis,(int)cmd.u.diseqc.params[0]);

	printf ("Lock Status: %d\n",status);

	int state1, state2;

	if (ioctl(frontend, FE_READ_SNR, &state1) < 0)
	{
		perror("FE_READ_SNR");
	}

	if (ioctl(frontend, FE_READ_SIGNAL_STRENGTH, &state2) < 0)
	{
		perror("FE_READ_SIGNAL_STRENGTH");
	}

	printf ("S/N: %d\n  Signal Strength: %d\n",state1,state2);

	close(frontend);

	return (status & FE_HAS_SIGNAL);


#endif
}


