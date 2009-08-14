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

#include "inputmenu.h"

InputMenu::InputMenu( struct Inputs & i )
	: eWindow(0), inputsRef( i )
{
	maxChanInt = i.maxChannelsPerInput;
	preInt = (int) i.pre;
	postInt = (int) i.post;

	setWidgetGeom( this,
		IM_TOPLEFTX, IM_TOPLEFTY,
		IM_WIDTH, IM_HEIGHT
	);
	setText( getStr( strInputsWindowTitle ) );

	gColor labelBackColour = getNamedColour( "std_dblue" );
	makeNewLabel( this, strLabelInputMenuHeaderInput, IMR_NAME_X, IM_HEADER_ROW_Y, IMR_NAME_WIDTH, IM_HEADER_ROW_HEIGHT, 0, &labelBackColour );
	makeNewLabel( this, strLabelInputMenuHeaderLocalDir, IMR_LOCAL_X, IM_HEADER_ROW_Y, IMR_LOCAL_WIDTH, IM_HEADER_ROW_HEIGHT, 0, &labelBackColour );
	makeNewLabel( this, strLabelInputMenuHeaderDownload, IMR_INTERNET_X, IM_HEADER_ROW_Y, IMR_INTERNET_WIDTH, IM_HEADER_ROW_HEIGHT, 0, &labelBackColour );

	int ypos = IM_FIRST_INPUT_Y;
	
	for ( int inputNo = 0; inputNo < MAX_NO_INPUTS; inputNo++ ) {
		rows[inputNo] = new InputMenuRow( this, inputsRef.confs[inputNo], defs );
		setWidgetGeom( rows[inputNo],
			IM_ROW_X, ypos,
			IM_ROW_WIDTH, IM_ROW_HEIGHT
		);
		rows[inputNo]->show();
		ypos += IM_DELTA_Y;
	}

	// PRE
	makeNewLabel( this, strLabelInputMenuPre, IM_PRE_X, ypos, 95, IM_ROW_HEIGHT );
	preP = makeNewNumber( this, &preInt,
		IM_PRE_X + 100, ypos,
		IM_PRE_WIDTH,IM_ROW_HEIGHT,
		IM_MAX_PRE_POST_DIGITS,
		0, IM_MAX_PRE
	);
	CONNECT( preP->numberChanged, InputMenu::preChanged );

	// POST
	makeNewLabel( this, strLabelInputMenuPost, IM_POST_X, ypos, 95, IM_ROW_HEIGHT );
	postP = makeNewNumber( this, &postInt,
                  IM_POST_X + 100, ypos,
                  IM_POST_WIDTH,IM_ROW_HEIGHT,
                  IM_MAX_PRE_POST_DIGITS,
                  0, IM_MAX_POST
        );
	CONNECT( postP->numberChanged, InputMenu::postChanged );

	// DETAILS LOCATION

	makeNewLabel( this, strLabelInputMenuDetailStore, IM_DETAIL_PATH_X_LABEL, IM_DETAIL_PATH_Y, IM_DETAIL_PATH_WIDTH, IM_DETAIL_PATH_HEIGHT  );

 	eComboBox *detailPathP =  makeNewComboBox(
                this,
		IM_DETAIL_PATH_X_BOX, IM_DETAIL_PATH_Y,
		IM_DETAIL_PATH_WIDTH, IM_DETAIL_PATH_HEIGHT,
		5
        );

	for ( int itemNo = 0; itemNo < noDirs(); itemNo++ )
        	new eListBoxEntryText( *detailPathP, indexToDir(itemNo).c_str(), (void*) itemNo );

	detailPathP->setCurrent( i.detailsDir );

        CONNECT( detailPathP->selchanged, InputMenu::detailPathChanged);

	// STORAGE DIR

	makeNewLabel( this, strLabelInputMenuStorageDir, IM_DETAIL_PATH_X_LABEL, IM_STORAGE_PATH_Y, IM_DETAIL_PATH_WIDTH, IM_DETAIL_PATH_HEIGHT  );

 	eComboBox *storagePathP =  makeNewComboBox(
                this,
		IM_DETAIL_PATH_X_BOX, IM_STORAGE_PATH_Y,
		IM_DETAIL_PATH_WIDTH, IM_DETAIL_PATH_HEIGHT,
		5
        );

	for ( int itemNo = 0; itemNo < noDirs(); itemNo++ )
        	new eListBoxEntryText( *storagePathP, indexToDir(itemNo).c_str(), (void*) itemNo );

	storagePathP->setCurrent( i.storageDir );

        CONNECT( storagePathP->selchanged, InputMenu::storagePathChanged);

	// AUTO CHECKBX

	eCheckbox *autoP = makeNewCheckbox(
                this, strCheckboxFeatureMenuAutoDirs,
		IM_AUTO_CHECKBOX_X, IM_DETAIL_PATH_Y,
		IM_AUTO_CHECKBOX_Y, IM_DETAIL_PATH_HEIGHT,
		i.autoDataDirsFlag
        );
        CONNECT( autoP->checked, InputMenu::autoChecked );

	// MAX CHANNELS NUMBER
	maxChanP = makeNewNumber( this, &maxChanInt,
		IM_MAX_CHANNELS_X, IM_MAX_CHANNELS_Y, 
		IM_MAX_CHANNELS_WIDTH, IM_MAX_CHANNELS_HEIGHT,
		IM_MAX_CHANNEL_DIGITS, 
		0, IM_MAX_PRE 
	);
	CONNECT( maxChanP->numberChanged, InputMenu::maxChanChanged );
	
	makeNewLabel( this, strLabelInputMenuMaxChannels, IM_MAX_CHANNELS_X + IM_MAX_CHANNELS_WIDTH + 5, IM_MAX_CHANNELS_Y, 150, IM_MAX_CHANNELS_HEIGHT );

	// AUTO RELOAD CHECKBOX

	autoP = makeNewCheckbox(
                this, strCheckboxFeatureMenuAutoReload,
		IM_AUTO_RELOAD_CHECKBOX_X, IM_MAX_CHANNELS_Y,
		IM_AUTO_RELOAD_CHECKBOX_WIDTH, IM_MAX_CHANNELS_HEIGHT,
		i.autoReloadFlag
        );
        CONNECT( autoP->checked, InputMenu::autoReloadChecked );

	// OK BUTTON

        eButton *ok = makeOKButton( this );
        CONNECT(ok->selected, eWidget::accept);
}

