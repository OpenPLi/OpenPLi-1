#ifndef DISABLE_CI
#include <lib/dvb/dvbci.h>
#include <lib/dvb/dvbservice.h>
#include <lib/dvb/si.h>
#include <lib/base/i18n.h>
#include <lib/system/init.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

//external defines (because missing dreambox header-files in cvs)
#define CI_RESET					0
#define CI_SOFTRESET			1
#define CI_GET_BUFFERSIZE	2
#define CI_GET_STATUS			3
#define CI_TS_ACTIVATE		6
#define CI_TS_DEACTIVATE	7

#define MAX_SESSIONS			32
#define STATE_FREE				0
#define STATE_OPEN				1

int eDVBCI::instance_count=0;

eDVBCI::eDVBCI()
	:pollTimer(this), caidcount(0), ml_bufferlen(0), messages(this, 1)
{
	init_eDVBCI();
}
void eDVBCI::init_eDVBCI()
{
	instance_count++;

	state=stateInit;

	eDebug("[DVBCI] start");

	if (instance_count == 1)
		fd=::open("/dev/ci",O_RDWR|O_NONBLOCK);
	else
		fd=::open("/dev/ci1",O_RDWR|O_NONBLOCK);
		
	if (fd<0)
	{
		if (instance_count == 1)
			eDebug("[DVBCI] error opening /dev/ci");
		else
			eDebug("[DVBCI] error opening /dev/ci1");

		state=stateError;
	}
	
	if(state!=stateError)
	{
		ci=new eSocketNotifier(this, fd, eSocketNotifier::Read, 0);
		CONNECT(ci->activated, eDVBCI::dataAvailable);
	}
 
	CONNECT(pollTimer.timeout,eDVBCI::poll);

	CONNECT(messages.recv_msg, eDVBCI::gotMessage);

	strcpy(appName, _("no module"));
	tempPMTentrys=0;

	for (int i=0; i < MAXTRANSPORTSESSIONS; ++i)
	{
		lpduReceiveQueues[i].numLPDUS=0;
		lpduReceiveQueues[i].firstLPDU=NULL;
	}

	run();
}

void eDVBCI::thread()
{
	exec();
	pollTimer.stop();
}

eDVBCI::~eDVBCI()
{
	messages.send(eDVBCIMessage(eDVBCIMessage::exit));
	kill();
	delete ci;
	if (fd >= 0)
		close(fd);
	instance_count--;
}

void eDVBCI::gotMessage(const eDVBCIMessage &message)
{
	switch (message.type)
	{
	case eDVBCIMessage::start:
		if (state == stateInit)
		{
			ci->start();
			ci_state=0;
		}
		dataAvailable(0);
		break;
	case eDVBCIMessage::reset:
//		eDebug("[DVBCI] got reset message..");
		while(sendqueue.size())
		{
			delete [] sendqueue.top().data;
			sendqueue.pop();
		}
		if(!ci_state)
		{
			strcpy(appName,_("no module"));
			ci_progress(appName);
		}
		ci_state=0;
		clearCAIDs();
//		if (::ioctl(fd,CI_RESET)<0 )
//			eDebug("CI_RESET failed (%m)");
		dataAvailable(1000);  // force reset
		break;
	case eDVBCIMessage::init:
//		eDebug("[DVBCI] got init message..");
		if(ci_state)
		{
			versions.clear();
			newService();
		}
		else
		{
			strcpy(appName,_("no module"));
			ci_progress(appName);
		}
		break;
	case eDVBCIMessage::go:
		newService();
		break;
	case eDVBCIMessage::mmi_begin:
//		eDebug("[DVBCI] got mmi_begin message..");
		mmi_begin();
		break;
	case eDVBCIMessage::mmi_end:
//		eDebug("[DVBCI] got mmi_end message..");
		mmi_end();
		break;
	case eDVBCIMessage::mmi_enqansw:
//		eDebug("[DVBCI] got mmi_answ message..");
		mmi_enqansw(message.data);
		delete[] message.data;
		break;
	case eDVBCIMessage::mmi_menuansw:
//		eDebug("[DVBCI] got mmi_menu_answ message..");
		mmi_menuansw((int)message.sid);
		break;
	case eDVBCIMessage::exit:
//		eDebug("[DVBCI] got exit message..");
		quit();
		break;
	case eDVBCIMessage::getcaids:
//		eDebug("[DVBCI] got getcaids message..");
		pushCAIDs();
		break;
	case eDVBCIMessage::PMTflush:
//		eDebug("[DVBCI] got PMTflush message..");
		PMTflush(message.sid);
		break;
	case eDVBCIMessage::PMTaddPID:
//		eDebug("[DVBCI] got PMTaddPID message..");
//		eDebug("addPID %04x, type %02x", message.pid, message.streamtype );
		PMTaddPID(message.sid,message.pid,message.streamtype);
		break;
	case eDVBCIMessage::PMTsetVersion:
		PMTsetVersion(message.sid, message.pid);
		break;
	case eDVBCIMessage::PMTaddDescriptor:
//		eDebug("[DVBCI] got PMTaddDescriptor message..");
//		eDebug("addDescr len %02x", message.data[1]+2);
//		for (int i=0; i < message.data[1]+2; ++i)
//			eDebugNoNewLine("%02x ", message.data[i]);
//		eDebug("");
		PMTaddDescriptor(message.sid, message.data);
		break;
	case eDVBCIMessage::getAppName:
		ci_progress(appName);
		break;
/*
// disabled... see dvbservice.cpp for more info
    
	case eDVBCIMessage::enable_ts:
	{
		int present=0;

		stopTimer();

		if (::ioctl(fd,CI_GET_STATUS,&present)<0)
		{
			eDebug("CI_GET_STATUS failed (%m)");
			break;
		}

		if( present )
		{
			if ( ::ioctl(fd, CI_TS_ACTIVATE) < 0 )
				eDebug("CI_TS_ACTIVATE failed (%m)");
		}
		break;
	}
	case eDVBCIMessage::disable_ts:
		if ( ::ioctl(fd, CI_TS_DEACTIVATE) < 0 )
			eDebug("CI_TS_DEACTIVATE failed (%m)");
		break;
*/
	}
}

