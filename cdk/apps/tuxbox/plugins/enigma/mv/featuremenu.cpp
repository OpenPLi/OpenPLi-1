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

	
	changes maded by Alex Shtol at 14.08.2004
	(c) 2004 http://www.mediatwins.com

	reintegrated into MV  20040829 by Pent Vaer
	<lee@dinglisch.net>
*/
/*************************************************************/

#include "featuremenu.h"

/*
	Featured Edit Menu
	This is a submenu allowing different feature
	sets to be chosen
*/

MainMenuFeatured::MainMenuFeatured( int currentViewNo ) : eListBoxWindow<eListBoxEntryText>( getStr( strMainMenuItemViewFeat ), MENU_F_NOITEMS, MENU_F_WIDTH, false )
{
        for ( int i = 1; i <= MENU_F_NOITEMS; i++ )
		new eListBoxEntryText( &list, getStr( strFeaturedMenuFirst + i - 1 ), (void*)i);

        move( ePoint( MENU_F_TOPLEFTX, MENU_F_TOPLEFTY ) );

        CONNECT( list.selected, MainMenuFeatured::listSelected );
}

void MainMenuFeatured::listSelected( eListBoxEntryText* t )
{
        if ( t == NULL )
                close( -1 );
        else
                close( (int) ( t->getKey() )  );
}

///////////////////////////////////////////////////////////////


static int timebarValues[FM_NO_TIMEBAR_VALUES] = { 0, 900, 1800, 3600, 7200 };

