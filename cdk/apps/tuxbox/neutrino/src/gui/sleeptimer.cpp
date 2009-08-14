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

/*
 * Code Konflikt - will be included in timerdaemon...
 * so this module will be removed...
*/


#include <stdlib.h>

#include <global.h>
#include <neutrino.h>

#include "sleeptimer.h"

#include "widget/stringinput.h"
#include "widget/messagebox.h"
#include "widget/hintbox.h"


//
// -- Input Widget for setting shutdown time
// -- Menue Handler Interface
// -- to fit the MenueClasses from McClean
// -- Add current channel to Favorites and display user messagebox
//

int CSleepTimerWidget::exec(CMenuTarget* parent, string)
{
	int    res = menu_return::RETURN_EXIT_ALL;
	int    shutdown_min;
	char   value[16];
	CStringInput  *inbox;

	if (parent)
	{
		parent->hide();
	}
   
	CTimerdClient * timerdclient = new CTimerdClient;

	shutdown_min = timerdclient->getSleepTimerRemaining();  // remaining shutdown time?
//	if(shutdown_min == 0)		// no timer set
//		shutdown_min = 10;		// set to 10 min default

	sprintf(value,"%03d",shutdown_min);
	inbox = new CStringInput("sleeptimerbox.title",value,3,"sleeptimerbox.hint1","sleeptimerbox.hint2","0123456789 ");
	inbox->exec (NULL, "");
	inbox->hide ();

	delete inbox;

	if(shutdown_min!=atoi(value))
	{
		shutdown_min = atoi (value);
		printf("sleeptimer min: %d\n",shutdown_min);
		if (shutdown_min == 0)			// if set to zero remove existing sleeptimer
		{
			if(timerdclient->getSleeptimerID() > 0)
			{
				timerdclient->removeTimerEvent(timerdclient->getSleeptimerID());
			}
		}
		else							// set the sleeptimer to actual time + shutdown mins and announce 1 min before
			timerdclient->setSleeptimer(time(NULL) + ((shutdown_min -1) * 60),time(NULL) + shutdown_min * 60,0);
	}
	delete timerdclient;
	return res;
}