void eDVBCI::mmi_begin()
{
	unsigned char buffer[10];
	
//	eDebug("start mmi");
	memcpy(buffer,"\x90\x2\x0\x2\x9f\x80\x22\x0",8);
	sendTPDU(0xA0,8,1,buffer);
}

void eDVBCI::mmi_end()
{
	unsigned char buffer[10];

//	eDebug("stop mmi");
	memcpy(buffer,"\x90\x2\x0\x4\x9f\x88\x00\x00",8);
	sendTPDU(0xA0,8,1,buffer);
}

void eDVBCI::mmi_enqansw(unsigned char *buf)
{
//	eDebug("got mmi_answer");
	unsigned char buffer[ buf[0]+8 ];
	memcpy(buffer,"\x90\x2\x0\x4\x9f\x88\x08",7);
	memcpy(buffer+7, buf, buf[0]+1 );

	int i=1;
	for(;i<MAX_SESSIONS;++i)  // search mmi session .. hope it exist only one :)
		if(sessions[i].state && sessions[i].service_class==0x400041)
		{
			buffer[3]=i;  // session
			sendTPDU(0xA0,buf[0]+8,1,buffer);
#if 0
			for (int i=0; i < buf[0]+8; ++i )
				eDebugNoNewLine("%02x ", buffer[i]);
			eDebug("");
#endif
			break;
		}
	if ( i == MAX_SESSIONS )
		eDebug("[DVBCI] no mmi session found(enq) !");
}

void eDVBCI::mmi_menuansw(int val)
{
//	eDebug("got mmi_menu_answer %d",val);
	unsigned char buffer[9];
	memcpy(buffer,"\x90\x2\x0\x4\x9f\x88\x0B\x1",8);

	int i=1;
	for(;i<MAX_SESSIONS;++i)  // search mmi session .. hope it exist only one :)
		if(sessions[i].state && sessions[i].service_class==0x400041)
		{
			buffer[3]=i;
			buffer[8]=val&0xff;
			sendTPDU(0xA0,9,1,buffer);
			break;
		}
	if ( i == MAX_SESSIONS )
		eDebug("[DVBCI] no mmi session found!");
}

void eDVBCI::PMTsetVersion(int sid, int version)
{
	for (CIServiceMap::iterator it( services.begin() );
		it != services.end(); ++it )
	{
		if ( it->first == sid )
		{
			for (std::list<tempPMT_t>::iterator i( it->second.begin() );
				i != it->second.end(); )
			{
				if ( i->type == 2 )
					delete [] i->descriptor;
				i = it->second.erase(i);
			}
			break;
		}
	}
	tempPMT_t tmp;
	tmp.type=0;
	tmp.version=version;
	services[sid].push_back(tmp);
}

void eDVBCI::PMTflush(int sid)
{
	for (CIServiceMap::iterator it( services.begin() );
		it != services.end(); )
	{
		if ( it->first == sid || sid == -1 )
		{
			bool scrambled=false;
			std::map<int,int>::iterator d = versions.find(it->first);
			if ( d != versions.end() )
			{
				for (std::list<tempPMT_t>::iterator i( it->second.begin() );
					i != it->second.end(); )
				{
					if ( i->type == 2 )
					{
						delete [] i->descriptor;
						i = it->second.erase(i);
						scrambled=true;
					}
					else
						++i;
				}
				if (scrambled)
				{
					int oldvers = d->second;
					it->second.front().version=((oldvers&0xC1)|((oldvers+2)&0x3E));
					newService();  // i hope the ci give now this "slot" free
				}
			}

			versions.erase(it->first); // remove from last send version map

			services.erase(it++);        // remove from known service map
			if ( sid != -1 )
				break;     // the only one service is flushed.. break now
		}
		else
			++it;
	}
}

void eDVBCI::PMTaddPID(int sid, int pid, int streamtype)
{
	//eDebug("got new PID:%x",pid);

	tempPMT_t entry;
	entry.type = 1;
	entry.streamtype = streamtype;
	entry.pid=pid;
	services[sid].push_back(entry);
}

