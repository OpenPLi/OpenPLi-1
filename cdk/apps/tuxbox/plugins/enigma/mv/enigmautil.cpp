/***************************************************************/
/*
    * Copyright (C) 2004 Lee Wilmot <lee@dinglisch.net>

    This file is part of MV.

    MV is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    MV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You can find a copy of the GNU General Public License in the
    file gpl.txt distributed with this file.
*/
/*************************************************************/

#include "enigmautil.h"

void showTimerEditWindow( eWidget *parent, int type, Channel *cp, Program *pp, time_t preOffset, time_t postOffset )
{
	int timerType = ePlaylistEntry::SwitchTimerEntry;

        switch ( type ) {
                case timerEditRecord:
                        timerType = ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recDVR;
                        break;
                case timerEditNGRAB:
                        timerType = ePlaylistEntry::RecTimerEntry|ePlaylistEntry::recNgrab;
                        break;
                default:
                        break;
        }

	EITEvent tempEvent;

        tempEvent.start_time = pp->getStartTime() - preOffset;

        time_t end = pp->getEndTime() + postOffset;

        tempEvent.duration = (int) ( end - tempEvent.start_time );

        eServiceReferenceDVB sref = cp->getServiceRef();

	// Want to restore this afterwards, in case we disturb
	// something else in enigma (I'm not clear what it's used
	// for)

	eString oldServiceDescr = sref.descr;

	sref.descr = getFullServiceName( cp->getServiceRef() ) + eString( "/" ) + pp->getTitle();

        eTimerManager *timerMgrP = eTimerManager::getInstance();

        if ( type == timerEditDelete ) {
                ePlaylistEntry* p = timerMgrP->findEvent( &sref, &tempEvent );

                if ( p )
                        timerMgrP->removeEventFromTimerList( parent, *p,eTimerManager::erase );
                else
                        showTimerListWindow();
        }
        else {
                eTimerEditView bob( tempEvent, timerType, sref );
		showExecHide( &bob );
        }

	sref.descr = oldServiceDescr;

        timerMgrP->saveTimerList();
}


void showTimerListWindow( void )
{
        eTimerListView bob;
	showExecHide( &bob );
}

/*eServiceReferenceDVB getPlayingServiceRef( void )
{
        eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI
();
        return sapi->service;
}*/

