/*
	Neutrino-GUI  -   DBoxII-Project

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

#include <dbox/event.h>

#include <eventserver.h>
#include <controldclient/controldclient.h>

#include "eventwatchdog.h"


CEventWatchDog::CEventWatchDog()
{
	bThreadRunning = false;

	Notifiers.insert( pair<uint, EventWatchdogNotifiers*>(WDE_VIDEOMODE, new EventWatchdogNotifiers));
	Notifiers.insert( pair<uint, EventWatchdogNotifiers*>(WDE_VCRONOFF, new EventWatchdogNotifiers));
	startThread();

}

void CEventWatchDog::startThread()
{
	pthread_mutex_init( &wd_mutex, NULL );

	if (pthread_create (&thrSender, NULL, watchdogThread, (void *) this) != 0 )
	{
		perror("CWatchdog: Create WatchDogThread failed\n");
	}
	bThreadRunning = true;
}

int CEventWatchDog::getVideoMode()
{
	int bitInfo = 0;
	char buffer[100];
	FILE* fp = fopen("/proc/bus/bitstream", "r");
	while (!feof(fp))
	{
		fgets(buffer, 100, fp);
		sscanf(buffer, "A_RATIO: %d", &bitInfo);
	}
	fclose(fp);
	return bitInfo;
}

int CEventWatchDog::getVCRMode()
{
	int val;
	int fp = open("/dev/dbox/fp0",O_RDWR);

	ioctl(fp, FP_IOCTL_GET_VCR, &val);

	close(fp);
	//printf("getVCRMode= %d\n", val);
	return val;
}

void CEventWatchDog::videoModeChanged( int nNewVideoMode )
{
	EventWatchdogNotifiers* notifiers = Notifiers.find(WDE_VIDEOMODE)->second;
	for (uint i=0; i<notifiers->size(); i++ )
	{
		((CAspectRatioNotifier*)(*notifiers)[i])->aspectRatioChanged( nNewVideoMode );
	}
	eventServer->sendEvent(CControldClient::EVT_MODECHANGED, CEventServer::INITID_CONTROLD, &nNewVideoMode, sizeof(nNewVideoMode));
}

void CEventWatchDog::vcrModeChanged( int nNewVCRMode )
{
	EventWatchdogNotifiers* notifiers = Notifiers.find(WDE_VCRONOFF)->second;
	for (uint i=0; i<notifiers->size(); i++ )
	{
		((CVCRModeNotifier*)(*notifiers)[i])->VCRModeChanged( nNewVCRMode );
	}
	eventServer->sendEvent(CControldClient::EVT_VCRCHANGED, CEventServer::INITID_CONTROLD, &nNewVCRMode, sizeof(nNewVCRMode));
}

void* CEventWatchDog::watchdogThread (void *arg)
{
	//printf("[controld] watchdogThread-pid: %d\n", getpid());
	try
	{

		CEventWatchDog* WatchDog = (CEventWatchDog*) arg;

		int fd_ev;

		fd_set rfds;
		struct timeval tvselect;

		struct event_t event;

        if ( (fd_ev = open( EVENT_DEVICE, O_RDWR | O_NONBLOCK ) ) < 0)
		{
			perror("open");
			return NULL;
		}

        if ( ioctl(fd_ev, EVENT_SET_FILTER, EVENT_VCR_CHANGED | EVENT_ARATIO_CHANGE /*| EVENT_VHSIZE_CHANGE*/ ) < 0 )
		{
			perror("ioctl");
			close(fd_ev);
			return NULL;
		}

		while (1)
		{
		    FD_ZERO(&rfds);
			FD_SET(fd_ev, &rfds);
			tvselect.tv_sec = 1;
			tvselect.tv_usec = 0;

			int status =  select(fd_ev+1, &rfds, NULL, NULL, &tvselect);

			if(FD_ISSET(fd_ev, &rfds))
			{
				do
				{
					//printf("[controld] before read\n", status);
					status = read(fd_ev, &event, sizeof(event));
					//printf("[controld] read result <%d>\n", status);
					if ( status == sizeof(event) )
					{
						if (event.event == EVENT_ARATIO_CHANGE)
						{
    		           		//printf("(event.event == EVENT_ARATIO_CHANGE)\n");
							int newVideoMode = WatchDog->getVideoMode();
							if ( (newVideoMode != WatchDog->VideoMode) && (newVideoMode != -1) )
							{
								pthread_mutex_lock( &WatchDog->wd_mutex );
								WatchDog->VideoMode = (uint)newVideoMode;
								WatchDog->videoModeChanged( newVideoMode );
								pthread_mutex_unlock( &WatchDog->wd_mutex );
							}
						}
						else if ( event.event == EVENT_VCR_CHANGED )
						{
    	        		   	//printf("(event.event == EVENT_VCR)\n");
							int newVCRMode = WatchDog->getVCRMode();
							if ( (newVCRMode != WatchDog->VCRMode) )
							{
								pthread_mutex_lock( &WatchDog->wd_mutex );
								WatchDog->VCRMode = newVCRMode;
								WatchDog->vcrModeChanged( newVCRMode );
								pthread_mutex_unlock( &WatchDog->wd_mutex );
							}
						}
					}
				} while ( status == sizeof(event) );
			}
		}

		close(fd_ev);
	}
	catch (std::exception& e)
	{
		fprintf(stderr, "[controld] caught std-exception ineventwatchdog %s!\n", e.what());
	}
	catch (...)
	{
	    fprintf(stderr, "[controld] caught exception in eventwatchdog!\n");
  	}

	pthread_exit(NULL);
}

void CEventWatchDog::registerNotifier( uint watchdogEvent, CEventWatchdogNotifier* notifier )
{
	if (bThreadRunning)
		pthread_mutex_lock( &wd_mutex );

	Notifiers.find(watchdogEvent)->second->insert( Notifiers.find(watchdogEvent)->second->end(), notifier);

	if (watchdogEvent== WDE_VIDEOMODE)
	{
		videoModeChanged( getVideoMode() );
	}
	if (watchdogEvent== WDE_VCRONOFF)
	{
		vcrModeChanged( getVCRMode() );
	}

	if (bThreadRunning)
		pthread_mutex_unlock( &wd_mutex );

	if (!bThreadRunning)
		startThread();
}

void CEventWatchDog::unregisterNotifier( uint watchdogEvent, CEventWatchdogNotifier* notifier )
{
	if (bThreadRunning)
		pthread_mutex_lock( &wd_mutex );

	EventWatchdogNotifiers* notifiers = Notifiers.find(watchdogEvent)->second;
	EventWatchdogNotifiers::iterator it;
	for (it=notifiers->end(); it>=notifiers->begin(); it--)
	{
		if (*it == notifier)
		{
			notifiers->erase(it);
		}
	}

	if (bThreadRunning)
		pthread_mutex_unlock( &wd_mutex );
}

