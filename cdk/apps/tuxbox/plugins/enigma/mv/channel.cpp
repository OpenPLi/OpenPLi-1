/*************************************************************/
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

#include <lib/gui/epicon.h>

#include "channel.h"
extern	InputManager *inputMgrP;

gPixmap * Channel::recordTimerPic = getNamedPixmapP("timer_rec_symbol");
gPixmap * Channel::switchTimerPic = getNamedPixmapP("timer_symbol" );

Channel::Channel( eWidget *parent )
        : eWidget( parent, 0 )
{
	for ( int programNo = 0; programNo < MAX_PROGRAMS_PER_CHANNEL; programNo++ )
		programs[programNo] = NULL;
	iconP = NULL;
	requestedIconFlag = false;
}

void Channel::baseReset( eString name, eServiceReferenceDVB sref, eWidget *sbar )
{
	myName = name;
	serviceRef = sref;
	statusBarP = sbar;
	needApplyFilmHeuristic = ( 
		( name.left(11) == "MULTIVISION" ) ||
		( name.left(6) == "TV1000" ) ||
		( name.left(6) == "Cinema" ) ||
		( name.left(5) == "Kiosk" ) ||
		( name.left(8) == eString( "CANAL+ (" ) ) ||
		( strcmp( name.c_str(), "CINEMA" ) == 0 ) ||
		( strcmp( name.c_str(), "Film" ) == 0 ) ||
		( strcmp( name.c_str(), "FILM" ) == 0 ) ||
		( name.left(3) == "TPS" ) 
	);

        isUpperCaseNamesChannel = (
		( name.left(11) == "MULTIVISION" ) ||
		( name.left(3) == "TPS" ) 
	);

	delete iconP;
	iconP = NULL;
	requestedIconFlag = false;
}

void Channel::reset()
{
	noPrograms = 0;
	currentProgram = -1;
	headerFlags = 0;
	statusBarFlags = 0; 
	programBoxFlags = 0;
	redrawNoClearFlag = false;
}

void Channel::requestIcon( void )
{
	// This is so that we only load the icon when needed,
	// if we do it on channel creation/reset, overall plugin
	// load time gets noticeably larger

	if ( ! requestedIconFlag ) {
		iconP = ePicon::loadPicon(serviceRef, eSize(40, 30));
		requestedIconFlag = true;
	}
}

gPixmap * Channel::getIconP( void )
{
	requestIcon();
	return iconP;
}

void Channel::setElapsedBars( bool toSet )
{
	showElapsedFlag = toSet;
}

void Channel::setProgramBoxFlags( int flags )
{
	programBoxFlags = flags;
}

void Channel::setHeaderFlags( int flags )
{
	headerFlags = flags;
}

void Channel::flashCursor( void )
{
	int temp = currentProgram;
	currentProgram = -1;
	softInvalidate();
	currentProgram = temp;
	microSleep( 100000 );
	softInvalidate();
}

void Channel::setStatusBarFlags( int flags )
{
	statusBarFlags = flags;
}

int Channel::cursorColumn( void )
{
	int col = -1;
	if ( 
		( currentProgram != -1 ) &&
		( currentProgram != MAX_PROGRAMS_PER_CHANNEL )
	) {
		col = 1;
		for ( int programNo = 0; programNo < currentProgram; programNo++ ) {
			if ( programs[programNo]->isVisible() )	
				col++;
		}
	}

	return col;
}

void Channel::setRange( time_t start, time_t end )
{
	windowStartTime = start;
	windowEndTime = end;
}

void Channel::setProgramsToShow( int toSet )
{
	noProgramsToShow = toSet;
}

void Channel::setProgramColours( gColor backColour, gColor pbc, gColor pfc, gColor backOne, gColor foreOne, gColor backTwo, gColor foreTwo, gColor filmBack, gColor filmFore, gColor favBack, gColor favFore ) 
{
	setBackgroundColor( backColour );
	programBackColour = backColour;
	playingBackColour = pbc;
	playingForeColour = pfc;
	programOneBackColour = backOne;
	programOneForeColour = foreOne;
	programTwoBackColour = backTwo;
	programTwoForeColour = foreTwo;
	filmBackColour = filmBack;
	filmForeColour = filmFore;
	favouriteBackColour = favBack;
	favouriteForeColour = favFore;
}

void Channel::setSeparators( int hSep, int vSep ) 
{
	horizontalSep = hSep;
	verticalSep = vSep;
}