void eDVBCI::PMTaddDescriptor(int sid, unsigned char *data)
{
	//eDebug("got new CA-Descr. for CAID:%.2x%.2x",data[2],data[3]);
	tempPMT_t entry;
	entry.type = 2;
	entry.descriptor = data;
	services[sid].push_back(entry);
}

void eDVBCI::newService()
{
	//eDebug("got new %d PMT entrys",tempPMTentrys);
	ci_progress(appName);
	unsigned char capmt[2048];

	memcpy(capmt,"\x90\x2\x0\x3\x9f\x80\x32",7); //session nr.3 & capmt-tag

	unsigned int cnt=0;
	for (std::map<int, std::list<tempPMT_t> >::iterator it = services.begin();
		it != services.end(); ++it )
	{
		cnt++;
		capmt[7]=0x81;

		if ( !versions.size() )
			capmt[9] = 0x03;   // only
		else
		{
			std::map<int,int>::iterator i = versions.find(it->first);
			if ( i != versions.end() )
			{
				if (it->second.front().version != i->second)
					capmt[9]=0x05;  // update
				else
					continue;  // this is the same pmt version.. don't send..
			}
			else
				capmt[9]=0x04;  // new service.. add
		}

		// store new PMT version
		versions[it->first]=it->second.front().version;

//		eDebug("ca_pmt_list_management = %d", capmt[9]);

		capmt[10]=(unsigned char)((it->first>>8) & 0xff);			//prg-nr
		capmt[11]=(unsigned char)(it->first & 0xff);					//prg-nr

		capmt[12]=it->second.front().version;	//reserved - version - current/next
		capmt[13]=0x00;	//reserved - prg-info len
		capmt[14]=0x00;	//prg-info len

		int lenpos=13;
		int len=0;
		int first=1;
		int wp=15;

		for( std::list<tempPMT_t>::iterator i = it->second.begin();
			i != it->second.end(); ++i )
		{
			switch(i->type)
			{
				case 1:				//PID
					capmt[lenpos]=((len & 0xf00)>>8);
					capmt[lenpos+1]=(len & 0xff);
					len=0;
					lenpos=wp+3;
					first=1;
					capmt[wp++]=(i->streamtype & 0xffff);
					capmt[wp++]=((i->pid >> 8) & 0xff);
					capmt[wp++]=(i->pid & 0xff);
					wp+=2;
					break;
				case 2:				//Descriptor
					unsigned int x=0;
					for(;x<caidcount;x++)
					{
						if(caids[x] == ((i->descriptor[2]<<8)|(i->descriptor[3])))
							break;
					}

					if(x!=caidcount)
					{
						if(first)
						{
							first=0;
							capmt[wp++]=0x01;				//ca_pmt_command_id
							len++;
						}

						memcpy(capmt+wp,i->descriptor,i->descriptor[1]+2);
						wp+=i->descriptor[1]+2;
						len+=i->descriptor[1]+2;
					}
					break;
			}
		}

		capmt[lenpos]=((len & 0xf00)>>8);
		capmt[lenpos+1]=(len & 0xff);

		for(int i=1;i<MAX_SESSIONS;++i)
			if(sessions[i].state && (sessions[i].service_class==0x30041 || sessions[i].service_class==0x34100))
		{
//			eDebug("[DVBCI] send capmt with session id %d", i);
			capmt[3]=i;		//session_id
			create_sessionobject(capmt+4,capmt+9,wp-9,i);
#if 0
			eDebug("CAPMT-LEN:%d",wp);
			for(int i=0;i<wp;++i)
				eDebugNoNewLine("%02x ",capmt[i]);
			eDebug("");
#endif
		}
	}
}

void eDVBCI::create_sessionobject(unsigned char *tag,unsigned char *data,unsigned int len,int session)
{
	unsigned char buffer[len+3+5+4];	//data + tag_len + max_lengtfieldlen + spdu_header
	int newlen=len+3+4;

	memcpy(buffer,"\x90\x2\x0\x3",4); //session nr.3 & capmt-tag
	buffer[3]=session;			//session_id

	memcpy(buffer+4,tag,3);

	if(len<128)
	{
		buffer[7]=len&0x7f;
		memcpy(buffer+8,data,len);
		newlen+=1;
	}
	else if(len>255)
	{
		buffer[7]=0x82;
		buffer[8]=(len&0xff00)>>8;
		buffer[9]=len&0xff;
		memcpy(buffer+10,data,len);
		newlen+=3;
	}
	else // len > 127
	{
		buffer[7]=0x81;
		buffer[8]=len&0xff;
		memcpy(buffer+9,data,len);
		newlen+=2;
	}

#if 0
	for(int i=0;i<newlen;++i)
		printf("%02x ",buffer[i]);
	printf("\n");
#endif
	sendTPDU(0xA0,newlen,1,buffer,true);

}

void eDVBCI::clearCAIDs()
{
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if(!sapi)
		return;

	sapi->clearCAlist();

	caidcount=0;
}

void eDVBCI::addCAID(int caid)
{
	caids[caidcount++]=caid & 0xffff;
}

void eDVBCI::pushCAIDs()
{
	eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
	if(!sapi)
		return;

//	eDebug("count for caids: %d",caidcount);
	singleLock s(eDVBServiceController::availCALock);
	for(unsigned int i=0;i<caidcount;++i)
		sapi->availableCASystems.insert(caids[i]);
}