FeatureMenu::FeatureMenu( struct ViewFeatures & feat, int currentViewNo, int featViewMode )
	: eWindow(1), featRef( feat )
{
	// Set correct window header to view mode
	setText( getStr( strFeaturedMenuFirst + featViewMode - 1) );

	eCheckbox *tb;

	// NEW GEOMETRY SETTINGS:
	#define C_STEP_Y1	35
	#define C_X1	10
	#define C_Y1	5
	#define C_L1W	250
	#define C_L1H	30
	
	#define C_X2	260
	#define C_L2W	150
	#define C_WW	C_X1*2+C_X2+C_L2W
	#define C_WH	C_STEP_Y1*9 + C_L1H + C_Y1*2

	//--------------------------------------------------//
	//-------------------- GLOBAL CONFIG ---------------//
	//--------------------------------------------------//

	// Both list and graph
	if ( featViewMode == MF_ADD_GLOBALVIEW )
	 {
		#define C_WX	145 //(720-C_WW) / 4
		#define C_WY	(576-C_WH) / 4

		setWidgetGeom( this,
			C_WX, C_WY,
			C_WW, C_WH
		);
		
		// ENTRIES/COLUMN COMBO

		int textIndex;
		if ( isGraphicalView( currentViewNo ) )
			textIndex = strLabelFeatMenuItemsPerCol;
		else
			textIndex = strLabelFeatMenuItemsPerChannel;

		makeNewLabel( this, textIndex, 
			C_X1, C_Y1, 
			C_L1W, C_L1H 
		);

		eComboBox *entriesPerColP =  makeNewComboBox(
			this,
			C_X2, C_Y1,
			C_L2W, C_L1H,
               		4
		);

		for ( int itemNo = 0; itemNo < 6; itemNo ++ ) {
			if ( itemNo == 0 ) {
				if ( isGraphicalView( currentViewNo ) )
					textIndex = strFeatMenuItemsPerColProportional;
				else
					textIndex = strFeatMenuItemsPerColAll;
			}
			else
				textIndex = strOne + itemNo - 1;
			 new eListBoxEntryText( *entriesPerColP, getStr(textIndex), (void*) itemNo );
		}

		entriesPerColP->setCurrent( featRef.entriesPerColumn );
		CONNECT( entriesPerColP->selchanged, FeatureMenu::entriesPerColChanged );
		entriesPerColP->show();
	}

	// Stuff for only graphical

	if ( 
		( featViewMode == MF_ADD_GLOBALVIEW ) &&
		( isGraphicalView( currentViewNo ) )
	) {
		// CHANNEL ROW
		// NO COLUMNS COMBO
		makeNewLabel( this, strLabelFeatMenuNoCols, C_X1, C_Y1+ C_STEP_Y1, C_L1W, C_L1H ); // [5,5;190,30]

		eComboBox *noColP = makeNewComboBox( this,
				 C_X2, C_Y1 + C_STEP_Y1,
				 C_L2W, C_L1H 
		);

		for ( int itemNo = 0; itemNo < 3; itemNo ++ ) 
			new eListBoxEntryText( *noColP, getStr( strOne + itemNo), (void*) itemNo );

		noColP->setCurrent( featRef.noColumns - 1 );
		CONNECT( noColP->selchanged, FeatureMenu::noColsChanged ); 
		noColP->show();

		// CHANNEL SHIFT TYPE
		makeNewLabel( this, strLabelFeatMenuChannelShiftType, 
			C_X1, C_Y1 + C_STEP_Y1*2, 
			C_L1W, C_L1H
		);
	
		eComboBox *csp =  makeNewComboBox( 
			this,
			C_X2, C_Y1 + C_STEP_Y1 * 2,
			C_L2W, C_L1H
		);

		for ( int itemNo = 0; itemNo < 3; itemNo ++ ) 
			new eListBoxEntryText( *csp, getStr( strFeatMenuChannelShiftTypeFirst + itemNo ), (void*) itemNo );

		csp->setCurrent( featRef.channelShiftType );
		CONNECT( csp->selchanged, FeatureMenu::channelShiftChanged ); 

		// PROGRAM SHIFT TYPE
		makeNewLabel( this, strLabelFeatMenuProgramShiftType, 
				C_X1, C_Y1 + C_STEP_Y1*3, 
				C_L1W, C_L1H 
		);
	
		eComboBox *psp =  makeNewComboBox( 
			this,
			C_X2, C_Y1 + C_STEP_Y1 * 3,
			C_L2W, C_L1H
		);

		for ( int itemNo = 0; itemNo < 3; itemNo ++ ) 
			new eListBoxEntryText( *psp, getStr( strFeatMenuProgramShiftTypeFirst + itemNo ), (void*) itemNo );

		psp->setCurrent( featRef.programShiftType );
		CONNECT( psp->selchanged, FeatureMenu::programShiftChanged ); 

	
		// TIMEBAR
		makeNewLabel( this, strLabelFeatMenuTimeBar,  
			C_X1, C_Y1 + C_STEP_Y1*4, 
			C_L1W, C_L1H 
		);
	
		eComboBox *timebarP =  makeNewComboBox( 
			this,
			C_X2, C_Y1 + C_STEP_Y1 * 4,
			C_L2W, C_L1H,
			FM_NO_TIMEBAR_VALUES
		);

		for ( int item = 0; item < FM_NO_TIMEBAR_VALUES; item++ )
	  		new eListBoxEntryText( *timebarP, getStr( strFeatMenuTimeBarLabelFirst + item ), (void*) item );

		timebarP->setCurrent( timebarValueToTimebarIndex( featRef.timebarPeriodSeconds ) );
		CONNECT( timebarP->selchanged, FeatureMenu::timebarChanged ); 

		// DAY BAR
		makeNewLabel( this, strLabelFeatMenuDayBar,  
			C_X1, C_Y1 + C_STEP_Y1*5, 
			C_L1W, C_L1H 
		);
	
		eComboBox *daybarP =  makeNewComboBox( 
			this,
			C_X2, C_Y1 + C_STEP_Y1 * 5,
			C_L2W, C_L1H,
			dayBarNoModes
		);

		for ( int item = 0; item < dayBarNoModes; item++ )
	  		new eListBoxEntryText( *daybarP, getStr( strFeatMenuDayBarLabelFirst + item ), (void*) item );

		daybarP->setCurrent( featRef.dayBarMode );
		CONNECT( daybarP->selchanged, FeatureMenu::daybarChanged );

		// VERT SEP CHECKBOX

		tb = makeNewCheckbox(
			this, strCheckboxFeatMenuVSep,
			C_X1, C_Y1 + C_STEP_Y1*6,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.verticalSep
		);
	       	CONNECT( tb->checked, FeatureMenu::verticalSepChanged );

		// HOR SEP CHECKBOX

		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuHSep,
			C_X1, C_Y1 + C_STEP_Y1*7,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.horizontalSep
		);
	        CONNECT( tb->checked, FeatureMenu::horizontalSepChanged );
	}

	//--------------------------------------------------//
	//------------ CHANNEL HEADER CONFIG ---------------//
	//--------------------------------------------------//

	// Channels View Section
	if ( featViewMode == MF_ADD_CHANNELS )
	{
		#define C1_WH	C_STEP_Y1*7 + C_L1H + C_Y1*2
		#define C1_WY	(576-C1_WH) / 4

		setWidgetGeom( this,
			C_WX, C1_WY,
			C_WW, C1_WH
		);
		
		// Channel NAME show
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuChannelHeaderName,
			C_X1, C_Y1,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.channelHeaderFlags & channelHeaderFlagShowName
		);
        	CONNECT( tb->checked, FeatureMenu::channelHeaderNameChanged );
		// Channel ICON show
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuChannelHeaderIcon,
			C_X1, C_Y1 + C_STEP_Y1,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.channelHeaderFlags & channelHeaderFlagShowIcon
		);
        	CONNECT( tb->checked, FeatureMenu::channelHeaderIconChanged );
	}

	// Graph view only stuff
	if ( 
		( featViewMode == MF_ADD_CHANNELS ) &&
		( isGraphicalView(currentViewNo) ) 
	) {

		// Channel NUMBER show
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuChannelHeaderNumber,
			C_X1, C_Y1 + C_STEP_Y1*2,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.channelHeaderFlags & channelHeaderFlagShowNumber
		);
		CONNECT( tb->checked, FeatureMenu::channelHeaderNumberChanged );
	
		// Channel Sat.Pos. show
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuChannelHeaderSatPos,
			C_X1, C_Y1 + C_STEP_Y1*3,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.channelHeaderFlags & channelHeaderFlagShowOrbital
		);
       	 	CONNECT( tb->checked, FeatureMenu::channelHeaderOrbitalChanged );
		
		// SHOW EMPTY CHANNELS CHECKBOX
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuEmptyChannels,
			C_X1, C_Y1 + C_STEP_Y1 * 4,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.showEmptyChannels
		);
       	 	CONNECT( tb->checked, FeatureMenu::emptyChannelsChanged );

		// START CHANNEL PULLDOWN
		makeNewLabel( this, strLabelFeatMenuStartChannel, 
			 C_X1, C_Y1 + C_STEP_Y1 * 5, 
			 C_L1W, C_L1H
		);

		eComboBox *firstChannelP =  makeNewComboBox(
       		         this,
			C_X2, C_STEP_Y1 * 5,
			C_L2W, C_L1H,
			2
       	 	);

       		for ( int itemNo = 0; itemNo < firstChannelNoItems; itemNo++ )
			new eListBoxEntryText( *firstChannelP, getStr( strFeatMenuStartChannelFirst + itemNo ), (void*) itemNo );

       		firstChannelP->setCurrent( featRef.firstChannel );
       		CONNECT( firstChannelP->selchanged, FeatureMenu::firstChannelChanged );
	}

	//--------------------------------------------------//
	//------------ PROGRAM CONFIG ----------------------//
	//--------------------------------------------------//
	
	if ( featViewMode == MF_ADD_PROGRAMMS )
	{	
		#define C2_WH	C_STEP_Y1*11 + C_L1H + C_Y1*2
		#define C2_WY	(576-C2_WH) / 4

		setWidgetGeom( this,
			C_WX, C2_WY,
			C_WW, C2_WH
		);
	}

	// List view only
	if ( 
		( featViewMode == MF_ADD_PROGRAMMS ) &&
		( ! isGraphicalView(currentViewNo) ) 
	) {

		// LIST VIEW SORT PULLDOWN
		makeNewLabel( this, strLabelFeatMenuSort, 
			C_X1, C_Y1, 
			C_L1W - 20, C_L1H 
		);

		eComboBox *sortP =  makeNewComboBox(
			this,
			C_X2 - 20, C_Y1,
			C_L2W + 20, C_L1H
       		 );

		// WARNING: if implement graphical sort, need change
		// handler sortChanged

		for ( int itemNo = sortTypeFirstList; itemNo <= sortTypeLastList; itemNo++ )
			new eListBoxEntryText( *sortP, getStr( strFeatMenuSortFirst + itemNo ), (void*) itemNo );

		sortP->setCurrent( featRef.sortType - sortTypeFirstList );
		CONNECT( sortP->selchanged, FeatureMenu::sortChanged );
		sortP->show();
	}
		
	// Graph view only
	if ( 
		( featViewMode == MF_ADD_PROGRAMMS ) &&
		( isGraphicalView(currentViewNo) ) 
	) {
		//Time Start
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxStart,
			C_X1, C_Y1,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagTime )
		);
		CONNECT( tb->checked, FeatureMenu::programBoxStartChanged );

		//Time End
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxEnd,
			C_X1, C_Y1 + C_STEP_Y1,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagEndTime )
		);
       		CONNECT( tb->checked, FeatureMenu::programBoxEndChanged );

		//Time Duration
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxDuration,
			C_X1, C_Y1 + C_STEP_Y1*2,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagDuration )
		);
       		CONNECT( tb->checked, FeatureMenu::programBoxDurationChanged );

		//Channel name
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxChannel,
			C_X1, C_Y1 + C_STEP_Y1*3,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagChannel )
		);
       		CONNECT( tb->checked, FeatureMenu::programBoxChannelChanged );

		//Time Diff
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxDifference,
			C_X1, C_Y1 + C_STEP_Y1*4,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagTimeDiff )
		);
       		CONNECT( tb->checked, FeatureMenu::programBoxStartDiffChanged );

		//Description
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxDescription,
			C_X1, C_Y1 + C_STEP_Y1*5,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagDescr )
		);
       		CONNECT( tb->checked, FeatureMenu::programBoxDescrChanged );
			
		// ELAPSED BAR 
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuElapsed,
			C_X1, C_Y1 + C_STEP_Y1*6,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.showElapsedBars
		);
       		CONNECT( tb->checked, FeatureMenu::showElapsedBarsChanged );
		
		// CHANNEL ICON
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxChannelIcon,
			C_X1, C_Y1 + C_STEP_Y1*7,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagChannelIcon )
		);
       		CONNECT( tb->checked, FeatureMenu::programBoxChannelIconChanged );

		// CENTRE TITLE
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuProgramBoxCentreTitle,
			C_X1, C_Y1 + C_STEP_Y1*8,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.programBoxFlags & pBoxFlagCentreTitle )
		);
       		CONNECT( tb->checked, FeatureMenu::programBoxCentreTitleChanged );

		// FORCE CURSOR CHECKBOX
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuForceCursor,
			C_X1, C_Y1 + C_STEP_Y1*9,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.forceCursorFlag
		);
       		CONNECT( tb->checked, FeatureMenu::forceCursorChanged );
	}

	//--------------------------------------------------//
	//------------ DESCRIPTION WINDOW MODE -------------//
	//--------------------------------------------------//

	if ( featViewMode == MF_ADD_DESCRIPTIONS )
	{	
		#define C3_WH	C_STEP_Y1*8 + C_L1H + C_Y1*2
		#define C3_WY	(576-C3_WH) / 4

		setWidgetGeom( this,
			C_WX, C3_WY,
			C_WW, C3_WH
		);
		
		// SHOW STATUS BAR
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuStatusBar,
			C_X1, C_Y1,
			C_X2+C_L2W-C_X1, C_L1H,
			featRef.showStatusBar
		);
        	CONNECT( tb->checked, FeatureMenu::statusBarChanged );
	
		// ----  sbar channel name

		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuStatusBarChannel,
			C_X1, C_Y1 + C_STEP_Y1,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.statusBarFlags & sBarFlagChannelName )
		);
        	CONNECT( tb->checked, FeatureMenu::statusBarChannelNameChanged );

		// ---- sbar date
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuStatusBarDate,
			C_X1, C_Y1 + C_STEP_Y1*2,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.statusBarFlags & sBarFlagDate )
		);
        	CONNECT( tb->checked, FeatureMenu::statusBarDateChanged );

		// ----  sbar time

		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuStatusBarTime,
			C_X1, C_Y1 + C_STEP_Y1*3,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.statusBarFlags & sBarFlagTime )
		);
        	CONNECT( tb->checked, FeatureMenu::statusBarTimeChanged );

		// ----  sbar duration
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuStatusBarDuration,
			C_X1, C_Y1 + C_STEP_Y1*4,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.statusBarFlags & sBarFlagDuration )
		);
        	CONNECT( tb->checked, FeatureMenu::statusBarDurationChanged );

		// ----  sbar program name
		tb = makeNewCheckbox( 
			this, strCheckboxFeatMenuStatusBarProgram,
			C_X1, C_Y1 + C_STEP_Y1*5,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.statusBarFlags & sBarFlagProgramName )
		);
        	CONNECT( tb->checked, FeatureMenu::statusBarProgramNameChanged );

		// ----  sbar descr
		tb = makeNewCheckbox(
			this, strCheckboxFeatMenuStatusBarDescription,
			C_X1, C_Y1 + C_STEP_Y1*6,
			C_X2+C_L2W-C_X1, C_L1H,
			( featRef.statusBarFlags & sBarFlagDescr )
		);
        	CONNECT( tb->checked, FeatureMenu::statusBarDescrChanged );
	}

	if ( featViewMode == MF_ADD_EPGSTYLE )
	{
		#define C4_WH	C_STEP_Y1*3 + C_L1H + C_Y1*2
		#define C4_WY	(576-C4_WH) / 4

		setWidgetGeom( this,
			C_WX, C4_WY,
			C_WW, C4_WH
		);
		
		eComboBox *nextViewP =  makeNewComboBox(
                this,
			C_X1, C_Y1 + C_STEP_Y1,
			C_X2+C_L2W-C_X1, C_L1H,
			6
        	);

	        for ( int itemNo = 0; itemNo < (NO_VIEWS-1); itemNo++ ) {
			char *txt = getStr( 
				( itemNo == (currentViewNo-1) ) ? 
					strFeatMenuNextViewThis : 
					( strEPGStyle1 + itemNo ) 
				);
				new eListBoxEntryText( *nextViewP, txt, (void*) itemNo );
			}

       	 	nextViewP->setCurrent( featRef.nextView - 1 );
       	 	CONNECT( nextViewP->selchanged, FeatureMenu::nextViewChanged );
        	nextViewP->show();
	}
	


    eButton *ok = makeOKButton( this );
    CONNECT(ok->selected, eWidget::accept);
}