void Channel::setHeaderColours( gColor back, gColor fore ) 
{
	headerBackColour = back;
	headerForeColour = fore;
}

Channel::~Channel()
{ 
	for ( int programNo = 0; programNo < MAX_PROGRAMS_PER_CHANNEL; programNo++ ) {
		if ( programs[programNo] != NULL )
			delete programs[programNo];
	}
	delete iconP;
}

void Channel::setHeaderFont( gFont fnt ) {
	headerFont = fnt;
}

void Channel::setProgramFonts( gFont pTimeFont, gFont pTitleFont, gFont pDescrFont, gFont pChannelFont ) {
	programTimeFont = pTimeFont;
	programTitleFont = pTitleFont;
	//programDescrFont = pDescrFont;
	programDescrFont = getNamedFont( "epg.description" );
	programChannelFont = pChannelFont;
}

void Channel::setHeaderWidth( int width ) 	{
	headerWidth = width;
}

// Programs are inserted in start time order.
// The algorithm is optimised for data that is already ordered,
// in that it starts insertion at the end
// Ordered data will never need to be shuffled
// Start times are never adjusted

bool Channel::addProgram( 
	unsigned int id, eString name,
	time_t startTime, time_t duration,
	bool isFilmFlag, bool isFavouriteFlag, bool isFromCacheFlag
	,short idxFic,long offset
) {
	if ( 
		( name == "No program" ) ||
		( name == "Sendeschluss" )
	)
		return false;

	if ( noPrograms >= MAX_PROGRAMS_PER_CHANNEL )
		return false;

	if ( duration < MIN_PROGRAM_LENGTH_SECONDS )
		return false;

	time_t endTime = startTime + duration;

	int insertProgramNo = noPrograms;

	if ( noPrograms > 0 ) {
	        // Look for an insertion point

       	 	while ( 
			( insertProgramNo > 0 ) &&
			( startTime < programs[insertProgramNo-1]->getStartTime() )
		)
			insertProgramNo--;
       	}

	Program *prevP = ( insertProgramNo > 0 ) ? programs[insertProgramNo - 1] : NULL;
	Program *nextP = ( insertProgramNo < noPrograms ) ? programs[insertProgramNo] : NULL;
			
	// Need take action if overlaps with next

	if (
		( nextP != NULL ) && 
		( endTime >= nextP->getStartTime() )
	) {
		// If it looks like duplicate of next,  stop

		if ( nextP->isSimilar( startTime, endTime ) )
			return false;

		// If the diff is too big, drop it, otherwise 
		// adjust to end just before other starts

		time_t newEndTime = nextP->getStartTime() - 1;
		time_t newDuration = newEndTime - startTime;

		// If new would be too small after adjust, stop

		if ( newDuration < MIN_PROGRAM_LENGTH_SECONDS )
			return false;

		// If duration adjustment too large, stop

		if ( ( (float) newDuration / (float) duration ) < MAX_FRACTION_DURATION_ADJUST )
			return false;

		endTime = newEndTime;
		duration = newDuration;
	}

	// Need take action if overlaps with previous
	if ( 	
		( prevP != NULL ) &&
		( startTime <= prevP->getEndTime() ) 
	) {
		 // If new is wholly within previous, drop it

                if ( endTime <= prevP->getEndTime() )
			return false;

		// If it looks like duplicate of previous stop
	
		if ( prevP->isSimilar( startTime, endTime ) )
			return false;

		// Else adjust end time of previous

                time_t newEndTime = startTime - 1;
		time_t newDuration = newEndTime - prevP->getStartTime();
			
		// If prev would be too small after adjust, stop

		if ( newDuration < MIN_PROGRAM_LENGTH_SECONDS )
			return false;

	        // if end adjust too large ignore new one
               if (
                        ( ( (float) newDuration ) / ( (float) prevP->getDuration() ) )
		 	< MAX_FRACTION_DURATION_ADJUST
               )
			return false;

                prevP->setEndTime( newEndTime );
	}

	// Now do the insertion by shifting up if necessary

	for ( int shiftProgramNo = noPrograms; shiftProgramNo > insertProgramNo; shiftProgramNo-- ) {
		programs[shiftProgramNo] = programs[shiftProgramNo - 1];
		programs[shiftProgramNo - 1] = NULL;
	}

	// If this is a reload, we might already
	// have created the program

	if ( programs[insertProgramNo] == NULL )
      		programs[insertProgramNo] = new Program();

	// OOM
	static bool reportedOOMFlag = false;
	if ( programs[insertProgramNo] == NULL ) {
		if ( ! reportedOOMFlag )
			dmsg( getStr( strOOM ) );
		reportedOOMFlag = true;
		return false;
	}

	// If this is a known film channel, try an heuristic to mark as film

	if ( 
		needApplyFilmHeuristic &&
		( ! isFilmFlag ) &&
		( duration > ( 60 * 70 ) )
	)
		isFilmFlag = 1;

	if ( isUpperCaseNamesChannel )
		name = mungeWordsToLowerCase( name );

	programs[insertProgramNo]->reset( id, name, startTime, duration, isFilmFlag, isFavouriteFlag, isFromCacheFlag ,idxFic,offset);
	noPrograms++;

        return true;
}

