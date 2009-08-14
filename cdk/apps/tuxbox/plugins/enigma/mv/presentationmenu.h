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

#ifndef __PRESENTATIONMENU_H__
#define __PRESENTATIONMENU_H__

#include <lib/gui/listbox.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include "defs.h"
#include "channel.h"
#include "presmisc.h"

#include "util.h"

#define PM_CHANNEL_SPACING			2

#define PM_COLOUR_STD_BLACK			0
#define PM_COLOUR_EPG_ENTRY_BACKGROUND 		1
#define PM_COLOUR_CONTENT			2
#define PM_COLOUR_ESTATUSBAR_BACKGROUND		3
#define PM_COLOUR_GLOBAL_NORMAL_BACKGROUND	4
#define PM_COLOUR_EWINDOW_TITLEBAR		5
#define PM_COLOUR_BACKGROUND			6
#define PM_COLOUR_STD_BLUE			7
#define PM_COLOUR_STD_DBLUE			8
#define PM_COLOUR_BLUE_NONTRANS			9
#define PM_COLOUR_BLUE				10
#define PM_COLOUR_STD_DRED			11
#define PM_COLOUR_STD_RED			12
#define PM_COLOUR_GLOBAL_SELECTED_BACKGROUND	13
#define PM_COLOUR_STD_DYELLOW			14
#define PM_COLOUR_EPG_ENTRY_BACKGROUND_SELECTED	15
#define PM_COLOUR_STD_DGREEN			16
#define PM_COLOUR_GREEN				17

#define PM_MAX_NUM_COLOURS			18
#define PM_MAX_COLOUR_NAME_LENGTH		30

#define PM_FONT_EPG_TITLE			0
#define PM_FONT_EPG_DESCR			1
#define PM_FONT_EPG_TIME			2
#define PM_MAX_NUM_FONT_FAMILIES		3
#define PM_MAX_FONT_FAMILY_NAME_LENGTH		30
#define PM_MIN_FONT_SIZE			12
#define PM_MAX_FONT_SIZE			40

#define PM_TOPLEFTX		65
#define PM_TOPLEFTY		65
#define PM_WIDTH		570
#define PM_HEIGHT		480

#define PM_BUTTON_WIDTH		120
#define PM_BUTTON_HEIGHT	40

#define PM_CHANNEL_ROW_Y	70
#define PM_PROGRAM_ROW_Y	190
#define PM_MISC_ROW_Y		250
#define PM_GEN_ROW_Y		360

#define PM_TESTCHANNEL_TOPLEFTX  	5
#define PM_TESTCHANNEL_MAX_HEIGHT      60
#define PM_TESTCHANNEL_TOPLEFTY  PM_HEIGHT - PM_TESTCHANNEL_MAX_HEIGHT - 30
#define PM_TESTCHANNEL_WIDTH	 PM_WIDTH - PM_OK_BUTTON_WIDTH - 70

enum {
	pmChanBackButton = 0,
	pmChanFontSizeButton,
	pmChanFontFamilyButton,

	pmProgOneBackButton,
	pmProgTwoBackButton,
	pmProgFontSizeButton,
	pmProgFontFamilyButton,

	pmMiscPlayingButton,
	pmMiscFavouriteButton,
	pmMiscFilmButton,
	pmMiscWindowButton,

	pmNoButtons
};

class Channel;
class PresentationMenu : public eWindow
{
	gColor selectedForeColour;

	eButton *buttons[pmNoButtons];

	Channel *testChannel;

	struct ViewPresentation &presRef;

	void rebuildTestChannel(void);

	void setButtonColour( int buttonNo, int colour);
	void setButtonColours( void );

	void handleFontSizePress( int *toChange );
	void handleFontFamilyPress( int *toChange );
	void handleColourPress( int *toChange );

	void chanFontFamilyButtonPressed( void);
	void chanFontSizeButtonPressed( void );
	void chanBackButtonPressed( void );

	void progTwoBackButtonPressed( void );
	void progOneBackButtonPressed( void );
	void progFontFamilyButtonPressed( void );
	void progFontSizeButtonPressed( void );

	void miscWindowButtonPressed( void );
	void miscFilmButtonPressed( void );
	void miscFavouriteButtonPressed( void );
	void miscPlayingButtonPressed( void );

	void focusChangedReceive( const eWidget * bob );

public:
	void display( void );
        PresentationMenu( struct ViewPresentation &options, int currentView );
};

#endif
