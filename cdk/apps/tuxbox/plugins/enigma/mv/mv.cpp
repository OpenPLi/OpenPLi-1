/****************************************************************/
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

#define HAVECACHE       1

#include "mv.h"
#include "enigma_processutils.h"
#include "enigma_dyn_utils.h"
#include "enigma_main.h"


#ifndef MVLITE
	#include "mv-conf.cpp"
#else
	#include "presmisc.cpp"
#endif
InputManager *inputMgrP;
struct Inputs inputs;

MV::~MV()
{ 
	// Clear start argument

	eConfig::getInstance()->setKey( ARG1_KEY, 0 );

	// The timer destructor stops it first

	if ( timerP != NULL )
		delete timerP;	

	if ( timeLabelP != NULL )
		delete timeLabelP;

	if ( inputMgrP != NULL ) {
		if ( inputMgrP->saveEnigmaCache( true, inputs ) ) {
			setIndicator( indLoading, true );
			setIndicator( indSaving, true );
			// Don't appear reliably otherwise
			microSleep( 50000 );
			inputMgrP->saveEnigmaCache( false, inputs );
			setIndicator( indSaving, false );
			setIndicator( indLoading, false );
		}

		delete inputMgrP;
	}

	if ( writeConfigFlag )
		writeConfig();

	for ( int indNo = 0; indNo < indNoIndicators; indNo++ ) {
		if ( indicators[indNo] != NULL ) {
			indicators[indNo]->hide();
			delete indicators[indNo];
		}
	}

	if ( userLangPref2Char != NULL )
		delete userLangPref2Char;
	if ( userLangPref3Char != NULL )
		delete userLangPref3Char;

	if ( sBarP != NULL )
		delete sBarP;

	if ( channels != NULL )
		delete channels;

	setRestoreAspectRatio( false, 0 );
}

void MV::storeRestoreIndicators( bool storeFlag )
{
	static bool storedFlag = false;
	static bool storedValues[indNoIndicators];

	// If already stored, don't want to store again...
	if ( storeFlag == storedFlag )
		return;

	storedFlag = storeFlag;

	for ( int indNo = 0; indNo < indNoIndicators; indNo++ ) {
		if ( storeFlag ) {
			storedValues[indNo] = ( indicators[indNo]->isVisible() );
			indicators[indNo]->hide();
		}
		// If something made it visible in the meantime, leave it
		else if ( ! indicators[indNo]->isVisible() )
			showHideWidget( indicators[indNo], storedValues[indNo] );
	}
}

MV::MV(): 
	eWidget( NULL, 1 ), 
	writeConfigFlag( false ),
	timeLabelP( NULL ),
	needRecreateChannelsFlag( true ),
        userLangPref2Char( NULL ),
        userLangPref3Char( NULL ),
	execFlag( false ),
	timerP( NULL ),
        sBarP( NULL ),
	errorCode( errorCodeNone ),
        currentMode( MV_MODE_VIEW ),
        currentView( 1 ),
	channels( NULL ),
	currentChannel(-1),
	fillingCacheFlag( false ),
	canAutoReloadFlag( false ),
	wasFillingCacheFlag( false ),
	windowHiddenFlag( false )

{
	inputMgrP=NULL;
	mylogReset();

	// Just in case anything between now and load
	// uses it...

	setLoadTime();

	// Have to NULLify ptrs at start in case we exit
	// the constructor prematurely due to problems

        for ( int indNo = 0; indNo < indNoIndicators; indNo++ )
		indicators[indNo] = NULL;

	// Get user language pref

	userLangPref3Char = new char[4];
        userLangPref2Char = new char[3];

        if ( ! getUserISOLangPref( userLangPref2Char, userLangPref3Char ) ) {
		strcpy( userLangPref2Char, "en" );
		strcpy( userLangPref3Char, "eng" );
	}

	// Load a language file
	// Error if we couldn't load OSD lang,
	// and OSD lang is english or we couldn't load english
	// either

	// Need to do this quick for error messages
	// further down the line

	if ( 
		( ! initStrings( userLangPref2Char ) ) &&
		( 
			( ! strcmp( userLangPref2Char, "en" ) ) ||
			( ! initStrings( "en", true ) ) 
		)
	) {
		setError( errorCodeBadStringFile );
		return;
	}

	if ( ! checkSensibleSystemTime() ) {
		dmsg( getStr( strSystemTimeBad ) );
		setError( errorCodeBadSystemTime );
		return;
	}

	if ( ! checkDir( eString( CONFIG_DIR ) ) )
		return;

	int readStatus = readConfig();

        if ( readStatus == configErrorBadFile ) {
		dmsg( getStr( strDeletingOldConfig ) );
		readStatus = configErrorNotFound;
	}

	if ( readStatus == configErrorNotFound )
                setConfigDefaults();

	if ( 
		( readStatus == configErrorNotFound ) ||
		( inputs.autoDataDirsFlag ) 
	)
		setupDataDirs();

	// At this point, we should have a sane config
	// to write
	// Without the flag, if we return due to a previous
	// error, a bad config will be written by the 
	// destructor

	writeConfigFlag = true;

	// Load skin and icon .esml into active skin
	// We use skin for images for two reasons:
	// - there's bad memory leaking from loadPNG and/or
	// libpng, so after 20 calls to MV the dreambox would
	// crash. Loading them into the skin, they're never
	// deleted
	// - easy to load/cache with queryImage (don't need to
	// write own icon cache routines)
	

	bool loadedFlag = loadSkin( formConfigPath( SKIN_NAME ), ESKIN_LOADED_VALUE_NAME, false );

	// Only parse if actually loaded something
	if ( loadedFlag )
		eSkin::getActive()->parseSkins();


	foreColour = getNamedColour( FORE_COLOUR );

	timerP = new eTimer( eApp );
	CONNECT( timerP->timeout, MV::INT_timerTimeout );

	chanBarP = new eProgress( this );
	chanBarP->setDirection( 1 ); 
	chanBarP->setParams( 0, 100 );
	// Left is the colour of the bar
	chanBarP->setLeftColor( getNamedColour( "white_blue" ) );

	for ( int barNo = 0; barNo < FM_MAX_NO_COLUMNS; barNo++ )
		tBarP[barNo] = new TimeBar( this );

	sBarP = new eStatusBar( NULL );

	// If we don't do this it gets vertically centred

	sBarP->setFlags( eStatusBar::flagOwnerDraw );
	sBarP->setFlags( RS_WRAP );
	sBarP->removeFlags( eLabel::flagVCenter );

	gFont timeLabelFont = getNamedFont( TIME_LABEL_FONT_FAMILY );
//	timeLabelFont.pointSize = TIME_LABEL_FONT_SIZE;
	timeLabelP = makeNewLabel( 
			NULL,
			"",
			10,10,
			stringWidthPixels( timeLabelFont, "00:00" ) + 4,
			timeLabelFont.pointSize
	);

	timeLabelP->setFont( timeLabelFont );

	eString indicatorLabels[indNoIndicators] = {
		"H", "U", "D", "X", 
#ifndef MVLITE
		"F", "E",
#endif
		"<", ">",
		"C", "L", "S"
	};

	gColor indColour = getNamedColour( INDICATOR_COLOUR );

	for ( int indNo = 0; indNo < indNoIndicators; indNo++ ) {
		indicators[indNo] = makeNewLabel( 
			NULL,
			indicatorLabels[indNo],
			1,1,
			INDICATORS_WIDTH, timeLabelFont.pointSize,
			eLabel::flagVCenter,
			&indColour
		);
	}
	makeInputDefFileIfNotExist();

        // If no conf, or version diff from current,
	// show release notes

        if (
                ( readStatus == configErrorNotFound ) ||
                ( strcmp( conf.versionString, getStr( strVersion ) ) != 0 )
        ) {
                showInfoBox( NULL, strVersion, strWelcomeText );
		safeStrncpy( conf.versionString, getStr( strVersion ) , MAX_VERSION_STRING_LENGTH );
	}

	// Make dirs if they don't exist

	if ( 
		( ! checkDir( indexToDir( inputs.storageDir ) ) ) ||
		( ! checkDir( indexToDir( inputs.detailsDir ) ) )
	)
		return;

	// Decide what view to start on, image preference 
	// has priority

	int imageViewPref = getEnigmaIntKey( ARG1_KEY );

	if ( 
		( imageViewPref > 0 ) &&
		( imageViewPref < 10 )
	)
		currentView = imageViewPref;
	else
		currentView = views[0].feat.nextView;

#ifdef MVLITE
	// LITE: we have no list views, but we could be configured
	// to startup in a list view by the full version
	if ( ! isGraphicalView( currentView ) )
		currentView = 1;
#endif

        copyView( currentView, 0 );

	dayBarP = new DayBar( this );

	for ( int allChannelNo = 0; allChannelNo < MAX_CHANNELS; allChannelNo++ )
		allChannels[allChannelNo] = NULL;

	// Needed for the 'channelsScrollable' calc below
	// And indicator positions
	setMainWindowGeom();

	load( 
		getLoadTime(),
		conf.windowInitialStartOffsetSeconds
	);

	if ( 
		( views[0].feat.firstChannel == initialPositionPlayingMid ) &&
		channelsScrollable()
	) {
		int playingChannelNo = getPlayingChannelNo();
		int lastScreenfulStart = noChannels - displayableChannels();

		// If in last portion, put last channel at bottom
		if ( playingChannelNo >= lastScreenfulStart ) 
			showChannelNoAtPositionNo( lastScreenfulStart, 0 );
		else
			showChannelNoAtPositionNo( playingChannelNo, displayableChannelsPerColumn() / 2 );
	}
	else
		showChannelNoAtPositionNo( 0, 0 );

        rebuildAll();

	// No problem if we reload now
	canAutoReloadFlag = true;

	// Don't want to risk a reload till we've set everything up

#if HAVECACHE == 1
	eEPGCache *eCache = eEPGCache::getInstance();

	CONNECT( eCache->EPGAvail, MV::INT_EPGAvailable );
#endif

	inputMgrP->startCache( inputs );

}

void MV::showChannelNoAtPositionNo( int channelNo, int visiblePositionNo )
{
	if ( channelNo == -1 )
		firstVisibleChannel = 0;
	else {
		firstVisibleChannel = channelNo - visiblePositionNo;
		if ( firstVisibleChannel < 0 )
			firstVisibleChannel = 0;
	}
}

void MV::run( void )
{
	setRestoreAspectRatio( true, conf.aspectRatioMode );

#ifndef MVLITE
	// If we need to start in a list view, we pretend a key
	// was pressed to show the list view

	int ret = 9999;

	if ( ! isGraphicalView( currentView ) )  {
		// Make sure we don't handle any auto-reload
		// stuff while in list window
		canAutoReloadFlag = false;
		ret = eventHandleViewMode( KEY_STATE_DOWN, viewNoToKeyNo( currentView ) );
		canAutoReloadFlag = true;
	}

	// When we exit the list view, if we didn't exit because
	// of EXIT key pressed, exec as normal.

	if ( ret == 0 )
		waitCanExit();
	else
#endif
		showExecHide( this );
}

bool MV::checkDir( const eString &dir )
{
	if ( makeDirIfNotExists( dir ) ) {
		return true;
	}
	else {
		dmsg( getStr( strErrMakingDir ), dir );
		setError( errorCodeDirCreate );
		return false;
	}
}

void MV::setDayBarGeom( void )
{
	// If we're showing channel headers, line the day bar
	// header up with them, otherwise specify 0 and the
	// day bar can choose

	int headerWidth;

	if ( views[0].feat.channelHeaderFlags ) 
		headerWidth = MAX( views[0].geom.headerWidthPixels, timeLabelP->getSize().width() ) + 2;
	else
		headerWidth = 0;

	dayBarP->redoGeom( 
		0, 0, 
		clientrect.width(), conf.dayBarHeight,
		headerWidth, conf.timePickerHeight
	);
}

#ifndef MVLITE

void MV::addFavouriteHandle( void )
{
	bool successFlag = false;

	if ( currentChannel != -1 ) {
		{
			Channel *cp = channels[currentChannel];
			if (cp != NULL)
			{
				Program *pp = cp->getCurrentProgramPtr();
				if ( pp != NULL ) {
					hideShowAll(true, true);
					successFlag = true;
		
					favP = new FavManager( (char *) formConfigPath( FAVOURITES_FILENAME ).c_str() );
					favP->addNameAndChannelEntry( pp->getTitle(), channels[currentChannel]->getName() );
					delete favP;
					hideShowAll(false, true);
				}
				if ( ! successFlag )
					cp->flashCursor();
			}
		}
	}
}

