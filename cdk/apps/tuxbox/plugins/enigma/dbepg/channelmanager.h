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

#ifndef __CHANNELMANAGER_H__
#define __CHANNELMANAGER_H__


#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/ebutton.h>


#include "util.h"
#include "filemap.h"

class ChannelManager : public eWindow
{
    enum ShowFlags
    {
        CM_SHOW_UNKNOWN  = 1,
        CM_SHOW_IGNORE	 = 2,
        CM_SHOW_MAPPED   = 4,
    };
	ShowFlags showFlags;
	eListBox<eListBoxEntryText> *fromBoxP;
    eListBox<eListBoxEntryText> *toBoxP;

	eLabel *toLabelP;
    eLabel *lbCurrentMode;
	std::list<eServiceReferenceDVB> serviceList;
    std::map<int, eServiceReferenceDVB> serviceRefs;
    std::map<eString, eServiceReferenceDVB> mapKey2Ref;  //lijst met de serviceKeys van rt, gemapt naar de oorspronkelijke

    int numberOfChanIdKeys;
    std::map<int, eString> localChanIdMap;
    FileMap& fileMap;

	void rebuildFromBox( void );
	void rebuildToBox( void );

	void fromSelectedItem( eListBoxEntryText *item );
	void fromSelectionChanged( eListBoxEntryText *item );
	void toSelectedItem( eListBoxEntryText *item );
    bool showEntry(const eString& chanId);

    void doneButtonHandle( void );
	void ignoreButtonHandle( void );
	void mappedButtonHandle( void );
	void unknownButtonHandle( void );
	void rebuild( void );

    void loadServices();
    void addToList( const eServiceReference& ref );
    eString getRtServiceKey(const eServiceReferenceDVB& ref);
    const eString getMappedName(const eString& ref);
public:
	ChannelManager(FileMap& map);
    void newFromListBoxEntry(const eString& name, const eString& chanId);

	eString getMapOrAddUnknown( eString originalName );
	void build();

};


#endif