bool Channel::shiftCursor( int posNeg, time_t leftBoundary, time_t rightBoundary )
{
	int oldCurrentProgram = currentProgram;

	if ( amCurrentChannel() ) {
		// Shift right
		if ( posNeg > 0 ) {
			if ( 
				( currentProgram < ( noPrograms - 1 ) ) &&
				( programs[currentProgram+1]->getStartTime() < rightBoundary )
			)
				currentProgram++;	
		}	
		else {
			if (
				( currentProgram > 0 ) &&
				( programs[currentProgram-1]->getEndTime() > leftBoundary )
			)
				currentProgram--;
		}
	}

	if ( currentProgram != oldCurrentProgram ) {
		if ( currentProgram != -1 )
			setStatusBarText();
		softInvalidate();
		return true;
	}
	else 
		return false;
}

bool Channel::amCurrentChannel( void )
{
	return (
		( currentProgram != -1 ) &&
		( currentProgram != MAX_PROGRAMS_PER_CHANNEL )
	);
}

void Channel::setStatusBarText( void )
{
	if ( amCurrentChannel() ) {
		eString toSet;
		Program *pp = programs[currentProgram];

		if ( statusBarFlags & sBarFlagChannelName )
       	         toSet = myName + " - " + pp->getStatusBarText( statusBarFlags );
		else
			toSet = pp->getStatusBarText( statusBarFlags );

		if ( 
			( statusBarFlags & sBarFlagDescr )
		) {
			char buffer[DC_MAX_DESCR_LENGTH+1];
			if ( inputMgrP->getDescription( this, pp, buffer ) ) {
	
				// Remove newlines from description, it
				// disturbs the wrapping
				char *charP;
				while ( ( charP = strchr( buffer, '\n' ) ) != NULL )
					*charP = ' ';

				toSet = toSet + eString( "\n" ) + eString( buffer );
			}
		}
		statusBarP->setText( toSet );
	}
}

Program * Channel::getCurrentProgramPtr( void )
{
	if ( amCurrentChannel() )
		return programs[currentProgram];
	else {
		return NULL;
	}
}

void Channel::getCurrentProgramTimes( time_t *start, time_t *end )
{
	if ( amCurrentChannel() ) {
		*start = programs[currentProgram]->getStartTime();
		*end = programs[currentProgram]->getEndTime();
	}
	else {
		*start = windowStartTime; *end = windowEndTime;
#ifdef DEBUG
		dmsg( "err: getCurrentProgramTimes: we don't have one!" );
#endif
	}
}

Program * Channel::getPlayingProgramPtr( void )
{
	int programNo = 0;

	time_t nowTime = getCurrentTime();

	while ( programNo < noPrograms ) {
		Program *pp = programs[programNo];
		if ( pp->timeOverlaps( nowTime ) )
			return pp;
		programNo++;
	}

	return NULL;
}

void Channel::setRedrawNoClear( void )
{
	redrawNoClearFlag = true;
}