void MV::rebuildListWindow( ProgramList &listWindow, int type, int channelToShow )
{
	int channelStart = 0;
	int channelEnd = noChannels;

	eString title;

	int progsPerChannel;
	if ( views[0].feat.entriesPerColumn > 0 )
		progsPerChannel = views[0].feat.entriesPerColumn;
	else 
		progsPerChannel = 9999;

#if HAVECACHE == 1
	if ( 
		( channelToShow != -1 ) &&
		( inputMgrP->haveCacheInput(inputs) )
	)
		inputMgrP->loadOneChannelCacheData( channels[channelToShow]->getServiceRef() );
#endif
	
	if ( type == programListTypeAllChannels ) {
		title = eString( "All Channels" );
	}
	else if ( type == programListTypeChannel ) {
	
		// If no current channel, and e.g. no data for
		// playing channel, show empty screen with
		// channel name

		if ( channelToShow == -1 ) {
			channelStart = 0;
			channelEnd = 0;

			title = getPlayingChannelName();
		}
		else {
			channelStart = channelToShow;
			channelEnd = channelToShow + 1;

			title = channels[channelToShow]->getName();
		}
	}
	else {
		if ( type == programListTypeFilm )
			title = eString( "Films" );
		else 
			title = eString( "Favourites" );
	}

	listWindow.reset( title );

	time_t minEndTime = sampleStartSecond + (time_t) inputs.pre;

	for ( int channelNo = channelStart; channelNo < channelEnd; channelNo++ ) {
		int pCount = 0;
		Channel *cp = channels[channelNo];
		Program *pp = cp->getNextProgramPtrByType( type, true, minEndTime  );
		while ( 
			( pp != NULL ) &&
			( pCount < progsPerChannel ) 
		) {
			listWindow.addEntry( cp, pp );
			pp = cp->getNextProgramPtrByType( type, false, minEndTime );
			pCount++;
		}
	}
}

// In order to show the main menu in a list view (which is
// a separate window, we need to exit the window, call
// doMenu() and go back to the window.

int MV::execListWindow( int viewNo, int type )
{
	copyView( viewNo, 0 );
	currentView = viewNo;
	setBorderIndicators();

	hideShowAll(true);

	int channelToShow;

	// Playing channel
	if ( currentChannel == -1 )
		channelToShow = getPlayingChannelNo();
	else {
		channelToShow = currentChannel;

#ifdef DEBUG
		if ( ! channels[currentChannel]->releaseCursor() )
			dmsg( "program err: execListWindow: current channel wouldn't release cursor: ", currentChannel );
#endif
		currentChannel = -1;
	}

	ProgramList listWindow( 
		views[0],
		conf.timerStartOffsetSeconds, 
		conf.timerEndOffsetSeconds, 
		sBarP, timeLabelP, 
		indicators[indWindowHidden],
		indicators[indEdit]
	);
	CONNECT( listWindow.hiding, MV::storeRestoreIndicators );
	CONNECT( listWindow.showInfoHidden, MV::showInfoWhenHidden );

	bool finishedFlag = false;
	int result = -1;
	int mode = MV_MODE_VIEW;

	while ( ! finishedFlag ) {

		rebuildListWindow( listWindow, type, channelToShow );

		listWindow.show();
		listWindow.setMode( mode );
		timeLabelP->show();

		setTimeLabelGeom();
        	showHideWidget( sBarP, views[0].feat.showStatusBar && ( listWindow.noEntries() > 0 ) );
		int newKey = listWindow.exec();

 		//KeyCatcher kc; showExecHide( &kc );

		sBarP->hide();

		// If we we're editing, store possible changes
		if ( mode == MV_MODE_EDIT ) {
			setIndicator( indEdit, false );
			copyView( 0, viewNo );
			mode = MV_MODE_VIEW;
		}

		finishedFlag = true;
		switch ( newKey ) {
			case VIEW_BOUQUET_SELECTOR:
				listWindow.hide();
				doBouquetSelector( false );
				finishedFlag = false;
				break;
			case VIEW_SWITCH_1:
			case VIEW_SWITCH_2:
			case VIEW_SWITCH_3:
			case VIEW_SWITCH_4:
			case VIEW_SWITCH_5:
				listWindow.hide();
       		                changeToView( keyNoToViewNo( newKey ) );
				hideShowAll(false);
				result = -1;
			case VIEW_EXIT:
			case VIEW_SWITCH_6:
			case VIEW_SWITCH_7:
			case VIEW_SWITCH_8:
			case VIEW_SWITCH_9:
				result = newKey;
				break;
			case VIEW_SHOW_MENU:
				listWindow.hide();
				if ( doMenu() == MENU_ITEM_EDIT_VIEW_GEOM ) {
					mode = MV_MODE_EDIT;
					setIndicator( indEdit, true );
				}
				finishedFlag = false;
				break;
			case VIEW_DAY_CENTRE:
				reload();
				finishedFlag = false;
				break;
			case VIEW_PICKER_UP:
				listWindow.hide();
				channelToShow = doChannelPicker( channelToShow, -1, true );
				type = programListTypeChannel;
				finishedFlag = false;
                                break;
                        case VIEW_PICKER_DN:
				listWindow.hide();
                                channelToShow = doChannelPicker( channelToShow, 1, true );
				type = programListTypeChannel;
				finishedFlag = false;
                                break;
			case VIEW_DAY_LEFT:
                                channelToShow = doChannelPicker( channelToShow, -1, false );
				type = programListTypeChannel;
				finishedFlag = false;
				break;
			case VIEW_DAY_RIGHT:
                                channelToShow = doChannelPicker( channelToShow, +1, false );
				type = programListTypeChannel;
				finishedFlag = false;
				break;
			default:
				result = -1;
		}
	}
	listWindow.hide();


	return result;
}

int MV::doChannelPicker( int oldChannel, int dir, bool showSelectorFlag )
{
	// Maybe we weren't in the show-channel view already
	copyView( 6, 0 );
	currentView = 6;

	int startFromChannel = oldChannel + dir;

	if ( startFromChannel < 0 )
		startFromChannel = noChannels - 1;
	else if ( startFromChannel >= noChannels )
		startFromChannel = 0;

	if ( ! showSelectorFlag )
		return startFromChannel;

	setListboxFontSize( CHANNEL_PICKER_LISTBOX_FONT_SIZE );

	ChannelPicker cp;

	eListBoxEntryText *curET = NULL;
	for ( int channelNo = 0; channelNo < noAllChannels; channelNo++ ) {
		eListBoxEntryText *et = cp.addEntry( allChannels[channelNo]->getName(), channelNo );

		if ( channelNo == startFromChannel )
			curET = et;	
	}
	if ( curET != NULL )
		cp.setCur( curET );

	int result = showExecHide( &cp );

	// Restore to normal fontsize, it's a system-wide
	// setting

	setListboxFontSize( 0 );

	if ( 
		( result >= 0 ) &&
		( result < noChannels )
	)
		return result;
	else 
		return oldChannel;
}

#endif

void MV::setIndicator( int indNo, bool toSet )
{
	showHideWidget( indicators[indNo], toSet );
}

void MV::showTimerEditWindowAux( int type )
{
	if ( currentChannel == -1 ) 
		return;

	Channel *cp = channels[currentChannel];
	Program *pp = cp->getCurrentProgramPtr();

	if ( pp ) {
		hideShowAll( true, true );
		showTimerEditWindow( 
			this,
			type, 
			cp, pp,
			conf.timerStartOffsetSeconds, 
			conf.timerEndOffsetSeconds
		);
		hideShowAll(false, true);
	}
	else {
		channels[currentChannel]->flashCursor();
	}
}

/*void MV::sortChannels( int sortType )
{
	Channel *sortedChannels[noChannels];
	int sortedNo = 0;

	if ( sortType == SORT_BY_USER_BOUQUET ) {
		eString bouquetName = inputMgrP->nextBouquetName( true );

		while ( bouquetName.length() > 0 ) {
			for ( int unsortedNo = 0; unsortedNo < noChannels; unsortedNo++ ) {
				if ( 		
					( channels[unsortedNo] != NULL ) &&
					( channels[unsortedNo]->getName() == bouquetName ) 
				) {
					sortedChannels[sortedNo++] = channels[unsortedNo];
					channels[unsortedNo] = NULL;
					break;
				}
			}		
			bouquetName = inputMgrP->nextBouquetName( false );
		}
	}

	// Stick any channels not found in bouqeut at end

	for ( int unsortedNo = 0; unsortedNo < noChannels; unsortedNo++ ) {
		if ( channels[unsortedNo] != NULL )
			sortedChannels[sortedNo++] = channels[unsortedNo];
	}

	if ( sortedNo != noChannels )
		dmsg( "program err: mismatch after sort" );

	// Copy sorted to actual channels

	for ( sortedNo = 0; sortedNo < noChannels; sortedNo++ )
		channels[sortedNo] = sortedChannels[sortedNo];
}
*/

void MV::rebuildAll( void )
{
	setBackgroundColor( colourOptionToColour( views[0].pres.backColour ) );

	setMainWindowGeom();

	configureChannels();
        rebuildTimeBars();

	rebuildChannels();

	showHideWidget( dayBarP, dayBarFixed() );

	showHideTimebars( timebarsEnabled(0) );

	// If view switching too requires status bar, set and show
	// otherwise hide

	if ( views[0].feat.showStatusBar ) {
		if ( currentChannel != -1 ) {
			channels[currentChannel]->setStatusBarText();
			sBarP->show();
		}
		else
			sBarP->hide();
	
	}
	else
		sBarP->hide();

/*	showHideWidget( sBarP, views[0].feat.showStatusBar && ( currentChannel != -1 ) );

	channels[currentChannel]->setStatusBarText();
*/

	showHideWidget( chanBarP, views[0].feat.showChannelBar );
}

void MV::load( time_t sampStartTime, time_t viewWindowOffset  )
{
	setLoadTime();
	timeLabelP->setText( timeToHourMinuteString( getLoadTime() ) );

	if ( viewWindowOffset < 0 ) 
		viewWindowOffset = 0;

	firstVisibleChannel = 0;
	lastVisibleChannel = 0; noVisibleChannels = 0;
	currentChannel = -1;

	sampleStartSecond = sampStartTime - inputs.pre;
	windowStartSecond = sampleStartSecond + viewWindowOffset;

	if ( inputMgrP != NULL ) 
		delete inputMgrP;

	inputMgrP = new InputManager( formConfigPath( MAP_FILENAME ), userLangPref3Char );

	// We only need to do this on startup, or when
	// switching bouquet

	if ( needRecreateChannelsFlag ) {
		recreateChannels();
		needRecreateChannelsFlag = false;
	}

	// Reset channels
	for ( int allChannelNo = 0; allChannelNo < noAllChannels; allChannelNo++ )
		allChannels[allChannelNo]->reset( );

	CONNECT( inputMgrP->gotData, MV::receiveData );
	CONNECT( inputMgrP->gotOneChannelCacheData, MV::receiveOneChannelCacheData );
	CONNECT( inputMgrP->XMLTVStatusChange, MV::INT_XMLTVStatusChange );
	CONNECT( inputMgrP->IMdownloadStarted, MV::downloadStarted );
	CONNECT( inputMgrP->IMallDownloadsFinished, MV::INT_downloadsDone );
	
	// Variables set in receive data

	nextProgramID = 0;
	lastReceivedChannelName = "";
#ifndef MVLITE
	favouriteChecksum = 0;
	favouritesNotifyFlag = false;

	// Need to check if programs match favourites as
	// they're read in

	favP = new FavManager( (char *) formConfigPath( FAVOURITES_FILENAME ).c_str() );
#endif

	setIndicator( indLoading, true );
	// Indicator doesn't show in time otherwise, something to do
	// with thread sequence I guess
	microSleep( 40000 );
	inputMgrP->readProgramData( sampStartTime, inputs );
	setIndicator( indLoading, false );

	// allChannels -> channels
	// sets noChannels

	selectChannels();

	// Don't need it anymore, save memory
#ifndef MVLITE
	delete favP;

	setIndicator( indFavourites, 
		(
		( favouritesNotifyFlag ) &&
		( favouriteChecksum != 0 ) &&
		( favouriteChecksum != conf.lastFavouriteChecksum )
		)
	);
#endif

	setIndicator( indUnknownChannels, inputMgrP->haveUnknownChannels() );

}

void MV::recreateChannels( void )
{
	noAllChannels = 0;

	if ( channels != NULL )
		delete channels;

	eString bouquetName = inputMgrP->nextBouquetName( true );
	while ( 
		( noAllChannels < MAX_CHANNELS ) &&
		( bouquetName.length() > 0 ) 
	) {
		eServiceReferenceDVB sRef;
		inputMgrP->getServiceRef( bouquetName, &sRef );

		// Create if not exist
		if ( allChannels[noAllChannels] == NULL )
			allChannels[noAllChannels] = new Channel( this );

		// OOM
		if ( allChannels[noAllChannels] == NULL ) {
			dmsg( getStr( strOOM ) );
			break;
		}
	
		allChannels[noAllChannels]->baseReset( bouquetName, sRef, sBarP );

		noAllChannels++;
		bouquetName = inputMgrP->nextBouquetName( false );
	}

	channels = new Channel * [noAllChannels];

	// OOM
	if ( channels == NULL )
		dmsg( getStr( strOOM ) );
}

void MV::selectChannels( void )
{
	noChannels = 0;

	for ( int allChannelNo = 0; allChannelNo < noAllChannels; allChannelNo++ ) {
		if ( 
			( views[0].feat.showEmptyChannels ) ||
			( allChannels[allChannelNo]->getNoPrograms() > 0 )
		) {
			channels[noChannels] = allChannels[allChannelNo];
			channels[noChannels]->setID( noChannels ); 
			noChannels++;
		}
	}
}

void MV::INT_downloadsDone( int noFilesDownloaded )
{
	// In theory, we only need to convert when there
	// were actually downloaded files. However, this
	// is the only place we call conversion to prevent
	// interference between conversion of local and
	// downloaded files
	// In other words, local files are not converted
	// until any downloads are complete. 

	// This just starts a thread potentially, will exit v quick
	inputMgrP->convertXMLTVFilesToEPGUI( inputs );

//	if ( noFilesDownloaded > 0 ) {
			
		// This works but...
		// Is reloading after download worthwhile ?
		// XMLTV convert gets disrupted, careful...
		// reloadIfDesiredAndPossible();
//	}

	setIndicator( indDownloading, false );
}

