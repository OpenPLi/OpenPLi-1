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




#include <dbox/fp.h>
#include <stdio.h>
#include <asm/types.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>

#include <utime.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <eventserver.h>

#include <global.h>
#include <neutrino.h>

#include "rcinput.h"

#define SA struct sockaddr
#define SAI struct sockaddr_in


void printbin( int a)
{
	for(int pos = 15;pos>=0;pos--)
	{
		printf("%d", a&(1<<pos)?1:0);
	}
	printf("\n");
}


/**************************************************************************
*	Constructor - opens rc-input device and starts threads
*
**************************************************************************/
CRCInput::CRCInput()
{
	timerid= 1;

	// pipe for internal event-queue
	// -----------------------------
	if (pipe(fd_pipe_high_priority) < 0)
	{
		perror("fd_pipe_high_priority");
		exit(-1);
	}

	fcntl(fd_pipe_high_priority[0], F_SETFL, O_NONBLOCK );
	fcntl(fd_pipe_high_priority[1], F_SETFL, O_NONBLOCK );

	if (pipe(fd_pipe_low_priority) < 0)
	{
		perror("fd_pipe_low_priority");
		exit(-1);
	}

	fcntl(fd_pipe_low_priority[0], F_SETFL, O_NONBLOCK );
	fcntl(fd_pipe_low_priority[1], F_SETFL, O_NONBLOCK );


	// open event-library
	// -----------------------------
	fd_event = 0;

	//network-setup
    struct sockaddr_un servaddr;
    int    clilen;
    memset(&servaddr, 0, sizeof(struct sockaddr_un));
    servaddr.sun_family = AF_UNIX;
    strcpy(servaddr.sun_path, NEUTRINO_UDS_NAME);
    clilen = sizeof(servaddr.sun_family) + strlen(servaddr.sun_path);
    unlink(NEUTRINO_UDS_NAME);

    //network-setup
    if ((fd_event = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        perror("[neutrino] socket\n");
    }

	if ( bind(fd_event, (struct sockaddr*) &servaddr, clilen) <0 )
	{
		perror("[neutrino] bind failed...\n");
		exit(-1);
	}


	if (listen(fd_event, 5) !=0)
	{
		perror("[neutrino] listen failed...\n");
		exit( -1 );
	}


	open();
}

void CRCInput::open()
{
	close();

	//+++++++++++++++++++++++++++++++++++++++
	fd_rc=::open("/dev/dbox/rc0", O_RDONLY);
	if (fd_rc<0)
	{
		perror("/dev/dbox/rc0");
		//exit(-1);
	}
	ioctl(fd_rc, RC_IOCTL_BCODES, 1);
	fcntl(fd_rc, F_SETFL, O_NONBLOCK );

	//+++++++++++++++++++++++++++++++++++++++
	fd_keyb = 0;
	/*
	::open("/dev/dbox/rc0", O_RDONLY);
	if (fd_keyb<0)
	{
		perror("/dev/stdin");
		exit(-1);
	}
	*/
	//fcntl(fd_keyb, F_SETFL, O_NONBLOCK );

	//+++++++++++++++++++++++++++++++++++++++

	calculateMaxFd();
}

void CRCInput::close()
{
	if(fd_rc)
	{
		::close(fd_rc);
		fd_rc = 0;
	}
/*
	if(fd_keyb)
	{
		::close(fd_keyb);
	}
*/
}

void CRCInput::calculateMaxFd()
{
	fd_max = fd_rc;
	if(fd_event > fd_max)
		fd_max = fd_event;
	if(fd_pipe_high_priority[0] > fd_max)
		fd_max = fd_pipe_high_priority[0];
	if(fd_pipe_low_priority[0] > fd_max)
		fd_max = fd_pipe_low_priority[0];
}


/**************************************************************************
*	Destructor - close the input-device
*
**************************************************************************/
CRCInput::~CRCInput()
{
	close();

	if(fd_pipe_high_priority[0])
		::close(fd_pipe_high_priority[0]);
	if(fd_pipe_high_priority[1])
		::close(fd_pipe_high_priority[1]);

	if(fd_pipe_low_priority[0])
		::close(fd_pipe_low_priority[0]);
	if(fd_pipe_low_priority[1])
		::close(fd_pipe_low_priority[1]);

	if(fd_event)
		::close(fd_event);
}

/**************************************************************************
*	stopInput - stop reading rcin for plugins
*
**************************************************************************/
void CRCInput::stopInput()
{
	close();
}


/**************************************************************************
*	restartInput - restart reading rcin after calling plugins
*
**************************************************************************/
void CRCInput::restartInput()
{
	open();
}

int CRCInput::messageLoop( bool anyKeyCancels, int timeout )
{
    int res = menu_return::RETURN_REPAINT;

	bool doLoop = true;

	if ( timeout == -1 )
		timeout = g_settings.timing_menu ;

	unsigned long long timeoutEnd = g_RCInput->calcTimeoutEnd( timeout );
	uint msg; uint data;

	while (doLoop)
	{
		g_RCInput->getMsgAbsoluteTimeout( &msg, &data, &timeoutEnd );

        if ( ( msg == CRCInput::RC_timeout ) ||
        	 ( msg == CRCInput::RC_home ) ||
        	 ( msg == CRCInput::RC_ok ) )
			doLoop = false;
		else
		{
			int mr = CNeutrinoApp::getInstance()->handleMsg( msg, data );

			if ( mr & messages_return::cancel_all )
			{
				res = menu_return::RETURN_EXIT_ALL;
				doLoop = false;
			}
			else if ( mr & messages_return::unhandled )
			{
				if ( msg <= CRCInput::RC_MaxRC )
				{
					if ( anyKeyCancels )
						doLoop = false;
					else
						timeoutEnd = g_RCInput->calcTimeoutEnd( timeout );
				}
			}
		}


	}
	return res;
}


int CRCInput::addTimer(unsigned long long Interval, bool oneshot, bool correct_time )
{
	struct timeval tv;

	gettimeofday( &tv, NULL );
	unsigned long long timeNow = (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);

	timer _newtimer;
	if (!oneshot)
		_newtimer.interval = Interval;
	else
		_newtimer.interval = 0;

	_newtimer.id = timerid++;
	if ( correct_time )
		_newtimer.times_out = timeNow+ Interval;
	else
		_newtimer.times_out = Interval;

	_newtimer.correct_time = correct_time;

	//printf("adding timer %d (0x%llx, 0x%llx)\n", _newtimer.id, _newtimer.times_out, Interval);

	vector<timer>::iterator e;
	for ( e= timers.begin(); e!= timers.end(); ++e )
		if ( e->times_out> _newtimer.times_out )
			break;

	timers.insert(e, _newtimer);
	return _newtimer.id;
}

int CRCInput::addTimer(struct timeval Timeout)
{
	unsigned long long timesout = (unsigned long long) Timeout.tv_usec + (unsigned long long)((unsigned long long) Timeout.tv_sec * (unsigned long long) 1000000);
	return addTimer( timesout, true, false );
}

int CRCInput::addTimer(const time_t *Timeout)
{
	return addTimer( (unsigned long long)*Timeout* (unsigned long long) 1000000, true, false );
}

void CRCInput::killTimer(uint id)
{
	//printf("killing timer %d\n", id);
	vector<timer>::iterator e;
	for ( e= timers.begin(); e!= timers.end(); ++e )
		if ( e->id == id )
		{
			timers.erase(e);
			break;
		}
}

int CRCInput::checkTimers()
{
	struct timeval tv;
	int _id = 0;

	gettimeofday( &tv, NULL );
	unsigned long long timeNow = (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);


	vector<timer>::iterator e;
	for ( e= timers.begin(); e!= timers.end(); ++e )
		if ( e->times_out< timeNow+ 2000 )
		{
//			printf("timeout timer %d %llx %llx\n",e->id,e->times_out,timeNow );
			_id = e->id;
			if ( e->interval != 0 )
			{
				timer _newtimer;
				_newtimer.id= e->id;
				_newtimer.interval= e->interval;
				_newtimer.times_out= e->times_out+ e->interval;
				_newtimer.correct_time= e->correct_time;

            	timers.erase(e);
				for ( e= timers.begin(); e!= timers.end(); ++e )
					if ( e->times_out> _newtimer.times_out )
						break;

				timers.insert(e, _newtimer);
			}
			else
				timers.erase(e);

			break;
        }
//        else
//    		printf("skipped timer %d %llx %llx\n",e->id,e->times_out, timeNow );
	return _id;
}



long long CRCInput::calcTimeoutEnd( int Timeout )
{
	struct timeval tv;

	gettimeofday( &tv, NULL );
	unsigned long long timeNow = (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);

	return ( timeNow + Timeout* 1000000 );
}

long long CRCInput::calcTimeoutEnd_MS( int Timeout )
{
	struct timeval tv;

	gettimeofday( &tv, NULL );


	unsigned long long timeNow = (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);

	return ( timeNow + Timeout* 1000 );
}


void CRCInput::getMsgAbsoluteTimeout(uint *msg, uint* data, unsigned long long *TimeoutEnd, bool bAllowRepeatLR)
{
	struct timeval tv;

	gettimeofday( &tv, NULL );
	unsigned long long timeNow = (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);

	unsigned long long diff;

	if ( *TimeoutEnd < timeNow+ 100 )
		diff = 100;  // Minimum Differenz...
	else
		diff = ( *TimeoutEnd - timeNow );

	getMsg_us( msg, data, diff, bAllowRepeatLR );

	if ( *msg == NeutrinoMessages::EVT_TIMESET )
	{
		// recalculate timeout....
		//unsigned long long ta= *TimeoutEnd;
		*TimeoutEnd= *TimeoutEnd + *(long long*) *data;

		//printf("[getMsgAbsoluteTimeout]: EVT_TIMESET - recalculate timeout\n%llx/%llx - %llx/%llx\n", timeNow, *(long long*) *data, *TimeoutEnd, ta );
	}
}

void CRCInput::getMsg(uint *msg, uint *data, int Timeout, bool bAllowRepeatLR)
{
	getMsg_us( msg, data, (unsigned long long) Timeout * 100* 1000, bAllowRepeatLR );
}

void CRCInput::getMsg_ms(uint *msg, uint *data, int Timeout, bool bAllowRepeatLR)
{
	getMsg_us( msg, data, (unsigned long long) Timeout * 1000, bAllowRepeatLR );
}

void CRCInput::getMsg_us(uint *msg, uint *data, unsigned long long Timeout, bool bAllowRepeatLR)
{
	static long long last_keypress=0;
	unsigned long long getKeyBegin;

	static __u16 rc_last_key = 0;
	static __u16 rc_last_repeat_key = 0;

	struct timeval tv, tvselect;
	unsigned long long InitialTimeout = Timeout;
	long long targetTimeout;

	int timer_id;
	fd_set rfds;
	__u16 rc_key;

	//set 0
	*data = 0;

	// wiederholung reinmachen - dass wirklich die ganze zeit bis timeout gewartet wird!
	gettimeofday( &tv, NULL );
	getKeyBegin = (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);

	while(1)
	{
		timer_id = 0;
		if ( timers.size()> 0 )
		{
			gettimeofday( &tv, NULL );
			unsigned long long t_n= (unsigned long long) tv.tv_usec + (unsigned long long)((unsigned long long) tv.tv_sec * (unsigned long long) 1000000);
			if ( timers[0].times_out< t_n )
			{
				timer_id = checkTimers();
       			*msg = NeutrinoMessages::EVT_TIMER;
				*data = timer_id;
				return;
			}
			else
			{
             	targetTimeout = timers[0].times_out - t_n;
				if ( (unsigned long long) targetTimeout> Timeout)
					targetTimeout= Timeout;
				else
					timer_id = timers[0].id;
			}
		}
		else
			targetTimeout= Timeout;

	    tvselect.tv_sec = targetTimeout/1000000;
		tvselect.tv_usec = targetTimeout%1000000;
		//printf("InitialTimeout= %lld:%lld\n", Timeout/1000000,Timeout%1000000);
        //printf("targetTimeout= %d:%d\n", tvselect.tv_sec,tvselect.tv_usec);

		FD_ZERO(&rfds);
		if (fd_rc> 0)
			FD_SET(fd_rc, &rfds);
		if (fd_keyb> 0)
			FD_SET(fd_keyb, &rfds);

		FD_SET(fd_event, &rfds);
		FD_SET(fd_pipe_high_priority[0], &rfds);
		FD_SET(fd_pipe_low_priority[0], &rfds);
		calculateMaxFd();

		int status =  select(fd_max+1, &rfds, NULL, NULL, &tvselect);

		if ( status == -1 )
		{
			perror("[neutrino - getMsg_us]: select returned ");
			// in case of an error return timeout...?!
			*msg = RC_timeout;
			*data = 0;
			return;
		}
		else if ( status == 0 ) // Timeout!
		{
			if ( timer_id != 0 )
			{
			    timer_id = checkTimers();
				if ( timer_id != 0 )
				{
        			*msg = NeutrinoMessages::EVT_TIMER;
					*data = timer_id;
					return;
				}
				else
					continue;
			}
			else
			{
				*msg = RC_timeout;
				*data = 0;
				return;
			}
		}

		if(FD_ISSET(fd_pipe_high_priority[0], &rfds))
		{
			uint buf[2];
			read(fd_pipe_high_priority[0], &buf, sizeof(buf));
			*msg = buf[0];
			*data = buf[1];
//printf("got event from high-pri pipe %x %x\n", *msg, *data );
			return;
		}

/*
		if(FD_ISSET(fd_keyb, &rfds))
		{
			char key = 0;
			read(fd_keyb, &key, sizeof(key));
			printf("keyboard: %d\n", rc_key);
		}
*/
		if(FD_ISSET(fd_event, &rfds))
		{
			//printf("[neutrino] event - accept!\n");
			socklen_t	clilen;
			SAI			cliaddr;
			clilen = sizeof(cliaddr);
			int fd_eventclient = accept(fd_event, (SA *) &cliaddr, &clilen);

			*msg = RC_nokey;
			//printf("[neutrino] network event - read!\n");
			CEventServer::eventHead emsg;
			int read_bytes= recv(fd_eventclient, &emsg, sizeof(emsg), MSG_WAITALL);
			//printf("[neutrino] event read %d bytes - following %d bytes\n", read_bytes, emsg.dataSize );
			if ( read_bytes == sizeof(emsg) )
			{
				bool dont_delete_p = false;

				unsigned char* p;
				p= new unsigned char[ emsg.dataSize + 1 ];
				if ( p!=NULL )
			 	{
			 		read_bytes= recv(fd_eventclient, p, emsg.dataSize, MSG_WAITALL);
			 		//printf("[neutrino] eventbody read %d bytes - initiator %x\n", read_bytes, emsg.initiatorID );

			 		if ( emsg.initiatorID == CEventServer::INITID_CONTROLD )
			 		{
			 			if (emsg.eventID==CControldClient::EVT_VOLUMECHANGED)
			 			{
			 				*msg = NeutrinoMessages::EVT_VOLCHANGED;
			 				*data = *(char*) p;
			 			}
			 			else if (emsg.eventID==CControldClient::EVT_MUTECHANGED)
			 			{
			 				*msg = NeutrinoMessages::EVT_MUTECHANGED;
			 				*data = *(bool*) p;
			 			}
			 			else if (emsg.eventID==CControldClient::EVT_VCRCHANGED)
			 			{
			 				*msg = NeutrinoMessages::EVT_VCRCHANGED;
			 				*data = *(int*) p;
			 			}
			 			else if (emsg.eventID==CControldClient::EVT_MODECHANGED)
			 			{
			 				*msg = NeutrinoMessages::EVT_MODECHANGED;
			 				*data = *(int*) p;
			 			}
			 			else
			 				printf("[neutrino] event INITID_CONTROLD - unknown eventID 0x%x\n",  emsg.eventID );
			 		}
			 		else if ( emsg.initiatorID == CEventServer::INITID_HTTPD )
			 		{
			 			if (emsg.eventID==NeutrinoMessages::SHUTDOWN)
			 			{
			 				*msg = NeutrinoMessages::SHUTDOWN;
			 				*data = 0;
			 			}
						else if (emsg.eventID==NeutrinoMessages::EVT_POPUP)
			 			{
			 				*msg = NeutrinoMessages::EVT_POPUP;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
			 			}
			 			else if (emsg.eventID==NeutrinoMessages::EVT_EXTMSG)
			 			{
			 				*msg = NeutrinoMessages::EVT_EXTMSG;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
			 			}
			 			else if (emsg.eventID==NeutrinoMessages::CHANGEMODE)	// Change 
			 			{
			 				*msg = NeutrinoMessages::CHANGEMODE;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==NeutrinoMessages::STANDBY_ON)
			 			{
			 				*msg = NeutrinoMessages::STANDBY_ON;
			 				*data = 0;
			 			}
						else if (emsg.eventID==NeutrinoMessages::STANDBY_OFF)
			 			{
			 				*msg = NeutrinoMessages::STANDBY_OFF;
			 				*data = 0;
			 			}
						else
							printf("[neutrino] event INITID_HTTPD - unknown eventID 0x%x\n",  emsg.eventID );
					}
					else if ( emsg.initiatorID == CEventServer::INITID_SECTIONSD )
			 		{
			 			//printf("[neutrino] event - from SECTIONSD %x %x\n", emsg.eventID, *(unsigned*) p);
			 			if (emsg.eventID==CSectionsdClient::EVT_TIMESET)
			 			{
			 				*msg = NeutrinoMessages::EVT_TIMESET;

			 				struct timeval tv;
        					gettimeofday( &tv, NULL );
        					long long timeOld = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);

        					stime((time_t*) p);

                            gettimeofday( &tv, NULL );
        					long long timeNew = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);

							delete p;
							p= new unsigned char[ sizeof(long long) ];
        					*(long long*) p = timeNew - timeOld;

							// Timer anpassen
							for ( vector<timer>::iterator e= timers.begin(); e!= timers.end(); ++e )
								if (e->correct_time)
									e->times_out+= *(long long*) p;

                            *data = (unsigned) p;
			 				dont_delete_p = true;
			 			}
			 			else if (emsg.eventID==CSectionsdClient::EVT_GOT_CN_EPG)
			 			{
			 				*msg = NeutrinoMessages::EVT_CURRENTNEXT_EPG;
			 				*data = *(unsigned*) p;
			 			}
			 			else
			 				printf("[neutrino] event INITID_SECTIONSD - unknown eventID 0x%x\n",  emsg.eventID );
			 		}
			 		else if ( emsg.initiatorID == CEventServer::INITID_ZAPIT )
			 		{
			 			//printf("[neutrino] event - from ZAPIT %x %x\n", emsg.eventID, *(unsigned*) p);
			 			if (emsg.eventID==CZapitClient::EVT_RECORDMODE_ACTIVATED)
			 			{
			 				*msg = NeutrinoMessages::EVT_RECORDMODE;
			 				*data = true;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_RECORDMODE_DEACTIVATED)
			 			{
			 				*msg = NeutrinoMessages::EVT_RECORDMODE;
			 				*data = false;
			 			}
						else if (emsg.eventID==CZapitClient::EVT_ZAP_COMPLETE)
			 			{
			 				*msg = NeutrinoMessages::EVT_ZAP_COMPLETE;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_ZAP_FAILED)
			 			{
			 				*msg = NeutrinoMessages::EVT_ZAP_FAILED;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_ZAP_SUB_FAILED)
			 			{
			 				*msg = NeutrinoMessages::EVT_ZAP_SUB_FAILED;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_ZAP_COMPLETE_IS_NVOD)
			 			{
			 				*msg = NeutrinoMessages::EVT_ZAP_ISNVOD;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_ZAP_SUB_COMPLETE)
			 			{
			 				*msg = NeutrinoMessages::EVT_ZAP_SUB_COMPLETE;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_SCAN_COMPLETE)
			 			{
			 				*msg = NeutrinoMessages::EVT_SCAN_COMPLETE;
			 				*data = 0;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_SCAN_NUM_TRANSPONDERS)
			 			{
			 				*msg = NeutrinoMessages::EVT_SCAN_NUM_TRANSPONDERS;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_SCAN_NUM_CHANNELS)
			 			{
			 				*msg = NeutrinoMessages::EVT_SCAN_NUM_CHANNELS;
			 				*data = *(unsigned*) p;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_SCAN_PROVIDER)
			 			{
			 				*msg = NeutrinoMessages::EVT_SCAN_PROVIDER;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_SCAN_SATELLITE)
			 			{
			 				*msg = NeutrinoMessages::EVT_SCAN_SATELLITE;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
			 			}
			 			else if (emsg.eventID==CZapitClient::EVT_BOUQUETS_CHANGED)
			 			{
			 				*msg = NeutrinoMessages::EVT_BOUQUETSCHANGED;
			 				*data = 0;
			 			}
			 			else
			 				printf("[neutrino] event INITID_ZAPIT - unknown eventID 0x%x\n",  emsg.eventID );
			 		}
			 		else if ( emsg.initiatorID == CEventServer::INITID_TIMERD )
			 		{
/*
						if (emsg.eventID==CTimerdClient::EVT_ANNOUNCE_NEXTPROGRAM)
			 			{
						}

						if (emsg.eventID==CTimerdClient::EVT_NEXTPROGRAM)
			 			{
			 				*msg = NeutrinoMessages::EVT_NEXTPROGRAM;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
			 			}
*/
						if (emsg.eventID==CTimerdClient::EVT_ANNOUNCE_RECORD)
			 			{
							*msg = NeutrinoMessages::ANNOUNCE_RECORD;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
						}
						else if (emsg.eventID==CTimerdClient::EVT_ANNOUNCE_ZAPTO)
			 			{
							*msg = NeutrinoMessages::ANNOUNCE_ZAPTO;
							*data = 0;							
						}
						else if (emsg.eventID==CTimerdClient::EVT_ANNOUNCE_SHUTDOWN)
			 			{
							*msg = NeutrinoMessages::ANNOUNCE_SHUTDOWN;
							*data = 0;							
						}
						else if (emsg.eventID==CTimerdClient::EVT_ANNOUNCE_SLEEPTIMER)
			 			{
							*msg = NeutrinoMessages::ANNOUNCE_SLEEPTIMER;
							*data = 0;							
						}
						else if (emsg.eventID==CTimerdClient::EVT_SLEEPTIMER)
			 			{
							*msg = NeutrinoMessages::SLEEPTIMER;
							*data = 0;							
						}
						else if (emsg.eventID==CTimerdClient::EVT_RECORD_START)
			 			{
							*msg = NeutrinoMessages::RECORD_START;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
						}
						else if (emsg.eventID==CTimerdClient::EVT_RECORD_STOP)
			 			{
							*msg = NeutrinoMessages::RECORD_STOP;
							*data = 0;
						}
						else if (emsg.eventID==CTimerdClient::EVT_ZAPTO)
			 			{
							*msg = NeutrinoMessages::ZAPTO;
							*data = (unsigned)  p;
			 				dont_delete_p = true;
						}
						else if (emsg.eventID==CTimerdClient::EVT_SHUTDOWN)
			 			{
			 				*msg = NeutrinoMessages::SHUTDOWN;
			 				*data = 0;
			 			}
			 			else if (emsg.eventID==CTimerdClient::EVT_STANDBY_ON)
			 			{
			 				*msg = NeutrinoMessages::STANDBY_ON;
			 				*data = 0;
			 			}
			 			else if (emsg.eventID==CTimerdClient::EVT_STANDBY_OFF)
			 			{
							*msg = NeutrinoMessages::STANDBY_OFF;
			 				*data = 0;
			 			}
						else if (emsg.eventID==CTimerdClient::EVT_REMIND)
			 			{
							*msg = NeutrinoMessages::REMIND;
			 				*data = (unsigned) p;
			 				dont_delete_p = true;
						}
			 			else
			 				printf("[neutrino] event INITID_TIMERD - unknown eventID 0x%x\n",  emsg.eventID );

			 		}
			 		else
			 			printf("[neutrino] event - unknown initiatorID 0x%x\n",  emsg.initiatorID);

			 		if ( !dont_delete_p )
			 		{
			 			delete p;
			 			p= NULL;
			 		}
			 	}
			}
			else
			{
				printf("[neutrino] event - read failed!\n");
			}

			::close(fd_eventclient);

			if ( *msg != RC_nokey )
			{
				// raus hier :)
				//printf("[neutrino] event 0x%x\n", *msg);
				return;
			}
		}

		if(FD_ISSET(fd_rc, &rfds))
		{
			status = read(fd_rc, &rc_key, sizeof(rc_key));
			if (status==2)
			{
				if(rc_key!=0x5cfe)
				{
					//printf("got key native key: %04x %04x, translate: %04x -%s-\n", rc_key, rc_key&0x1f, translate(rc_key), getKeyName(translate(rc_key)).c_str() );
					long long now_pressed;
					bool keyok = true;

					gettimeofday( &tv, NULL );
					now_pressed = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);
					//printf("diff: %lld - %lld = %lld should: %d\n", now_pressed, last_keypress, now_pressed-last_keypress, repeat_block);

					//alter nokia-rc-code - lastkey löschen weil sonst z.b. nicht zweimal nacheinander ok gedrückt werden kann
					if((rc_key&0xff00)==0x5c00)
					{
						rc_last_key = 0;
					}
					//test auf wiederholenden key (gedrückt gehalten)
					if (rc_key == rc_last_key)
					{
						keyok = false;
						//nur diese tasten sind wiederholbar
						int trkey = translate(rc_key);
						if  ( (trkey==RC_up) || (trkey==RC_down) || (trkey==RC_plus) || (trkey==RC_minus) || (trkey==RC_standby) ||
							  ((bAllowRepeatLR) && ((trkey==RC_left) || (trkey==RC_right))) )
						{
							if( rc_last_repeat_key!=rc_key )
							{
								if(abs(now_pressed-last_keypress)>repeat_block)
								{
									keyok = true;
									rc_last_repeat_key = rc_key;
								}
							}
							else
							{
									keyok = true;
							}
						}
					}
					else
					{
						rc_last_repeat_key = 0;
					}
					rc_last_key = rc_key;
                    //printf("!!!!!!!  native key: %04x %04x\n", rc_key, rc_key&0x1f );
					if(abs(now_pressed-last_keypress)>repeat_block_generic)
					{
						if(keyok)
						{
							last_keypress = now_pressed;
							uint trkey= translate(rc_key);
							//printf("--!!!!!  translated key: %04x\n", trkey );
							if (trkey!=RC_nokey)
							{
								*msg = trkey;
								*data = 0;
								return;
							}
						}
					}

				}
			}
		}

		if(FD_ISSET(fd_pipe_low_priority[0], &rfds))
		{
			uint buf[2];
			read(fd_pipe_low_priority[0], &buf, sizeof(buf));
			*msg = buf[0];
			*data = buf[1];
//printf("got event from low-pri pipe %x %x\n", *msg, *data );
			return;
		}

        if ( InitialTimeout == 0 )
		{
			//nicht warten wenn kein key da ist
			*msg = RC_timeout;
			*data = 0;
			return;
		}
		else
		{
			//timeout neu kalkulieren
			gettimeofday( &tv, NULL );
			long long getKeyNow = (long long) tv.tv_usec + (long long)((long long) tv.tv_sec * (long long) 1000000);
			long long diff = (getKeyNow - getKeyBegin);
			if( Timeout <= (unsigned long long) diff )
			{
				*msg = RC_timeout;
				*data = 0;
				return;
			}
			else
				Timeout -= diff;
		}
	}
}