void eDVBCI::sendTPDU(unsigned char tpdu_tag,unsigned int len,unsigned char tc_id,unsigned char *data,bool dontQueue)
{
	unsigned char *buffer = new unsigned char[len+7];

	buffer[0]=tc_id;
	buffer[1]=0;
	buffer[2]=tpdu_tag;

	if(len>127)
	{
		buffer[3]=0x82;
		buffer[4]=((len+1)>>8);
		buffer[5]=((len+1) & 0xff);
		buffer[6]=tc_id;
		memcpy(buffer+7,data,len);

/*		for ( int i=0; i < len+5; ++i )
			eDebugNoNewLine("%02x ", buffer[i+2] );
		eDebug("");*/

		if ( sendqueue.empty() )
		{
//			eDebug("queue empty.. try to send data");

			if ( !sendData(tc_id,buffer+2,len+5) )
			{
//				eDebug("CI is busy... push to sendqueue..");
				sendqueue.push( queueData(tc_id, buffer, len+5) );
			}
			else
				delete [] buffer;
		}
		else  // already data in sendqueue.. push to end of queue
		{
//			eDebug("queuesize = %d.. append new data to queue", sendqueue.size());
			sendqueue.push( queueData(tc_id, buffer, len+5) );
		}
	}
	else
	{
		buffer[3]=len+1;
		buffer[4]=tc_id;
		memcpy(buffer+5,data,len);
/*		for ( int i=0; i < len+3; ++i )
			eDebugNoNewLine("%02x ", buffer[i+2] );
		eDebug("");*/
		if ( sendqueue.empty() )
		{
//			eDebug("queue empty.. try to send data");
			if ( !sendData(tc_id,buffer+2,len+3) )
			{
//				eDebug("CI is busy... push to sendqueue..");
				sendqueue.push( queueData(tc_id, buffer, len+3) );
			}
			else
				delete [] buffer;
		}
		else
		{
//			eDebug("queusize = %d.. append new data to queue", sendqueue.size());
			sendqueue.push( queueData(tc_id, buffer, len+3) );
		}
	}
}

bool eDVBCI::sendData(unsigned char tc_id,unsigned char *data,unsigned int len)
{
	stopTimer();

	if(write(fd,data,len)<0)
	{
		if (errno == EBUSY)
			return false;
		eDebug("[DVBCI] write error");
		ci_state=0;
		dataAvailable(0);
	}

	return true;
}

void eDVBCI::help_manager(unsigned int session)
{
	switch(sessions[session].internal_state)
	{
		case 0:
		 {
				unsigned char buffer[12];
				eDebug("[DVBCI] [HELP MANAGER] up to now nothing happens -> profile_enq");

				memcpy(buffer,"\x90\x2\x0\x1\x9f\x80\x10\x0",8);
				sendTPDU(0xA0,8,sessions[session].tc_id,buffer);

				sessions[session].internal_state=1;
				break;
			}
		case 1:
			{
				unsigned char buffer[12];

				eDebug("[DVBCI] [HELP MANAGER] profile_change");

				memcpy(buffer,"\x90\x2\x0\x1\x9f\x80\x12\x0",8);
				sendTPDU(0xA0,8,sessions[session].tc_id,buffer);

				sessions[session].internal_state=2;
				break;
			}
		case 2:
			{
				unsigned char buffer[40];

				eDebug("[DVBCI] [HELP MANAGER] profile_reply");
				//was wir alles koennen :)
				memcpy(buffer,"\x90\x2\x0\x1\x9f\x80\x11",7);

				buffer[7]=0x10;
				buffer[8]=0x00;			//res. manager
				buffer[9]=0x01;
				buffer[10]=0x00;
				buffer[11]=0x41;		//? :)

				buffer[12]=0x00;
				buffer[13]=0x02; //02
				buffer[14]=0x00;
				buffer[15]=0x41;		//CA

				buffer[16]=0x00;
				buffer[17]=0x03;
				buffer[18]=0x00; //00		//date-time
				buffer[19]=0x41;
				buffer[20]=0x00;
				buffer[21]=0x40;
				buffer[22]=0x00;		//mmi
				buffer[23]=0x41;

				sendTPDU(0xA0,24,sessions[session].tc_id,buffer);
				sessions[session].internal_state=3;
				break;
			}
		default:
			//printf("[HELP MANAGER] undefined state\n");  //or ready ;)
			break;
	}
}

void eDVBCI::app_manager(unsigned int session)
{
	switch(sessions[session].internal_state)
	{
		case 0:
			{
				unsigned char buffer[12];
				eDebug("[DVBCI] [APPLICATION MANAGER] up to now nothing happens -> app_info_enq");
				memcpy(buffer,"\x90\x2\x0\x2\x9f\x80\x20\x0",8);
				sendTPDU(0xA0,8,sessions[session].tc_id,buffer);
				sessions[session].internal_state=1;
				break;
			}
			case 1:
			break;
			default:
			break;
	}
}