int FeatureMenu::timebarValueToTimebarIndex( int value )
{
  int index = 0;
  while ( 
	( index < FM_NO_TIMEBAR_VALUES ) &&
	( timebarValues[index] != value )
  )
	index++;

  if ( index == FM_NO_TIMEBAR_VALUES )
    index = 0;

	return index;
}


void FeatureMenu::daybarChanged( eListBoxEntryText *ep )
{
	featRef.dayBarMode = (int) (ep->getKey() );
}

void FeatureMenu::timebarChanged( eListBoxEntryText *ep )
{
  featRef.timebarPeriodSeconds = timebarValues[(int)(ep->getKey())];
}

void FeatureMenu::noColsChanged( eListBoxEntryText *ep )
{
  featRef.noColumns = (int) ( ep->getKey() ) + 1;
}

void FeatureMenu::firstChannelChanged( eListBoxEntryText *ep )
{
  featRef.firstChannel = (int) ( ep->getKey() );
}

void FeatureMenu::nextViewChanged( eListBoxEntryText *ep )
{
  featRef.nextView = (int) ( ep->getKey() ) + 1;
}

void FeatureMenu::programShiftChanged( eListBoxEntryText *ep )
{
	featRef.programShiftType= (int)(ep->getKey());
}

void FeatureMenu::channelShiftChanged( eListBoxEntryText *ep )
{
	featRef.channelShiftType= (int)(ep->getKey());
}