void MV::INT_XMLTVStatusChange( bool newStatus )
{
	setIndicator( indXMLTV, newStatus );
}

bool MV::reloadIfDesiredAndPossible( void )
{
	if ( 
		( inputs.autoReloadFlag ) &&
		( canAutoReloadFlag )
	) {
		reload();
		return true;
	}
	else
		return false;
}

void MV::downloadStarted( void )
{
	setIndicator( indDownloading, true );
}

void MV::moveCursorToChannel( int toChannel, int posNegRetry, int startEnd )
{
	time_t oldStart, oldEnd;
	
	// If there's a currentChannel, try take cursor from it
	// If fails, assume there wasn't a current channel and
	// take middle time values

	if ( toChannel == currentChannel ) {
#ifdef DEBUG
		dmsg( "program err: moveCursorToChannel: oops tried to move to same channel: ", toChannel );
#endif
		return;
	}
	else if ( currentChannel != -1 ) {
		if ( ! channels[currentChannel]->releaseCursor( &oldStart, &oldEnd ) ) {
#ifdef DEBUG
			dmsg("program err: moveCursorToChannel: current channel wouldn't release cursor: ", currentChannel );
#endif
			currentChannel = -1;
		}
		sBarP->hide();
	}

	// If there wasn't a current channel, pick middle values

	if ( currentChannel == -1 ) {
		if ( views[0].feat.entriesPerColumn == 0 ) {
			oldStart = windowMiddleSecond();
			oldEnd = oldStart + 2;
		}
		else {
			oldStart = 1;
			oldEnd = -1;
		}
	}	

	// Could be there's no programs in the channel
	// asked to move to. If so, need to keep trying
	// in direction posNegRetry until we find a
	// channel that will accept

	// If at top or bottom, always retry moving away,
	// whatever the caller asked

	if ( toChannel == firstVisibleChannel )
		posNegRetry = +1;
	else if ( toChannel == lastVisibleChannel )
		posNegRetry = -1;
	
	while ( 
		( toChannel >= firstVisibleChannel ) &&
		( toChannel <= lastVisibleChannel )
	) {
		if ( startEnd > 0 ) {
			oldStart = 1;
			oldEnd = -1;
		}
		else if ( startEnd < 0 ) {
			oldStart = views[0].feat.entriesPerColumn;
			oldEnd = -1;
		}

		if ( channels[toChannel]->takeCursor( oldStart, oldEnd, views[0].feat.forceCursorFlag ) ) {
			showHideWidget( sBarP,
				(
					( channels[toChannel]->getCurrentProgramPtr() != NULL ) &&
		        		( views[0].feat.showStatusBar )
				)
			);

			currentChannel = toChannel;
			break;
		}
		else
			toChannel += posNegRetry;
	}

	// If we couldn't find a channel to take cursor,
	// call the whole thing off

	if ( currentChannel != toChannel ) {
#ifdef DEBUG
		dmsg("program err: moveCursorToChannel::boo hoo no one wants me anymore");
#endif
		currentChannel = -1;
	}
}


void MV::initCursor( eString preferredChannelName )
{
	int direction = +1;
	int targetChannel = -1;

	if ( noVisibleChannels == 1 )
		targetChannel = firstVisibleChannel;
	else if ( noVisibleChannels > 0 ) {
		int halfwayChannel = firstVisibleChannel + ( noVisibleChannels / 2 ) - 1;

		if ( preferredChannelName.length() > 0 ) {

			int channelNo = firstVisibleChannel;

			while ( 
				( targetChannel == -1 ) &&
				( channelNo < lastVisibleChannel )
			) {
				if ( channels[channelNo]->getName() == preferredChannelName )
					targetChannel = channelNo;
				else
					channelNo++;
			}

			// Prefer going backwards if we're past halfway, more
			// chance of finding a candidate

			if ( targetChannel > halfwayChannel )
				direction = -1;
		}

		if ( targetChannel == -1 )
			targetChannel = halfwayChannel;
	}

	if ( targetChannel != -1 )
		moveCursorToChannel( targetChannel, direction );
}

void MV::receiveOneChannelCacheData( struct ProgramData &p )
{
	int channelNo = allChannelNameToChannelNo( p.channelName );

	// If it's an old channel, or if it's a new channel
	// and we havn't hit the limit..

	if ( channelNo != -1 ) {
		allChannels[channelNo]->addProgram( 
			nextProgramID, 
			p.name, p.startTime, p.duration, 
			( p.flags & programDataFlagFilm ),
			false,
			true,p.idxFic,p.offset
		);
	}
}

void MV::receiveData( struct ProgramData &p )
{
	static int channelNo = -1;

	// Stop if it doesn't fit in the sample window
	// (but we accept all future programs if they came from
	// the cache)

	time_t endTime = p.startTime + p.duration;

//	( ! ( p.flags & programDataFlagFromCache ) ) &&

	if ( 
		( endTime <= sampleStartSecond ) || 
		( p.startTime >= sampleEndSecond() )
	)
		return;

	// Find which channel no it is
	// Speedup: don't need to do it if the channel name
	// is the same as the last one

	if ( p.channelName != lastReceivedChannelName )
		channelNo = allChannelNameToChannelNo( p.channelName );

	// If it's an old channel, or if it's a new channel
	// and we havn't hit the limit..

	if ( channelNo != -1 ) {

#ifndef MVLITE
		bool isFavouriteFlag;
		bool notifyFlag = false;
		favP->checkFavourite( &p, &isFavouriteFlag, &notifyFlag );

		if ( notifyFlag )
			favouritesNotifyFlag = true;
#endif

		// Rest of details in program

		bool successFlag = allChannels[channelNo]->addProgram( 
			nextProgramID, 
			p.name, p.startTime, p.duration, 
			( p.flags & programDataFlagFilm ),
#ifdef MVLITE
			false,
#else
			isFavouriteFlag,
#endif
			( p.flags & programDataFlagFromCache )
			,p.idxFic,p.offset
		);

		// Add desc to cache if available
		// and didn't come from cache

		if ( successFlag ) {
#ifndef MVLITE
			if ( notifyFlag )
				favouriteChecksum += p.startTime;
#endif
		}
	}
}

void MV::rebuildTimeBars( void )
{
	for ( int barNo = 0; barNo < views[0].feat.noColumns; barNo++ )
		tBarP[barNo]->rebuild( windowStartSecond, windowEndSecond(), (time_t) views[0].feat.timebarPeriodSeconds );
}

// If entries aren't time based, a time bar is not much use
bool MV::timebarsEnabled( int viewNo )
{
	return ( 
		( views[viewNo].feat.timebarPeriodSeconds > 0 ) &&
		( views[viewNo].feat.entriesPerColumn == 0 )
	);
}

bool MV::dayBarFixed( void )
{
	return ( views[0].feat.dayBarMode == dayBarModeFixed );
}

void MV::rebuildChannels( void )
{
	int channelNo = firstVisibleChannel;
	noVisibleChannels = 0;
	int screenChannelNo = 0;

	int yPos;

	// Timer bar

	int yPosStart = dayBarFixed() ? conf.dayBarHeight : 0;

	if (  timebarsEnabled( 0 ) )
		yPosStart += conf.timeBarHeight;

	yPos = yPosStart;

	// Calc column width

	int xPos = ( views[0].feat.showChannelBar ) ? views[0].geom.channelBarWidthPixels : 0;
	int availableWidth = clientrect.width() - xPos;

	int columnWidth = availableWidth / views[0].feat.noColumns;
	int leftoverWidth = availableWidth - ( columnWidth * views[0].feat.noColumns );

	int channelsPerColumn = displayableChannelsPerColumn();
	
	int columnNo = 1;

	// Rebuild each channel

	while (
		( screenChannelNo < displayableChannels() ) &&
		( channelNo < noChannels )
        ) {
		Channel *cp = channels[channelNo];

		// Have to do this first. Otherwise, if it's
		// visible and we move it, stuff gets left

		// Add leftover width to the last column

		setWidgetGeom( cp, 
			xPos, yPos, 
			( columnNo == views[0].feat.noColumns ) ? ( columnWidth + leftoverWidth ) : columnWidth, rowHeight()
		);

		cp->setRange( windowStartSecond, windowEndSecond() );

		if ( views[0].feat.channelHeaderFlags )
			cp->setHeaderWidth( views[0].geom.headerWidthPixels );

		if ( ! cp->isVisible() )
			cp->show();
		else
			cp->invalidate();

		channelNo++;
		screenChannelNo++;
		noVisibleChannels++;

		if ( ( screenChannelNo % channelsPerColumn ) == 0  ) {
			columnNo++; 
			xPos += columnWidth;
			yPos = yPosStart;
		}
		else {
			yPos += cp->height();
		}
	}

	// Will go -1 if noVisibleChannels is 0
	lastVisibleChannel = firstVisibleChannel + noVisibleChannels - 1;

	if ( views[0].feat.showChannelBar )
		setChannelBarGeom();
}

void MV::configureChannels( void )
{
	gFont programTimeFont = makeFontFromOptions( views[0].pres.programTimeFontFamily, views[0].pres.programTimeFontSize );
	gFont programTitleFont = makeFontFromOptions( views[0].pres.programTitleFontFamily, views[0].pres.programTitleFontSize );
	gFont programDescrFont = makeFontFromOptions( views[0].pres.programDescrFontFamily, views[0].pres.programDescrFontSize );
	gFont programChannelFont = makeFontFromOptions( views[0].pres.programChannelFontFamily, views[0].pres.programChannelFontSize );

	gFont channelHeaderFont = makeFontFromOptions( views[0].pres.channelHeaderFontFamily, views[0].pres.channelHeaderFontSize );

	for ( int cn = 0; cn < noChannels; cn++ ) {
		Channel *cp = channels[cn];

		cp->setRange( windowStartSecond, windowEndSecond() );
		cp->setHeaderFlags( views[0].feat.channelHeaderFlags );
		cp->setSeparators( views[0].feat.horizontalSep, views[0].feat.verticalSep );
		cp->setElapsedBars( views[0].feat.showElapsedBars );

		cp->setHeaderFont( channelHeaderFont );
		cp->setProgramFonts( programTimeFont, programTitleFont, programDescrFont, programChannelFont );
		cp->setProgramBoxFlags( views[0].feat.programBoxFlags );
		cp->setStatusBarFlags( views[0].feat.statusBarFlags );

		cp->setHeaderColours( 
			colourOptionToColour( views[0].pres.channelHeaderBackColour ), 
			foreColour
		);

		cp->setProgramColours( 
			colourOptionToColour( views[0].pres.backColour), 
			colourOptionToColour( views[0].pres.playingBackColour), 
			foreColour,
			colourOptionToColour( views[0].pres.programOneBackColour ),
			foreColour,
			colourOptionToColour( views[0].pres.programTwoBackColour ), 
			foreColour,
			colourOptionToColour( views[0].pres.filmBackColour ),
			foreColour,
			colourOptionToColour( views[0].pres.favouriteBackColour ),
			foreColour
		);
		cp->setProgramsToShow( views[0].feat.entriesPerColumn );
	}
}

void MV::setMainWindowGeom( void )
{
	setTimeLabelGeom();

	if ( ! windowHiddenFlag )
	        move( ePoint( views[0].geom.topLeftX, views[0].geom.topLeftY ) );

        resize( eSize( views[0].geom.widthPixels, views[0].geom.heightPixels));

	setDayBarGeom();

	if ( timebarsEnabled( 0 ) )
		setTimebarGeom();

        if ( views[0].feat.showStatusBar )
                setStatusBarGeom();

        if ( views[0].feat.showChannelBar )
		setChannelBarGeom();
}

void MV::setChannelBarGeom( void )
{
	int chanBarYpos = 0;
	int chanBarHeight;

	if ( timebarsEnabled(0) ) 
		chanBarYpos += conf.timeBarHeight;
	if ( dayBarFixed() )
		chanBarYpos += conf.dayBarHeight;

	int availableHeight = clientrect.height() - chanBarYpos;

	if ( 
		( noChannels > 0 ) &&
		( channelsScrollable() )
	) {
		chanBarHeight = (int) ( ( (float) noVisibleChannels / (float) noChannels ) * (float) availableHeight );
		chanBarYpos += (int) ( ( (float) firstVisibleChannel / (float) noChannels ) * ( float ) availableHeight );
	}
	else {
		chanBarHeight = availableHeight;
	}

	// If we don't hide it, the old area doesnt get cleared

	chanBarP->hide();
	setWidgetGeom( chanBarP, 
		0, chanBarYpos,
		views[0].geom.channelBarWidthPixels, chanBarHeight 
	);
	chanBarP->show();
}

void MV::setTimebarGeom( void )
{
	int columnWidth = clientrect.width() / views[0].feat.noColumns;
	int leftoverWidth = clientrect.width() - ( columnWidth * views[0].feat.noColumns );
	
	// Start to right of channel bar, if it's on
	int xpos = ( views[0].feat.showChannelBar ) ? views[0].geom.channelBarWidthPixels : 0;

	// Start below the day bar, if it's fixed
	int ypos = dayBarFixed() ? conf.dayBarHeight : 0;

	int thisWidth, thisX;
	for ( int barNo = 0; barNo < views[0].feat.noColumns; barNo++ ) {
		if ( views[0].feat.channelHeaderFlags ) {
			thisWidth = columnWidth - views[0].geom.headerWidthPixels;
			thisX = xpos + views[0].geom.headerWidthPixels;
		}
		else {
			thisWidth = columnWidth;
			thisX = xpos;
		}

		if ( barNo == ( views[0].feat.noColumns - 1 ) )
			thisWidth += leftoverWidth;

		setWidgetGeom( tBarP[barNo],
			thisX, ypos,
			thisWidth, conf.timeBarHeight
		);
		xpos += columnWidth;
	}
}

