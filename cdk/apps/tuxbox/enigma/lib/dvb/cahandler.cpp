#include <unistd.h>
#include <string.h>
#include <iostream>
#include <stdint.h>

#include <lib/dvb/cahandler.h>
#include <lib/base/eerror.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/info.h>

ePMTClient::ePMTClient(eDVBCAHandler *handler, int socket) : eUnixDomainSocket(socket, 1, eApp), parent(handler)
{
	// std::cout << "[PMTClient] setup start socket: " << std::hex << socket << std::endl;
	receivedTag[0] = 0;
	receivedLength = 0;
	receivedValue = NULL;
	displayText = NULL;
	CONNECT(connectionClosed_, ePMTClient::connectionLost);
	CONNECT(readyRead_, ePMTClient::dataAvailable);
	// std::cout << "[PMTClient] setup done" << std::endl;
}

void ePMTClient::connectionLost()
{
	// std::cout << "[PMTClient] connectionLost fomr on old interface server" << std::endl;
	if (parent) parent->connectionLost(this);
}

void ePMTClient::dataAvailable()
{
	// std::cout << "[PMTClient] dataAvailable: " <<  std::dec << bytesAvailable() << " bytes" << std::endl;
	/* this handler might be called multiple times (by the socket notifier), till we have the complete message */
	while (1)
	{
		if (!receivedTag[0])
		{
			if (bytesAvailable() < 4) return;
			/* read the tag (3 bytes) + the first byte of the length */
			if(readBlock((char*)receivedTag, 4) < 4) return;
			receivedLength = 0;
						// std::cout << "[PMTClient] dataAvailable: header  |" <<  std::hex << (int) receivedTag[0] << "-" << std::hex << (int) receivedTag[1] << "-" << std::hex << (int) receivedTag[2] << "-" << std::hex << (int) receivedTag[3] << "|" <<std::endl;
		}
		if (receivedTag[0] && !receivedLength)
		{
			if (receivedTag[3] & 0x80)
			{
				/* multibyte length field */
				unsigned char lengthdata[128];
				int lengthdatasize;
				int i;
				lengthdatasize = receivedTag[3] & 0x7f;
				if (bytesAvailable() < lengthdatasize) return;
				if (readBlock((char*)lengthdata, lengthdatasize) < lengthdatasize) return;
				for (i = 0; i < lengthdatasize; i++)
				{
					receivedLength = (receivedLength << 8) | lengthdata[i];
				}
			}
			else
			{
				/* singlebyte length field */
				receivedLength = receivedTag[3] & 0x7f;
			}
						// std::cout << "[PMTClient] dataAvailable: expecting " << std::dec << receivedLength << " bytes for value" << std::endl;
		}

		if (receivedLength)
		{
			// std::cout << "[PMTClient] dataAvailable: have " <<  std::dec << bytesAvailable() << " value bytes" << std::endl;
			if (bytesAvailable() < receivedLength) return;
			if (receivedValue) delete [] receivedValue;
			receivedValue = new unsigned char[receivedLength];
			if (readBlock((char*)receivedValue, receivedLength) < receivedLength) return;
		}

		if (receivedValue)
		{
			// std::cout << "[PMTClient] dataAvailable:  message complete" << std::endl;
			/* the client message is complete, handle it */
			clientTLVReceived(receivedTag, receivedLength, receivedValue);
			/* prepare for a new message */
			delete [] receivedValue;
			receivedValue = NULL;
			receivedTag[0] = 0;
		}
	}
}

