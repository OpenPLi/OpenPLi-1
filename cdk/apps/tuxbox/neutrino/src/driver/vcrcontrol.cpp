/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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
#include <sys/un.h>
#include <string.h>

#include <global.h>
#include <neutrinoMessages.h>

#include <gui/widget/messagebox.h>

#include "vcrcontrol.h"


#define SA struct sockaddr
#define SAI struct sockaddr_in

#define LIRCDIR "/var/tuxbox/config/"


CVCRControl* CVCRControl::getInstance()
{
	static CVCRControl* vcrControl = NULL;

	if(!vcrControl)
	{
		vcrControl = new CVCRControl();
		printf("[neutrino] vcrControl Instance created\n");
	}
	else
	{
		//printf("[neutrino] vcrControl Instace requested\n");
	}
	return vcrControl;
}

//-------------------------------------------------------------------------
CVCRControl::CVCRControl()
{
	Devices.clear();
}

//-------------------------------------------------------------------------
CVCRControl::~CVCRControl()
{
	if(Devices.size() > 0)
	{
		for(CDeviceMap::iterator pos = Devices.begin();pos != Devices.end();pos++)
		{
			delete pos->second;
			Devices.erase(pos->first);
		}
	}

}
//-------------------------------------------------------------------------

void CVCRControl::setDeviceOptions(int deviceID, CDeviceInfo *deviceInfo)
{
	if(Devices[deviceID]->deviceType == DEVICE_SERVER)
	{
		CServerDevice * device = (CServerDevice *) Devices[deviceID];     
		CServerDeviceInfo *serverinfo = (CServerDeviceInfo *) deviceInfo;
		if(serverinfo->Name.length() > 0)
			device->Name = serverinfo->Name;
		if(serverinfo->ServerAddress.length() > 0)
			device->ServerAddress = serverinfo->ServerAddress;
		if(serverinfo->ServerPort > 0)
			device->ServerPort = serverinfo->ServerPort;
		device->StopPlayBack = serverinfo->StopPlayBack;
		device->StopSectionsd = serverinfo->StopSectionsd;
	}
}
//-------------------------------------------------------------------------

int CVCRControl::registerDevice(CVCRDevices deviceType, CDeviceInfo *deviceInfo)
{
	static int i = 0;
	if(deviceType == DEVICE_SERVER)
	{
		CServerDevice * device =  new CServerDevice(i++);		
		Devices[device->deviceID] = (CDevice*) device;
		setDeviceOptions(device->deviceID,deviceInfo);
		printf("CVCRControl registered new serverdevice: %u %s\n",device->deviceID,device->Name.c_str());
		return device->deviceID;
	}
	else if(deviceType == DEVICE_VCR)
	{
		CVCRDevice * device = new CVCRDevice(i++);
		device->Name = deviceInfo->Name;
		Devices[device->deviceID] = (CDevice*) device;
		printf("CVCRControl registered new vcrdevice: %u %s\n",device->deviceID,device->Name.c_str());
		return device->deviceID;
	}
	else
		return -1;
}



//-------------------------------------------------------------------------
void CVCRControl::CVCRDevice::IRDeviceDisconnect()
{
	close(sock_fd);
}

//-------------------------------------------------------------------------
bool CVCRControl::CVCRDevice::sendCommand(std::string command, const t_channel_id channel_id, unsigned long long epgid, uint apid)
{
	if(IRDeviceConnect())
	{
		std::stringstream ostr;
		ostr << "SEND_ONCE " << Name << " " << command << std::endl << std::ends;
		write(sock_fd, ostr.str().c_str(), ostr.str().length());
		IRDeviceDisconnect();
		return true;
	}
	else
		return false;
}
//-------------------------------------------------------------------------
bool CVCRControl::CVCRDevice::IRDeviceConnect()
{
	struct sockaddr_un addr;

	addr.sun_family=AF_UNIX;
	strcpy(addr.sun_path, "/dev/lircd");
	sock_fd = socket(AF_UNIX,SOCK_STREAM,0);
	if(!sock_fd)
	{
		printf("could not open lircd-socket\n");
		return false;
	};

	if(!connect(sock_fd,(struct sockaddr *)&addr,sizeof(addr))==-1)
	{
		printf("could not connect to lircd-socket\n");
		return false;
	};
	return true;

}
//-------------------------------------------------------------------------
bool CVCRControl::CVCRDevice::Stop()
{
	deviceState = CMD_VCR_STOP;
	// leave scart mode
	g_RCInput->postMsg( NeutrinoMessages::VCR_OFF, 0 );
	return ParseFile(LIRCDIR "stop.lirc");
}

