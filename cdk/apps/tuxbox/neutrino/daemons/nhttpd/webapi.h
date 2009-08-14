/*      
        webserver  -   DBoxII-Project

        Copyright (C) 2001/2002 Dirk Szymanski 'Dirch'

        $Id: webapi.h,v 1.4 2002/10/15 20:39:47 woglinde Exp $

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


#ifndef __webapi__
#define __webapi__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

#include "request.h"
#include "webdbox.h"


using namespace std;

//-------------------------------------------------------------------------
class CWebAPI
{
	private:
		CWebDbox * Parent;

// show functions for Execute (web api)
		bool ShowDboxMenu(CWebserverRequest* request);
		bool ShowTimerList(CWebserverRequest* request);
		bool ShowEventList(CWebserverRequest* request, t_channel_id channel_id);
		bool ShowBouquet(CWebserverRequest *request,int BouquetNr = -1);
		bool ShowBouquets(CWebserverRequest *request);
		bool ShowControlpanel(CWebserverRequest* request);
		bool ShowCurrentStreamInfo(CWebserverRequest* request);
		bool ShowEpg(CWebserverRequest* request,string EpgID,string Startzeit = "");
		bool ShowEPG(CWebserverRequest *request,string Title, string Info1, string Info2);
		bool ShowActualEpg(CWebserverRequest *request);

		bool Test(CWebserverRequest* request);
		bool Timer(CWebserverRequest* request);
		bool Info(CWebserverRequest* request);
		bool Dbox(CWebserverRequest* request);
		bool Channellist(CWebserverRequest* request);
		bool Controlpanel(CWebserverRequest* request);
		bool ActualEPG(CWebserverRequest* request);
		bool EPG(CWebserverRequest* request);
		bool Switch(CWebserverRequest* request);

		void loadTimerMain(CWebserverRequest* request);
		void correctTime(struct tm *zt);
		void showTimer(CWebserverRequest *request);
		void doModifyTimer(CWebserverRequest *request);
		void modifyTimerForm(CWebserverRequest *request, unsigned timerId);
		void newTimerForm(CWebserverRequest *request);
		void doNewTimer(CWebserverRequest *request);


	public:
		CWebAPI(CWebDbox *parent){Parent = parent;};
		~CWebAPI(){};
		bool Execute(CWebserverRequest* request);

	friend class CControlAPI;
};

#endif
