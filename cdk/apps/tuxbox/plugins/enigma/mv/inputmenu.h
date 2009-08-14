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

#ifndef __INPUTMENU_H__
#define __INPUTMENU_H__

#include <lib/gui/listbox.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/combobox.h>

#include "defs.h"
#include "inputdefreader.h"
#include "channel.h"
#include "util.h"

#define INPUT_FLAGS_ALL  	0

#define IM_MAX_PRE_POST_DIGITS  6
#define IM_MAX_PRE		36000
#define IM_MAX_POST		36000

#define IM_TOPLEFTX			60
#define IM_TOPLEFTY		65
#define IM_WIDTH		590
#define IM_HEIGHT		480

#define IM_HEADER_ROW_Y		3
#define IM_HEADER_ROW_HEIGHT	32
#define IM_FIRST_INPUT_Y	IM_HEADER_ROW_Y + IM_HEADER_ROW_HEIGHT + 20
#define IM_DELTA_Y		40

#define IM_ROW_X		5
#define IM_ROW_HEIGHT		35
#define IM_ROW_WIDTH		IM_WIDTH

#define BOTTOM_HEIGHT		35

// ---- THIRD FROM BOTTOM ROW ----- //

#define BOTTOM_Y1		IM_HEIGHT - BOTTOM_HEIGHT - 180

// ---- SECOND FROM BOTTOM ROW ----- //

#define BOTTOM_Y2		BOTTOM_Y1 + BOTTOM_HEIGHT + 5

#define IM_AUTO_CHECKBOX_WIDTH	70
#define IM_DETAIL_PATH_HEIGHT	BOTTOM_HEIGHT
#define IM_DETAIL_PATH_WIDTH	200
#define IM_DETAIL_PATH_X_LABEL	5
#define IM_DETAIL_PATH_X_BOX	( IM_WIDTH / 2 ) - 50
#define IM_DETAIL_PATH_Y	BOTTOM_Y2
#define IM_AUTO_CHECKBOX_X	IM_DETAIL_PATH_X_BOX +  IM_DETAIL_PATH_WIDTH + 20
#define IM_AUTO_CHECKBOX_Y	IM_DETAIL_PATH_Y + IM_DETAIL_PATH_HEIGHT 

#define BOTTOM_Y3		BOTTOM_Y2 + BOTTOM_HEIGHT + 10

#define IM_STORAGE_PATH_Y	BOTTOM_Y3


// --------- BOTTOM ROW ------- //

#define BOTTOM_Y4		BOTTOM_Y3 + BOTTOM_HEIGHT + 5

#define IM_OK_BUTTON_HEIGHT	BOTTOM_HEIGHT
#define IM_OK_BUTTON_WIDTH	90
#define IM_OK_BUTTON_TOPLEFTX	IM_WIDTH - IM_OK_BUTTON_WIDTH  - 40
#define IM_OK_BUTTON_TOPLEFTY	BOTTOM_Y4

#define IM_MAX_CHANNELS_X		5
#define IM_MAX_CHANNELS_Y		BOTTOM_Y4
#define IM_MAX_CHANNELS_WIDTH		90
#define IM_MAX_CHANNELS_HEIGHT		BOTTOM_HEIGHT
#define IM_MAX_CHANNEL_DIGITS		3
#define IM_MAX_CHANNEL_MIN		1

#define IM_AUTO_RELOAD_CHECKBOX_WIDTH	150
#define IM_AUTO_RELOAD_CHECKBOX_X	IM_DETAIL_PATH_X_BOX

#define IM_PRE_WIDTH			100
#define IM_POST_WIDTH			100
#define IM_PRE_X 			5
#define IM_POST_X			IM_DETAIL_PATH_X_BOX

/***** ROW *****/

#define IMR_CHECKBOX_WIDTH	30
#define IMR_NAME_WIDTH		200
#define IMR_LOCAL_WIDTH		200

#define IMR_CHECKBOX_X		0
#define IMR_NAME_X		IMR_CHECKBOX_X + IMR_CHECKBOX_WIDTH + 5
#define IMR_LOCAL_X		IMR_NAME_X + IMR_NAME_WIDTH + 5
#define IMR_INTERNET_X		IMR_LOCAL_X + IMR_LOCAL_WIDTH + 5
#define IMR_INTERNET_WIDTH	clientrect.width() - IMR_INTERNET_X - 5

class InputMenuRow : public eWidget
{
	struct InputConf &confRef;
	InputDefReader &defs;


	eCheckbox *checkboxP;
	eComboBox *nameP;
	eLabel *localP, *internetP;

	void checkboxChecked( int value );
	void nameChanged( eListBoxEntryText *ep );
	void rebuild( void );
	void hideRestOfRow();
	void showRestOfRow();
public:
	InputMenuRow( eWidget *parent, InputConf &c, InputDefReader &def );
};

class InputMenu : public eWindow
{
	struct Inputs &inputsRef;
	struct InputMenuRow *rows[MAX_NO_INPUTS];
	eNumber *maxChanP;
	int maxChanInt;
	eNumber *preP, *postP;
	int preInt, postInt;

	InputDefReader defs;

	void maxChanChanged( void );
	void detailPathChanged( eListBoxEntryText *ep );
	void storagePathChanged( eListBoxEntryText *ep );
	void autoChecked( int value );
	void autoReloadChecked( int value );
	void preChanged( void );
	void postChanged( void );
public:
        InputMenu( struct Inputs &inputs );
        ~InputMenu();
};

#endif
