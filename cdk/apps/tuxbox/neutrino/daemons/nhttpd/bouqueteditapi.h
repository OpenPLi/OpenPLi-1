/*      
        webserver  -   DBoxII-Project

        Copyright (C) 2001/2002 Dirk Szymanski 'Dirch'

        $Id: bouqueteditapi.h,v 1.3 2002/10/15 20:39:47 woglinde Exp $

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


#ifndef __bouqueteditapi__
#define __bouqueteditapi__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/timeb.h>

#include "request.h"
#include "webdbox.h"


using namespace std;

//-------------------------------------------------------------------------
class CBouqueteditAPI
{
	private:
		CWebDbox * Parent;

	bool showBouquets(CWebserverRequest* request);
	bool addBouquet(CWebserverRequest* request);
	bool moveBouquet(CWebserverRequest* request);
	bool deleteBouquet(CWebserverRequest* request);
	bool saveBouquet(CWebserverRequest* request);
	bool renameBouquet(CWebserverRequest* request);
	bool editBouquet(CWebserverRequest* request);
	bool changeBouquet(CWebserverRequest* request);
	bool setBouquet(CWebserverRequest* request);


	public:
		CBouqueteditAPI(CWebDbox *parent){Parent = parent;};
		~CBouqueteditAPI(){};
		bool Execute(CWebserverRequest* request);
};

#endif
