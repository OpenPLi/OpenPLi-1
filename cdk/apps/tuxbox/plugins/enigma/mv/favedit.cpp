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

#include "favedit.h"
#include "util.h"

FavEdit::FavEdit( struct Favourite *toEdit )
	: eWindow( 0 ), favP( toEdit )
{
	setText( getStr( strFavEditTitle ) );
	setWidgetGeom( this,
        	FAV_EDIT_X, FAV_EDIT_Y,
        	FAV_EDIT_WIDTH, FAV_EDIT_HEIGHT 
	);

	fromInt = favP->hourA;
	toInt = favP->hourB;

	// --- TITLE ROW

	makeNewLabel( 
		this, strLabelFavEditTitle,
                FE_LABEL_X, FE_TITLE_Y,
                FE_LABEL_WIDTH, FE_TEXT_HEIGHT
        );

	titleTextP = makeNewTextInput( 
		this, favP->title,
		FE_TEXT_X, FE_TITLE_Y,
		FE_TEXT_WIDTH, FE_TEXT_HEIGHT
	);

	eComboBox *cp =  makeNewComboBox(
	        this,
		FE_BOOL_X, FE_TITLE_Y,
		FE_BOOL_WIDTH, FE_BOOL_HEIGHT,
		4
	);

	for ( int itemNo = 0; itemNo < favNoBools; itemNo++ )
		new eListBoxEntryText( *cp, getStr(strFavEditBoolFirst + itemNo), (void *) itemNo );
	cp->setCurrent( favP->titleBool );
	CONNECT( cp->selchanged, FavEdit::titleBoolChanged );

	// --- DESCR ROW

	makeNewLabel( 
		this,  strLabelFavEditDescr,
                FE_LABEL_X, FE_DESCR_Y,
                FE_LABEL_WIDTH, FE_TEXT_HEIGHT
        );

	descrTextP = makeNewTextInput( 
		this, favP->descr,
		FE_TEXT_X, FE_DESCR_Y,
		FE_TEXT_WIDTH, FE_TEXT_HEIGHT
	);

	cp =  makeNewComboBox(
	        this, 
		FE_BOOL_X, FE_DESCR_Y,
		FE_BOOL_WIDTH, FE_BOOL_HEIGHT,
		4
	);

	for ( int itemNo = 0; itemNo < favNoBools; itemNo++ )
		new eListBoxEntryText( *cp, getStr( strFavEditBoolFirst + itemNo ), (void *) itemNo );
	cp->setCurrent( favP->descrBool );
	CONNECT( cp->selchanged, FavEdit::descrBoolChanged );

	// --- CHANNEL ROW

	makeNewLabel( 
		this, strLabelFavEditChannel,
                FE_LABEL_X, FE_CHANNEL_Y,
                FE_LABEL_WIDTH, FE_TEXT_HEIGHT
        );

	channelTextP = makeNewTextInput( 
		this, favP->channel,
		FE_TEXT_X, FE_CHANNEL_Y,
		FE_TEXT_WIDTH, FE_TEXT_HEIGHT
	);

	// -- HOUR ROW

	cp =  makeNewComboBox(
	        this, 
		FE_LABEL_X, FE_NUMBERS_Y,
		FE_LABEL_WIDTH, FE_FROM_LABEL_HEIGHT,
		2
	);

	new eListBoxEntryText( *cp, getStr( strLabelFavEditTimeBetween ), (void *) 0 );
	new eListBoxEntryText( *cp, getStr( strLabelFavEditTimeOutside ), (void *) 1 );

	cp->setCurrent( favP->hourCombine );
	CONNECT( cp->selchanged, FavEdit::hourCombineChanged );

        fromNumberP = makeNewNumber( 
		this, &fromInt,
                FE_FROM_NUMBER_X, FE_NUMBERS_Y,
                FE_NUMBERS_WIDTH, FE_NUMBERS_HEIGHT,
                2, 0, 23
        );
        CONNECT( fromNumberP->numberChanged, FavEdit::fromNumberChanged );

	makeNewLabel( 
		this, strLabelFavEditTimeAnd,
                FE_FROM_LABEL_X, FE_NUMBERS_Y,
                FE_FROM_LABEL_WIDTH, FE_FROM_LABEL_HEIGHT
        );

        toNumberP = makeNewNumber( 
		this, &toInt,
                FE_TO_NUMBER_X, FE_NUMBERS_Y,
                FE_NUMBERS_WIDTH, FE_NUMBERS_HEIGHT,
                2, 0, 23
        );
        CONNECT( toNumberP->numberChanged, FavEdit::toNumberChanged );

	eCheckbox *tb = makeNewCheckbox(
                this, strCheckBoxFavEditNotify,
                FE_NOTIFY_X, FE_NUMBERS_Y,
                FE_NOTIFY_WIDTH, FE_NUMBERS_HEIGHT,
		favP->notifyFlag
        );
        CONNECT( tb->checked, FavEdit::notifyChanged );

	eButton *ok = makeOKButton( this );
	CONNECT(ok->selected, eWidget::accept);
}

void FavEdit::notifyChanged( int newVal )
{
	favP->notifyFlag = newVal;
}

void FavEdit::fromNumberChanged( void )
{
        fromInt = fromNumberP->getNumber();
        favP->hourA = fromInt;
}

void FavEdit::toNumberChanged( void )
{
        toInt = toNumberP->getNumber();
        favP->hourB = toInt;
}

void FavEdit::hourCombineChanged( eListBoxEntryText *ep )
{
  	favP->hourCombine = (int)(ep->getKey());
}

void FavEdit::titleBoolChanged( eListBoxEntryText *ep )
{
  	favP->titleBool = (int)(ep->getKey());
}

void FavEdit::descrBoolChanged( eListBoxEntryText *ep )
{
  	favP->descrBool = (int)(ep->getKey());
}

FavEdit::~FavEdit()
{
	favP->title = stringReplace( titleTextP->getText(), ' ', ',' );
	favP->descr = stringReplace( descrTextP->getText(), ' ', ',' );
	favP->channel = stringReplace( channelTextP->getText(), ' ', ',' );
}