void ePMTClient::parseTLVObjects(unsigned char *data, int size)
{
	// std::cout << "[PMTClient] parseTLVObjects: parsing message of " << size << " bytes" << std::endl;
	/* parse all tlv objects in the buffer */
	while (1)
	{
		int length = 0;
		int lengthdatasize = 0;
		size -= 4;
		if (size <= 0) break;
		if (data[3] & 0x80)
		{
			/* multibyte length field */
			unsigned char lengthdata[128];
			int i;
			lengthdatasize = data[3] & 0x7f;
			size -= lengthdatasize;
			if (size <= 0) break;
			for (i = 0; i < lengthdatasize; i++)
			{
				length = (length << 8) | lengthdata[i];
			}
		}
		else
		{
			/* singlebyte length field */
			length = data[3] & 0x7f;
		}
		if (size < length) break;

		/* we have a complete TLV object, handle it */
		clientTLVReceived(data, length, &data[3 + lengthdatasize]);
	}
}

void ePMTClient::clientTLVReceived(unsigned char *tag, int length, unsigned char *value)
{
	// std::cout << "[ePMTClient] clientTLVReceived: len=" << length << " ";
	// for(int i=0;i<length;i++)
	// 	std::cout << "-" << value[i];
	// std::cout << "|" << std::endl;

	if (tag[0] != 0x9F) return; /* unknown command class */

	switch (tag[1])
	{
	case 0x80: /* application / CA / resource */
		switch (tag[2])
		{
		case 0x31: /* ca_info */
			
			break;
		case 0x33: /* ca_pmt_reply */
			/* currently not used */
			break;
		}
		break;
	case 0x84: /* host control / datetime */
		/* currently not used */
		break;
	case 0x88: /* MMI */
		switch (tag[2])
		{
		case 0x10: /* display message */
			/* display message contains several TLV objects (we're interested in the text objects) */
			if (displayText) delete [] displayText;
			displayText = new char[length];
			parseTLVObjects(value, length);
			/* TODO: display the message */
			if (displayText)
			{
				delete [] displayText;
				displayText = NULL;
			}
			break;
		case 0x04: /* text more */
			if (displayText)
			{
				strncat(displayText, (const char*)value, length);
				strncat(displayText, "\n", 1);
			}
			break;
		case 0x05: /* text last */
			if (displayText)
			{
				strncat(displayText, (const char*)value, length);
			}
			break;
		default:
			break;
		}
		break;
	case 0x8C: /* comms */
		/* currently not used */
		break;
	case 0x70: /* custom */
		switch (tag[2])
		{
		case 0x10: /* client name */
			parent->clientname((const char*)value);
			break;
		case 0x20: /* client info */
			parent->clientinfo((const char*)value);
			break;
		case 0x21: /* used caid */
			if (length == sizeof(int32_t))
			{
				parent->usedcaid(ntohl(*(int32_t*)value));
			}
			break;
		case 0x22: /* verbose info */
			parent->verboseinfo((const char*)value);
			break;
		case 0x23: /* decode time (ms) */
			if (length == sizeof(int32_t))
			{
				parent->decodetime(ntohl(*(int32_t*)value));
			}
			break;
		case 0x24: /* used cardid */
			parent->usedcardid((const char*)value);
			break;
		default: /* unknown */
			break;
		}
		break;
	default: /* unknown command group */
		break;
	}
}

eDVBCAHandler *eDVBCAHandler::instance = NULL;

eDVBCAHandler::eDVBCAHandler()
 : eServerSocket(PMT_SERVER_SOCKET, eApp), serviceLeft(eApp)
{
	instance = this;
	// std::cout << "[DVBCAHandler] new instance" << std::endl;
	services.setAutoDelete(true);
	clients.setAutoDelete(true);
	CONNECT(serviceLeft.timeout, eDVBCAHandler::serviceGone);
	eDVBCaPMTClientHandler::registerCaPMTClient(this);  // static method...
}

eDVBCAHandler::~eDVBCAHandler()
{
	// std::cout << "[DVBCAHandler] instance destroyed" << std::endl;
	eDVBCaPMTClientHandler::unregisterCaPMTClient(this);  // static method...
}

void eDVBCAHandler::newConnection(int socket)
{
	// std::cout << "[DVBCAHandler] newConnection socket: " << std::hex << socket << std::endl;
	ePMTClient *client = new ePMTClient(this, socket);
	clients.push_back(client);

	/* inform the new client about our current services, if we have any */
	distributeCAPMT();
}

