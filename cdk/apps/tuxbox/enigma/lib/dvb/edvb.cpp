#include <lib/dvb/edvb.h>
#include <config.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <string.h>

#ifndef DMX_LOW_BITRATE
#define DMX_LOW_BITRATE 0x4000
#endif

#ifdef PROFILE
	#include <sys/time.h>
#endif

#include <lib/base/console.h>
#include <lib/driver/eavswitch.h>
#include <lib/driver/rfmod.h>
#include <lib/dvb/esection.h>
#include <lib/dvb/si.h>
#include <lib/dvb/frontend.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/record.h>
#ifndef DISABLE_CI
	#include <lib/dvb/dvbci.h>
#endif
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/dvbscan.h>
#include <lib/dvb/dvbfastscan.h>
#include <lib/dvb/service.h>

#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>

eDVBController::~eDVBController()
{
}

eDVB *eDVB::instance;

eString eDVB::getVersion()
{
	return "eDVB lib 1.0, compiled " __DATE__;
}

void eDVB::event(const eDVBEvent &event)
{
	eventOccured(event);
	if (controller)
		controller->handleEvent(event);
}

void eDVB::tunedIn(eTransponder *trans, int err)
{
	event(eDVBEvent(eDVBEvent::eventTunedIn, err, trans));
}

eDVB::eDVB()
	:parentEIT(0), recorder(0)
#ifndef DISABLE_CI
	, DVBCI(0), DVBCI2(0)
#endif
	,state(eDVBState::stateIdle)
#ifndef DISABLE_NETWORK
	,udhcpc(0)
	,delayTimer(eApp)
#endif
{
	init_eDVB();
}
void eDVB::init_eDVB()
{
#ifndef DISABLE_NETWORK
	CONNECT(delayTimer.timeout, eDVB::restartSamba);
#endif
	settings=0;
	time_difference=0;

		// singleton
	if (instance)
		eFatal("eDVB already initialized!");
	instance=this;

#ifndef DISABLE_CI
	int numCI = eSystemInfo::getInstance()->hasCI();
	if ( numCI > 0 )
	{
		DVBCI=new eDVBCI();
		DVBCI->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::start));
	}
	if ( numCI > 1 )
	{
		DVBCI2=new eDVBCI();
		DVBCI2->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::start));
	}
#endif
	if (eFrontend::open(eSystemInfo::getInstance()->getFEType())<0)
		eFatal("couldn't open frontend");

	settings = new eDVBSettings(*this);
		// tuned-in handling
	CONNECT(eFrontend::getInstance()->tunedIn, eDVB::tunedIn);

		// set to idle state
	controller=0;
	setState(eDVBState(eDVBState::stateIdle));

		// intialize to service mode
	setMode(controllerService);
		// init AV switch
	switch (eSystemInfo::getInstance()->getHwType())
	{
	case eSystemInfo::dbox2Philips:
		new eAVSwitchPhilips;
		break;
	case eSystemInfo::dbox2Sagem:
		new eAVSwitchSagem;
		break;
	case eSystemInfo::dbox2Nokia:
	default:
		new eAVSwitchNokia;
		break;
	}

#ifdef ENABLE_RFMOD
	if ( eSystemInfo::getInstance()->hasRFMod() )
	{
		new eRFmod;
		eRFmod::getInstance()->init();
	}
#endif

	eDebug("eDVB::eDVB done.");
}

eDVB::~eDVB()
{
	delete settings; 

#ifndef DISABLE_FILE
	recEnd();
#endif // DISABLE_FILE

	delete eAVSwitch::getInstance();

#ifdef ENABLE_RFMOD
	if(eRFmod::getInstance())
		delete eRFmod::getInstance();
#endif

	Decoder::Close();

	eFrontend::close();

	delete controller;

#ifndef DISABLE_CI
	delete DVBCI;
	delete DVBCI2;	
#endif

#ifndef DISABLE_NETWORK
	if (udhcpc) 
	{
		delete udhcpc;
		udhcpc = 0;
	}
#endif
	instance=0;
}

		// for external access only
PMT *eDVB::getPMT()
{
	return tPMT.ready()?tPMT.getCurrent():0;
}

EIT *eDVB::getEIT()
{
	if ( tEIT.ready() )
		return tEIT.getCurrent();
	else if ( parentEIT )
	{
		parentEIT->lock();
		return parentEIT;
	}
	else
		return 0;
}

SDT *eDVB::getSDT()
{
	return tSDT.ready()?tSDT.getCurrent():0;
}

#ifndef DISABLE_FILE
void eDVB::recMessage(int msg)
{
	switch (msg)
	{
	case eDVBRecorder::recWriteError:
		event(eDVBEvent(eDVBEvent::eventRecordWriteError));
		break;
	}
}

