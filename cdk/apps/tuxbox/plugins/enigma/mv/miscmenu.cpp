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

#include "miscmenu.h"

//int             timerStartOffsetSeconds;
 //       int             timerEndOffsetSeconds;

MiscMenu::MiscMenu( struct Conf &cnf )
	: eWindow(0), conf( cnf )
{
	setText( getStr( strMiscMenuTitle ) );
	setWidgetGeom( this,
		MISC_MENU_TOPLEFTX, MISC_MENU_TOPLEFTY,
        	MISC_MENU_WIDTH, MISC_MENU_HEIGHT
	);

	startOffInt = cnf.timerStartOffsetSeconds;
	endOffInt = cnf.timerEndOffsetSeconds;
	winOffInt = cnf.windowInitialStartOffsetSeconds;

	eLabel * tl = makeNewLabel( this, strLabelMiscMenuTimerStartOffset,
		START_LABEL_X, OFFSET_ROW_Y,
		START_LABEL_WIDTH, OFFSET_ROW_HEIGHT
	);

	startOffsetP = makeNewNumber( this, &startOffInt,
		START_NUM_X, OFFSET_ROW_Y,
		START_NUM_WIDTH, OFFSET_ROW_HEIGHT,
		4, 0, 3600
	);
	CONNECT( startOffsetP->numberChanged, MiscMenu::startOffsetChanged );
		
	tl = makeNewLabel( this, strLabelMiscMenuTimerEndOffset,
		START_LABEL_X, OFFSET_ROW_Y + STEP_OFFSET_Y,
		START_LABEL_WIDTH, OFFSET_ROW_HEIGHT
	);
	endOffsetP = makeNewNumber( this, &endOffInt,
		START_NUM_X, OFFSET_ROW_Y + STEP_OFFSET_Y,
		END_NUM_WIDTH, OFFSET_ROW_HEIGHT,
		4, 0, 3600
	);
	CONNECT( endOffsetP->numberChanged, MiscMenu::endOffsetChanged );

	// Window Start offset
	tl = makeNewLabel( this, strLabelMiscMenuViewOffset,
		START_LABEL_X, OFFSET_ROW_Y + STEP_OFFSET_Y*2,
		START_LABEL_WIDTH, OFFSET_ROW_HEIGHT
	);
	winOffsetP = makeNewNumber( this, &winOffInt,
		START_NUM_X, OFFSET_ROW_Y + STEP_OFFSET_Y*2,
		WINDOW_NUM_WIDTH, OFFSET_ROW_HEIGHT,
		4, 0, 3600
	);
	CONNECT( winOffsetP->numberChanged, MiscMenu::winOffsetChanged );

	// Aspect ratio selector

        makeNewLabel( this, strLabelMiscMenuAspectRatio, 
		START_LABEL_X, OFFSET_ROW_Y + STEP_OFFSET_Y*3,
		AR_LABEL_WIDTH, AR_HEIGHT
	);

	eComboBox *arsP =  makeNewComboBox(
                this,
		255, OFFSET_ROW_Y + STEP_OFFSET_Y*3, 
		AR_WIDTH, AR_HEIGHT
        );

        for ( int itemNo = 0; itemNo < 3; itemNo ++ )
                new eListBoxEntryText( *arsP, getStr( strMiscMenuAspectRatioFirst + itemNo ), (void*) itemNo );

        arsP->setCurrent( conf.aspectRatioMode );
        CONNECT( arsP->selchanged, MiscMenu::aspectRatioChanged );

	// OK button

	eButton *ok = makeOKButton( this );
        CONNECT(ok->selected, eWidget::accept);
}

void MiscMenu::aspectRatioChanged( eListBoxEntryText *ep )
{
	conf.aspectRatioMode = (int)(ep->getKey());

	// If enigma isn't set on 16:9 warn user it was
	// pointless

	if ( getEnigmaIntKey(PIN8_KEY) != 3 )
		dmsg( getStr( strMiscMenuAspectRatioWarning ) );
	else
		setRestoreAspectRatio( true, conf.aspectRatioMode );
}

void MiscMenu::startOffsetChanged( void )
{
	startOffInt = startOffsetP->getNumber();
	conf.timerStartOffsetSeconds = startOffInt;
}

void MiscMenu::winOffsetChanged( void )
{
	winOffInt = winOffsetP->getNumber();
	conf.windowInitialStartOffsetSeconds = winOffInt;
}
void MiscMenu::endOffsetChanged( void )
{
	endOffInt = endOffsetP->getNumber();
	conf.timerEndOffsetSeconds = endOffInt;
}