void eDVBCAHandler::connectionLost(ePMTClient *client)
{
	// std::cout << "[DVBCAHandler] connectionLost from new interface client" << std::endl;
	ePtrList<ePMTClient>::iterator it = std::find(clients.begin(), clients.end(), client );
	if (it != clients.end())
	{
		clients.erase(it);
	}
}

void eDVBCAHandler::enterService( const eServiceReferenceDVB &service )
{
	// std::cout << "[DVBCAHandler] enterService" << std::endl;
	ePtrList<CAService>::iterator it =
		std::find(services.begin(), services.end(), service );
	if ( it == services.end() )
	{
		serviceLeft.stop();
		services.push_back(new CAService( service ));
	}

	/*
	 * our servicelist has changed, but we have to wait till we receive PMT data
	 * for this service, before we distribute a new list of CAPMT objects to our clients.
	 */
}

void eDVBCAHandler::serviceGone()
{
	std::cout << "[DVBCAHandler] serviceGone" << std::endl;
	if (!services.size())
	{
		std::cout << "[DVBCAHandler] no more services" << std::endl;
		clients.clear();
	}
}
 
void eDVBCAHandler::leaveService( const eServiceReferenceDVB &service )
{
	// std::cout << "[DVBCAHandler] leaveService" << std::endl;
	ePtrList<CAService>::iterator it =
		std::find(services.begin(), services.end(), service );
	if ( it != services.end() )
	{
		serviceLeft.startLongTimer(2);
		services.erase(it);
	}

	usedcaid(0);
	/* our servicelist has changed, distribute the list of CAPMT objects to all our clients */
	distributeCAPMT();
}

void eDVBCAHandler::distributeCAPMT()
{
	/*
	 * write the list of CAPMT objects to each connected client, if it's not empty
	 */
	// std::cout << "[DVBCAHandler] distrinuteCAPMT" << std::endl;
	if (services.empty()) return;

	ePtrList<ePMTClient>::iterator client_it = clients.begin();
	for ( ; client_it != clients.end(); ++client_it)
	{
		if (client_it->state() == eSocket::Connection)
		{
			unsigned char list_management = LIST_FIRST;
			ePtrList<CAService>::iterator it = services.begin();
			for ( ; it != services.end(); )
			{
				CAService *current = it;
				++it;
				if (it == services.end()) list_management |= LIST_LAST;
				current->writeCAPMTObject(*client_it, list_management);
				list_management = LIST_MORE;
			}
		}
	}
}

void eDVBCAHandler::handlePMT( const eServiceReferenceDVB &service, PMT *pmt )
{
	bool isUpdate;

	// std::cout << "[DVBCAHandler] handlePMT" << std::endl;
	ePtrList<CAService>::iterator it =
		std::find(services.begin(), services.end(), service );
	if (it != services.end())
	{
		/* we found the service in our list */
		if (it->getCAPMTVersion() == pmt->version)
		{
			eDebug("[eDVBCAHandler] dont send the self pmt version");
			return;
		}
		
		isUpdate = (it->getCAPMTVersion() >= 0);

		/* prepare the data */
		it->buildCAPMT(pmt);

		/* send the data to the listening client */
		it->sendCAPMT();

		if (isUpdate)
		{
			/* this is a PMT update, we should distribute the new CAPMT object to all our connected clients */
			ePtrList<ePMTClient>::iterator client_it = clients.begin();
			for ( ; client_it != clients.end(); ++client_it)
			{
				if (client_it->state() == eSocket::Connection)
				{
					it->writeCAPMTObject(*client_it, LIST_UPDATE);
				}
			}
		}
		else
		{
			/*
			 * this is PMT information for a new service, so we can now distribute
			 * the CAPMT objects to all our connected clients
			 */
			distributeCAPMT();
		}
	}
}
 