void eDVB::recBegin(const char *filename, eServiceReferenceDVB service)
{
	if (recorder)
		recEnd();

	PMT *pmt=getPMT();
	PAT *pat=tPAT.ready()?tPAT.getCurrent():0;
	recorder=new eDVBRecorder(pmt, pat);
	if ( pat )
		pat->unlock();
	recorder->recRef=(eServiceReferenceDVB&)eServiceInterface::getInstance()->service;

	eServiceHandler *handler = eServiceInterface::getInstance()->getService();

	recorder->scrambled = handler->getFlags() & eServiceHandler::flagIsScrambled;

	recorder->open(filename);

	CONNECT(recorder->recMessage, eDVB::recMessage);

//	recorder->addNewPID(0); // PAT

//	if (Decoder::parms.pmtpid != -1)
//		recorder->addNewPID(Decoder::parms.pmtpid);

	if (!pmt)
	{
		if (Decoder::parms.apid != -1)
			recorder->addNewPID(Decoder::parms.apid);
		if (Decoder::parms.vpid != -1)
			recorder->addNewPID(Decoder::parms.vpid);
		if (Decoder::parms.tpid != -1)
			recorder->addNewPID(Decoder::parms.tpid);
		if (Decoder::parms.pcrpid != -1)
			recorder->addNewPID(Decoder::parms.pcrpid);
	}
	else
	{
		recorder->addNewPID(pmt->PCR_PID);
#ifdef RECORD_ECM
		for (ePtrList<Descriptor>::iterator i(pmt->program_info.begin()); i != pmt->program_info.end(); ++i)
			if (i->Tag() == 9)
				recorder->addNewPID(((CADescriptor*)*i)->CA_PID);
#endif
		for (ePtrList<PMTEntry>::iterator i(pmt->streams); i != pmt->streams.end(); ++i)
		{
			int record=0;
			int isUndocumentedNA = 0;
			switch (i->stream_type)
			{
			case 1:	// video..
			case 2:
				record=1;
				break;
			case 3:	// audio..
			case 4:
				record=1;
				break;
			case 0x80:
			case 0x81:
			case 0x82:
			case 0x83:
				isUndocumentedNA = 1;
			case 6:
				for (ePtrList<Descriptor>::iterator it(i->ES_info); it != i->ES_info.end(); ++it)
				{
					switch (it->Tag())
					{
						case DESCR_AC3:
						{
							int norecord=0;
							eConfig::getInstance()->getKey("/enigma/noac3recording", norecord);
							if (!norecord)
								record=1;
							break;
						}
						case DESCR_ISO639_LANGUAGE:
							if (isUndocumentedNA)
							{
								record = 1;
							}
							break;
#ifdef RECORD_TELETEXT
						case DESCR_TELETEXT:
						{
							int norecord=0;
							eConfig::getInstance()->getKey("/enigma/nottxrecording", norecord);
							if (!norecord)
								record=2;  // low bitrate
							break;
						}
#endif
#ifdef RECORD_SUBTITLES
						case DESCR_SUBTITLING:
						{
							record=2;  // low bitrate
							break;
						}
#endif
					}
				}
				break;
			}
			if (record)
				recorder->addNewPID(i->elementary_PID, record==2?DMX_LOW_BITRATE:0);
#ifdef RECORD_ECM
			for (ePtrList<Descriptor>::iterator it(i->ES_info); it != i->ES_info.end(); ++it)
				if (it->Tag() == 9)
					recorder->addNewPID(((CADescriptor*)*it)->CA_PID);
#endif
		}
		pmt->unlock();
	}

	// build SMI table.
	
	// build SIT:
	__u8 sit[4096];
	int pos=0;
	sit[pos++]=0x7F;              // TID
	sit[pos++]=0x80;              // section_syntax_indicator, length
	sit[pos++]=0;                 // length
	sit[pos++]=0; sit[pos++]=0;   // reserved
	sit[pos++]=1;                 // reserved, version number, current/next indicator
	sit[pos++]=0;                 // section number
	sit[pos++]=0;                 // last section number
	sit[pos++]=0;                 // loop length
	sit[pos++]=0;                 // "    "
	int loop_pos=pos;
		// TODO: build Partitial Transport Stream descriptor containing valid values about rate
	sit[loop_pos-2]|=(pos-loop_pos)>>8;
	sit[loop_pos-1]|=(pos-loop_pos)&0xFF;
	
	// create one single entry: our service...
	sit[pos++]=service.getServiceID().get()>>8;
	sit[pos++]=service.getServiceID().get()&0xFF;
	sit[pos++]=3<<4;              // running state
	sit[pos++]=0;
	loop_pos=pos;

	// create our special descriptor:
	sit[pos++]=0x80;              // private	
	sit[pos++]=0;
	int descr_pos=pos;
	memcpy(sit+pos, "ENIGMA", 6);
	pos+=6;

	if (Decoder::parms.vpid != -1)
	{
		sit[pos++]=eServiceDVB::cVPID;
		sit[pos++]=2;
		sit[pos++]=Decoder::parms.vpid>>8;
		sit[pos++]=Decoder::parms.vpid&0xFF;
	}

	if (Decoder::parms.apid != -1)
	{
		sit[pos++]=eServiceDVB::cAPID;
		sit[pos++]=3;
		sit[pos++]=Decoder::parms.apid>>8;
		sit[pos++]=Decoder::parms.apid&0xFF;
		sit[pos++]=Decoder::parms.audio_type;
	}
	
	if (Decoder::parms.tpid != -1)
	{
		sit[pos++]=eServiceDVB::cTPID;
		sit[pos++]=2;
		sit[pos++]=Decoder::parms.tpid>>8;
		sit[pos++]=Decoder::parms.tpid&0xFF;
	}

	if (Decoder::parms.pcrpid != -1)
	{
		sit[pos++]=eServiceDVB::cPCRPID;
		sit[pos++]=2;
		sit[pos++]=Decoder::parms.pcrpid>>8;
		sit[pos++]=Decoder::parms.pcrpid&0xFF;
	}

	sit[descr_pos-2]|=(pos-descr_pos)>>8;
	sit[descr_pos-1]|=(pos-descr_pos)&0xFF;

		// fix lengths
	sit[loop_pos-2]|=(pos-loop_pos)>>8;
	sit[loop_pos-1]|=(pos-loop_pos)&0xFF;
	
		// add CRC (calculate later)
	sit[pos++]=0;
	sit[pos++]=0;
	sit[pos++]=0;
	sit[pos++]=0;

		// fix length
	sit[1]|=(pos-3)>>8;
	sit[2]|=(pos-3)&0xFF;

	// generate CRC :)

	unsigned int cc=0;
	// write section.
	recorder->writeSection(sit, 0x1f, cc);

	recorder->validatePIDs();
}

