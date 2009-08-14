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

#ifndef __FEATUREMENU_H__
#define __FEATUREMENU_H__

#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>

#include "defs.h"
#include "util.h"

#define FM_MAX_NO_COLUMNS                 3
#define FM_NO_TIMEBAR_VALUES              5
#define FM_TOPLEFTX		58
#define FM_TOPLEFTY		80
#define FM_WIDTH		590
#define FM_HEIGHT		450

#define FM_OK_BUTTON_TOPLEFTX	SECOND_COLUMN_X
#define FM_OK_BUTTON_TOPLEFTY	10
#define FM_OK_BUTTON_WIDTH	80
#define FM_OK_BUTTON_HEIGHT	30

#define FM_SHIFT_TYPE_SCROLL	0
#define FM_SHIFT_TYPE_HALFPAGE	1
#define FM_SHIFT_TYPE_PAGE	2

enum {
	dayBarModeOff = 0,
	dayBarModeFixed,
	dayBarModePopup,
	dayBarNoModes
};

enum {
	channelHeaderFlagShowName = 1,
	channelHeaderFlagShowOrbital = 2,
	channelHeaderFlagShowNumber = 4,
	channelHeaderFlagShowIcon = 8
};

enum {
	initialPositionPlayingMid = 0,
	initialPositionTop,
	firstChannelNoItems
};

enum {
        sBarFlagChannelName = 1,
        sBarFlagDate = 2,
        sBarFlagTime = 4,
        sBarFlagDuration = 8,
        sBarFlagProgramName = 16,
        sBarFlagDescr = 32,
	sBarFlagEndTime = 64
};

enum {
 pBoxFlagTime = 1,
        pBoxFlagTimeDiff = 2,
        pBoxFlagDescr = 4,
        pBoxFlagChannel = 8,
        pBoxFlagDuration = 16,
        pBoxFlagEndTime = 32,
        pBoxFlagChannelIcon = 64,
        pBoxFlagCentreTitle = 128

};

class FeatureMenu : public eWindow
{
	int timebarValueToTimebarIndex( int value );

	void timebarChanged( eListBoxEntryText *ep );
	void daybarChanged( eListBoxEntryText *ep );
	void noColsChanged( eListBoxEntryText *ep );
	void entriesPerColChanged( eListBoxEntryText *ep );
	void programShiftChanged( eListBoxEntryText *ep );
	void channelShiftChanged( eListBoxEntryText *ep );
	void channelHeaderNameChanged( int newValue );
	void channelHeaderOrbitalChanged( int newValue );
	void channelHeaderNumberChanged( int newValue );
	void channelHeaderIconChanged( int newValue );
	void verticalSepChanged( int newValue );
	void horizontalSepChanged( int newValue );
	void forceCursorChanged( int newValue );
	void emptyChannelsChanged( int newValue );
	void statusBarChanged( int newValue );
	void statusBarChannelNameChanged( int newValue );
	void statusBarDateChanged( int newValue );
	void statusBarTimeChanged( int newValue );
	void statusBarDurationChanged( int newValue );
	void statusBarProgramNameChanged( int newValue );
	void statusBarDescrChanged( int newValue );
	void programBoxDescrChanged( int newValue );
	void programBoxChannelIconChanged( int newValue );
	void programBoxCentreTitleChanged( int newValue );
	void programBoxChannelChanged( int newValue );
	void programBoxStartChanged( int newValue );
	void programBoxEndChanged( int newValue );
	void programBoxDurationChanged( int newValue );
	void programBoxStartDiffChanged( int newValue );
	void sortChanged( eListBoxEntryText *ep );

	void nextViewChanged( eListBoxEntryText *ep );
	void firstChannelChanged( eListBoxEntryText *ep );
	void showElapsedBarsChanged( int newValue );

	struct ViewFeatures &featRef;
public:
        FeatureMenu( struct ViewFeatures &feat, int currentView, int featViewMode );
};

#endif


#ifndef __MENU_F2_H__
#define __MENU_F2_H__

#include <lib/gui/listbox.h>

#include "util.h"
//#include "text.h"

#define MENU_F_ITEM_NONE				0
#define MENU_F_ITEM_VIEW				1
#define MENU_F_ITEM_CHANNELS			2	
#define MENU_F_ITEM_PROGRAMMS			3
#define MENU_F_DESCRIPTIONS				4
#define MENU_F_EPGSTYLE					5
#define MENU_F_NOITEMS					5 // ???

#define MF_ADD_GLOBALVIEW				1
#define MF_ADD_CHANNELS					2	
#define MF_ADD_PROGRAMMS				3
#define MF_ADD_DESCRIPTIONS				4
#define MF_ADD_EPGSTYLE					5


#define MENU_F_TOPLEFTX		190
#define MENU_F_TOPLEFTY		100
#define MENU_F_WIDTH		310
#define MENU_F_HEIGHT		400


class MainMenuFeatured : public eListBoxWindow<eListBoxEntryText> {
        void listSelected( eListBoxEntryText* t );
public:
        MainMenuFeatured( int currentViewNo );
};

#endif
