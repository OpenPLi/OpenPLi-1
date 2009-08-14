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

#ifndef MVLITE
	#include <lib/gui/ewindow.h>
	#include <lib/gui/listbox.h>
	#include <lib/gui/ebutton.h>
#endif

#include "util.h"
#include "enigmacache.h"
#include "filemap.h"

class ChannelManager : public FileMap
#ifndef MVLITE
	, public eWindow 
#endif
{
#ifndef MVLITE
	int showFlags;
	eListBox<eListBoxEntryText> * fromBoxP, *toBoxP;
	eLabel * toLabelP;
	std::list<eServiceReferenceDVB> *serviceList;

	void rebuildFromBox( void );
	void rebuildToBox( void );

	void fromSelectedItem( eListBoxEntryText *item );
	void fromSelectionChanged( eListBoxEntryText *item );
	void toSelectedItem( eListBoxEntryText *item );
	bool matchesCurrentShowFlags( eString name );

	void ignoreButtonHandle( void );
	void mappedButtonHandle( void );
	void unknownButtonHandle( void );
	void rebuild( void );
#endif

public:
	ChannelManager( eString path );

	void addInput( eString originalName );
#ifndef MVLITE
	void build( std::list<eServiceReferenceDVB> * list );
#endif
};


#endif