void eDVBCI::ca_manager(unsigned int session)
{
//	eDebug("[DVBCI] ca_manager session %d", session );
	switch(sessions[session].internal_state)
	{
		case 0:
			{
				unsigned char buffer[12];
				sessions[session].internal_state=1;


/*  // disabled.. for more info see dvbservice.cpp
				eServiceHandler *serviceHandler =
					eServiceInterface::getInstance()->getService();
				if ( serviceHandler && serviceHandler->getFlags() & eServiceHandler::flagIsScrambled )*/
				{
					if ( ::ioctl(fd,CI_TS_ACTIVATE)<0 )
						eDebug("CI_TS_ACTIVATE failed (%m)");
				}

				clearCAIDs();
				eDebug("[DVBCI] [CA MANAGER] up to now nothing happens -> ca_info_enq");

				memcpy(buffer,"\x90\x2\x0\x3\x9f\x80\x30\x0",8);

				if(!sessions[session].state)
					eDebug("CA-MANAGER SESSION %d not assigned !", session);
				else
				{
					buffer[3]=session;
					if ( sessions[session].service_class==0x30041 ||
						sessions[session].service_class==0x34100 )
						sendTPDU(0xA0,8,sessions[session].tc_id,buffer);
					else
						eDebug("CA-MANAGER SESSION %d has invalid service_class %08x", sessions[session].service_class );
				}
				break;
			}
		case 1:
			{
				eDebug("[DVBCI] [CA MANAGER] send ca_pmt");
				//sendCAPMT();
				versions.clear();
				newService();  // send with "only" service
				sessions[session].internal_state=2;
				break;
			}
		default:
			eDebug("unhandled session(%d) state %d", session, sessions[session].internal_state );
			break;
	}
}

void eDVBCI::handle_session(unsigned char *data,int len)
{
#if 0
	eDebugNoNewLine("[DVBCI] dump handle session tag:");
	for (int i=0; i < len; ++i )
		eDebugNoNewLine("%02x ", data[i]);
	eDebug("");
#endif

	if(data[4]==0x9f && data[5]==0x80 && data[6]==0x11)
		help_manager(data[3]);
	else if(data[4]==0x9f && data[5]==0x80 && data[6]==0x10)
	{
		help_manager(data[3]);
		ci_progress("help-manager init");
	}
	else if(data[4]==0x9f && data[5]==0x80 && data[6]==0x21)
	{
		eDebug("[DVBCI] APP-INFO");
		memcpy(appName,data+14,data[13]);
		appName[data[13]]=0x0;
		ci_progress("application manager-init");
	}
	else if(data[4]==0x9f && data[5]==0x80 && data[6]==0x31)
	{
		int i;
		eDebug("[DVBCI] CA-INFO");

		if(data[7]>(len+8))
			eDebug("[DVBCI] [CA MANAGER] error in ca-info");

		ci_progress("[DVBCI]ca-manager init");

		for(i=8;i<data[7]+8;i+=2)
		{
			eDebug("[DVBCI] [CA MANAGER] add CAID: %04x",data[i]<<8|data[i+1]);
			addCAID(data[i]<<8|data[i+1]);
		}
		pushCAIDs();

		if ( !sessions[data[3]].state )
			eDebug("[DVBCI] SESSION %d is not assigned !");
		else
		{
			if ( sessions[data[3]].service_class==0x30041 ||
				 sessions[data[3]].service_class==0x34100)
				ca_manager(data[3]);
			else
				eDebug("SESSION ID %d is no CA-MANAGER");
		}
	}
	else if(data[4]==0x9f && data[5]==0x88 && data[6]==0x01)
	{
		unsigned char buffer[20];
		eDebug("[DVBCI] [APPLICATION MANAGER] -> display-control");
		memcpy(buffer,"\x90\x2\x0\x4\x9f\x88\x2\x2\x1\x1",10);
		buffer[3]=data[3];  // session id
		sendTPDU(0xA0,10,1,buffer);
	}
	else if(data[4]==0x9f && data[5]==0x84 && data[6]==0x40)
	{
		unsigned char buffer[20];
		eDebug("[DVBCI] [DATE TIME ENQ]");
		memcpy(buffer,"\x90\x2\x0\x5\x9f\x88\x41\x5\xcd\x8A\x23\x13\x12",13);
		buffer[3]=data[3];  // session id
		sendTPDU(0xA0,13,1,buffer);
	}
	else if(data[4]==0x9f && data[5]==0x88)
	{
		char buffer[len-4];
		eDebug("[DVBCI] [APPLICATION MANAGER] -> mmi_menu");
#if 0
		eDebug("[DVBCI] mmi len:%d",len);
		for(int i=0;i<len;++i)
			eDebugNoNewLine("%02x ",data[i]);
		eDebug("");
#endif
		memcpy(buffer,data+4,len-4);
		ci_mmi_progress(buffer,len-4);
	}
	else
	{
		eDebug("[DVBCI] unhandled session:");
		for(int i=0;i<len;++i)
			eDebugNoNewLine("%02x ",data[i]);
		eDebug("");
	}
}

