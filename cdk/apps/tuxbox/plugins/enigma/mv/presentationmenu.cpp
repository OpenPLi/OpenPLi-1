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

#include "presentationmenu.h"

#include "presmisc.cpp"

bool listViewShowButton[pmNoButtons] = {
	false, // pmChanBackButton = 0,
        true, // pmChanFontSizeButton,
        true, // pmChanFontFamilyButton,

        false, // pmProgOneBackButton,
        false, // pmProgTwoBackButton,
        true, // pmProgFontSizeButton,
        true, // pmProgFontFamilyButton,

        true,  // pmMiscPlayingButton,
        true,   // pmMiscFavouriteButton,
        true,   // pmMiscFilmButton,
        false  // pmMiscWindowButton,
};

PresentationMenu::PresentationMenu( struct ViewPresentation & opt, int currentView )
	: eWindow(1), presRef( opt )
{
	selectedForeColour = eSkin::getActive()->queryScheme("button.normal.background");

	cmove(ePoint(PM_TOPLEFTX, PM_TOPLEFTY));
	cresize(eSize(PM_WIDTH, PM_HEIGHT));
	setText( getStr( strPresentationWindowTitle ) );

	int gap = ( clientrect.width()  - 40 )/ 4;
	int x1 = 20;
	int x2 = x1 + gap;
	int x3 = x2 + gap;
	int x4 = x3 + gap;

	// --------- CHANNEL ROW ------------ //

	makeNewLabel( this, strLabelPresMenuChannelHeader,
                5, PM_CHANNEL_ROW_Y - 50,
		200,40
        );

	// CHAN BACK COLOUR
	eButton * bc = makeNewButton(
		this, strButtonPresMenuBackground, 
		x1, PM_CHANNEL_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmChanBackButton] = bc;
	CONNECT(bc->selected, PresentationMenu::chanBackButtonPressed );

	// CHAN FONT SIZE
	bc = makeNewButton(
		this, strButtonPresMenuFontSize,
		x2, PM_CHANNEL_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmChanFontSizeButton] = bc;
	CONNECT(bc->selected, PresentationMenu::chanFontSizeButtonPressed );

	// CHAN FONT FAMILY
	bc = makeNewButton(
		this, strButtonPresMenuFontFamily,
		x3, PM_CHANNEL_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmChanFontFamilyButton] = bc;
	CONNECT(bc->selected, PresentationMenu::chanFontFamilyButtonPressed );

	// --------- PROGRAM ROW ------------ //

	makeNewLabel( this, strLabelPresMenuProgramBox,
                5, PM_PROGRAM_ROW_Y - 50,
		200,40
        );

	// PROG ONE BACK COLOUR
	bc = makeNewButton(
		this, strButtonPresMenuBackOne,
		x1, PM_PROGRAM_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmProgOneBackButton] = bc;
	CONNECT(bc->selected, PresentationMenu::progOneBackButtonPressed );
	CONNECT( this->focusChanged, PresentationMenu::focusChangedReceive );

	// PROG TWO BACK COLOUR
	bc = makeNewButton(
		this, strButtonPresMenuBackTwo,
		x2, PM_PROGRAM_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmProgTwoBackButton] = bc;
	CONNECT(bc->selected, PresentationMenu::progTwoBackButtonPressed );

	// PROG FONT SIZE
	bc = makeNewButton(
		this, strButtonPresMenuFontSize,
		x3, PM_PROGRAM_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmProgFontSizeButton] = bc;
	CONNECT(bc->selected, PresentationMenu::progFontSizeButtonPressed );

	// PROG FONT FAMILY
	bc = makeNewButton(
		this, strButtonPresMenuFontFamily,
		x4, PM_PROGRAM_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmProgFontFamilyButton] = bc;
	CONNECT(bc->selected, PresentationMenu::progFontFamilyButtonPressed );

	// --------- MISC ROW ------------ //

	// PLAYING COLOUR
	bc = makeNewButton(
		this, strButtonPresMenuPlaying,
		x1, PM_MISC_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmMiscPlayingButton] = bc;
	CONNECT(bc->selected, PresentationMenu::miscPlayingButtonPressed );

	// FILM COLOUR
	bc = makeNewButton(
		this, strButtonPresMenuFilm,
		x2, PM_MISC_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmMiscFilmButton] = bc;
	CONNECT(bc->selected, PresentationMenu::miscFilmButtonPressed );

	// FAVOURITE COLOUR
	bc = makeNewButton(
		this, strButtonPresMenuFavourite,
		x3, PM_MISC_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmMiscFavouriteButton] = bc;
	CONNECT(bc->selected, PresentationMenu::miscFavouriteButtonPressed );

	// --------- GENERAL ROW ---------- //

	makeNewLabel( this, strLabelPresMenuWindow,
                5, PM_GEN_ROW_Y - 50,
		200,40
        );

	// WINDOW BACK COLOUR
	bc = makeNewButton(
		this, strButtonPresMenuBackground,
		x1, PM_GEN_ROW_Y,
		PM_BUTTON_WIDTH, PM_BUTTON_HEIGHT
	);
	buttons[pmMiscWindowButton] = bc;
	CONNECT(bc->selected, PresentationMenu::miscWindowButtonPressed );
	
	// Hide buttons not relevant to lists views
	if ( ! isGraphicalView( currentView ) ) {
		for ( int butNo = 0; butNo < pmNoButtons; butNo++ )
			if ( ! listViewShowButton[butNo] )
				buttons[butNo]->hide();	
	}

	setButtonColours();

	// TEST CHANNEL

	testChannel = new Channel( this );
	testChannel->baseReset( getStr( strPresMenuTestChannelHeader ), eServiceReferenceDVB(), NULL );
	testChannel->reset( );
	setWidgetGeom( testChannel,
		PM_TESTCHANNEL_TOPLEFTX, clientrect.height() - PM_MAX_FONT_SIZE - 20,
		clientrect.width() - PM_TESTCHANNEL_TOPLEFTX - 90 - 15, PM_MAX_FONT_SIZE + 10
	);
	testChannel->addProgram( 0, getStr( strPresMenuTestChannelProgOne ), 50200, 1000, false, false, false ,-1, -1);
	testChannel->addProgram( 1, getStr( strPresMenuTestChannelProgTwo ), 51500, 1000,false, false, false ,-1, -1);
	testChannel->addProgram( 2, getStr( strPresMenuTestChannelCursor ), 52510, 860, false, false, false ,-1, -1);
	testChannel->addProgram( 3, getStr( strPresMenuTestChannelFavourite ), 53400, 1250, false, true, false ,-1, -1);
	testChannel->addProgram( 4, getStr( strPresMenuTestChannelFilm ), 54700, 1000, true, false, false,-1, -1 );

	rebuildTestChannel();

	// OK BUTTON

        eButton *ok = makeOKButton( this );
        CONNECT(ok->selected, eWidget::accept);

}

