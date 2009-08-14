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

#ifndef __MISCMENU_H__
#define __MISCMENU_H__

#include <lib/gui/ewindow.h>
#include <lib/gui/enumber.h>

#include "defs.h"
#include "util.h"

#define MISC_MENU_TOPLEFTX	90
#define MISC_MENU_TOPLEFTY	70
#define MISC_MENU_WIDTH		550
#define MISC_MENU_HEIGHT	300

#define OFFSET_ROW_Y		50
#define OFFSET_ROW_HEIGHT	30

#define STEP_OFFSET_Y		40
#define START_LABEL_X		5
#define START_LABEL_WIDTH	350
#define START_NUM_X		435//START_LABEL_X + START_LABEL_WIDTH + 5
#define START_NUM_WIDTH		100

#define END_LABEL_X		( ( MISC_MENU_WIDTH - 10 ) / 2 ) + 5
#define END_LABEL_WIDTH		150
#define END_NUM_X		435//END_LABEL_X + END_LABEL_WIDTH + 5
#define END_NUM_WIDTH		100

#define WINDOW_OFFSET_LABEL_X		( ( MISC_MENU_WIDTH - 10 ) / 2 ) + 5
#define WINDOW_OFFSET_ROW_Y		OFFSET_ROW_Y + OFFSET_ROW_HEIGHT + 10
#define WINDOW_OFFSET_LABEL_WIDTH	150

#define WINDOW_NUM_X		WINDOW_OFFSET_LABEL_X + WINDOW_OFFSET_LABEL_WIDTH + 5
#define WINDOW_NUM_WIDTH	100

#define AR_LABEL_X		260//START_LABEL_X
#define AR_Y			WINDOW_OFFSET_ROW_Y + OFFSET_ROW_HEIGHT + 5
#define AR_LABEL_WIDTH		250
#define AR_X			AR_LABEL_X + AR_LABEL_WIDTH + 5
#define AR_WIDTH		280
#define AR_HEIGHT		OFFSET_ROW_HEIGHT

class MiscMenu : public eWindow
{
	struct Conf &conf;
	int startOffInt, endOffInt, winOffInt;

	eNumber *startOffsetP, *endOffsetP, *winOffsetP;
	void startOffsetChanged( void );
	void endOffsetChanged( void );
	void winOffsetChanged( void );
	void aspectRatioChanged( eListBoxEntryText *ep );
public:
	MiscMenu( struct Conf &cnf );
};

#endif

