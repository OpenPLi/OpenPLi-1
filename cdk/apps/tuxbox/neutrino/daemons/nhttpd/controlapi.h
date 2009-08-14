/*      
        webserver  -   DBoxII-Project

        Copyright (C) 2001/2002 Dirk Szymanski 'Dirch'

        $Id: controlapi.h,v 1.3 2002/10/18 12:42:13 thegoodguy Exp $

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


#ifndef __controlapi__
#define __controlapi__

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "request.h"
#include "webdbox.h"


class CControlAPI
{
	private:
		CWebDbox * Parent;

// send functions for ExecuteCGI (controld api)
		void SendEventList(CWebserverRequest *request,t_channel_id channel_id);
		void SendcurrentVAPid(CWebserverRequest* request);
		void SendSettings(CWebserverRequest* request);
		void SendStreamInfo(CWebserverRequest* request);
		void SendBouquets(CWebserverRequest *request);
		void SendBouquet(CWebserverRequest *request,int BouquetNr);
		void SendChannelList(CWebserverRequest *request);
		void SendTimers(CWebserverRequest* request);


// CGI functions for ExecuteCGI
		bool TimerCGI(CWebserverRequest *request);
		bool SetModeCGI(CWebserverRequest *request);
		bool StandbyCGI(CWebserverRequest *request);
		bool GetDateCGI(CWebserverRequest *request);
		bool GetTimeCGI(CWebserverRequest *request);
		bool SettingsCGI(CWebserverRequest *request);
		bool GetServicesxmlCGI(CWebserverRequest *request);
		bool GetBouquetsxmlCGI(CWebserverRequest *request);
		bool GetChannel_IDCGI(CWebserverRequest *request);
		bool MessageCGI(CWebserverRequest *request);
		bool InfoCGI(CWebserverRequest *request);
		bool ShutdownCGI(CWebserverRequest *request);
		bool VolumeCGI(CWebserverRequest *request);
		bool ChannellistCGI(CWebserverRequest *request);
		bool GetBouquetCGI(CWebserverRequest *request);
		bool GetBouquetsCGI(CWebserverRequest *request);
		bool EpgCGI(CWebserverRequest *request);
		bool VersionCGI(CWebserverRequest *request);
		bool ZaptoCGI(CWebserverRequest *request);

	public:
		CControlAPI(CWebDbox *parent){Parent = parent;};
		~CControlAPI(){};
		bool Execute(CWebserverRequest* request);
};

#endif