void MV::showHideTimebars( bool showFlag )
{
	for ( int barNo = 0; barNo < views[0].feat.noColumns; barNo++ )
		showHideWidget( tBarP[barNo], showFlag );
}

int MV::rowHeight( void )
{
	int height = views[0].geom.channelHeightPixels;

	if ( views[0].feat.horizontalSep )	
		height += 1;
		
	return height;
}

eString MV::getPlayingChannelName( void )
{
        eString result="";

        eServiceDVB *current=eDVB::getInstance()->settings->getTransponders()->searchService( getPlayingServiceRef() );
        if(current)
                return buildShortServiceName( current->service_name );
        else 
               return "";
}

int MV::getServiceOrbitalPos( eServiceReferenceDVB ref )
{
	return ( ref.getDVBNamespace().get()>>16 );
}

eString MV::getPlayingTSID( void )
{
    	return eString().sprintf("%04xh", getPlayingServiceRef().getTransportStreamID().get() );
}

int MV::error( void )
{
        return errorCode;
}

void MV::copyView( int from, int to )
{
	memcpy( &views[to], &views[from], sizeof(struct ViewConf) );
}

void MV::setError( int toSet )
{
        errorCode = toSet;
}

int MV::eventHandler(const eWidgetEvent &event)
{
        switch (event.type)
        {
        case eWidgetEvent::execBegin:
		execFlag = true;
                if ( sBarP->getPosition().isNull() ) {
                        setStatusBarGeom();
                        sBarP->loadDeco();
                }
                break;
        case eWidgetEvent::execDone:
		execFlag = false;
		break;
	case eWidgetEvent::evtKey:
	{
		canAutoReloadFlag = false;
		int result = eventHandleMain( (event.key)->flags, (event.key)->code );
		canAutoReloadFlag = true;
		if ( result )
		 	return 1;
	}
		break;
	default:
		break;
	}

	return eWidget::eventHandler(event);
}

int MV::eventHandleMain( int keyState, int rc )
{
	// Machinations to avoid showing a 'C' when 
	// we were doing an autoscan, it's annoying

	if ( ! fillingCacheFlag ) {
		wasFillingCacheFlag = false;

		if ( indicators[indFillCache]->isVisible() )
			setIndicator( indFillCache, false );
	}

	if ( 
		windowHiddenFlag &&
		( keyState == KEY_STATE_DOWN ) &&
		( rc != RC_VOLUP ) &&
		( rc != RC_MUTE ) &&
		( rc != RC_VOLDN ) 
	){
		if ( rc == VIEW_SHOW_INFO ) {
			showInfoWhenHidden();
		}
		else
			doHideShow( false );
		return 1;
	}
	else if ( currentMode == MV_MODE_VIEW ) {
		if ( dayBarP->timePickerIsActive() )
			return eventHandleTimePicker( keyState, rc );
		else if ( dayBarP->isActive() )
			return eventHandleDayBar( keyState, rc );
		else
			return eventHandleViewMode( keyState, rc );
	}
#ifndef MVLITE
	else if ( 
		( currentMode == MV_MODE_EDIT ) ||
		( currentMode == MV_MODE_EDIT_SBAR ) ||
		( currentMode == MV_MODE_EDIT_IND )
	) {
		return eventHandleEditMode( keyState, rc ) ;
	}
#endif
	else {
		return 0;
	}
}
/*
int MV::eventHandleHidden( int rc )
{
	int channelToShow = -1;

	int playChan = getPlayingChannelNo();
	if ( playChan == -1 )
		playChan = 0;

	if (
              ( rc == RC_BOUQUP ) ||
              ( rc == RC_UP )
        )
            channelToShow = doChannelPicker( playChan, -1);
        else if (
                        ( rc == RC_BOUQDN ) ||
                        ( rc == RC_DN )
        )
            channelToShow = doChannelPicker( playChan, +1);
        else if ( rc == RC_LEFT )
            channelToShow = playChan - 1;
        else if ( rc == RC_RIGHT )
            channelToShow = playChan + 1;
        else
		return 0;

        if (
            ( channelToShow >= 0 ) &&
            ( channelToShow < noChannels ) &&
            ( channelToShow != playChan )
        )
            channels[channelToShow]->play();

	return 1;
}
*/

int MV::eventHandleViewMode( int keyState, int key )
{
	if ( keyState == KEY_STATE_UP )
		return 0;
		

	int newKey = key;
	int handled = 1;
	while ( newKey != -1 ) {
		if ( newKey != -1 ) {
			key = newKey;
			newKey = -1;
		}
		switch ( key ) {
		case VIEW_PROG_LEFT:
			viewHandleLeftRight( -1 );
			break;
		case VIEW_PROG_RIGHT:
			viewHandleLeftRight( +1 );
			break;
		case VIEW_INC_CHANNEL:
			viewHandleUpDown( +1 );
			break;
		case VIEW_DEC_CHANNEL:
			viewHandleUpDown( -1 );
			break;
		case VIEW_BOUQUET_SELECTOR:
			doBouquetSelector( true );
			break;
		case VIEW_SWITCH_1:  
		case VIEW_SWITCH_2: 
		case VIEW_SWITCH_3: 
		case VIEW_SWITCH_4: 
		case VIEW_SWITCH_5: 
                        changeToView( keyNoToViewNo( key ) );
			break;
#ifndef MVLITE
		case VIEW_SHOW_MENU:
                        doMenu();
			break;
#endif
		case VIEW_SHOW_HELP:
			showInfoBox( this, strViewHelpTitle, strViewHelp );
			break;
		case VIEW_SELECT:
			handleSelect();
			break;
		case VIEW_EXIT:
			// Just exit if we have no cursor
			if ( currentChannel == -1 ) {
				// Don't allow exit if we're still downloading
				handled = inputMgrP->downloading();
				if ( handled ) 
					flashMessage( getStr( strDownloadInProgress ) );
				else {
					if ( execFlag ) {
						waitCanExit();
						close( 0 );
					}
				}
			}
			else {
#ifdef DEBUG
				if ( ! channels[currentChannel]->releaseCursor() )
					dmsg("program err: eventHandleViewMode: current channel wouldn't release cursor: ", currentChannel );
#endif
				sBarP->hide();
				currentChannel = -1;
			}
			break;
#ifndef MVLITE
		case VIEW_SWITCH_6:
			newKey = execListWindow( 6, programListTypeChannel );
		
			break;
		case VIEW_SWITCH_7:
			newKey = execListWindow( 7, programListTypeFilm );
			break;
		case VIEW_SWITCH_8:
			// Record that we've seen this checksum
			if ( conf.lastFavouriteChecksum != favouriteChecksum )
				conf.lastFavouriteChecksum = favouriteChecksum;

			// And turn off the indicator
			setIndicator( indFavourites, false );

			newKey = execListWindow( 8, programListTypeFavourite );
			break;
		case VIEW_SWITCH_9:
			newKey = execListWindow( 9, programListTypeAllChannels );
			break;
		case VIEW_ADD_FAVOURITE:
			addFavouriteHandle();
			break;
#endif
		case VIEW_FILL_CACHE_CHANNEL: //TV : channel under cursor or playing channel
			handleCacheFill( fillCacheSpecifiedChannel );
			break;
		case VIEW_FILL_CACHE_PLAYING: //RADIO : bouquet on current satellite
			handleCacheFill( fillCacheCurrentPos );
			break;
		case VIEW_FILL_CACHE: //TEXT : bouquet on all satellites
			handleCacheFill( fillCacheAllPos );
			break;
		case VIEW_SHOW_INFO:
			showInfo();
			break;
		case VIEW_SHOW_TIMER_LIST:
			hideShowAll(true, true );
			showTimerListWindow();
			hideShowAll(false, true);
			break;
		case VIEW_SHOW_TIMER_EDIT_REC:
			showTimerEditWindowAux( timerEditRecord );
			break;
		case VIEW_SHOW_TIMER_EDIT_NGRAB:
			showTimerEditWindowAux( timerEditNGRAB );
			break;
		case VIEW_SHOW_TIMER_EDIT_DELETE:
			showTimerEditWindowAux( timerEditDelete );
			break;
		case VIEW_SHOW_TIMER_EDIT_SWITCH:
			showTimerEditWindowAux( timerEditSwitch );
			break;
		case VIEW_DAY_LEFT:
			if ( dayBarUsable() ) {
				dayBarP->activate();
				dayBarP->shiftCursorLabel( -1 );
			}
			break;
		case VIEW_DAY_RIGHT:
			if ( dayBarUsable() ) {
				dayBarP->activate();
				dayBarP->shiftCursorLabel( +1 );
			}
			break;
		case VIEW_PICKER_UP:
			if ( dayBarUsable() ) {
				dayBarP->activate();
				dayBarP->doTimePickerFunc( pfuncShow );
				dayBarP->doTimePickerFunc( pfuncShiftUp );
			}
			break;
		case VIEW_PICKER_DN:
			if ( dayBarUsable() ) {
				dayBarP->activate();
				dayBarP->doTimePickerFunc( pfuncShow );
				dayBarP->doTimePickerFunc( pfuncShiftDown );
			}
			break;
		case VIEW_DAY_CENTRE:
			if ( dayBarUsable() ) {
				dayBarP->reset();
				reload();
			}
			else if ( ! fillingCacheFlag )
				reload();
			
			break;
		case VIEW_HIDE_WINDOW:
			doHideShow( true );
			break;
		default:
			handled = 0;
			break;
		}
	}

	return handled;
}

bool MV::dayBarUsable( void )
{
	return ( 
		( views[0].feat.dayBarMode != dayBarModeOff ) &&
		( ! fillingCacheFlag )
	);
}

void MV::doBouquetSelector( bool hideMainWindowFlag )
{
	// Can't change bouquet while filling cache, stop
	if ( fillingCacheFlag )
		handleCacheFill( 0 );

	if ( hideMainWindowFlag )
		hideShowAll( true, true );

	eZapMain::getInstance()->showServiceSelector( -1, eZapMain::pathBouquets );
	needRecreateChannelsFlag = true;
	reload();

	if ( hideMainWindowFlag )
		hideShowAll( false,false );
}

void MV::hideShowAll( bool hideFlag, bool doInd )
{
	if ( hideFlag ) {
		hide();
		sBarP->hide();
	}
	else {
		show();
		if ( 
			( views[0].feat.showStatusBar ) &&
			( currentChannel != -1 )
		)
			sBarP->show();
	}

	if ( doInd )
		storeRestoreIndicators( hideFlag );
}

void MV::doHideShow( bool todo )
{
	windowHiddenFlag = todo;

	// WANT TO HIDE
	if ( todo ) {
		hideShowAll(true, true);
		move( ePoint( 900, 900 ) );
		// Need to see events...
		show();
		setIndicator( indWindowHidden, true );
	}

	// WANT TO SHOW
	else {
		hide();
		rebuildAll();
		hideShowAll(false, true);
		setIndicator( indWindowHidden, false );
	}
}

int MV::getPlayingChannelNo( void )
{
	int channelNo = 0;
	int playingChannelNo = -1;
	while ( channelNo < noChannels ) {
		if ( channels[channelNo]->isPlaying() )  {
			playingChannelNo = channelNo;
			break;
		}
		channelNo++;
	}

	return playingChannelNo;
}

void MV::handleCacheFill( int mode )
{
	static int originalPlayingChannel = -1;

	if ( fillingCacheFlag ) {
		timerP->stop();
		fillingCacheFlag = false;
		
		if ( 
			( originalPlayingChannel != - 1 ) &&
			( originalPlayingChannel != getPlayingChannelNo() )
		) {
			channels[originalPlayingChannel]->play();
		}

		reload();

		if ( windowHiddenFlag )
			doHideShow( false );

		setIndicator( indFillCache, false );
	}
	else {
		fillingCacheFlag = true;

		originalPlayingChannel = getPlayingChannelNo();

		if ( mode == fillCacheSpecifiedChannel ) {
			if ( currentChannel == -1 ) 
				fillCacheChannelNo = originalPlayingChannel;
			else
				fillCacheChannelNo = currentChannel;

			if ( fillCacheChannelNo == -1 )	
				return;
		}
		wasFillingCacheFlag = true;
		doHideShow( true );
		fillCacheType = mode;
		fillCache( true );
	}
}

int MV::eventHandleTimePicker( int keyState, int key )
{
	if ( keyState == KEY_STATE_UP )
		return 0;

	int handled = 1;
	switch ( key ) {
		case VIEW_PICKER_UP:
		case VIEW_DEC_CHANNEL:
			dayBarP->doTimePickerFunc( pfuncShiftUp );
			break;
		case VIEW_PICKER_DN:
		case VIEW_INC_CHANNEL:
			dayBarP->doTimePickerFunc( pfuncShiftDown );
			break;
		case VIEW_EXIT:
			dayBarP->reject( ! dayBarFixed() );
			refreshVisibleChannels();
			break;
		case VIEW_PROG_LEFT:
		case VIEW_DAY_LEFT:
			dayBarP->shiftCursorLabel( -1 );
			refreshVisibleChannels();
			dayBarP->doTimePickerFunc( pfuncRedraw );
			break;
		case VIEW_DAY_RIGHT:
		case VIEW_PROG_RIGHT:
			dayBarP->shiftCursorLabel( +1 );
			refreshVisibleChannels();
			dayBarP->doTimePickerFunc( pfuncRedraw );
			break;
		case VIEW_SELECT:
		case VIEW_DAY_CENTRE:
			reload( dayBarP->accept( ! dayBarFixed() ), conf.windowInitialStartOffsetSeconds );
			break;
		default:
			handled = 0;
			break;
	}

	return handled;
}

