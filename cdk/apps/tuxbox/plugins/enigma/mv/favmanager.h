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

#ifndef __FAVMANAGER_H__
#define __FAVMANAGER_H__

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>

#include "util.h"
#include "favedit.h"
#include "favmanentry.h"
#include "inputmanager.h"

#define DEFAULT_FE_TITLE	""
#define DEFAULT_FE_DESCR	""
#define DEFAULT_FE_CHANNEL	""
#define DEFAULT_FE_TITLE_BOOL	favAndVal
#define DEFAULT_FE_DESCR_BOOL	favAndVal
#define DEFAULT_FE_HOUR_A	0
#define DEFAULT_FE_HOUR_B	23
#define DEFAULT_FE_HOUR_COMBINE favHourCombineInside
#define DEFAULT_FE_NOTIFY	false

#define FAV_MANAGER_X		70
#define FAV_MANAGER_Y		70
#define FAV_MANAGER_WIDTH	500
#define FAV_MANAGER_HEIGHT	400

#define BUTTON_HEIGHT	40
#define	BUTTON_ROW_Y	clientrect.height() - BUTTON_HEIGHT
#define BUTTON_WIDTH	( ( clientrect.width() / 4 ) - 5 )

#define FAV_LISTBOX_X		5
#define FAV_LISTBOX_Y		5
#define FAV_LISTBOX_WIDTH	clientrect.width() - 10
#define FAV_LISTBOX_HEIGHT	clientrect.height() - BUTTON_HEIGHT - 30

#define FAV_RED_BUTTON_X	5
#define FAV_GREEN_BUTTON_X	FAV_RED_BUTTON_X + BUTTON_WIDTH + 5
#define FAV_YELLOW_BUTTON_X	FAV_GREEN_BUTTON_X + BUTTON_WIDTH + 5
#define FAV_BLUE_BUTTON_X	FAV_YELLOW_BUTTON_X + BUTTON_WIDTH + 5



class FavManager : public eWindow
{
	eString 			filePath;
	eListBox<FavManEntry> *		theList;
	bool				dirtyFlag;
	bool				matchesInitialisedFlag;

	void selected( FavManEntry *entry );
	void deleteHandle( void );
	void addHandleOne( struct Favourite *newFavP = NULL );
	void addHandle( void );
	void editHandle( void );
	bool doEdit( struct Favourite *toEditP );
	void readFile( void );
	void writeFile( void );
public:
	FavManager( eString filePath );
	~FavManager();

	void addNameAndChannelEntry( eString pName, eString cName );
	void checkFavourite( struct ProgramData *pp, bool * isFav, bool * notifyFlag );
};

#endif