void Channel::redrawWidget(gPainter *d, const eRect &wArea)
{
	eRect wholeArea = wArea;

	if ( noProgramsToShow > 0 ) {
		if ( verticalSep ) 
			wholeArea.setLeft( wArea.left() + 1 );

		if ( horizontalSep )
			wholeArea.setTop( wArea.top() + 2 );
	}

	int elapsedBarWidth = 3;
	char detailBuffer[DC_MAX_DESCR_LENGTH+1];

	time_t channelWidthSeconds = windowEndTime - windowStartTime;
	time_t nowTime = getLoadTime();

	bool thisChannelPlayingFlag = isPlaying();

	if ( channelWidthSeconds < 1 ) {
#ifdef DEBUG
		flashIntMessage("program err: Channel::redrawWidget: wSeconds < 1 ", ID );
#endif
		return;
	}

	// Width and first Xpos is dependent on whether we show
	// the channel header or not

	eRect programArea;
	gColor foreColour, backColour;

	if ( thisChannelPlayingFlag ) {
		foreColour = playingForeColour;
		backColour = playingBackColour;
	}
	else {
		foreColour = headerForeColour;
		backColour = headerBackColour;
	}

	// --------------- HEADER ------------------ #

	if ( headerFlags ) {
		eRect headerRect = eRect( 0,0, headerWidth, wholeArea.height() );

//		if ( ! redrawNoClearFlag ) {
			d->setForegroundColor( backColour );
			d->fill( headerRect );
//		}

		d->setBackgroundColor( backColour );
		d->setForegroundColor( foreColour );

		gFont orbFont = headerFont;
		orbFont.pointSize -= 4;

		int xbase = 0;

		// Channel icon
		if ( headerFlags & channelHeaderFlagShowIcon ) {
			requestIcon();
                        if ( iconP ) {
				ePoint pos( 
					centreImage( CHANNEL_ICON_MAX_WIDTH, iconP->x ),
					centreImage( wArea.height(), iconP->y )
				);
				d->blit( *iconP, pos, eRect(), 0 );
				xbase = CHANNEL_ICON_MAX_WIDTH;
			}
			// If no icon, keep line if not showing names
			else if ( ! ( headerFlags & channelHeaderFlagShowName ) )
				xbase = CHANNEL_ICON_MAX_WIDTH;
		}
			
		int namexpos = xbase;
		int nameypos = 0;

		// Channel number
		if ( headerFlags & channelHeaderFlagShowNumber ) {
			eString numberString = eString().sprintf( "%d.", ID + 1 );
			gFont numberFont;

			// If can't fit two lines, put one line
			if ( ( headerFont.pointSize + orbFont.pointSize ) > headerRect.height() ) {
				numberFont = headerFont;
				namexpos = stringWidthPixels( numberFont, numberString ) + 2;
			}
			else {
				numberFont = orbFont;
				nameypos = numberFont.pointSize + 2;
			}
			d->setFont( numberFont );
			eRect numberRect( xbase,0, headerWidth, numberFont.pointSize );
	 	 	d->renderText( numberRect, numberString, RS_FADE );
		}

		// Draw name

		if ( headerFlags & channelHeaderFlagShowName ) {
			d->setFont( headerFont );
			eRect nameRect( namexpos, nameypos, headerWidth - xbase - 4, headerFont.pointSize );
			d->clip( nameRect );
 		 	d->renderText( nameRect, getName(), RS_FADE );
			d->clippop();
		}

		// Draw orbital pos
		if ( headerFlags & channelHeaderFlagShowOrbital ) {
			d->setFont( orbFont );

			eString orbString = getOrbitalPositionString();
			int orbWidth = stringWidthPixels( orbFont, orbString ); 
			int orbxpos, orbypos;
			if ( 
				( headerFlags & channelHeaderFlagShowNumber ) && 
				( headerFlags & channelHeaderFlagShowName ) 
			) {
				orbxpos = headerWidth - orbWidth - 1;
				orbypos = 0;
			}
			else {
				orbxpos = xbase;
				orbypos = headerFont.pointSize + 2;
			}

			d->renderText( eRect( orbxpos, orbypos, orbWidth, orbFont.pointSize ), orbString, RS_FADE );
		}

		programArea = eRect( headerWidth + 1,0, wholeArea.width() - headerWidth - 1, wholeArea.height() );

	}
	else {
		programArea = wholeArea;
	}

	// -------------------- PROGRAM AREA --------------------- //

	// Maybe there's no space leftover after we put the header in

	if ( programArea.width() <= 1 )
		return;

	int pixelsPerProgram = 1;
	int leftoverPixels = 1;
	float pixelsPerSecond = 1;

	if ( noProgramsToShow == 0 ) {
		pixelsPerSecond = (float) programArea.width() / (float) ( channelWidthSeconds );
	}
	else {
		pixelsPerProgram = programArea.width() / noProgramsToShow;
		leftoverPixels = programArea.width() - ( pixelsPerProgram * noProgramsToShow );
	}
	
	int programNo = 0;
	int shownPrograms = 0;

	// Don't draw programs outside the program area

       	d->clip( programArea );

	// Clear channel background
	// Colour code has turned into a big mess :-(

	if ( currentProgram == MAX_PROGRAMS_PER_CHANNEL ) {
		gColor tempColour = backColour;
		backColour = foreColour;
		foreColour = tempColour;
		d->setForegroundColor( backColour );
	}
	else if (
		( headerFlags ) ||
		( getNoPrograms() > 0 )
	)
		d->setForegroundColor( headerBackColour );
	else
		d->setForegroundColor( backColour );

//	if ( ! redrawNoClearFlag ) 
	       	d->fill( programArea );

	// If we don't have a header or programs, show the channel name
	// centred in the program area so that we can browse the channels

	if ( 
		( ! headerFlags ) &&
		( getNoPrograms() == 0 )
	) {
		int chanTitleWidth = stringWidthPixels( headerFont, getName() );

		int hborder = ( programArea.width() - chanTitleWidth ) / 2;
		if ( hborder < 0 )
			hborder = 0;

		int vborder = ( programArea.height() - headerFont.pointSize ) / 2;
		if ( vborder < 0 )
			vborder = 0;

		eRect chanRect(
			hborder, vborder, programArea.width() - hborder, programArea.height() - vborder
		);

		d->setBackgroundColor( backColour );
		d->setForegroundColor( foreColour );

		d->setFont( headerFont );
        	d->renderText( chanRect, getName(), RS_FADE );
	}

	while ( programNo < noPrograms ) {
		Program *pp = programs[programNo];

		// Hide it if it isn't in the time window
		if ( 
			( 
				( noProgramsToShow == 0 ) &&
				( 
					( pp->getEndTime() <= windowStartTime || pp->getStartTime() >= windowEndTime )
				)
			) ||
			// Fixed no of programs: only start counting
			// at playing program, only show required number
			// of programs
			( 	
				( noProgramsToShow != 0 ) &&
				(
					( pp->getEndTime() < nowTime ) ||
					( shownPrograms >= noProgramsToShow ) 
				)
			) 
		) {
			programNo++;
			pp->setVisible( false );
			continue;
		}

		// Clip start/end time if they go out of
		// the viewable time window

		int useStartTime, useEndTime;

		if ( pp->getStartTime() < windowStartTime )
			useStartTime = windowStartTime;
		else 
			useStartTime = pp->getStartTime();

		if ( pp->getEndTime() > windowEndTime )
			useEndTime = windowEndTime;
		else
			useEndTime = pp->getEndTime();

		// Calc where to put it

		int xPos, width;
		if ( noProgramsToShow == 0 ) {
			xPos = programArea.left() + (int) ((float) ( useStartTime - windowStartTime ) * pixelsPerSecond );
			width = (int) ( (float) ( useEndTime - useStartTime ) * pixelsPerSecond );
		}
		else  {
			xPos = programArea.left() + ( shownPrograms * pixelsPerProgram );
			width = pixelsPerProgram;
			if ( shownPrograms == ( noProgramsToShow - 1 ) )
				width += leftoverPixels;
		}

		bool thisProgramPlayingSomewhereFlag = pp->timeOverlaps( nowTime );
		bool thisProgramPlayingFlag = thisChannelPlayingFlag && thisProgramPlayingSomewhereFlag;

		// Set colours
		if ( programNo == currentProgram ) {
			if ( thisProgramPlayingFlag ) {
				backColour = playingForeColour;
				foreColour = playingBackColour;
			}
			else if ( pp->isFavourite() ) {
				backColour = favouriteForeColour;
				foreColour = favouriteBackColour;
			}
			else if ( pp->isFilm() ) {
				backColour = filmForeColour;
				foreColour = filmBackColour;
			}
			else if ( ( programNo + ID ) % 2 ) {
				backColour = programOneForeColour;
				foreColour = programOneBackColour;
			}
			else {
				backColour = programTwoForeColour;
				foreColour = programTwoBackColour;
			}
		}
		else if ( thisProgramPlayingFlag ) {
			backColour = playingBackColour;
			foreColour = playingForeColour;
		}
		else if ( pp->isFavourite() ) {
			backColour = favouriteBackColour;
			foreColour = favouriteForeColour;
		}
		else if ( pp->isFilm() ) {
			backColour = filmBackColour;
			foreColour = filmForeColour;
		}
		else if ( ( programNo + ID )  % 2 ) {
			backColour = programOneBackColour;
			foreColour = programOneForeColour;
		}
		else {
			backColour = programTwoBackColour;
			foreColour = programTwoForeColour;
		}
	
        	eRect prog( xPos, 0, width, programArea.height() );

		if ( verticalSep && ( shownPrograms > 0 ) ) {
			prog.setLeft( xPos + 1 );
			prog.setWidth( width - 1 );
		}

		if ( horizontalSep && ( noProgramsToShow != 1  ) )
			prog.setHeight( programArea.height() - 1 );

       		d->clip( prog );

		d->setForegroundColor( backColour );
		d->fill( prog );

		int yDiff = showElapsedFlag ? (elapsedBarWidth + 1 ): 0;

		d->setForegroundColor( foreColour );
		d->setBackgroundColor( backColour );

		if ( programBoxFlags & pBoxFlagChannelIcon ) {
			requestIcon();
			if ( iconP ) {
				ePoint imPos(
					centreImage( CHANNEL_ICON_MAX_WIDTH, iconP->x, xPos ),
					( centreImage( prog.height() - yDiff, iconP->y ) + yDiff )
				);
				d->blit( *iconP, imPos, eRect(), 0 );
				// Move other program details to right of it
				prog.setLeft( xPos + CHANNEL_ICON_MAX_WIDTH );
			}
		}

		
		// Render time text if options set

		eString timeString = "";
		if ( programBoxFlags & pBoxFlagTime )
			timeString = pp->getStartEndTimeText( true );

		// End time
		if ( programBoxFlags & pBoxFlagEndTime )
			timeString = timeString + eString( " - " ) + pp->getStartEndTimeText( false );

		// Duration
		if ( programBoxFlags & pBoxFlagDuration )
			timeString += eString().sprintf( " (%d)", pp->getDurationMinutes() );

		// Diff start time/ current time
		if ( programBoxFlags & pBoxFlagTimeDiff ) {
			timeString += eString(" [") + pp->getStartTimeDiffText() + eString( "]" );
		}

		// Render channel name if requested

		if ( programBoxFlags & pBoxFlagChannel ) {
			if ( timeString.length() > 0 )
				timeString += " / ";
			timeString += getName();
		}

		if ( timeString.length() > 0 ) {
			eRect timeRect = prog;
			timeRect.setTop( prog.top() + yDiff );
			d->setFont( programTimeFont );
        		d->renderText( timeRect, timeString, RS_FADE );
			yDiff += ( programTimeFont.pointSize + 3 );
		}

		// Render title text

		d->setFont(  programTitleFont );

		// Centre if requested
		int xOffset = 0;
		if ( programBoxFlags & pBoxFlagCentreTitle ) {
			int titleWidth =  stringWidthPixels( programTitleFont, pp->getTitle() );

			// Looks pretty silly centering when title
			// is larger than available space
			if ( titleWidth < prog.width() )
				xOffset = centreImage( 
					prog.width(), 
					stringWidthPixels( programTitleFont, pp->getTitle() )
				);
		}

		eRect titleRect( 
			prog.left() + xOffset,
			prog.top() + yDiff, 
			prog.width() - xOffset, prog.height() 
		);

        	d->renderText( titleRect, pp->getTitle(), RS_FADE );
		yDiff += ( programTitleFont.pointSize + 2 );

		// Render description if requested
		if ( 
			( programBoxFlags & pBoxFlagDescr ) 
		) {
			eRect descrRect = prog;
			descrRect.setTop( prog.top() + yDiff );
			d->setFont( programDescrFont );
			if ( inputMgrP->getDescription( this, pp, detailBuffer ) ) 
	        		d->renderText( descrRect, eString( detailBuffer ), RS_WRAP );
		}

		// Draw timer image if it's on timer
		EITEvent tempEvent;
		tempEvent.start_time = pp->getStartTime();
		tempEvent.duration = pp->getDuration();

		ePlaylistEntry* p = eTimerManager::getInstance()->findEvent( &serviceRef, &tempEvent );
		if ( p ) {
			// Decide on a picture (or none)
			gPixmap *picP = NULL;
	                if ( p->type & ePlaylistEntry::SwitchTimerEntry )
				picP = switchTimerPic;
                	else if ( p->type & ePlaylistEntry::RecTimerEntry )
				picP = recordTimerPic;
				
			// Draw it, put a white background in first otherwise
			// it looks terrible (it's drawn in red...)
			if ( picP ) {
				d->setForegroundColor( getNamedColour( "white_trans" ) );
				ePoint pnt( prog.right() - picP->x - 1, prog.top() );
       				d->fill( eRect( pnt, picP->getSize() ) );
				d->blit( *picP, pnt, eRect(), gPixmap::blitAlphaTest );
			}
		}

		// Draw remaining time bar 
	
		if ( showElapsedFlag ) {
			// If we drew an icon previously, we want
			// elapsed bar to go over the top of it,
			// shift program left back

			if ( 
				( iconP ) &&
				( programBoxFlags & pBoxFlagChannelIcon ) 
			)
				prog.setLeft( prog.left() - CHANNEL_ICON_MAX_WIDTH );

			int centreX;
			if ( thisProgramPlayingSomewhereFlag ) {
				if ( noProgramsToShow == 0 ) {
					time_t elapsed = nowTime - useStartTime;
					centreX = prog.left() + (int) ( (float) elapsed * pixelsPerSecond );
				}
				else {
					time_t elapsed = nowTime - pp->getStartTime();
					float fracElapsed = (float ) elapsed / (float) pp->getDuration();
					centreX = prog.left() + (int) ( fracElapsed * (float) width );
				}
			}
			else if ( pp->getEndTime() < nowTime )
				centreX = prog.right();
			else
				centreX = prog.left();
	
			if ( centreX > prog.left() ) {

				d->setForegroundColor( getNamedColour( "blue_nontrans" ) );
				d->fill( eRect( prog.left(), 0, centreX , elapsedBarWidth ));
			}
			if ( centreX < prog.right() ) {
				d->setForegroundColor( getNamedColour( "std_dgreen" )  );
				d->fill( eRect( centreX, 0, prog.right(), elapsedBarWidth ));
			}
		}

		d->clippop();

		pp->setVisible( true );

		shownPrograms++;
		programNo++;
	}

	// Draw current time markers

	if ( 		
		( noProgramsToShow == 0 ) &&
		( nowTime > windowStartTime ) &&
		( nowTime < windowEndTime )
	) {

		int markerX = programArea.left() + (int) ((float) ( nowTime - windowStartTime ) * pixelsPerSecond );
		//d->setForegroundColor( colourOptionToColour( PM_COLOUR_DEEP_GREEN ) );
		// textinput_white
		d->setForegroundColor( getNamedColour("white_trans") );
		d->line( ePoint( markerX, 0 ), ePoint( markerX, 4 ) );
		d->line( ePoint( markerX, wholeArea.height() - 4), ePoint( markerX, wholeArea.height() ) );
	}

	d->clippop();

	redrawNoClearFlag = false;
}