void FeatureMenu::entriesPerColChanged( eListBoxEntryText *ep )
{
  featRef.entriesPerColumn = (int)(ep->getKey());
}

void FeatureMenu::emptyChannelsChanged( int newValue )
{
        featRef.showEmptyChannels = newValue;
}

void FeatureMenu::horizontalSepChanged( int newValue )
{
        featRef.horizontalSep = newValue;
}

void FeatureMenu::verticalSepChanged( int newValue )
{
        featRef.verticalSep = newValue;
}

void FeatureMenu::forceCursorChanged( int newValue )
{
        featRef.forceCursorFlag = newValue;
}

void FeatureMenu::channelHeaderNumberChanged( int newValue )
{
        changeFlagValue( featRef.channelHeaderFlags, channelHeaderFlagShowNumber, newValue );
}

void FeatureMenu::channelHeaderOrbitalChanged( int newValue )
{
        changeFlagValue( featRef.channelHeaderFlags, channelHeaderFlagShowOrbital, newValue );
}

void FeatureMenu::channelHeaderIconChanged( int newValue )
{
        changeFlagValue( featRef.channelHeaderFlags, channelHeaderFlagShowIcon, newValue );
}
void FeatureMenu::channelHeaderNameChanged( int newValue )
{
        changeFlagValue( featRef.channelHeaderFlags, channelHeaderFlagShowName, newValue );
}