void PresentationMenu::focusChangedReceive( const eWidget * bob )
{
	for ( int buttonNo = 0; buttonNo < pmNoButtons; buttonNo++ ) {
		if ( bob == buttons[buttonNo] )
			buttons[buttonNo]->setForegroundColor( selectedForeColour );
			
	}
	setButtonColours();
}

void PresentationMenu::setButtonColour( int buttonNo, int colour)
{
	buttons[buttonNo]->setBackgroundColor( colourOptionToColour( colour ) );
}

void PresentationMenu::display( void )
{
	testChannel->takeCursor( 52650, 52800 );
	show();
}

void PresentationMenu::rebuildTestChannel( void )
{
	gColor foreColour = getNamedColour( FORE_COLOUR );
	testChannel->setBackgroundColor( colourOptionToColour( presRef.backColour ) );
	testChannel->setHeaderWidth( 80 );
        testChannel->setHeaderFlags( channelHeaderFlagShowName );
	testChannel->setHeaderFont( makeFontFromOptions( presRef.channelHeaderFontFamily, presRef.channelHeaderFontSize ) );
        testChannel->setProgramFonts( 
		makeFontFromOptions( presRef.programTimeFontFamily, presRef.programTimeFontSize ),
		makeFontFromOptions( presRef.programTitleFontFamily, presRef.programTitleFontSize ),
		makeFontFromOptions( presRef.programDescrFontFamily, presRef.programDescrFontSize ),
		makeFontFromOptions( presRef.programChannelFontFamily, presRef.programChannelFontSize )
	);
        testChannel->setHeaderColours(
                colourOptionToColour( presRef.channelHeaderBackColour ),
		foreColour
        );

        testChannel->setProgramColours(
		colourOptionToColour( presRef.backColour ),
                colourOptionToColour( presRef.playingBackColour ),
		foreColour,
                colourOptionToColour( presRef.programOneBackColour ),
		foreColour,
                colourOptionToColour( presRef.programTwoBackColour ),
		foreColour,
                colourOptionToColour( presRef.filmBackColour ),
		foreColour,
                colourOptionToColour( presRef.favouriteBackColour ),
		foreColour
	);

	testChannel->setRange( 50000, 55702 );
	testChannel->setProgramsToShow( 0 );

	if ( ! testChannel->isVisible() )
              testChannel->show();
         else
              testChannel->invalidate();

}