int eDVBCI::service_available(unsigned long service_class)
{
	switch(service_class)
	{
		case 0x010041:
		case 0x020041:

		case 0x030041:
		case 0x034100:	//WTF? find the bug ... endianess fool on integer-fields?
										//if it is so...perhaps its better you switch the xa to 8bit mode *g*
		case 0x400041:
		case 0x240041:
			return 1;
		default:
			eDebug("[DVBCI] service %08x not available", service_class);
			return 0;
	}
}

void eDVBCI::handle_spdu(unsigned int tpdu_tc_id,unsigned char *data,int len)
{
	unsigned char buffer[40];

//	eDebug("[DVBCI]handle_spdu(%02x)", data[0]);

	switch(data[0])
	{
		case 0x90:
			handle_session(data,len);
			break;
		case 0x91:
			if(service_available(*(unsigned long*)(data+2)))
			{
#if 0
				eDebugNoNewLine("[DVBCI] dump open session tag:");
				for (int i=0; i < len; ++i )
					eDebugNoNewLine("%02x ", data[i]);
				eDebug("");
#endif
				int i=1;
				for(;i<MAX_SESSIONS;++i)
					if(sessions[i].state==STATE_FREE)
						break;
				if(i==MAX_SESSIONS)
				{
					eDebug("[DVBCI] no free sessions left");
					memcpy(buffer,"\x92\x7\xf0\x0\x0\x0\x0\x0\x0",9);
					memcpy(buffer+3,data+2,4);
					sendTPDU(0xA0,9,tpdu_tc_id,buffer);
					return;
				}

				sessions[i].state=STATE_OPEN;
				sessions[i].service_class=*(unsigned long*)(data+2);
				sessions[i].tc_id=tpdu_tc_id;
				sessions[i].internal_state=0;

				memcpy(buffer,"\x92\x7\x0",3);
				memcpy(buffer+3,data+2,4);
				buffer[7]=i>>8;
				buffer[8]=i& 0xff;
				sendTPDU(0xA0,9,tpdu_tc_id,buffer);

				eDebug("[DVBCI] serviceclass (%x) request accepted on %d",*(unsigned long*)(data+2),i);

				if(sessions[i].service_class==0x10041)
					help_manager(i);
				if(sessions[i].service_class==0x20041)
					app_manager(i);
				if(sessions[i].service_class==0x30041)
					ca_manager(i);
				if(sessions[i].service_class==0x34100)
					ca_manager(i);
			}
			else
			{
				eDebug("[DVBCI] unknown serviceclass (%x) requested",*(unsigned long*)(data+2));
				memcpy(buffer,"\x92\x7\xf0",3);
				memcpy(buffer+3,data+2,4);
				sendTPDU(0xA0,9,tpdu_tc_id,buffer);
			}
			break;
		case 0x95:		//T_close_session
			{
#if 0
				eDebugNoNewLine("[DVBCI] dump close session tag:");
				for (int i=0; i < len; ++i )
					eDebugNoNewLine("%02x ", data[i]);
				eDebug("");
#endif
				memcpy(buffer,"\x96\x3\xf0",3);
				buffer[3]=data[2];
				buffer[4]=data[3];
				if( sessions[data[3]].state != STATE_FREE )
				{
					buffer[2]=0; // session freed
					sessions[data[3]].state=STATE_FREE;
					eDebug("[DVBCI] freeing session %d",data[3]);
					if(sessions[data[3]].service_class == 0x400041)
					{
						eDebug("[DVBCI] close mmi after session free");
						char *buf="\x9f\x88\x00\x00";
						ci_mmi_progress(buf,4);
					}
				}
				sendTPDU(0xA0,5,tpdu_tc_id,buffer);
				break;
			}
		default:
			eDebug("[DVBCI] unknown SPDU-TAG:%x",data[0]);
	}
}

void eDVBCI::receiveTPDU(unsigned char tpdu_tag,unsigned int tpdu_len,unsigned char tpdu_tc_id,unsigned char *data)
{
//	eDebug("[DVBCI] receiveTPDU %02x", tpdu_tag);
	switch(tpdu_tag)
	{
		case 0x80:
			//if(data[0]==0x80)
			//	sendTPDU(0x81,0,tpdu_tc_id,0);
			//else
			//	startTimer();
			break;
		case 0x83:
			eDebug("[DVBCI] T_C_ID %d wurde erstellt",tpdu_tc_id);

			//startTimer();
			break;
		case 0xA0:
			if(tpdu_len)
			{
				if(data[0] >= 0x90 && data[0] <= 0x96)
				{
					handle_spdu(tpdu_tc_id,data,tpdu_len);
					//startTimer();
				}
				else
				{
					eDebug("[DVBCI] unknown spdu-tag:%x",data[0]);
				}
			}
			break;
		default:
			eDebug("[DVBCI] unknown tpdu-tag:%x, len = %d", tpdu_tag, tpdu_len );
	}
}