void MV::storeRestoreFocusDetails( int mode )
{
	// We record the name. The channelNo could change after a reload

	static eString focusChannelName;
	static time_t focusProgramStart;
	static time_t focusProgramEnd;
	static int focusChannelNo;

	if ( ! isGraphicalView( currentView ) )
		return;

	if ( mode == 1 ) {
		focusProgramStart = -1;
		focusChannelName = "";

		// No cursor: make sure centre channel still in
		// centre

		int middleChannel = firstVisibleChannel + ( displayableChannels() / 2 );
		if ( middleChannel >= noChannels )
			middleChannel = noChannels - 1;

		if ( currentChannel == -1 ) {
			if ( noChannels > 0  )
				focusChannelName = channels[middleChannel]->getName();
		}

		// Cursor: keep cursor channel in same position

		else {
			focusChannelName = channels[currentChannel]->getName();

			if ( ! channels[currentChannel]->releaseCursor( &focusProgramStart, &focusProgramEnd ) ) {
#ifdef DEBUG
				dmsg("program err: reload: current channel wouldn't release cursor: ", currentChannel );
#endif
				focusProgramStart = -1;
			}
		}
	}

	// Move to channel
	else if ( mode == 2 ) {
		if ( 
			channelsScrollable() &&
			( focusChannelName.length() > 0 )
		) {
			focusChannelNo = channelNameToChannelNo( focusChannelName );

			// Put focus channel middle screen if was one
			// At this point, noVisibleChannels is not set

			showChannelNoAtPositionNo( focusChannelNo, MIN( noChannels, displayableChannels() ) / 2 );
		}
		else
			showChannelNoAtPositionNo( 0, 0 );
	}
	// Do cursor
	else {
		// Restore cursor if we had one and we can (maybe cursor program not visible
		// anymore...)
		if ( 
			( focusChannelNo != -1 ) &&
			( focusProgramStart != -1 ) &&
			( channels[focusChannelNo]->takeCursor( focusProgramStart, focusProgramEnd, false, true ) )
		) {
			//dmsg( "s/e ", (int)focusProgramStart, (int)focusProgramEnd );
			currentChannel = focusChannelNo;
		}
		else {
			currentChannel = -1;
			sBarP->hide();
		}
	}
}

void MV::reload( time_t sampStartTime, time_t windowStartOffset )
{
	static bool reloadingFlag = false;
	
	if ( 
		reloadingFlag ||
		fillingCacheFlag
	)
		return;

	reloadingFlag = true;

	if ( sampStartTime == -1 )
		sampStartTime = sampleStartSecond + inputs.pre;
	if ( windowStartOffset == -1 )
		windowStartOffset = ( windowStartSecond - sampleStartSecond );

	storeRestoreFocusDetails( 1 );

	hideVisibleChannels();

	load( sampStartTime, windowStartOffset );

	// Set first vis chan

	storeRestoreFocusDetails( 2 );

        rebuildAll();

	// Restore cursor

	storeRestoreFocusDetails( 3 );

	reloadingFlag = false;
}


int MV::eventHandleDayBar( int keyState, int key )
{
	if ( keyState == KEY_STATE_UP )
		return 0;

	int handled = 1;
	switch ( key ) {
		case VIEW_DAY_LEFT:
		case VIEW_PROG_LEFT:
			dayBarP->shiftCursorLabel( -1 );
			break;
		case VIEW_DAY_RIGHT:
		case VIEW_PROG_RIGHT:
			dayBarP->shiftCursorLabel( +1 );
			break;
		case VIEW_PICKER_UP:
		case VIEW_PICKER_DN:
		case VIEW_INC_CHANNEL:
		case VIEW_DEC_CHANNEL:
			dayBarP->doTimePickerFunc( pfuncShow );
			break;
		case VIEW_DAY_CENTRE:
		case VIEW_SELECT:
			reload( dayBarP->accept( ! dayBarFixed() ), conf.windowInitialStartOffsetSeconds );
			break;
		case VIEW_EXIT:
			dayBarP->reject( ! dayBarFixed() );
			refreshVisibleChannels();
			break;
		default:
			handled = 0;
			break;
	}

	return handled;
}

void MV::changeWindowSeconds( int posNeg )
{
        if ( posNeg == 1 ) {
                views[0].geom.widthSeconds += conf.widthSecondsChangeRate;
                if ( views[0].geom.widthSeconds > sampleWidthSeconds() )
                        views[0].geom.widthSeconds = sampleWidthSeconds();
        }
        else {
                views[0].geom.widthSeconds -= conf.widthSecondsChangeRate;
                if ( views[0].geom.widthSeconds < 3600 )
                        views[0].geom.widthSeconds = 3600;
        }

        rebuildTimeBars();
        hideVisibleChannels();
	
        rebuildChannels();
}

void MV::showInfoWhenHidden( void )
{
		int chanNo = getPlayingChannelNo();
		if ( chanNo != -1 ) {
			Program *pp = channels[chanNo]->getPlayingProgramPtr();
			char buffer[DC_MAX_DESCR_LENGTH+1];
			if ( 
				( pp == NULL ) ||
				( ! inputMgrP->getDescription( channels[chanNo], pp, buffer ) )
			)
				buffer[0] = '\0';

			doExtInfo( pp, buffer, colourOptionToColour( views[0].pres.filmBackColour ) );
		}
}

void MV::showInfo( void ) {
#ifndef MVLITE
	if ( currentChannel == -1 ) {
		eStreaminfo info( 0, getPlayingServiceRef() );
		hideShowAll(true, true);
		showExecHide( &info );
		hideShowAll(false, true);
		return;
	}
#endif
		char buffer[DC_MAX_DESCR_LENGTH+1];
		Program *pp = channels[currentChannel]->getCurrentProgramPtr();
		if ( 
			( pp == NULL ) ||
			( ! inputMgrP->getDescription( channels[currentChannel], pp, buffer ) )
		)
			buffer[0] = '\0';
 
		if ( 
			( strlen( buffer ) == 0 ) &&
			( 
				( ! haveNetwork() ) ||
				( pp == NULL ) ||
				( ! pp->isFilm() )
			)
		)
			channels[currentChannel]->flashCursor();
		else {
			hideShowAll( true, true );
			doExtInfo( pp, buffer, colourOptionToColour( views[0].pres.filmBackColour ) );
			hideShowAll( false, true );
		}
}

void MV::waitCanExit( void )
{
	inputMgrP->checkXMLTVNotConverting( true );
}

void MV::handleSelect( void )
{
	if ( currentChannel == -1 ) {
		initCursor();
	}
	else {
		if ( channels[currentChannel]->isPlaying() ) {
			if ( execFlag ) {
				waitCanExit();
				close(0);
			}
		}
		else { 

			if ( channels[currentChannel]->play() ) {
				invalidate();
			}
		}
	}
}

// 1-based
int MV::cursorColumnNo( void )
{
	return ( 
		( 
			( currentChannel - firstVisibleChannel ) / displayableChannelsPerColumn() 
		) 
		+ 1 
	); 
}

void MV::viewHandleLeftRight( int posNeg )
{
	if ( currentChannel == -1 ) {
		if ( views[0].feat.entriesPerColumn == 0 )
			shiftPrograms( posNeg, windowMiddleSecond(), windowMiddleSecond() );
	}
	else {
		int newCurrentChannel = -1;

		// We can only scroll proportional times, the end is
		// indeterminate

		if ( views[0].feat.entriesPerColumn > 0 ) {
			
			int inChannelCol = channels[currentChannel]->cursorColumn();
			if ( posNeg > 0 ) {
				// If we're in the mid of channel, shift
				if ( 
					( inChannelCol != -1 ) &&
					( inChannelCol < views[0].feat.entriesPerColumn )
				)
					channels[currentChannel]->shiftCursor( +1, sampleStartSecond, sampleEndSecond() );

				// Otherwise, we're at end of column.
				// If it's the not the last column, try jump to
				// next column

				else if ( cursorColumnNo() < views[0].feat.noColumns )
					newCurrentChannel = currentChannel + displayableChannelsPerColumn();
			}
			else {
				// If we're in the mid of channel, shift left
				if ( inChannelCol > 1 )
					channels[currentChannel]->shiftCursor( -1, sampleStartSecond, sampleEndSecond() );

				// Otherwise, we're at end of column.
				// If it's the last column shift, otherwise try jump
				// next column

				else if ( cursorColumnNo() > 1 )
					newCurrentChannel = currentChannel - displayableChannelsPerColumn();
			}
	
			// If we want jump, try jump. If couldn't shift
			if ( 
				( newCurrentChannel >= firstVisibleChannel ) &&	
				( newCurrentChannel <= lastVisibleChannel )
			)
				moveCursorToChannel( newCurrentChannel, +1, posNeg );
		}

		// If we can shift, see if we're off the screen and scroll if nec.
		// Don't scroll off sample window
		else if ( 
			channels[currentChannel]->shiftCursor( posNeg, sampleStartSecond, sampleEndSecond() ) &&
			( ( posNeg < 0 ) || ( ! atRightBorder() ) ) &&
			( ( posNeg > 0 ) || ( ! atLeftBorder() ) )
		) {
			time_t startTime, endTime;
			channels[currentChannel]->getCurrentProgramTimes( &startTime, &endTime );
			if ( 
				( startTime < windowStartSecond ) ||
				( endTime > windowEndSecond() )
			) 
				shiftPrograms( posNeg, startTime, endTime );
		}
	}
}

time_t MV::windowMiddleSecond( void )
{
	return windowStartSecond + (time_t) ( views[0].geom.widthSeconds / 2 );
}

time_t MV::windowEndSecond( void )
{
	return windowStartSecond + (time_t) views[0].geom.widthSeconds;
}

time_t MV::sampleEndSecond( void )
{
	return sampleStartSecond + sampleWidthSeconds();
}

void MV::shiftChannels( int posNeg )
{
	if ( channelsScrollable() ) {
		int toShift = 0;
		if ( views[0].feat.channelShiftType == FM_SHIFT_TYPE_SCROLL ) 
			toShift = 1;
		else if ( views[0].feat.channelShiftType == FM_SHIFT_TYPE_HALFPAGE ) {
			toShift = ( displayableChannels() / 2 );	
		}
		else
			toShift = displayableChannels();

		// 1 /2 = 0
		if ( toShift == 0 )
			toShift = 1;

		// Down
		if ( posNeg > 0 ) {
			if ( ( lastVisibleChannel + toShift ) >= noChannels )
				toShift = noChannels - lastVisibleChannel - 1;

			if ( toShift > 0 )
				firstVisibleChannel += toShift;
		}
		// Up
		else {
			if ( toShift > firstVisibleChannel )
				toShift = firstVisibleChannel;

			if ( toShift > 0 )
				firstVisibleChannel -= toShift;
		} 
 
		// Redraw
		if ( toShift > 0 ) {
			hideVisibleChannels();
			rebuildChannels();
		}
	} 
}

void MV::viewHandleUpDown( int posNeg )
{
	if ( currentChannel == -1 )
		shiftChannels( posNeg );
	else {
		int newChannel = currentChannel;

		// Cursor down
		if ( posNeg > 0 ) {
			if ( currentChannel < ( noChannels - 1 ) ) {
				if ( currentChannel == lastVisibleChannel ) {
					shiftChannels( posNeg );
					newChannel = lastVisibleChannel;
				}
				else
					newChannel = currentChannel + 1;
			}
		}
		// Cursor up
		else {
			if ( currentChannel > 0 ) {
				if ( currentChannel == firstVisibleChannel ) {
					shiftChannels( posNeg );
					newChannel = firstVisibleChannel;
				}
				else
					newChannel = currentChannel - 1;
			}
		}
		if ( newChannel != currentChannel )
			moveCursorToChannel( newChannel, posNeg );
	}
}

bool MV::channelsScrollable( void )
{
	return ( displayableChannels() < noChannels );
}

time_t MV::sampleWidthSeconds( void )
{
	return ( inputs.pre + inputs.post );
}