//-------------------------------------------------------------------------
bool CVCRControl::CVCRDevice::ParseFile(string filename)
{
FILE *inp;
char buffer[101];
char outbuffer[200];
int wait_time;

	if( (inp = fopen(filename.c_str(),"r")) > 0)
	{
		IRDeviceConnect();
		std::stringstream ostr;

		while(!feof(inp))
		{
			if(fgets(buffer,100,inp) != NULL)
			{
				if(strncmp(buffer,"WAIT",4) == 0)		
				{			// if wait command then sleep for n seconds
					sscanf(&buffer[4],"%d",&wait_time);
					if(wait_time > 0)
						sleep(wait_time);
				}
				else
				{
					sprintf(outbuffer,"SEND_ONCE %s %s \n",Name.c_str(),buffer);
					printf("lirc send line: '%s'\n",outbuffer);
	//				ostr << "SEND_ONCE " << Name << " " << buffer << std::endl << std::ends;
					write(sock_fd, outbuffer, strlen(outbuffer)+1);
				}
			}
		}
		IRDeviceDisconnect();
		return true;
	}
	else
		printf("konnte datei %s nicht oeffnen\n",filename.c_str());
	return false;
}
//-------------------------------------------------------------------------
bool CVCRControl::CVCRDevice::Record(const t_channel_id channel_id, unsigned long long epgid, uint apid)
{
	if(channel_id != 0)		// wenn ein channel angegeben ist
	{
		if(g_Zapit->getCurrentServiceID() != channel_id)	// und momentan noch nicht getuned ist
		{
			g_Zapit->zapTo_serviceID(channel_id);		// dann umschalten
		}
	}
	if(apid !=0) //selbiges für apid
	{
 		CZapitClient::CCurrentServiceInfo si = g_Zapit->getCurrentServiceInfo ();
		if(si.apid != apid)
		{
			printf("Setting Audio channel to %x\n",apid);
			g_Zapit->setAudioChannel(apid);
		}
	}
	deviceState = CMD_VCR_RECORD;
	// switch to scart mode
	g_RCInput->postMsg( NeutrinoMessages::VCR_ON, 0 );
/*
sendCommand(POWER
CONTROL Wait 1
IR Send CH1
CONTROL Wait 1
IR Send CH1
CONTROL Wait 1
IR Send CH0
CONTROL Wait 1
IR Send RECOTR
*/
	return ParseFile(LIRCDIR "record.lirc");
}

//-------------------------------------------------------------------------
bool CVCRControl::CVCRDevice::Pause()
{
	return ParseFile(LIRCDIR "pause.lirc");
}

//-------------------------------------------------------------------------
bool CVCRControl::CVCRDevice::Resume()
{
	return ParseFile(LIRCDIR "resume.lirc");
}

//-------------------------------------------------------------------------
bool CVCRControl::CServerDevice::Stop()
{
	printf("Stop\n"); 
	if(!g_Zapit->isPlayBackActive())
		g_Zapit->startPlayBack();
	g_Sectionsd->setPauseScanning(false);
	g_Zapit->setRecordMode( false );
	if(sendCommand(CMD_VCR_STOP))
		return true;
	else
		return false;
}

//-------------------------------------------------------------------------
bool CVCRControl::CServerDevice::Record(const t_channel_id channel_id, unsigned long long epgid, uint apid) 
{
	printf("Record channel_id: %x epg: %llx, apid %u\n", channel_id, epgid, apid);
	if(channel_id != 0)		// wenn ein channel angegeben ist
		if(g_Zapit->getCurrentServiceID() != channel_id)	// und momentan noch nicht getuned ist
		{
			g_Zapit->zapTo_serviceID(channel_id);		// dann umschalten
		}

	if(StopPlayBack && g_Zapit->isPlayBackActive())	// wenn playback gestoppt werden soll und noch läuft
		g_Zapit->stopPlayBack();					// dann playback stoppen

	if(StopSectionsd)								// wenn sectionsd gestoppt werden soll
		g_Sectionsd->setPauseScanning(true);		// sectionsd stoppen

	g_Zapit->setRecordMode( true );					// recordmode einschalten

	if(!sendCommand(CMD_VCR_RECORD,channel_id,epgid,apid))
	{
		if(!g_Zapit->isPlayBackActive())			// wenn command nicht gesendet werden konnte
			g_Zapit->startPlayBack();				// dann alles rueckgaengig machen
		g_Sectionsd->setPauseScanning(false);
		g_Zapit->setRecordMode( false );

		ShowMsg ( "messagebox.info", g_Locale->getText("streamingserver.noconnect"), CMessageBox::mbrBack, CMessageBox::mbBack, "error.raw" );

		return false;
	}
	else
		return true;
}


//-------------------------------------------------------------------------
void CVCRControl::CServerDevice::serverDisconnect()
{
	close(sock_fd);
}

