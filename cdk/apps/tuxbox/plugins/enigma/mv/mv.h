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


#ifndef __MV_H__
#define __MV_H__

#include <lib/gui/ewidget.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/statusbar.h>
#include <lib/gdi/font.h>
#include <enigma.h>
#include <timer.h>

#include "defs.h"
#ifndef MVLITE
	#include <streaminfo.h>
	#include "favmanager.h"
	#include "programlist.h"
#endif

#include "debug.h"
#include "channel.h"
#include "program.h"
#include "extinfo.h"
#include "miscmenu.h"
#include "inputmanager.h"
#include "timebar.h"
#include "keys.h"
#include "util.h"
#include "conf.h"
#include "inputdefreader.h"
#include "enigma_main.h"  // Bouquet selector only

#include "inputmenu.h"
#include "featuremenu.h"
#include "enigmautil.h"
#include "menu.h"
#include "presentationmenu.h"
#include "daybar.h"
#include "chanpicker.h"

#define MIN_CHANNEL_HEIGHT   30

enum {
	errorCodeNone = 0,
	errorCodeMemory,
	errorCodeDirCreate,
	errorCodeBadStringFile,
	errorCodeCantGetLangPref,
	errorCodeBadSystemTime
};

enum {
	indWindowHidden = 0,
	indUnknownChannels,
	indDownloading,
	indXMLTV,
#ifndef MVLITE
	indFavourites,
	indEdit,
#endif
	indLeftOfSample,
	indRightOfSample,
	indFillCache,
	indLoading,
	indSaving,
	indNoIndicators
};

enum {
	fillCacheSpecifiedChannel,
	fillCacheCurrentPos,
	fillCacheAllPos
};

class eStatusBar;
class InputManager;

#ifndef MVLITE
	class FavManager;
#endif
extern	InputManager *inputMgrP;
extern	struct Inputs inputs;