#if 0
void eDVBCI::incoming(unsigned char *buffer,int len)
{
	int tc_id;
	int m_l;
	int tpdu_tag;
	int tpdu_len;
	int tpdu_tc_id;
	int x=0;
#if 0
	printf("incoming:");
	for(int i=0;i<len;++i)
		printf("%02x ",buffer[i]);
	printf("\n");
#endif
	tc_id=buffer[x++];
	m_l=buffer[x++];

	if(buffer[0]!=0x1)
		return;

	if(!ml_bufferlen)
		if((buffer[2] & 0xF0) != 0x80 && (buffer[2] & 0xF0) != 0xA0)
			return;

	if(len<6)
		return;
	//the cheapest defrag on earth *g*
	if(m_l && ml_bufferlen==0)			//first fragment
	{
		int y;
		tpdu_tag=buffer[x++];
		tpdu_len=y=buffer[x++];
		if(y&0x80)						//aua fix me
		{
			//eDebug("y & 0x80 %x",tpdu_len);
			y&=0x7f;
			int i;
			for(i=0;i<y;++i)
				tpdu_len=(buffer[x++]&0xff)<<(((y*8)-8)-(i*8));
			//x++;
			//tpdu_len=buffer[x++];
			//eDebug("len:%d\n",tpdu_len);
		}
		tpdu_len--;
		tpdu_tc_id=buffer[x++];

		memcpy(ml_buffer,buffer+x,len-7);
		ml_bufferlen=len-7;
		ml_buffersize=tpdu_len;
		dataAvailable(0);
	}
	else if(!m_l && ml_bufferlen)		//last fragment
	{
		memcpy(ml_buffer+ml_bufferlen,buffer+2,len-2);
		receiveTPDU(0xA0,ml_buffersize,1,ml_buffer);
		ml_bufferlen=0;
	}
	else														//not fragmented
	{
		while(x<len)
		{
			int y;
			tpdu_tag=buffer[x++];
			tpdu_len=y=buffer[x++];
			if(y&0x80)						//aua fix me
			{
				//eDebug("y & 0x80 %x",tpdu_len);
				y&=0x7f;
				int i;
				for(i=0;i<y;++i)
					tpdu_len=(buffer[x++]&0xff)<<(((y*8)-8)-(i*8));
				//tpdu_len=buffer[x++];
				//eDebug("len:%d\n",tpdu_len);
			}
			tpdu_len--;
			//if(tpdu_len>(len-6))
			//	tpdu_len=len-6;
			tpdu_tc_id=buffer[x++];
#if 0
			printf("tpdu (%02x):",tpdu_tag);
			for(int i=0;i<tpdu_len;++i)
				printf("%02x ",buffer[(x-tpdu_len+1)+i]);
			printf("\n");
#endif
			receiveTPDU(tpdu_tag,tpdu_len,tpdu_tc_id,buffer+x);
			x+=tpdu_len;
		}
	}
}
#else

ptrlpduQueueElem eDVBCI::AllocLpduQueueElem(unsigned char t_c_id)
{
	ptrlpduQueueElem curElem;

	curElem = new lpduQueueElem;

	curElem->lpduLen = 0;

	(curElem->lpdu)[0] = t_c_id;

	curElem->nextElem = NULL;

	return curElem;
}

int eDVBCI::lpduQueueElemIsMore(ptrlpduQueueElem curElem)
// Check header of contained LPDU for "more" flag
{

	if ((curElem->lpdu)[1] & 0x80)
		return 1;
	else
		return 0;
}

void eDVBCI::incoming(unsigned char *buffer,int len)
{
	ptrlpduQueueElem curElem, lastElem;

	unsigned char t_c_id;

	unsigned char * payloadData;
	long payloadIndex;
	long length;
	unsigned char from_t_c_id;

	payloadData = NULL;
	length = 0;
	from_t_c_id = 0;

	// Allocate LPDU queue element
	curElem = AllocLpduQueueElem(0);

	// Get LPDU
	memcpy(curElem->lpdu,buffer,len);
	curElem->lpduLen=len;

	// get transport ID
	t_c_id = (curElem->lpdu)[0];

	// Append to the current receive queue for the transport ID
	lastElem = lpduReceiveQueues[t_c_id].firstLPDU;

	if (lastElem == NULL)
	{
		lpduReceiveQueues[t_c_id].firstLPDU = curElem;
	}
	else
	{
		while (lastElem->nextElem != NULL)
			lastElem = lastElem->nextElem;

		lastElem->nextElem = curElem;
	}
			// Increment LPDU count
	lpduReceiveQueues[t_c_id].numLPDUS++;

	if (!lpduQueueElemIsMore(curElem))
	{
		payloadIndex = 0;

		length = lpduReceiveQueues[t_c_id].numLPDUS * LPDUPAYLOADLEN;
		from_t_c_id = t_c_id;

		payloadData = new unsigned char[length];
		length = 0;

		curElem = lpduReceiveQueues[t_c_id].firstLPDU;

		while (curElem != NULL)
		{
			memcpy(payloadData + payloadIndex, (curElem->lpdu) + LPDUHEADERLEN, (curElem->lpduLen) - LPDUHEADERLEN);

			payloadIndex += ((curElem->lpduLen) - LPDUHEADERLEN);
			(length) += ((curElem->lpduLen) - LPDUHEADERLEN);

			lastElem = curElem;
			curElem = curElem->nextElem;

			// And remove element
			delete lastElem;
		}

		lpduReceiveQueues[t_c_id].numLPDUS = 0;
		lpduReceiveQueues[t_c_id].firstLPDU = NULL;

		//printf("data assembled\n");
		//for(int i=0;i<length;++i)
		//	printf("%02x ",payloadData[i]);
		//printf("\n");

		//if(payloadData[length-4] != 0x80)
		//	printf("Status-Field broken!!! (tag)\n");

		//if(payloadData[length-3] != 0x2)
		//	printf("Status-Field broken!!! (len)\n");

		if(payloadData[length-1] == 0x80)
		{
//			eDebug("[DVBCI] query data");
			sendTPDU(0x81,0,payloadData[length-2],0);
		}

		length-=4;

		int cl=payloadData[1] & 0x7f;
		int lenfield=1;
		if(payloadData[1] & 0x80)
		{
			lenfield = payloadData[1] & 0x7f;

			cl=0;
			for(int i=0;i<lenfield;++i)
				cl |= payloadData[2+i] << ((lenfield-(i+1))*8);

			lenfield++;
			//printf("lenfield:%d len:%d\n",lenfield,cl);
		}

		//printf("tpdu\n");
		//for(int i=0;i<length;++i)
		//	printf("%02x ",payloadData[i]);
		//printf("\n");

		if(length>1)
			receiveTPDU(payloadData[0],cl,t_c_id,payloadData+(2+lenfield));

		delete [] payloadData;

		//startTimer();
	}
}
#endif