bool Channel::releaseCursor( time_t *oldStart, time_t *oldEnd )
{
	bool releasedFlag = false;

	if ( currentProgram == MAX_PROGRAMS_PER_CHANNEL ) {
		if ( oldStart != NULL )
			*oldStart = windowStartTime + 1;
		if ( oldEnd != NULL )
			*oldEnd = windowEndTime - 1;
		currentProgram = -1;
		softInvalidate();
		releasedFlag = true;
	}
	else if ( currentProgram != -1 ) {
		if ( noProgramsToShow == 0 ) {
			if ( oldStart != NULL )
				*oldStart = MYMAX( windowStartTime, programs[currentProgram]->getStartTime());
			if ( oldEnd != NULL )
				*oldEnd = MYMIN( windowEndTime, programs[currentProgram]->getEndTime() );
		}
		else {
			if ( oldStart != NULL )
				*oldStart = (time_t) cursorColumn();
			if ( oldEnd != NULL )
				*oldEnd = -1;
		}

		currentProgram = -1;
		softInvalidate();
		releasedFlag = true;
	}

	return releasedFlag;
}

bool Channel::takeCursor( time_t oldStart, time_t oldEnd, bool forceFlag, bool mustFitFlag )
{
        int programNo = 0;
        int closestProgramNo = -1;
	currentProgram = -1;

	if ( ! isVisible() )
		return false;

	if ( getNoPrograms() > 0 ) {
		// Fixed no programs: oldStart specifies column to
		// move to

		if ( oldEnd == -1 ) {
			// Find first visible program
			while ( 
				( programNo < noPrograms ) &&
				( ! programs[programNo]->isVisible() )
			)
				programNo++;

			// Then move to right column
			while ( 
				( programNo < noPrograms ) &&
				( oldStart > 1 )
			) {
				programNo++;
				oldStart--;
			}

			if ( 
				( programNo < noPrograms ) &&
				( programs[programNo]->isVisible() )
			)
				closestProgramNo = programNo;
		}
		else {
       		 	time_t closestDistance = 999999;

		        time_t oldMiddle = oldStart + ( ( oldEnd - oldStart ) / 2 );


		        while ( programNo < noPrograms ) {
				Program *pp = programs[programNo];
	
				if ( ! pp->isVisible() ) {
					programNo++;
					continue;
				}

				if ( 
					mustFitFlag &&
					( ! pp->timeOverlaps( oldMiddle ) )
				) {
					programNo++;
					continue;
				}

       			         time_t startDiff = absTimeDiff( pp->getStartTime(), oldMiddle );
       			         time_t endDiff  = absTimeDiff( pp->getEndTime(), oldMiddle );

       			         if ( startDiff < closestDistance ) {
       			                 closestDistance = startDiff;
       			                 closestProgramNo = programNo;
       			         }
       			         if ( endDiff < closestDistance ) {
       			                 closestDistance = endDiff;
					closestProgramNo = programNo;
       			         }

       			       programNo++;
       		 	}
		}
	}
	
	// If found one, take it

        if ( closestProgramNo != -1 ) {
		currentProgram = closestProgramNo;
		if ( statusBarP != NULL )
			setStatusBarText();
		if ( isVisible() )
			softInvalidate();
        }

	// Otherwise, take only if forced

	else if ( forceFlag ) {
		currentProgram = MAX_PROGRAMS_PER_CHANNEL;
		if ( isVisible() )
			softInvalidate();
	}

        return ( currentProgram != -1 );
}