void MV::shiftPrograms( int posNeg, time_t startMustBeVisible, time_t endMustBeVisible )
{
	// Shift window to right

	time_t toShift = 0;
	time_t averageMustBeVisible = startMustBeVisible + ( ( endMustBeVisible - startMustBeVisible ) / 2 );

	if ( posNeg > 0 ) {
		if ( atRightBorder() ) { 
			if ( ! fillingCacheFlag ) {
				setIndicator( indRightOfSample, false );
				reload( sampleEndSecond(), 0 );
				setBorderIndicators();
			}
		}
		else {
			time_t sampleEnd = sampleEndSecond();
			time_t windowEnd = windowEndSecond();

			// Shift the difference it's out of the window
			if ( views[0].feat.programShiftType == FM_SHIFT_TYPE_SCROLL )
				if ( endMustBeVisible > windowEnd )
					toShift = endMustBeVisible - windowEnd;
				else
					toShift = conf.startTimeChangeRate;

			// Shift to get the spec. time in the middle
			else if ( views[0].feat.programShiftType == FM_SHIFT_TYPE_HALFPAGE ) {
				if ( windowEnd > averageMustBeVisible )
					toShift = windowEnd - averageMustBeVisible;
				else
					toShift = ( views[0].geom.widthSeconds  / 2 );	
			}
			// Page
			else {
				toShift = views[0].geom.widthSeconds;
			}

			// Don't go off the end of the world 
			if ( ( windowEnd + toShift ) >= sampleEnd ) {
				toShift = sampleEnd - windowEnd;
			}
			
			windowStartSecond += toShift;
		}
	}

	// Shift window to left
	else {
		if ( atLeftBorder() ) {
			if ( ! fillingCacheFlag ) {
				setIndicator( indLeftOfSample, false );
				reload( 
					( windowStartSecond - inputs.post  + inputs.pre ), 
					( sampleWidthSeconds() - views[0].geom.widthSeconds )
				);
				setBorderIndicators();
			}
		}
		else {
			if ( views[0].feat.programShiftType == FM_SHIFT_TYPE_SCROLL ) {
				if ( startMustBeVisible < windowStartSecond )
					toShift = windowStartSecond - startMustBeVisible;
				else
					toShift = conf.startTimeChangeRate;
			}
			// Shift to get the spec. time in the middle
			else if ( views[0].feat.programShiftType == FM_SHIFT_TYPE_HALFPAGE ) {
				if ( windowStartSecond < averageMustBeVisible )
					toShift = averageMustBeVisible - windowStartSecond;
				else 
					toShift = ( views[0].geom.widthSeconds  / 2 );	
			}
			else {
				toShift = views[0].geom.widthSeconds;
			}

			// Don't go off the end
			if ( ( windowStartSecond - toShift ) < sampleStartSecond ) 
				toShift = windowStartSecond - sampleStartSecond;

			windowStartSecond -= toShift;
		}
	}

	if ( toShift > 0 ) {
		rebuildTimeBars();
		hideVisibleChannels();
		rebuildChannels();
		setBorderIndicators();
	}
}

void MV::setBorderIndicators( void )
{
	setIndicator( indLeftOfSample, atLeftBorder() );
	setIndicator( indRightOfSample, atRightBorder() );
}

bool MV::atLeftBorder( void )
{
	return (
		( views[0].feat.entriesPerColumn == 0 ) &&
		( isGraphicalView( currentView ) ) &&
		( windowStartSecond == sampleStartSecond )
	);
}
	
bool MV::atRightBorder( void )
{
	return (
		( views[0].feat.entriesPerColumn == 0 ) &&
		( windowEndSecond() == sampleEndSecond() )
	);
}

void MV::setTimeLabelGeom( void )
{
	timeLabelP->hide();
	eSize timeLabelSize = timeLabelP->getSize();
	timeLabelP->move( 
		ePoint( views[0].geom.timeLabelX, views[0].geom.timeLabelY )
	);
	timeLabelP->show();
	
	int xpos = views[0].geom.timeLabelX  + timeLabelSize.width() + 3;

	storeRestoreIndicators( true );
	for ( int indNo = 0; indNo < indNoIndicators; indNo++ ) {
			indicators[indNo]->move( ePoint( xpos, views[0].geom.timeLabelY ) );
		xpos += INDICATORS_WIDTH + 3;
	}
	storeRestoreIndicators( false );
}

void MV::setStatusBarGeom( void )
{
	setWidgetGeom( sBarP, 
		views[0].geom.statusBarX, views[0].geom.statusBarY,
		views[0].geom.statusBarWidthPixels, views[0].geom.statusBarHeightPixels
	);
}

void MV::changeToView( int viewNo )
{
	if ( viewNo != currentView ) {
		if ( timebarsEnabled( 0 ) && ! timebarsEnabled( viewNo )  )
			showHideTimebars(false);

		if ( 
			dayBarFixed() && 
			( views[viewNo].feat.dayBarMode != dayBarModeFixed )
		)
			dayBarP->hide();

		hideVisibleChannels();
		hideShowAll(true);
	        currentView = viewNo;
       	 	copyView( viewNo, 0 );

		storeRestoreFocusDetails( 1 );

		selectChannels();
		setMainWindowGeom();

		storeRestoreFocusDetails( 2 );

		rebuildAll();

		show();
		timeLabelP->show();

		storeRestoreFocusDetails( 3 );

		setBorderIndicators();
	}

}

void MV::refreshVisibleChannels( void )
{
	if ( noVisibleChannels > 0 ) {
		for ( int cn = firstVisibleChannel; cn <= lastVisibleChannel; cn++ ) {
			if ( channels[cn]->isVisible() ) {
				channels[cn]->setRedrawNoClear();
				channels[cn]->invalidate();
			}
		}
	}
}

void MV::hideVisibleChannels( void )
{
	for ( int cn = 0; cn <noChannels; cn++ )
		channels[cn]->hide();
}

int MV::displayableChannels( void )
{
	return ( displayableChannelsPerColumn() * views[0].feat.noColumns );
}

int MV::displayableChannelsPerColumn( void )
{
	int base = clientrect.height();
//	if ( views[0].feat.showStatusBar )
//		base -= views[0].geom.statusBarHeightPixels;
	if ( timebarsEnabled( 0 ) )
		base -= conf.timeBarHeight;
	if ( dayBarFixed() )
		base -= conf.dayBarHeight;

	return ( ( base / rowHeight() ) );
}

void MV::setupDataDirs( void )
{
	/* "/var/mnt/usb/mv",
		/media/usb
	 "/var/mnt/cf/mv",
		/media/cf
        "/media/hdd/mv",
        "/var/tuxbox/config/mv",
        "/var/tmp/mv" */

	inputs.storageDir = -1;

	// This is messy, but I'm bored with this issue :-)
	// TODO (dAF2000): Ugly coded, remove /var/mnt, see also util.cpp

	eString mountDirs[4] = {
		"/var/mnt/usb",
		"/media/usb",
		"/var/mnt/cf",
		"/media/cf"
	};

	// If USB/CF: both that

	for ( int dirNo = 0; dirNo < 4; dirNo++ ) {
		if ( dirIsMountPoint( mountDirs[dirNo].c_str() ) ) {
			inputs.storageDir = dirNo;
			inputs.detailsDir = dirNo;
			break;
		}
	}

	if ( inputs.storageDir == -1 ) {

		// Otherwise, detail cache in /tmp (don't like
		// disk whirring) and storage in /media/hdd for preference

		inputs.detailsDir = 6;

		inputs.storageDir = 5;
	}
}