void FeatureMenu::statusBarChanged( int newValue )
{
        featRef.showStatusBar = newValue;
}

void FeatureMenu::statusBarChannelNameChanged( int newValue )
{
	changeFlagValue( featRef.statusBarFlags, sBarFlagChannelName, newValue );
}

void FeatureMenu::statusBarDateChanged( int newValue )
{
	changeFlagValue( featRef.statusBarFlags, sBarFlagDate, newValue );
}

void FeatureMenu::showElapsedBarsChanged( int newValue )
{
	featRef.showElapsedBars = newValue;
}

void FeatureMenu::statusBarTimeChanged( int newValue )
{
	changeFlagValue( featRef.statusBarFlags, sBarFlagTime, newValue );
	changeFlagValue( featRef.statusBarFlags, sBarFlagEndTime, newValue );
}

void FeatureMenu::statusBarDurationChanged( int newValue )
{
	changeFlagValue( featRef.statusBarFlags, sBarFlagDuration, newValue );
}

void FeatureMenu::statusBarProgramNameChanged( int newValue )
{
	changeFlagValue( featRef.statusBarFlags, sBarFlagProgramName, newValue );
}

void FeatureMenu::statusBarDescrChanged( int newValue )
{
	changeFlagValue( featRef.statusBarFlags, sBarFlagDescr, newValue );
}

void FeatureMenu::programBoxChannelIconChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagChannelIcon, newValue );
}
void FeatureMenu::programBoxCentreTitleChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagCentreTitle, newValue );
}
void FeatureMenu::programBoxDescrChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagDescr, newValue );
}
void FeatureMenu::programBoxDurationChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagDuration, newValue );
}
void FeatureMenu::programBoxEndChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagEndTime, newValue );
}
void FeatureMenu::programBoxStartChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagTime, newValue );
}
void FeatureMenu::programBoxChannelChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagChannel, newValue );
}

void FeatureMenu::sortChanged( eListBoxEntryText *ep )
{
        featRef.sortType = (int) ( ep->getKey() );
}

void FeatureMenu::programBoxStartDiffChanged( int newValue )
{
	changeFlagValue( featRef.programBoxFlags, pBoxFlagTimeDiff, newValue );
}