void InputMenu::maxChanChanged( void )
{
	maxChanInt = maxChanP->getNumber();
	inputsRef.maxChannelsPerInput = maxChanInt;
}
void InputMenu::autoReloadChecked( int newValue )
{
	inputsRef.autoReloadFlag = newValue;
}
void InputMenu::autoChecked( int newValue )
{
	inputsRef.autoDataDirsFlag = newValue;
}

InputMenu::~InputMenu()
{
	setFocus( this );
}

/*********************** ROW ***********************/

InputMenuRow::InputMenuRow( eWidget *parent, InputConf &c, InputDefReader &defs ) : eWidget( parent ), confRef( c ), defs(defs), checkboxP(NULL), nameP(NULL), localP(NULL), internetP(NULL)
{
	rebuild();
}

void InputMenuRow::rebuild( void )
{
	struct inputDef *defP = defs.getDefP( confRef.name );
	if ( defP == NULL ) {
		strcpy( confRef.name, ENIGMA_CACHE_NAME );
		confRef.enabledFlag = 0;
	        defP = defs.getDefP( confRef.name );
	}

	if ( checkboxP ) 
		checkboxP->hide();

	hideRestOfRow();

	// CHECKBOX
	checkboxP = makeNewCheckbox(
                this, strNoStrings,
		IMR_CHECKBOX_X, 0,
		IMR_CHECKBOX_WIDTH, IM_ROW_HEIGHT,
		confRef.enabledFlag
        );
        CONNECT( checkboxP->checked, InputMenuRow::checkboxChecked );

	// NAME SELECTER 
 	nameP = makeNewComboBox(
                this,
		IMR_NAME_X, 0,
		IMR_NAME_WIDTH, IM_ROW_HEIGHT,
		8	
        );

        struct inputDef *next = defs.getNextDefP( true );
	int itemNo = 0;
        while ( next != NULL ) {
        	new eListBoxEntryText( *nameP, next->name, (void*) itemNo );
		if ( strcmp( next->name, defP->name ) == 0 ) 
			nameP->setCurrent( itemNo );
                next = defs.getNextDefP( false );
		itemNo++;
        }
        CONNECT( nameP->selchanged, InputMenuRow::nameChanged);

	// LOCALDIR LABEL
	localP = makeNewLabel( this, defP->localDir, IMR_LOCAL_X, IM_HEADER_ROW_Y, IMR_LOCAL_WIDTH, IM_HEADER_ROW_HEIGHT );

	// INTERNET
	int networkText;

	if ( remoteType( defP->remoteDir ) == inputDefRemoteTypeHTTP )  {
		if ( ! haveNetwork() )
			networkText = strLabelInputMenuNetworkNeededNoNetwork;
		else
			networkText = strLabelInputMenuNetworkNeeded;
	}
	else
		networkText = strLabelInputMenuNetworkNotNeeded;


	internetP = makeNewLabel( this, networkText, IMR_INTERNET_X, IM_HEADER_ROW_Y, IMR_INTERNET_WIDTH, IM_HEADER_ROW_HEIGHT );

	if ( confRef.enabledFlag )
		showRestOfRow();
	else
		hideRestOfRow();
}

void InputMenuRow::nameChanged( eListBoxEntryText *ep )
{
	if ( 
		( ep->getText() != "MyXMLTV" ) ||
		( userConfirms( strMyXMLTVConfirmText ) )
	)
		strcpy( confRef.name, ep->getText().c_str() );
	rebuild();
}

void InputMenu::detailPathChanged( eListBoxEntryText *ep )
{
	eString theDir = ep->getText();
	inputsRef.detailsDir = dirToIndex( theDir );
	makeDirIfNotExists( theDir );
}

void InputMenu::storagePathChanged( eListBoxEntryText *ep )
{
	eString theDir = ep->getText();
	inputsRef.storageDir = dirToIndex( theDir );
	makeDirIfNotExists( theDir );
}

void InputMenu::preChanged( void )
{
	preInt = preP->getNumber();
	inputsRef.pre = (time_t) preInt;
}

void InputMenu::postChanged( void )
{
	postInt = postP->getNumber();
	inputsRef.post = (time_t) postInt;
}

void InputMenuRow::hideRestOfRow( void )
{
	if ( nameP ) nameP->hide();
	if ( localP ) localP->hide();
	if (internetP) internetP->hide();
}

void InputMenuRow::showRestOfRow( void )
{
	nameP->show();
	localP->show();
	internetP->show();
}


void InputMenuRow::checkboxChecked( int newValue )
{
	confRef.enabledFlag = newValue;
	if ( newValue ) 
		showRestOfRow();
	else
		hideRestOfRow();
}