void MV::setConfigDefaults( void )
{
	setupDataDirs();

	// Input Dirs
        inputs.flags = DEFAULT_INPUT_FLAGS;
        inputs.maxChannelsPerInput = DEFAULT_INPUTS_MAX_CHANNELS;

	inputs.pre = DEFAULT_INPUTS_PRE;
	inputs.post = DEFAULT_INPUTS_POST;
	inputs.autoDataDirsFlag = DEFAULT_INPUTS_AUTO_DATA_DIRS_FLAG;
	inputs.autoReloadFlag = DEFAULT_INPUTS_AUTO_RELOAD_FLAG;

        for ( int dno = 0; dno < MAX_NO_INPUTS; dno++ ) {
                strcpy( inputs.confs[dno].name, ENIGMA_CACHE_NAME );
                inputs.confs[dno].enabledFlag = 0;
        }

#if HAVECACHE == 1
        inputs.confs[0].enabledFlag = 1;
#endif

	// Views

        views[1] = (struct ViewConf) {
                {
                        DEFAULT_CONF_1_GEOM_WIDTH_SECONDS,
                        DEFAULT_CONF_1_GEOM_TOP_LEFT_X,
                        DEFAULT_CONF_1_GEOM_TOP_LEFT_Y,
                        DEFAULT_CONF_1_GEOM_WIDTH_PIXELS,
                        DEFAULT_CONF_1_GEOM_HEIGHT_PIXELS,
                        DEFAULT_CONF_1_GEOM_HEADER_WIDTH_PIXELS,
                        DEFAULT_CONF_1_GEOM_CHANNEL_HEIGHT_PIXELS,
                        DEFAULT_CONF_1_GEOM_CHANNEL_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_1_GEOM_STATUS_BAR_X,
                        DEFAULT_CONF_1_GEOM_STATUS_BAR_Y,
                        DEFAULT_CONF_1_GEOM_STATUS_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_1_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                        DEFAULT_CONF_1_GEOM_TIME_LABEL_X,
                        DEFAULT_CONF_1_GEOM_TIME_LABEL_Y
                },
                {
                        DEFAULT_CONF_1_FEAT_SHOW_TITLE_BAR,
                        DEFAULT_CONF_1_FEAT_SHOW_STATUS_BAR,
                        DEFAULT_CONF_1_FEAT_NO_COLUMNS,
                        DEFAULT_CONF_1_FEAT_TIMEBAR_PERIOD_SECONDS,
                        DEFAULT_CONF_1_FEAT_ENTRIES_PER_COLUMN,
                        DEFAULT_CONF_1_FEAT_CHANNEL_SHIFT_TYPE,
                        DEFAULT_CONF_1_FEAT_PROGRAM_SHIFT_TYPE,
                        DEFAULT_CONF_1_FEAT_HORIZONTAL_SEP,
                        DEFAULT_CONF_1_FEAT_VERTICAL_SEP,
			DEFAULT_CONF_1_FEAT_HEADER_FLAGS,
                        DEFAULT_CONF_1_FEAT_STATUS_BAR_FLAGS,
                        DEFAULT_CONF_1_FEAT_PROGRAM_BOX_FLAGS,
        		DEFAULT_CONF_1_FEAT_NEXT_VIEW,
        		DEFAULT_CONF_1_FEAT_FIRST_CHANNEL,
        		DEFAULT_CONF_1_FEAT_SHOW_ELAPSED_BARS,
        		DEFAULT_CONF_1_FEAT_SHOW_CHANNEL_BAR,
        		DEFAULT_CONF_1_FEAT_DAY_BAR_MODE,
        		DEFAULT_CONF_1_FEAT_SHOW_EMPTY_CHANNELS,
        		DEFAULT_CONF_1_FEAT_FORCE_CURSOR,
			DEFAULT_CONF_1_FEAT_SORT
                },
                {
                        DEFAULT_CONF_1_PRES_BACK_COLOUR,
                        DEFAULT_CONF_1_PRES_PROGRAM_ONE_BACK_COLOUR,
                        DEFAULT_CONF_1_PRES_PROGRAM_TWO_BACK_COLOUR,
                        DEFAULT_CONF_1_PRES_FILM_BACK_COLOUR,
                        DEFAULT_CONF_1_PRES_PROGRAM_TIME_FONT_SIZE,
                        DEFAULT_CONF_1_PRES_PROGRAM_TIME_FONT_FAMILY,
                        DEFAULT_CONF_1_PRES_PROGRAM_TITLE_FONT_SIZE,
                        DEFAULT_CONF_1_PRES_PROGRAM_TITLE_FONT_FAMILY,
                        DEFAULT_CONF_1_PRES_PROGRAM_DESCR_FONT_SIZE,
                        DEFAULT_CONF_1_PRES_PROGRAM_DESCR_FONT_FAMILY,
                        DEFAULT_CONF_1_PRES_PROGRAM_CHANNEL_FONT_SIZE,
                        DEFAULT_CONF_1_PRES_PROGRAM_CHANNEL_FONT_FAMILY,
                        DEFAULT_CONF_1_PRES_CHANNEL_HEADER_BACK_COLOUR,
                        DEFAULT_CONF_1_PRES_CHANNEL_HEADER_FONT_SIZE,
                        DEFAULT_CONF_1_PRES_CHANNEL_HEADER_FONT_FAMILY,
                        DEFAULT_CONF_1_PRES_PLAYING_BACK_COLOUR,
                        DEFAULT_CONF_1_PRES_FAV_BACK_COLOUR
                }
        };

        views[2] = (struct ViewConf) {
                {
                        DEFAULT_CONF_2_GEOM_WIDTH_SECONDS,
                        DEFAULT_CONF_2_GEOM_TOP_LEFT_X,
                        DEFAULT_CONF_2_GEOM_TOP_LEFT_Y,
                        DEFAULT_CONF_2_GEOM_WIDTH_PIXELS,
                        DEFAULT_CONF_2_GEOM_HEIGHT_PIXELS,
                        DEFAULT_CONF_2_GEOM_HEADER_WIDTH_PIXELS,
                        DEFAULT_CONF_2_GEOM_CHANNEL_HEIGHT_PIXELS,
                        DEFAULT_CONF_2_GEOM_CHANNEL_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_2_GEOM_STATUS_BAR_X,
                        DEFAULT_CONF_2_GEOM_STATUS_BAR_Y,
                        DEFAULT_CONF_2_GEOM_STATUS_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_2_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                        DEFAULT_CONF_2_GEOM_TIME_LABEL_X,
                        DEFAULT_CONF_2_GEOM_TIME_LABEL_Y
                },
                {
                        DEFAULT_CONF_2_FEAT_SHOW_TITLE_BAR,
                        DEFAULT_CONF_2_FEAT_SHOW_STATUS_BAR,
                        DEFAULT_CONF_2_FEAT_NO_COLUMNS,
                        DEFAULT_CONF_2_FEAT_TIMEBAR_PERIOD_SECONDS,
                        DEFAULT_CONF_2_FEAT_ENTRIES_PER_COLUMN,
                        DEFAULT_CONF_2_FEAT_CHANNEL_SHIFT_TYPE,
                        DEFAULT_CONF_2_FEAT_PROGRAM_SHIFT_TYPE,
                        DEFAULT_CONF_2_FEAT_HORIZONTAL_SEP,
			DEFAULT_CONF_2_FEAT_VERTICAL_SEP,
			DEFAULT_CONF_2_FEAT_HEADER_FLAGS,
                        DEFAULT_CONF_2_FEAT_STATUS_BAR_FLAGS,
                        DEFAULT_CONF_2_FEAT_PROGRAM_BOX_FLAGS,
                        DEFAULT_CONF_2_FEAT_NEXT_VIEW,
        		DEFAULT_CONF_2_FEAT_FIRST_CHANNEL,
        		DEFAULT_CONF_2_FEAT_SHOW_ELAPSED_BARS,
        		DEFAULT_CONF_2_FEAT_SHOW_CHANNEL_BAR,
        		DEFAULT_CONF_2_FEAT_DAY_BAR_MODE,
        		DEFAULT_CONF_2_FEAT_SHOW_EMPTY_CHANNELS,
        		DEFAULT_CONF_2_FEAT_FORCE_CURSOR,
			DEFAULT_CONF_2_FEAT_SORT
                },
                {
                        DEFAULT_CONF_2_PRES_BACK_COLOUR,
                        DEFAULT_CONF_2_PRES_PROGRAM_ONE_BACK_COLOUR,
                        DEFAULT_CONF_2_PRES_PROGRAM_TWO_BACK_COLOUR,
                        DEFAULT_CONF_2_PRES_FILM_BACK_COLOUR,
                        DEFAULT_CONF_2_PRES_PROGRAM_TIME_FONT_SIZE,
                        DEFAULT_CONF_2_PRES_PROGRAM_TIME_FONT_FAMILY,
                        DEFAULT_CONF_2_PRES_PROGRAM_TITLE_FONT_SIZE,
                        DEFAULT_CONF_2_PRES_PROGRAM_TITLE_FONT_FAMILY,
                        DEFAULT_CONF_2_PRES_PROGRAM_DESCR_FONT_SIZE,
                        DEFAULT_CONF_2_PRES_PROGRAM_DESCR_FONT_FAMILY,
                        DEFAULT_CONF_2_PRES_PROGRAM_CHANNEL_FONT_SIZE,
                        DEFAULT_CONF_2_PRES_PROGRAM_CHANNEL_FONT_FAMILY,
                        DEFAULT_CONF_2_PRES_CHANNEL_HEADER_BACK_COLOUR,
                        DEFAULT_CONF_2_PRES_CHANNEL_HEADER_FONT_SIZE,
                        DEFAULT_CONF_2_PRES_CHANNEL_HEADER_FONT_FAMILY,
                        DEFAULT_CONF_2_PRES_PLAYING_BACK_COLOUR,
                        DEFAULT_CONF_2_PRES_FAV_BACK_COLOUR
                }
        };
        views[3] = (struct ViewConf) {
                {
                        DEFAULT_CONF_3_GEOM_WIDTH_SECONDS,
                        DEFAULT_CONF_3_GEOM_TOP_LEFT_X,
                        DEFAULT_CONF_3_GEOM_TOP_LEFT_Y,
                        DEFAULT_CONF_3_GEOM_WIDTH_PIXELS,
                        DEFAULT_CONF_3_GEOM_HEIGHT_PIXELS,
                        DEFAULT_CONF_3_GEOM_HEADER_WIDTH_PIXELS,
                        DEFAULT_CONF_3_GEOM_CHANNEL_HEIGHT_PIXELS,
                        DEFAULT_CONF_3_GEOM_CHANNEL_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_3_GEOM_STATUS_BAR_X,
                        DEFAULT_CONF_3_GEOM_STATUS_BAR_Y,
                        DEFAULT_CONF_3_GEOM_STATUS_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_3_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                        DEFAULT_CONF_3_GEOM_TIME_LABEL_X,
                        DEFAULT_CONF_3_GEOM_TIME_LABEL_Y
                },
                {
                        DEFAULT_CONF_3_FEAT_SHOW_TITLE_BAR,
                        DEFAULT_CONF_3_FEAT_SHOW_STATUS_BAR,
                        DEFAULT_CONF_3_FEAT_NO_COLUMNS,
                        DEFAULT_CONF_3_FEAT_TIMEBAR_PERIOD_SECONDS,
                        DEFAULT_CONF_3_FEAT_ENTRIES_PER_COLUMN,
                        DEFAULT_CONF_3_FEAT_CHANNEL_SHIFT_TYPE,
			DEFAULT_CONF_3_FEAT_PROGRAM_SHIFT_TYPE,
                        DEFAULT_CONF_3_FEAT_HORIZONTAL_SEP,
                        DEFAULT_CONF_3_FEAT_VERTICAL_SEP,
			DEFAULT_CONF_3_FEAT_HEADER_FLAGS,
                        DEFAULT_CONF_3_FEAT_STATUS_BAR_FLAGS,
                        DEFAULT_CONF_3_FEAT_PROGRAM_BOX_FLAGS,
                        DEFAULT_CONF_3_FEAT_NEXT_VIEW,
                        DEFAULT_CONF_3_FEAT_FIRST_CHANNEL,
        		DEFAULT_CONF_3_FEAT_SHOW_ELAPSED_BARS,
        		DEFAULT_CONF_3_FEAT_SHOW_CHANNEL_BAR,
        		DEFAULT_CONF_3_FEAT_DAY_BAR_MODE,
        		DEFAULT_CONF_3_FEAT_SHOW_EMPTY_CHANNELS,
        		DEFAULT_CONF_3_FEAT_FORCE_CURSOR,
			DEFAULT_CONF_3_FEAT_SORT
                },
                {
                        DEFAULT_CONF_3_PRES_BACK_COLOUR,
                        DEFAULT_CONF_3_PRES_PROGRAM_ONE_BACK_COLOUR,
                        DEFAULT_CONF_3_PRES_PROGRAM_TWO_BACK_COLOUR,
                        DEFAULT_CONF_3_PRES_FILM_BACK_COLOUR,
                        DEFAULT_CONF_3_PRES_PROGRAM_TIME_FONT_SIZE,
                        DEFAULT_CONF_3_PRES_PROGRAM_TIME_FONT_FAMILY,
                        DEFAULT_CONF_3_PRES_PROGRAM_TITLE_FONT_SIZE,
                        DEFAULT_CONF_3_PRES_PROGRAM_TITLE_FONT_FAMILY,
                        DEFAULT_CONF_3_PRES_PROGRAM_DESCR_FONT_SIZE,
                        DEFAULT_CONF_3_PRES_PROGRAM_DESCR_FONT_FAMILY,
                        DEFAULT_CONF_3_PRES_PROGRAM_CHANNEL_FONT_SIZE,
                        DEFAULT_CONF_3_PRES_PROGRAM_CHANNEL_FONT_FAMILY,
                        DEFAULT_CONF_3_PRES_CHANNEL_HEADER_BACK_COLOUR,
                        DEFAULT_CONF_3_PRES_CHANNEL_HEADER_FONT_SIZE,
                        DEFAULT_CONF_3_PRES_CHANNEL_HEADER_FONT_FAMILY,
                        DEFAULT_CONF_3_PRES_PLAYING_BACK_COLOUR,
                        DEFAULT_CONF_3_PRES_FAV_BACK_COLOUR
                }
        };

        views[4] = (struct ViewConf) {
                {
                        DEFAULT_CONF_4_GEOM_WIDTH_SECONDS,
                        DEFAULT_CONF_4_GEOM_TOP_LEFT_X,
                        DEFAULT_CONF_4_GEOM_TOP_LEFT_Y,
                        DEFAULT_CONF_4_GEOM_WIDTH_PIXELS,
                        DEFAULT_CONF_4_GEOM_HEIGHT_PIXELS,
                        DEFAULT_CONF_4_GEOM_HEADER_WIDTH_PIXELS,
                        DEFAULT_CONF_4_GEOM_CHANNEL_HEIGHT_PIXELS,
                        DEFAULT_CONF_4_GEOM_CHANNEL_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_4_GEOM_STATUS_BAR_X,
                        DEFAULT_CONF_4_GEOM_STATUS_BAR_Y,
                        DEFAULT_CONF_4_GEOM_STATUS_BAR_WIDTH_PIXELS,
                        DEFAULT_CONF_4_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                        DEFAULT_CONF_4_GEOM_TIME_LABEL_X,
                        DEFAULT_CONF_4_GEOM_TIME_LABEL_Y
                },
                {
                        DEFAULT_CONF_4_FEAT_SHOW_TITLE_BAR,
                        DEFAULT_CONF_4_FEAT_SHOW_STATUS_BAR,
                        DEFAULT_CONF_4_FEAT_NO_COLUMNS,
                        DEFAULT_CONF_4_FEAT_TIMEBAR_PERIOD_SECONDS,
                        DEFAULT_CONF_4_FEAT_ENTRIES_PER_COLUMN,
                        DEFAULT_CONF_4_FEAT_CHANNEL_SHIFT_TYPE,
			DEFAULT_CONF_4_FEAT_PROGRAM_SHIFT_TYPE,
                        DEFAULT_CONF_4_FEAT_HORIZONTAL_SEP,
                        DEFAULT_CONF_4_FEAT_VERTICAL_SEP,
			DEFAULT_CONF_4_FEAT_HEADER_FLAGS,
                        DEFAULT_CONF_4_FEAT_STATUS_BAR_FLAGS,
                        DEFAULT_CONF_4_FEAT_PROGRAM_BOX_FLAGS,
                        DEFAULT_CONF_4_FEAT_NEXT_VIEW,
                        DEFAULT_CONF_4_FEAT_FIRST_CHANNEL,
        		DEFAULT_CONF_4_FEAT_SHOW_ELAPSED_BARS,
        		DEFAULT_CONF_4_FEAT_SHOW_CHANNEL_BAR,
        		DEFAULT_CONF_4_FEAT_DAY_BAR_MODE,
        		DEFAULT_CONF_4_FEAT_SHOW_EMPTY_CHANNELS,
        		DEFAULT_CONF_4_FEAT_FORCE_CURSOR,
			DEFAULT_CONF_4_FEAT_SORT
                },
                {
                        DEFAULT_CONF_4_PRES_BACK_COLOUR,
                        DEFAULT_CONF_4_PRES_PROGRAM_ONE_BACK_COLOUR,
                        DEFAULT_CONF_4_PRES_PROGRAM_TWO_BACK_COLOUR,
                        DEFAULT_CONF_4_PRES_FILM_BACK_COLOUR,
                        DEFAULT_CONF_4_PRES_PROGRAM_TIME_FONT_SIZE,
                        DEFAULT_CONF_4_PRES_PROGRAM_TIME_FONT_FAMILY,
                        DEFAULT_CONF_4_PRES_PROGRAM_TITLE_FONT_SIZE,
                        DEFAULT_CONF_4_PRES_PROGRAM_TITLE_FONT_FAMILY,
                        DEFAULT_CONF_4_PRES_PROGRAM_DESCR_FONT_SIZE,
                        DEFAULT_CONF_4_PRES_PROGRAM_DESCR_FONT_FAMILY,
                        DEFAULT_CONF_4_PRES_PROGRAM_CHANNEL_FONT_SIZE,
                        DEFAULT_CONF_4_PRES_PROGRAM_CHANNEL_FONT_FAMILY,
                        DEFAULT_CONF_4_PRES_CHANNEL_HEADER_BACK_COLOUR,
                        DEFAULT_CONF_4_PRES_CHANNEL_HEADER_FONT_SIZE,
                        DEFAULT_CONF_4_PRES_CHANNEL_HEADER_FONT_FAMILY,
                        DEFAULT_CONF_4_PRES_PLAYING_BACK_COLOUR,
                        DEFAULT_CONF_4_PRES_FAV_BACK_COLOUR
                }
	};

	// 5 is modified copy of 4

        views[5] = views[4];
	views[5].feat.nextView = DEFAULT_CONF_5_FEAT_NEXT_VIEW;
	views[5].feat.channelHeaderFlags = DEFAULT_CONF_5_FEAT_HEADER_FLAGS;
	views[5].feat.programBoxFlags = DEFAULT_CONF_5_FEAT_PROGRAM_BOX_FLAGS;
	views[5].geom.widthSeconds = DEFAULT_CONF_5_GEOM_WIDTH_SECONDS;
	views[5].geom.channelHeightPixels = DEFAULT_CONF_5_GEOM_CHANNEL_HEIGHT_PIXELS;
	views[5].geom.headerWidthPixels = DEFAULT_CONF_5_HEADER_WIDTH_PIXELS;
	views[5].pres.programTitleFontSize = DEFAULT_CONF_5_PRES_PROGRAM_TITLE_FONT_SIZE;

	// Basic conf for list views, most entries aren't used

	for ( int viewNo = FIRST_LIST_VIEW; viewNo < NO_VIEWS; viewNo++ ) {
		views[viewNo] = views[1];
		views[viewNo].feat.nextView = viewNo;
		views[viewNo].feat.statusBarFlags = DEFAULT_CONF_LIST_VIEW_FEAT_STATUS_BAR_FLAGS;
		views[viewNo].feat.showStatusBar = true;
		views[viewNo].feat.entriesPerColumn = 0;
		views[viewNo].feat.channelHeaderFlags = DEFAULT_CONF_LIST_VIEW_FEAT_CHANNEL_HEADERS_FLAGS;

		views[viewNo].pres.programTitleFontFamily = DEFAULT_CONF_LIST_VIEW_PRES_PROGRAM_FONT_FAMILY;
		views[viewNo].pres.programTitleFontSize = DEFAULT_CONF_LIST_VIEW_PRES_PROGRAM_FONT_SIZE;

		views[viewNo].pres.channelHeaderFontFamily = DEFAULT_CONF_LIST_VIEW_PRES_CHANNEL_FONT_FAMILY;
		views[viewNo].pres.channelHeaderFontSize = DEFAULT_CONF_LIST_VIEW_PRES_CHANNEL_FONT_SIZE;
	}


	// Geom is the main part we're interested in

	views[6].feat.channelHeaderFlags = DEFAULT_CONF_6_FEAT_CHANNEL_HEADER_FLAGS;

       	views[6].geom = (struct ViewGeometry) {
		0,
		DEFAULT_CONF_6_GEOM_TOP_LEFT_X,
		DEFAULT_CONF_6_GEOM_TOP_LEFT_Y,
		DEFAULT_CONF_6_GEOM_WIDTH_PIXELS,
		DEFAULT_CONF_6_GEOM_HEIGHT_PIXELS,
		0, 0, 0,
		DEFAULT_CONF_6_GEOM_STATUS_BAR_X,
		DEFAULT_CONF_6_GEOM_STATUS_BAR_Y,
		DEFAULT_CONF_6_GEOM_STATUS_BAR_WIDTH_PIXELS,
		DEFAULT_CONF_6_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                DEFAULT_CONF_6_GEOM_TIME_LABEL_X,
                DEFAULT_CONF_6_GEOM_TIME_LABEL_Y
	};
	views[6].feat.sortType = DEFAULT_CONF_6_FEAT_SORT;
	views[6].feat.entriesPerColumn = DEFAULT_CONF_6_FEAT_ENTRIES_PER_COLUMN;

       	views[7].geom = (struct ViewGeometry) {
		0,
		DEFAULT_CONF_7_GEOM_TOP_LEFT_X,
		DEFAULT_CONF_7_GEOM_TOP_LEFT_Y,
		DEFAULT_CONF_7_GEOM_WIDTH_PIXELS,
		DEFAULT_CONF_7_GEOM_HEIGHT_PIXELS,
		0, 0, 0,
		DEFAULT_CONF_7_GEOM_STATUS_BAR_X,
		DEFAULT_CONF_7_GEOM_STATUS_BAR_Y,
		DEFAULT_CONF_7_GEOM_STATUS_BAR_WIDTH_PIXELS,
		DEFAULT_CONF_7_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                DEFAULT_CONF_7_GEOM_TIME_LABEL_X,
                DEFAULT_CONF_7_GEOM_TIME_LABEL_Y
	};
	views[7].feat.showStatusBar = false;
	views[7].feat.sortType = DEFAULT_CONF_7_FEAT_SORT;
	views[7].feat.entriesPerColumn = DEFAULT_CONF_7_FEAT_ENTRIES_PER_COLUMN;

       	views[8].geom = (struct ViewGeometry) {
		0,
		DEFAULT_CONF_8_GEOM_TOP_LEFT_X,
		DEFAULT_CONF_8_GEOM_TOP_LEFT_Y,
		DEFAULT_CONF_8_GEOM_WIDTH_PIXELS,
		DEFAULT_CONF_8_GEOM_HEIGHT_PIXELS,
		0, 0, 0,
		DEFAULT_CONF_8_GEOM_STATUS_BAR_X,
		DEFAULT_CONF_8_GEOM_STATUS_BAR_Y,
		DEFAULT_CONF_8_GEOM_STATUS_BAR_WIDTH_PIXELS,
		DEFAULT_CONF_8_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                DEFAULT_CONF_8_GEOM_TIME_LABEL_X,
                DEFAULT_CONF_8_GEOM_TIME_LABEL_Y
	};
	views[8].feat.sortType = DEFAULT_CONF_8_FEAT_SORT;
	views[8].feat.entriesPerColumn = DEFAULT_CONF_8_FEAT_ENTRIES_PER_COLUMN;

       	views[9].geom = (struct ViewGeometry) {
		0,
		DEFAULT_CONF_9_GEOM_TOP_LEFT_X,
		DEFAULT_CONF_9_GEOM_TOP_LEFT_Y,
		DEFAULT_CONF_9_GEOM_WIDTH_PIXELS,
		DEFAULT_CONF_9_GEOM_HEIGHT_PIXELS,
		0, 0, 0,
		DEFAULT_CONF_9_GEOM_STATUS_BAR_X,
		DEFAULT_CONF_9_GEOM_STATUS_BAR_Y,
		DEFAULT_CONF_9_GEOM_STATUS_BAR_WIDTH_PIXELS,
		DEFAULT_CONF_9_GEOM_STATUS_BAR_HEIGHT_PIXELS,
                DEFAULT_CONF_9_GEOM_TIME_LABEL_X,
                DEFAULT_CONF_9_GEOM_TIME_LABEL_Y
	};
	views[9].feat.showStatusBar = false;
	views[9].feat.sortType = DEFAULT_CONF_9_FEAT_SORT;
	views[9].feat.entriesPerColumn = DEFAULT_CONF_9_FEAT_ENTRIES_PER_COLUMN;

        // Current state
        views[0] = views[1];

        conf = (struct Conf) {
                DEFAULT_WIDTH_SECONDS_CHANGE_RATE,
                DEFAULT_START_TIME_CHANGE_RATE,
                DEFAULT_TIME_BAR_HEIGHT,
		"",
                0, 0,
                DEFAULT_DAY_BAR_HEIGHT,
		DEFAULT_TIME_PICKER_HEIGHT,
		0,
		DEFAULT_MAX_WAIT_FOR_EPG_SECONDS,
		DEFAULT_WINDOW_INITIAL_START_OFFSET_SECONDS,
		DEFAULT_ASPECT_RATIO_MODE,
        };
        safeStrncpy( conf.versionString, getStr( strVersion ), MAX_VERSION_STRING_LENGTH  );
}