CAService::CAService( const eServiceReferenceDVB &service )
	: eUnixDomainSocket(eApp), lastPMTVersion(-1), me(service), capmt(NULL), retry(eApp)
{
	int socketReconnect = 0;
	eConfig::getInstance()->getKey("/elitedvb/extra/cahandlerReconnect", socketReconnect);
	if (socketReconnect)
	{
		CONNECT(connectionClosed_, CAService::connectionLost);
	}
	CONNECT(retry.timeout, CAService::sendCAPMT);
//		eDebug("[eDVBCAHandler] new service %s", service.toString().c_str() );
}

void CAService::connectionLost()
{
	// std::cout << "[CAService] connectionLost" << std::endl;
	/* reconnect in 1s */
	retry.startLongTimer(1);
}

void CAService::buildCAPMT( PMT *pmt )
{
	// std::cout << "[CAService] buildCAPMT" << std::endl;
	if ( !capmt )
		capmt = new unsigned char[1024];

	memcpy(capmt,"\x9f\x80\x32\x82\x00\x00", 6);

	capmt[6]=lastPMTVersion==-1 ? LIST_ONLY : LIST_UPDATE;
	capmt[7]=(unsigned char)((pmt->program_number>>8) & 0xff);			//prg-nr
	capmt[8]=(unsigned char)(pmt->program_number & 0xff);					//prg-nr

	capmt[9]=pmt->version;	//reserved - version - current/next
	capmt[10]=0x00;	//reserved - prg-info len
	capmt[11]=0x00;	//prg-info len

	capmt[12]=CMD_OK_DESCRAMBLING;  // ca pmt command id
	capmt[13]=0x81;  // private descr.. dvbnamespace
	capmt[14]=0x08;
	capmt[15]=me.getDVBNamespace().get()>>24;
	capmt[16]=(me.getDVBNamespace().get()>>16)&0xFF;
	capmt[17]=(me.getDVBNamespace().get()>>8)&0xFF;
	capmt[18]=me.getDVBNamespace().get()&0xFF;
	capmt[19]=me.getTransportStreamID().get()>>8;
	capmt[20]=me.getTransportStreamID().get()&0xFF;
	capmt[21]=me.getOriginalNetworkID().get()>>8;
	capmt[22]=me.getOriginalNetworkID().get()&0xFF;

	capmt[23]=0x82;  // demuxer kram..
	capmt[24]=0x02;

	switch(eSystemInfo::getInstance()->getHwType())
	{
		case eSystemInfo::DM7000:
		case eSystemInfo::DM7020:
			capmt[25]=0x03;  // descramble on demux0 and demux1
			capmt[26]=0x01;  // get section data from demux1
			break;
		case eSystemInfo::DM500PLUS: /* DM500 only has one demux */
		case eSystemInfo::DM600PVR: /* DM600 only has one demux */
			capmt[25]=0x01;  // descramble on demux0 // demux 1 is just a fake demux 0
			capmt[26]=0x01;  // get section data from demux1
			break;
		default:
			capmt[25]=0x01;  // only descramble on demux0
			capmt[26]=0x00;  // get section data from demux0
			break;
	}

	capmt[27]=0x84;  // pmt pid
	capmt[28]=0x02;
	capmt[29]=pmt->pid>>8;
	capmt[30]=pmt->pid&0xFF;

	lastPMTVersion=pmt->version;
	int lenpos=10;
	int len=19;
	int first=0;
	int wp=31;

	// program_info
	for (ePtrList<Descriptor>::const_iterator i(pmt->program_info);
		i != pmt->program_info.end(); ++i)
	{
		if (i->Tag()==9)	// CADescriptor
		{
			CADescriptor *ca=(CADescriptor*)*i;
			memcpy(capmt+wp, ca->data, ca->data[1]+2);
			wp+=ca->data[1]+2;
			len+=ca->data[1]+2;
		}
	}

	for (ePtrList<PMTEntry>::iterator i(pmt->streams); i != pmt->streams.end(); ++i)
	{
		PMTEntry *pe=*i;

		first=1;
		capmt[lenpos]=((len & 0xf00)>>8);
		capmt[lenpos+1]=(len & 0xff);
		len=0;
		lenpos=wp+3;
		first=1;
		capmt[wp++]=(pe->stream_type & 0xffff);
		capmt[wp++]=((pe->elementary_PID >> 8) & 0xff);
		capmt[wp++]=(pe->elementary_PID & 0xff);
		wp+=2;

		switch (pe->stream_type)
		{
			case 1: // ISO/IEC 11172 Video
			case 2: // ITU-T Rec. H.262 | ISO/IEC 13818-2 Video or ISO/IEC 11172-2 constrained parameter video stream
			case 3: // ISO/IEC 11172 Audio
			case 4: // ISO/IEC 13818-3 Audio
			case 0x80:
			case 0x81:
			case 0x82:
			case 0x83:
			case 6: // private stream ( ttx or AC3 or DTS )
				for (ePtrList<Descriptor>::const_iterator i(pe->ES_info);
					i != pe->ES_info.end(); ++i)
				{
					if (i->Tag()==9)	// CADescriptor
					{
						CADescriptor *ca=(CADescriptor*)*i;
						if(first)
						{
							first=0;
							capmt[wp++]=0x01;				//ca_pmt_command_id
							len++;
						}
						memcpy(capmt+wp, ca->data, ca->data[1]+2);
						wp+=ca->data[1]+2;
						len+=ca->data[1]+2;
					}
				}
			default:
				break;
		}
	}
	capmt[lenpos]=((len & 0xf00)>>8);
	capmt[lenpos+1]=(len & 0xff);

	capmt[4]=((wp-6)>>8) & 0xff;
	capmt[5]=(wp-6) & 0xff;

#if 0
		for(int i=0;i<wp;i++)
			eDebugNoNewLine("%02x ",capmt[i]);
		eDebug("");
#endif
}