void eDVB::recPause()
{
	recorder->stop();
}

void eDVB::recResume()
{
	recorder->start();
}

void eDVB::recEnd()
{
	if (!recorder)
		return;
	delete recorder;
	recorder=0;
	if ( controller && controllertype == controllerService )
		((eDVBServiceController*)controller)->disableFrontend();
}
void eDVB::recSetSlice(int slice)
{
	if (!recorder)
		return;
	recorder->SetSlice(slice);
}
#endif //DISABLE_FILE

void eDVB::setMode(int mode)
{
	if (controllertype == mode)
		return;
	controllertype = mode;
	if (controller)
		delete controller;
	switch (mode)
	{
	case controllerScan:
		controller = new eDVBScanController(*this);
		break;
	case controllerFastscan:
		controller = new eDVBFastscanController(*this);
		break;
	case controllerService:
		controller = new eDVBServiceController(*this);
#ifndef DISABLE_CI
		if ( DVBCI )
			DVBCI->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::getcaids));
		if ( DVBCI2 )
			DVBCI2->messages.send(eDVBCI::eDVBCIMessage(eDVBCI::eDVBCIMessage::getcaids));
#endif
	break;
	}
}

eDVBServiceController *eDVB::getServiceAPI()
{
	if (controllertype != controllerService)
		return 0;
	return (eDVBServiceController*)controller;
}

eDVBScanController *eDVB::getScanAPI()
{
	if (controllertype != controllerScan)
		return 0;
	return (eDVBScanController*)controller;
}

eDVBFastscanController *eDVB::getFastscanAPI()
{
	if (controllertype != controllerFastscan)
		return 0;
	return (eDVBFastscanController*)controller;
}

////////////////////////////////// NETWORK STUFF //////////////////////////////
#ifndef DISABLE_NETWORK
static void unpack(__u32 l, int *t)
{
	for (int i=0; i<4; i++)
		*t++=(l>>((3-i)*8))&0xFF;
}

void eDVB::UDHCPC_DataAvail(eString str)
{
	eDebug("[DHCPC] %s", str.c_str());
	if ( str.find("lease") != eString::npos
		&& str.find("obtained") != eString::npos )
	{
		delayTimer.start(2000,true);
	}
}

void eDVB::UDHCPC_Closed(int d)
{
	eDebug("[DHCPC] shutdown");
}

void eDVB::restartSamba()
{
	int samba=1;
	eConfig::getInstance()->getKey("/elitedvb/network/samba", samba);
	if (samba != 0)
	{
		eDebug("[eDVB] restart Samba");
		if (fork() == 0)
		{
			for (unsigned int i=3; i < 90; ++i )
				close(i);
			system("killall nmbd smbd");
			system("smbd -D");
			system("nmbd -D");
			_exit(0);
		}
	}
}