void CRCInput::postMsg(uint msg, uint data, bool Priority)
{
//	printf("postMsg %x %x %d\n", msg, data, Priority );
	uint buf[2];
	buf[0] = msg;
	buf[1] = data;
	if ( Priority )
		write(fd_pipe_high_priority[1], &buf, sizeof(buf));
	else
		write(fd_pipe_low_priority[1], &buf, sizeof(buf));
}


void CRCInput::clearRCMsg()
{
	__u16 rc_key;
	int status;

	if (fd_rc)
	{
		do
		{
    		status = read(fd_rc, &rc_key, sizeof(rc_key));
		} while (status== 2);
	}
}

/**************************************************************************
*       isNumeric - test if key is 0..9
*
**************************************************************************/
bool CRCInput::isNumeric(unsigned int key)
{
	if( (key>=RC_0) && (key<=RC_9))
		return true;
	else
		return false;
}

/**************************************************************************
*       transforms the rc-key to string
*
**************************************************************************/
string CRCInput:: getKeyName(int code)
{
	switch(code)
	{
			case RC_standby:
			return "standby";
			case RC_home:
			return "home";
			case RC_setup:
			return "setup";
			case RC_0:
			return "0";
			case RC_1:
			return "1";
			case RC_2:
			return "2";
			case RC_3:
			return "3";
			case RC_4:
			return "4";
			case RC_5:
			return "5";
			case RC_6:
			return "6";
			case RC_7:
			return "7";
			case RC_8:
			return "8";
			case RC_9:
			return "9";
			case RC_red:
			return "red button";
			case RC_green:
			return "green button";
			case RC_yellow:
			return "yellow button";
			case RC_blue:
			return "blue button";
			case RC_page_up:
			return "page up";
			case RC_page_down:
			return "page down";
			case RC_up:
			return "cursor up";
			case RC_down:
			return "cursor down";
			case RC_left:
			return "cursor left";
			case RC_right:
			return "cursor right";
			case RC_ok:
			return "ok";
			case RC_plus:
			return "vol. inc";
			case RC_minus:
			return "vol. dec";
			case RC_spkr:
			return "mute";
			case RC_help:
			return "help";
			case RC_top_left:
			return "cursor top+left";
			case RC_top_right:
			return "cursor top+right";
			case RC_bottom_left:
			return "cursor bottom+left";
			case RC_bottom_right:
			return "cursor bottom+right";
			case RC_timeout:
			return "timeout";
			case RC_nokey:
			return "none";

			default:
			return "unknown";
	}
}