void Channel::softInvalidate( void )
{
	setRedrawNoClear();
	invalidate();
}

time_t Channel::lastPlaySecond;
time_t Channel::secondsAgoLastPlay( void )
{
	return ( getCurrentTime() - lastPlaySecond );
}


// We can't check whether success or failure, it always
// returns -1
bool Channel::play( void )
{
	eZapMain::getInstance()->playService( serviceRef, eZapMain::psSetMode );
	lastPlaySecond = getCurrentTime();
	return true;
}

bool Channel::isPlaying( void )
{
	return ( getPlayingServiceRef() == serviceRef );
}

eServiceReferenceDVB Channel::getServiceRef( void )
{
	return serviceRef;
}

int Channel::getOrbitalPosition( void ) 
{
         return serviceRef.getDVBNamespace().get()>>16;
}

eString Channel::getOrbitalPositionString( void )
{
	int oPos = getOrbitalPosition();

	return eString().sprintf( 
		"%d.%d\xC2\xB0%c", 
		abs(oPos / 10), 
		abs(oPos % 10), 
		oPos > 0 ? 'E' : 'W'
	 );
}

Program * Channel::getNextProgramPtrByType( int type, bool reset, time_t minEndTime )
{
	static int curProgram = 0;

	if ( reset )
		curProgram = 0;

	Program *toReturn = NULL;

	while ( 
		( curProgram < noPrograms ) &&
		( toReturn == NULL )
	) {
		if ( 
			( type == programListTypeChannel ) ||
			( type == programListTypeAllChannels ) ||
			( 
				( type == programListTypeFilm ) &&
				( programs[curProgram]->isFilm() )
			) ||
			(
				( type == programListTypeFavourite ) &&
				( programs[curProgram]->isFavourite() )
			) 
		) {
			if ( programs[curProgram]->getEndTime() > minEndTime )
				toReturn = programs[curProgram];
		}

		curProgram++;
	}
	
	return toReturn;
}

bool Channel::currentProgramVisible( void )
{
	return (
		amCurrentChannel() &&
		programs[currentProgram]->isVisible()
	);
}

eString Channel::getTSID( void )
{
	 return eString().sprintf("%04xh", serviceRef.getTransportStreamID().get() );
}

int Channel::getNoPrograms( void )
{
	return noPrograms;
}

void Channel::setID( int toset )
{
	ID = toset;
}