void CAService::sendCAPMT()
{
#if 0
	std::cout << "[CAService] sendCAPMT - state " ;
	switch (state()) {
 		case Invalid:
			std::cout << "Invalid" << std::endl;
			break;
		case Idle:
			std::cout << "Idle" << std::endl;
			break;
		case HostLookup:
			std::cout << "Hostlookup" << std::endl;
			break;
		case Connecting:
			std::cout << "Connecting" << std::endl;
			break;
		case Listening:
			std::cout << "Listening" << std::endl;
			break;
		case Connection:
			std::cout << "Connection" << std::endl;
			break;
		case Closing:
			std::cout << "Closing" << std::endl;
			break;
		default:
			std::cout << "Unknown-out of enum????" << std::endl;
			break;
	}
#endif
 	if (state() == Idle || state() == Invalid)
 	{
 		/* we're not connected yet */
		// std::cout << "[CAService] sendCAPMT - connecting to " << PMT_CLIENT_SOCKET << std::endl;
 		connectToPath(PMT_CLIENT_SOCKET);
 	}
 	
 	if (state() == Connection)
 	{
		writeCAPMTObject(this, LIST_ONLY);
 	}
 	else
 	{
		/* we're not connected, try again in 5s */
		retry.startLongTimer(5);
	}
}

int CAService::writeCAPMTObject(eSocket *socket, int list_management)
{
	int wp;
	// std::cout << "[CAService] writeCAPMTObject: socket: " << std::hex << *(long int *)socket << std::endl;
	if (!capmt) return 0;

	if (list_management >= 0) capmt[6] = (unsigned char)list_management;

	wp = capmt[4] << 8;
	wp |= capmt[5];
	wp+=6;

	return socket->writeBlock((const char*)capmt, wp);
}

eAutoInitP0<eDVBCAHandler> init_eDVBCAHandler(eAutoInitNumbers::osd-2, "eDVBCAHandler");
