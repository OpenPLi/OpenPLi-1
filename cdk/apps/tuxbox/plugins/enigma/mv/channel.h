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

#ifndef __CHANNEL_H__
#define __CHANNEL_H__

#include <lib/gui/elabel.h>
#include <enigma.h>
#include <timer.h>
#include <enigma_main.h>
#include <lib/gdi/grc.h>

#include "debug.h"
#include "program.h"
#include "featuremenu.h"
#include "util.h"
#include "proglistdef.h"
#include "enigmacache.h"
#include "presentationmenu.h"
//#include "mv.h"



class Channel : public eWidget
{
	static time_t lastPlaySecond;
	static gPixmap *switchTimerPic;
	static gPixmap *recordTimerPic;

	// This should be a vector or list... oops
     	Program *programs[MAX_PROGRAMS_PER_CHANNEL];

	int ID;
        int noPrograms;
	int currentProgram;
        eString myName;

	gPixmap *iconP;
	bool requestedIconFlag;

	gColor programBackColour;
	gColor playingBackColour;
	gColor playingForeColour;
        gColor programOneBackColour;
        gColor programOneForeColour;
        gColor programTwoBackColour;
        gColor programTwoForeColour;
        gColor filmBackColour;
        gColor filmForeColour;
	gColor favouriteBackColour;
	gColor favouriteForeColour;

	gColor headerBackColour;
	gColor headerForeColour;

	gFont programTimeFont;
	gFont programTitleFont;
	gFont programDescrFont;
	gFont programChannelFont;

	gFont 			headerFont;
	int 			headerWidth;
	eServiceReferenceDVB 	serviceRef;
	eWidget *statusBarP;

	time_t windowStartTime, windowEndTime;
	int noProgramsToShow;

	int horizontalSep, verticalSep;

	int headerFlags;
	int statusBarFlags;
	int programBoxFlags;
	bool showElapsedFlag;
	bool redrawNoClearFlag;
	bool needApplyFilmHeuristic;
	bool isUpperCaseNamesChannel;

	void redrawWidget(gPainter *d, const eRect &area);
	bool amCurrentChannel();

	void requestIcon();
	void softInvalidate();
public:
	static time_t secondsAgoLastPlay( void );

	void setStatusBarText( void );
        Channel( eWidget *parent );
	void baseReset( eString name, eServiceReferenceDVB sref, eWidget *sbar );
	void reset( );
	void setID( int toSet );

        ~Channel();

        eString &getName( void ) { return myName; }
        bool addProgram( unsigned int id, eString name, time_t startTime, time_t duration, bool isFilmFlag, bool isFavouriteFlag, bool isFromCacheFlag 
	,short idxFic,long offset);
	
	void setHeaderFont( gFont fnt );
	void setHeaderWidth( int width );
	void setHeaderColours( gColor back, gColor fore );

	void setProgramColours( 
		gColor backColour,
		gColor playingBackColour, gColor playingForeColour,
		gColor backOne, gColor foreOne, 
		gColor backTwo, gColor foreTwo,
		gColor filmBack, gColor filmFore,
		gColor favBack, gColor favFore	
	);
	void setProgramFonts( gFont programTimeFont, gFont programTitleFont, gFont programDescrFont, gFont programChannelFont );
	void setSeparators( int hSep, int vSep );

	void setStatusBarFlags( int flags );
	void setProgramBoxFlags( int flags );
	void setHeaderFlags( int flags );

	bool releaseCursor( time_t *oldStart = NULL, time_t *oldEnd = NULL );
	bool takeCursor( time_t oldStart, time_t oldEnd, bool forceFlag = false, bool mustFitFlag = false );
	bool shiftCursor( int posNeg, time_t leftBound, time_t rightBound );
	void flashCursor( void );


	Program * getPlayingProgramPtr( void );

	void getCurrentProgramTimes( time_t *start, time_t *end );
	bool play( void );
	void setRange( time_t start, time_t end );
	void setProgramsToShow( int toSet );
	void setElapsedBars( bool toSet );

	bool isPlaying( void );
//	void setPlayingFlag( bool toSet );
	int cursorColumn( void );
	eServiceReferenceDVB getServiceRef( void );
	bool haveServRef( void );
	int getOrbitalPosition( void );
	eString getOrbitalPositionString( void );
	void setRedrawNoClear( void );
	Program * getNextProgramPtrByType( int type, bool restart, time_t minEndTime );
	Program * getCurrentProgramPtr( void );

	eString getTSID( void );
	void deletePrograms( void );


	bool currentProgramVisible( void );
	int getNoPrograms( void );
	gPixmap *getIconP( void );
};


#endif