void eDVBCI::startTimer()
{
	pollTimer.start(80,true);
}

void eDVBCI::stopTimer()
{
	pollTimer.stop();
}

void eDVBCI::dataAvailable(int what)
{
	int present;
	unsigned char buffer[1024];
	int size;

	stopTimer();

	if ( what != 1000 )
	{
		if (::ioctl(fd,CI_GET_STATUS,&present)<0)
			eDebug("[DVBCI] GET_STATUS failed (%m)");

		if(present == 2 || present != 1)
		{
			eDebug("[DVBCI] module removed");

			// clear receive lpdu queues
			ptrlpduQueueElem curElem, lastElem;
			for (int i=0; i < MAXTRANSPORTSESSIONS; ++i)
			{
				if (lpduReceiveQueues[i].numLPDUS)
				{
					curElem = lpduReceiveQueues[i].firstLPDU;
					while (curElem != NULL)
					{
						lastElem = curElem;
						curElem = curElem->nextElem;
						// And remove element
						delete lastElem;
					}
				}
			}

			// clear sendqueue
			while(sendqueue.size())
			{
				delete [] sendqueue.top().data;
				sendqueue.pop();
			}

			strcpy(appName,_("no module"));
			ci_progress(appName);

			char *buf="\x9f\x88\x00\x00";
			ci_mmi_progress(buf,4);

			for(int i=1;i<MAX_SESSIONS;++i)
				sessions[i].state=STATE_FREE;

			::read(fd,&buffer,0);
			ci_state=0;
			clearCAIDs();

			return;
		}
	}

	if(ci_state==0)						//CI plugged
	{
		int i;
		eDebug("[DVBCI] module inserted");

		ci_progress("module found");

//		eZapMain::getInstance()->postMessage(eZapMessage(1,"Common Interface","CI inserted - initializing...",10),0);
		// SEND FAKE Message to eZapMain... now we can show ci plug message
		char *buf="INIT";
		ci_mmi_progress(buf,4);

		for(i=1;i<MAX_SESSIONS;++i)
			sessions[i].state=STATE_FREE;

		ml_bufferlen=0;

		::read(fd,&buffer,0);

		if (::ioctl(fd,CI_RESET)<0 )  // != 0
		{
			eDebug("[DVBCI] RESET failed (%m)");
			ci_state=0;
			clearCAIDs();
			return;
		}

		// clear sendqueue
		while ( sendqueue.size() )
		{
//			eDebug("[DVBCI] clear queue");
			delete [] sendqueue.top().data;
			sendqueue.pop();
		}

		ci_state=1;
	}

	size=::read(fd,&buffer,sizeof(buffer));
	//eDebug("READ:%d",size);

	if(size>0)
	{
		int i;
#if 0
		for(i=0;i<size;++i)
			printf("%02x ",buffer[i]);
		printf("\n");	
#endif	
		incoming(buffer,size);

		if ( sendqueue.size() )
		{
			queueData d = sendqueue.top();
			sendqueue.pop();
			if ( !sendData( d.tc_id, d.data+2, d.len ) )
			{
				// add this entry with higher priority to sendqueue..
				sendqueue.push( queueData( d.tc_id, d.data, d.len, 1 ) );
			}
			else
				delete [] d.data;
		}

		if (::ioctl(fd,8,&i)<0)
			eDebug("[DVBCI] GET failed (%m)");

		if(i==4)
		{
			startTimer();
		}
		else
			stopTimer();
		return;
	}	
	
	if(ci_state==1)
	{
		sendTPDU(0x82,0,1,0);	
		ci_state=2;
	}
}

void eDVBCI::poll()
{
	int present;

	stopTimer();

#if 0
	eDebug("[DVBCI] TIMER");
#endif

	if (::ioctl(fd,CI_GET_STATUS,&present)<0)
		eDebug("CI_GET_STATUS failed (%m)");

	if(present)						//CI removed
		sendTPDU(0xA0,0,1,0);
}

#endif