//-------------------------------------------------------------------------
bool CVCRControl::CServerDevice::sendCommand(CVCRCommand command, const t_channel_id channel_id, unsigned long long epgid, uint apid)
{
	printf("Send command: %d channel_id: %x epgid: %llx\n",command, channel_id, epgid);
	if(serverConnect())
	{
		char tmp[40];
		string extCommand="unknown";
		string ext_channel_id = "error";
		string ext_channel_name = "unknown";
		string extEpgid="error";
		string extVideoPID="error";
		string extAudioPID="error";
		string extEPGTitle="not available";
		sprintf(tmp,"%u", channel_id);
		ext_channel_id = tmp;
		sprintf(tmp,"%llu", epgid);
		extEpgid = tmp;
//		sprintf(tmp,"%u", g_RemoteControl->current_PIDs.PIDs.vpid );
		CZapitClient::responseGetPIDs pids;
		g_Zapit->getPIDS (pids);
 		CZapitClient::CCurrentServiceInfo si = g_Zapit->getCurrentServiceInfo ();
		sprintf(tmp,"%u", si.vdid );
		extVideoPID = tmp;
//		sprintf(tmp,"%u", g_RemoteControl->current_PIDs.APIDs[g_RemoteControl->current_PIDs.PIDs.selected_apid].pid);
		if(apid==0)
			apid=(uint)si.apid;
		sprintf(tmp,"%u", apid);
		extAudioPID = tmp;

		CZapitClient::BouquetChannelList channellist;     
		g_Zapit->getChannels(channellist);
		CZapitClient::BouquetChannelList::iterator channel = channellist.begin();
		for(; channel != channellist.end();channel++)
		{
			if(channel->channel_id==channel_id)
			{
				ext_channel_name=channel->name;
				break;
			}
		}

		CSectionsdClient sections;
		CSectionsdClient::responseGetCurrentNextInfoChannelID current_next;
		if(sections.getCurrentNextServiceKey(channel_id, current_next))
		{
			extEPGTitle=current_next.current_name;
		}

		switch(command)
		{
			case CMD_VCR_RECORD: extCommand="record";
				break;
			case CMD_VCR_STOP: extCommand="stop";
				break;
			case CMD_VCR_PAUSE: extCommand="pause";
				break;
			case CMD_VCR_RESUME: extCommand="resume";
				break;
			case CMD_VCR_AVAILABLE: extCommand="available";
				break;
			case CMD_VCR_UNKNOWN:
			default:
				printf("CVCRControl: Unknown Command\n");
		}

		string extMessage = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n\n";
		extMessage +="<neutrino commandversion=\"1\">\n";
		extMessage +="    <record command=\"" + extCommand + "\">\n";
		extMessage +="        <channelname>" + ext_channel_name + "</channelname>\n";
		extMessage +="        <epgtitle>" + extEPGTitle + "</epgtitle>\n";
		extMessage +="        <onidsid>" + ext_channel_id + "</onidsid>\n";
		extMessage +="        <epgid>" + extEpgid + "</epgid>\n";
		extMessage +="        <videopid>" + extVideoPID + "</videopid>\n";
		extMessage +="        <audiopids selected=\"" + extAudioPID + "\">\n";
		// super hack :-), der einfachste weg an die apid descriptions ranzukommen
		g_RemoteControl->current_PIDs = pids;
		g_RemoteControl->processAPIDnames();
		bool apidFound=false;
		for(unsigned int i= 0; i< pids.APIDs.size(); i++)
		{
			sprintf(tmp, "%u",  pids.APIDs[i].pid );
			extMessage +="            <audio pid=\"" + string(tmp) + "\" name=\"" + string(g_RemoteControl->current_PIDs.APIDs[i].desc)  + "\">\n";
			if(pids.APIDs[i].pid==apid)
				apidFound=true;
		}
		if(!apidFound)
		{
			// add spec apid to available
			extMessage +="            <audio pid=\"" + extAudioPID + "\" name=\"" + extAudioPID  + "\"/>\n";
		}
		extMessage +="        </audiopids>\n";
		extMessage +="    </record>\n";
		extMessage +="</neutrino>\n";

		printf("sending to vcr-client:\n\n%s\n", extMessage.c_str());
		write(sock_fd, extMessage.c_str() , extMessage.length() );

		serverDisconnect();

		deviceState = command;
		return true;
	}
	else
		return false;

}

//-------------------------------------------------------------------------
bool CVCRControl::CServerDevice::serverConnect()
{

	printf("connect to server: %s:%d\n",ServerAddress.c_str(),ServerPort);

	sock_fd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SAI servaddr;
	memset(&servaddr,0,sizeof(SAI));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(ServerPort);
	inet_pton(AF_INET, ServerAddress.c_str(), &servaddr.sin_addr);


	if(connect(sock_fd, (SA *)&servaddr, sizeof(servaddr))==-1)
	{
		perror("[cvcr] -  cannot connect to streamingserver\n");
		return false;
	}

	return true;
}
//-------------------------------------------------------------------------