void eDVB::configureNetwork()
{
#ifndef USE_IFUPDOWN
	__u32 sip=0, snetmask=0, sdns=0, sgateway=0;
	int ip[4], netmask[4], dns[4], gateway[4];
	int sdosetup=0, maxmtu=0, useDHCP=0;

	eConfig::getInstance()->getKey("/elitedvb/network/ip", sip);
	eConfig::getInstance()->getKey("/elitedvb/network/netmask", snetmask);
	eConfig::getInstance()->getKey("/elitedvb/network/dns", sdns);
	eConfig::getInstance()->getKey("/elitedvb/network/gateway", sgateway);
	eConfig::getInstance()->getKey("/elitedvb/network/dosetup", sdosetup);
	eConfig::getInstance()->getKey("/elitedvb/network/maxmtu", maxmtu);
	eConfig::getInstance()->getKey("/elitedvb/network/usedhcp", useDHCP);

	unpack(sip, ip);
	unpack(snetmask, netmask);
	unpack(sdns, dns);
	unpack(sgateway, gateway);

	FILE *file=fopen("/proc/mounts","r");
	if (!file)
		return;
	char buf[256];
	fgets(buf,sizeof(buf),file);
	while(fgets(buf,sizeof(buf),file))
	{
		if (strstr(buf, "/dev/root / nfs"))
		{
			eDebug("running via Network (nfs) .. do not configure network");
			fclose(file);
			return;
		}
	}
	fclose(file);

	if (sdosetup)
	{
		if ( useDHCP )
		{
			eDebug("[eDVB] use DHCP");
			if (udhcpc)
			{
				delete udhcpc;
				udhcpc = 0;
			}
			system("killall -9 udhcpc");
			FILE *file=fopen("/etc/hostname","r");
			eString cmd("/bin/udhcpc -f");
			if (!file)
				eDebug("couldn't get hostname.. /etc/hostname not exist");
			else
			{
				char buf[256];
				fgets(buf,sizeof(buf),file);
				fclose(file);

				struct stat s;
				if ( !stat("/var/share/udhcpc/default.script", &s) )
					cmd.sprintf("/bin/udhcpc --hostname=%s --foreground --script=/var/share/udhcpc/default.script", buf);
				else
					cmd.sprintf("/bin/udhcpc --hostname=%s --foreground --script=/share/udhcpc/default.script", buf);
			}
			udhcpc = new eConsoleAppContainer(cmd.c_str());
			CONNECT(udhcpc->dataAvail, eDVB::UDHCPC_DataAvail);
			CONNECT(udhcpc->appClosed, eDVB::UDHCPC_Closed);
		}
		else
		{
			if (udhcpc)
			{
				delete udhcpc;
				udhcpc=0;
			}

			FILE *f=fopen("/etc/resolv.conf", "wt");
			if (!f)
				eDebug("couldn't write resolv.conf");
			else
			{
				fprintf(f, "# Generated by enigma\nnameserver %d.%d.%d.%d\n", dns[0], dns[1], dns[2], dns[3]);
				fclose(f);
			}
			eString buffer;
			buffer.sprintf("/sbin/ifconfig eth0 %d.%d.%d.%d up netmask %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3], netmask[0], netmask[1], netmask[2], netmask[3]);
			if (system(buffer.c_str())>>8)
				eDebug("'%s' failed.", buffer.c_str());
			else
			{
				system("/sbin/route del default 2> /dev/null");
				buffer.sprintf("/sbin/route add default gw %d.%d.%d.%d", gateway[0], gateway[1], gateway[2], gateway[3]);
				if (system(buffer.c_str())>>8)
					eDebug("'%s' failed", buffer.c_str());
				if (maxmtu != 0)
				{
					buffer.sprintf("/sbin/ifconfig eth0 mtu %d", maxmtu);
					printf("[EDVB] configureNetwork: setting MAXMTU to %d\n", maxmtu);
					if (system(buffer.c_str())>>8)
						eDebug("'%s' failed", buffer.c_str());
				}
				eDebug("[eDVB] use IP: %d.%d.%d.%d, Netmask: %d.%d.%d.%d, gateway %d.%d.%d.%d, DNS: %d.%d.%d.%d",
					ip[0], ip[1],  ip[2], ip[3],
					netmask[0], netmask[1],  netmask[2], netmask[3],
					gateway[0], gateway[1], gateway[2], gateway[3],
					dns[0], dns[1], dns[2], dns[3]);
			}
			delayTimer.start(2000,true);
		}
	}
#if 0
	else
	{
		eDebug("[eDVB] disable Network!");
		system("/sbin/ifconfig eth0 down");
		system("killall -9 smbd nmbd");
	}
#endif

#endif // USE_IFUPDOWN
}
#endif
///////////////////////////////////////////////////////////////////////////////

eAutoInitP0<eDVB> init_dvb(eAutoInitNumbers::dvb, "eDVB lib");