class MV : 
	//public eWindow
	public eWidget
{
	bool	writeConfigFlag;
	eLabel *timeLabelP;
	bool needRecreateChannelsFlag;
	char *userLangPref2Char;
	char *userLangPref3Char;

	gColor foreColour;
	bool execFlag;
	eTimer *timerP;
	eProgress *chanBarP;
	eStatusBar *sBarP;

	int errorCode;
	int currentMode;
	int currentView;

	// These should be a std::list or vector... oops
	int noChannels;
	int noAllChannels;

	int firstVisibleChannel, lastVisibleChannel, noVisibleChannels;
//        std::vector<Channel *> channels;

	Channel *allChannels[MAX_CHANNELS];
	Channel **channels;

	struct Conf conf; 
	struct ViewConf views[NO_VIEWS]; // 0 is current
	int currentChannel;

	time_t sampleStartSecond;
	time_t windowStartSecond;

	TimeBar *tBarP[FM_MAX_NO_COLUMNS];

	eLabel *indicators[indNoIndicators];

	DayBar *dayBarP;

	// These three keep state between calls to
	// receiveData
	time_t favouriteChecksum;
	int nextProgramID;
	eString lastReceivedChannelName;
	bool favouritesNotifyFlag;

	bool fillingCacheFlag;
	bool canAutoReloadFlag;
	bool wasFillingCacheFlag;
	int fillCacheType;
	int fillCacheChannelNo;   // Only used with fillCacheSpecifiedChannel

	bool windowHiddenFlag;

	void viewHandleLeftRight( int posNeg );
	void viewHandleUpDown( int posNeg );
	void shiftPrograms( int posNeg, time_t startLim, time_t endLim );
	void shiftChannels( int posNeg );
	void showInfo(void);
	void showInfoWhenHidden( void );


	void storeRestoreIndicators( bool storeFlag );
	void setError( int toSet );

	void showHideTimebars( bool );

	int rowHeight( void );
	int displayableChannels( void );
	int displayableChannelsPerColumn( void );
	bool channelsScrollable(void);
	int cursorColumnNo( void );

	bool atRightBorder( void );
	bool atLeftBorder( void );

	void waitCanExit( void );
	void fillCache( bool startupFlag );
	int channelNameToChannelNo( const eString &name );
	int allChannelNameToChannelNo( const eString &name );

	bool needScanService( eServiceReferenceDVB ref );

	bool checkDir( const eString &dir );
	
	unsigned int calcConfigChecksum( void );


	void load( time_t sampStartTime, time_t viewWindowOffset );
	void reload( time_t sampStartTime = -1, time_t viewWindowOffset = -1 );
	bool reloadIfDesiredAndPossible( void );

	bool timebarsEnabled( int viewNo );
	bool dayBarFixed( void );
	bool dayBarUsable( void );
	void hideVisibleChannels( void );
	void doHideShow( bool newValue );
	void hideShowAll( bool hideFlag, bool showInd = false );

	eString getPlayingChannelName( void );
	eString getPlayingTSID( void );
	int getServiceOrbitalPos( eServiceReferenceDVB ref );
	int getPlayingChannelNo( void );

	void showChannelNoAtPositionNo( int channelNo, int visiblePositionNo );
	void doBouquetSelector( bool hideMainWindowFlag );
	void recreateChannels( void );

	void setupDataDirs( void );
	int eventHandler(const eWidgetEvent &event);
//	int eventHandleHidden( int rc );
	int eventHandleViewMode( int keyState, int key );
	int eventHandleDayBar( int keyState, int key );
	int eventHandleTimePicker( int keyState, int key );

	void handleSelect( void );
	void handleCacheFill( int mode );

	time_t windowEndSecond( void );
	time_t windowMiddleSecond( void );
	time_t sampleEndSecond( void );

	void moveCursorToChannel( int toChannel, int posNegRetry, int startEnd = 0 );
	void changeToView( int viewNo );

        void receiveData( struct ProgramData &p );
	void receiveOneChannelCacheData( struct ProgramData &p );

	void refreshVisibleChannels( void );
	
	// Stuff initiaing serious code possibily
	// running in parallel

	void INT_EPGAvailable( bool emptyFlag  );
	void INT_timerTimeout( void );
	int  eventHandleMain( int keyState, int key );
	void INT_downloadsDone( int noFilesDownloaded );

	void setMainWindowGeom( void );
	void setStatusBarGeom( void );
	void setDayBarGeom( void );
	void configureChannels( void );
	void setTimebarGeom( void );
	void setChannelBarGeom( void );
	void rebuildChannels( void );
	void rebuildTimeBars( void );

	void initCursor( eString preferredChannelName = "" );
	void changeWindowSeconds(int posNeg);
	void sortChannels( int sortType );
	void selectChannels( void );
	void storeRestoreFocusDetails( int mode );
	void showTimerEditWindowAux( int type );
	void setBorderIndicators( void );

	eString formConfigPath( const char *filename );

	void setConfigDefaults( void );
	int readConfig( void );
	void writeConfig( void );

	void rebuildAll( void );
	void copyView( int viewNoFrom, int viewNoTo );
	void setIndicator( int indNo, bool toSet );

	time_t sampleWidthSeconds( void );

	void INT_XMLTVStatusChange( bool newStatus );
	void downloadStarted( void );
	int doChannelPicker( int startFromChannel, int dir, bool showSelectorFlag );
	void setTimeLabelGeom( void );

#ifndef MVLITE
	FavManager *favP;

	void rebuildListWindow( ProgramList &listWindow, int type, int curCHann );

	void changeIndValue( int *toChange, int amount );
	int execListWindow( int view, int type );
	void addFavouriteHandle( void );
	int doMenu( void );
	void doMenuItemCredits(void);
        void doMenuItemEditViewGeom(int mode);
        void doMenuItemEditViewPresentation(void);
	void doMenuItemFeedback( void );
	void doMenuItemEditInputs( void );
	void doMenuItemEditViewFilter( void );
	void doMenuItemEditViewFeatures( void );
	void doMenuItemManageAliases( void );
	void doMenuItemManageFavourites( void );
	void doMenuItemMiscSettings( void );
	void doMenuItemReset( void );
	
	int eventHandleEditMode( int keyState, int key );
	void changeSbarValue( int *toChange, int amount );
	void changeTopLeftX( int posNeg );
	void changeTopLeftY( int posNeg );
	void changeWidthPixels( int posNeg );
	void changeHeightPixels( int posNeg );
	void changeChannelHeight( int posNeg );
	void changeStatusBarHeight( int posNeg );
	void changeHeaderWidth( int posNeg );
	void stopEdit( void );
#endif

public:
	MV();
	~MV();

	int error( void );
	void run( void );
};

#endif