/**************************************************************************
*	transforms the rc-key to generic - internal use only!
*
**************************************************************************/
int CRCInput::translate(int code)
{

	//printf("try to translate key: %d\n", code);

	if ((code&0xFF00)==0x5C00)
	{
		switch (code&0xFF)
		{
				case 0x0C:
				return RC_standby;
				case 0x20:
				return RC_home;
				case 0x27:
				return RC_setup;
				case 0x00:
				return RC_0;
				case 0x01:
				return RC_1;
				case 0x02:
				return RC_2;
				case 0x03:
				return RC_3;
				case 0x04:
				return RC_4;
				case 0x05:
				return RC_5;
				case 0x06:
				return RC_6;
				case 0x07:
				return RC_7;
				case 0x08:
				return RC_8;
				case 0x09:
				return RC_9;
				case 0x3B:
				return RC_blue;
				case 0x52:
				return RC_yellow;
				case 0x55:
				return RC_green;
				case 0x2D:
				return RC_red;
				case 0x54:
				return RC_page_up;
				case 0x53:
				return RC_page_down;
				case 0x0E:
				return RC_up;
				case 0x0F:
				return RC_down;
				case 0x2F:
				return RC_left;
				case 0x2E:
				return RC_right;
				case 0x30:
				return RC_ok;
				case 0x16:
				return RC_plus;
				case 0x17:
				return RC_minus;
				case 0x28:
				return RC_spkr;
				case 0x82:
				return RC_help;
				default:
				//perror("unknown old rc code");
				return RC_nokey;
		}
	}
	else if ((code&0xFF00)==0xFF00)
	{
		//Fronttasten
		//printf("-!!!!!!  before 0xFF key: %04x\n", code );
		switch (code&0xFF)
		{
				case 0x12:
				case 0x9d:
				return RC_standby;
				case 0x48:
				case 0xab:
				return RC_down;
				case 0x24:
				case 0xc7:
				return RC_up;
				case 0x20:
				case 0x40:
				case 0xaf:
				case 0xcf:
				return RC_nokey;
				case 0x10:
				case 0x9f:
				return RC_standby_release;
		}
	}
	else if (!(code&0x00))
	{
		//printf("-!!!!!!  before not-translated key: %04x\n", code );
		return code&0x3F;
	}
	//else
	//perror("unknown rc code");
	return RC_nokey;
}