void MV::writeConfig( void )
{
        FILE *fp = fopen( formConfigPath( CONFIG_FILENAME ).c_str(), "wb" );
        if ( !fp ) {
                dmsg( getStr( strErrWriteOpenConf ) );
        }
        else {
		// We'll check this whenever config is read
		// to prevent a corrupted config causing repeat
		// crashes on startup
		// Request from Alex Shtol
		conf.checksum = calcConfigChecksum();
		
                size_t written =  fwrite( &inputs, sizeof( struct Inputs ), 1, fp );
		written += fwrite( &conf, sizeof( struct Conf ), 1, fp );
		written += fwrite( views, sizeof( struct ViewConf ), NO_VIEWS, fp );
                fclose( fp );
                if ( written != ( 2 + NO_VIEWS ) )
                        dmsg( getStr( strErrWritingConf ) );
        }
}

// Checksum is just a sum of the bytes, should
// be sufficient for most cases

unsigned int MV::calcConfigChecksum( void )
{
	// Don't allow checksum itself to affect the result
	// of the calculation
	conf.checksum = 0;

	unsigned int csum = 0;

	// We want to calc checksum over all of conf, view
	// confs and inputs
	char * ptrs[3] = {
		(char *) &conf, (char *) views, (char *) &inputs
	};
	unsigned int sizes[3] = {
		sizeof( struct Conf ),
		sizeof( struct ViewConf ) * NO_VIEWS,
		sizeof( struct Inputs )
	};

	for ( unsigned int ptrNo= 0; ptrNo < 3; ptrNo++ ) {
		char *d = ptrs[ptrNo];
		unsigned int structSize = sizes[ptrNo];
		
		for ( unsigned int i = 0; i < structSize; i++ ) {
			csum += (unsigned int) (*((unsigned char *) d));
			d++;
		}
	}

	return csum;
}

int MV::readConfig( void )
{
	eString configPath = formConfigPath( CONFIG_FILENAME );
        FILE *fp = fopen( configPath.c_str(), "rb" );
        if ( ! fp ) {
                return configErrorNotFound;
        }
        else {
                size_t read = fread( &inputs, sizeof( struct Inputs ), 1, fp );
                read += fread( &conf, sizeof( struct Conf ), 1, fp );
                read += fread( views, sizeof( struct ViewConf ), NO_VIEWS, fp );
                fclose( fp );

                if ( read != ( 2 + NO_VIEWS ) )
                        return configErrorBadFile;

		unsigned int tempChecksum = conf.checksum;
		if ( tempChecksum != calcConfigChecksum() )	
			return configErrorBadFile;
        }

        return configErrorNone;
}

void MV::INT_EPGAvailable( bool nothingToDoFlag )
{
	if ( fillingCacheFlag ) {
		// This comes when have init data, and no
		// extended EPG to get, or the extended is finished

		if ( nothingToDoFlag ) { 
			timerP->stop();
			fillCache( false );
		}
	}
	else if ( 
		nothingToDoFlag &&
		( ! wasFillingCacheFlag ) &&
		( Channel::secondsAgoLastPlay() >= CACHE_IND_MIN_SECONDS_ACTIVATE )
	) {
		setIndicator( indFillCache, true );
		if ( reloadIfDesiredAndPossible() )
			setIndicator( indFillCache, false );
	}
}

void MV::INT_timerTimeout( void )
{
	if ( fillingCacheFlag )
		fillCache( false );
}

int MV::allChannelNameToChannelNo( const eString &channelName )
{
	for ( int channelNo = 0; channelNo < noAllChannels; channelNo++ ) {
		if ( allChannels[channelNo]->getName() == channelName )
			return channelNo;
	}

	return  -1;
}

int MV::channelNameToChannelNo( const eString &channelName )
{
	for ( int channelNo = 0; channelNo < noChannels; channelNo++ ) {
		if ( channels[channelNo]->getName() == channelName )
			return channelNo;
	}

	return  -1;
}

bool MV::needScanService( eServiceReferenceDVB ref )
{
	eString sName = getServiceName( ref ) ;

	if ( 
		( fillCacheType != fillCacheSpecifiedChannel ) &&
		( 
			( sName == eString( "VH1" ) ) ||
			( sName == eString( "VH1 Classic" ) ) ||
			( sName == eString( "MTV HITS" ) )
		)
	)
		return false;

	int channelNo = channelNameToChannelNo( sName );

	if ( fillCacheType == fillCacheSpecifiedChannel ) 
		return ( channelNo == fillCacheChannelNo );
	else
		return ( 
			( channelNo == -1 ) ||
			( channels[channelNo]->getNoPrograms() < 2 )
		);
}

void MV::fillCache( bool startupFlag )
{
	static std::set<int> orbs;
	static std::set<int>::iterator orbIt;

	static std::map<eTransportStreamID,eServiceReferenceDVB> idToRef;
	static std::map<eTransportStreamID,eServiceReferenceDVB>::iterator idToRefIt;

	// If we're starting, init

	if ( startupFlag ) {

		setIndicator( indFillCache, true );

		// We only now what programs we've got if we're at the current time

		reload();

		orbs.clear();
		idToRef.clear();

		int playingSatellitePos = getServiceOrbitalPos( getPlayingServiceRef() );

		// Get sat positions we need to scan

		eServiceReferenceDVB *srefP = inputMgrP->nextBouquetRef( true );
		while ( srefP != NULL ) {
			if ( needScanService( *srefP ) ) {
				int pos = getServiceOrbitalPos( *srefP );

				if ( 
					( 
						( fillCacheType != fillCacheCurrentPos ) ||
						( pos == playingSatellitePos )
					)
					&&
					// Don't add the position if it's already recorded
					( orbs.find( pos ) == orbs.end() )
				) {
					orbs.insert( pos );
				}
			}

			srefP = inputMgrP->nextBouquetRef( false );
		}

		idToRefIt = idToRef.end();
	}
	
	// If we're at the end of a sat's TSID's,
	// switch to next sat, otherwise just next TSID

	if ( idToRefIt == idToRef.end() ) {

		// If we're just starting, take the first sat,
		// otherwise next sat

		if ( startupFlag )
			orbIt = orbs.begin();
		else 
			orbIt++;
			
		if ( orbIt == orbs.end() ) {
			handleCacheFill( 0 );
		}
		else {
			idToRef.clear();

			// Init idToRef for this sat
			// Pick out the IDs we need to cover for this satellite

			eServiceReferenceDVB * srefP = inputMgrP->nextBouquetRef( true );
			while ( srefP != NULL ) {
				if ( 
					( needScanService( *srefP ) ) &&
					( getServiceOrbitalPos( *srefP ) == *orbIt )
				) {
					eTransportStreamID id = srefP->getTransportStreamID();
					if ( idToRef.find( id ) == idToRef.end() )
						idToRef[id] = *srefP;
				}
				srefP = inputMgrP->nextBouquetRef( false );
			}
			idToRefIt = idToRef.begin();
		}
	}

	if ( fillingCacheFlag ) {
		// Note: we must have found a TSID even if we just inited a sat,
		// because otherwise the sat wouldn't be in the list
	
		// Tempting not to call play if we're already in the selected channel
		// looks ugly, but then we don't enterservice in the EPG and have
		// to wait for timeout.

		eServiceInterface::getInstance()->play( idToRefIt->second );
	
		// Takes milliseconds
		timerP->start( (long) 1000 * (long) conf.maxWaitForEPGSeconds, true );

		idToRefIt++;
	}
}

eString MV::formConfigPath( const char *filename )
{
        return prefixDir( eString( CONFIG_DIR ),  eString( filename ) );
}