void PresentationMenu::setButtonColours( void )
{
	setButtonColour( pmChanBackButton, presRef.channelHeaderBackColour );
	setButtonColour( pmProgOneBackButton, presRef.programOneBackColour );
	setButtonColour( pmProgTwoBackButton, presRef.programTwoBackColour );
	setButtonColour( pmMiscWindowButton, presRef.backColour );
	setButtonColour( pmMiscPlayingButton, presRef.playingBackColour );
	setButtonColour( pmMiscFilmButton, presRef.filmBackColour );
	setButtonColour( pmMiscFavouriteButton, presRef.favouriteBackColour );
}

void PresentationMenu::miscFilmButtonPressed( void ) {
	handleColourPress( &(presRef.filmBackColour) );
}
void PresentationMenu::miscPlayingButtonPressed( void ) {
	handleColourPress( &(presRef.playingBackColour) );
}
void PresentationMenu::miscFavouriteButtonPressed( void ) {
	handleColourPress( &(presRef.favouriteBackColour) );
}
void PresentationMenu::miscWindowButtonPressed( void ) {
	handleColourPress( &(presRef.backColour) );
}
void PresentationMenu::progTwoBackButtonPressed( void ) {
	handleColourPress( &(presRef.programTwoBackColour) );
}

void PresentationMenu::progOneBackButtonPressed( void ) {
	handleColourPress( &(presRef.programOneBackColour) );
}
void PresentationMenu::progFontFamilyButtonPressed( void ) {
	handleFontFamilyPress( &(presRef.programTitleFontFamily) );
}
void PresentationMenu::progFontSizeButtonPressed( void ) {
	handleFontSizePress( &(presRef.programTitleFontSize) );
}

void PresentationMenu::chanFontFamilyButtonPressed( void ) {
	handleFontFamilyPress( &(presRef.channelHeaderFontFamily) );
}
void PresentationMenu::chanFontSizeButtonPressed( void ) {
	handleFontSizePress( &(presRef.channelHeaderFontSize) );
}
void PresentationMenu::chanBackButtonPressed( void ) {
	handleColourPress( &(presRef.channelHeaderBackColour) );
}

void PresentationMenu::handleFontSizePress( int *toChange )
{
	*toChange += 2;
	if ( *toChange > PM_MAX_FONT_SIZE ) {
		*toChange = PM_MIN_FONT_SIZE;
		testChannel->hide();
	}
	rebuildTestChannel();
	invalidate();
}

void PresentationMenu::handleFontFamilyPress( int *toChange )
{
	*toChange = ( *toChange + 1 ) % PM_MAX_NUM_FONT_FAMILIES;
	rebuildTestChannel();
	invalidate();
}

void PresentationMenu::handleColourPress( int *toChange )
{
	*toChange  = ( ( *toChange + 1 ) % PM_MAX_NUM_COLOURS );
	setButtonColours();
	rebuildTestChannel();
	invalidate();
}
