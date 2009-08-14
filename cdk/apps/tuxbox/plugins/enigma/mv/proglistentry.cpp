#include "proglistentry.h"

gPixmap *ProgramListEntry::inTimer = 0;
gPixmap *ProgramListEntry::inTimerRec = 0;

ProgramListEntry::ProgramListEntry( 
	eListBox<ProgramListEntry> *listBoxP, Channel *cp, Program *pp, 
	int flags, 
	gColor playingBackColour, gColor favouriteBackColour, gColor filmBackColour ) : 
	eListBoxEntry( (eListBox<eListBoxEntry> *) listBoxP), 
	pp(pp), cp(cp), 
	flags(flags), 
	playingBackColour( playingBackColour ), favouriteBackColour( favouriteBackColour ), filmBackColour( filmBackColour ),
	paraTime(NULL), paraTitle(NULL), paraChannel(NULL)
{
}

int ProgramListEntry::getEntryHeight( void )
{
	if ( ! inTimerRec ) {
                inTimer = getNamedPixmapP( "timer_symbol" );
                inTimerRec = getNamedPixmapP( "timer_rec_symbol" );
	}

	// Height is max of all the font height + 4

	int titleHeight = calcFontHeight( titleFont );
	int timeHeight = calcFontHeight( timeFont );
	int channelHeight = calcFontHeight( channelFont );

	// If showing icons, it's the max icon height, otherwise
	// fontsize determined

	int maxSoFar = MAX( titleHeight, timeHeight );
	maxSoFar = MAX( maxSoFar, channelHeight );

	if ( maxSoFar < CHANNEL_ICON_MAX_HEIGHT )
		maxSoFar = CHANNEL_ICON_MAX_HEIGHT;
	else
		maxSoFar += 2;

	return maxSoFar;
}

// This function is purely to allow an entry to decide if
// it's top of the list. The 'top' ePtrList in listbox.h
// is protected :-(

struct isTopFunc: public std::unary_function<ProgramListEntry &, void>
{
	Program *checkPP;
	isTopFunc( Program *pp ) : checkPP( pp ) {}

        bool operator() ( eListBoxEntry &entryToCheck ) {
		if ( ( ( (ProgramListEntry &)entryToCheck).getProgramP() == checkPP ) )
			return 1;
		checkPP = NULL;
		return 0;
	}
};

const eString& ProgramListEntry::redraw(
	gPainter *rc, const eRect& rect, 
	gColor coActiveB, gColor coActiveF, 
	gColor coNormalB, gColor coNormalF, 
	int state 
) {
	static int last_wday = -1;
	static eString last_channel = "";

	drawEntryRect(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );
	
	int startX = rect.left() + 2;
	int xpos = startX;

	gColor thisForeColour = state ? coActiveF : coNormalF;

	// Shade playing on current and other
	// channels differently
	if ( pp->timeOverlaps( getCurrentTime() ) ) {
		rc->setForegroundColor( cp->isPlaying() ?
			playingBackColour :
			adjustedColour( playingBackColour, +50, +20, +50 )
		);
	       	rc->fill( eRect( startX, 0, timeXSize, rect.bottom() ) );
		rc->setForegroundColor( thisForeColour );
	}

	// Return foreground colour to normal

	// DRAW TIME
	if ( ! paraTime) {
		time_t startTime = pp->getStartTime();
		tm startTimeStruct = *localtime(&startTime);

                eString tmp;

		time_t this_wday = startTimeStruct.tm_wday;

		// Show the day of the week if it's different
		// from the day of week for last item, or
		// if it's top item in the list
		if ( 
			( flags & programListEntryFlagShowDate ) &&
			(
				( this_wday != last_wday ) ||
				( ! listbox->forEachVisibleEntry( isTopFunc( getProgramP() ) ) ) 
			)
		) {
			last_wday = this_wday;
	                tmp.sprintf("%02d:%02d %s", 
				 startTimeStruct.tm_hour, startTimeStruct.tm_min,
				getStr( strDaybarFirst + this_wday )
			);
		}
		else
	                tmp.sprintf("%02d:%02d", startTimeStruct.tm_hour, startTimeStruct.tm_min);

		paraTime = makeNewTextPara( 
			tmp,
			0, 0, timeXSize + dayXSize, rect.bottom(),
			timeFont,
			eTextPara::dirLeft
		);

		timeYOffset = ((rect.height() - paraTime->getBoundBox().height()) / 2 ) - paraTime->getBoundBox().top();
        }
        rc->renderPara( *paraTime, ePoint( xpos, rect.top() + timeYOffset ) );

        xpos += timeXSize + dayXSize + 9;

	if ( pp->isFilm() ) {
		rc->setForegroundColor( filmBackColour );
	       	rc->fill( eRect( xpos, rect.top() + 1, FILM_FAV_WIDTH, rect.height() - 2 ) );
	}
        xpos += FILM_FAV_WIDTH;

	if ( pp->isFavourite() ) {
		rc->setForegroundColor( favouriteBackColour );
	       	rc->fill( eRect( xpos, rect.top() + 1, FILM_FAV_WIDTH, rect.height() - 2 ) );
	}

	rc->setForegroundColor( thisForeColour );

	xpos += FILM_FAV_WIDTH + 3;

	// DRAW TIMER ICONS
	EITEvent tempEvent;
	tempEvent.start_time = pp->getStartTime();
	tempEvent.duration = pp->getDuration();
		
	eServiceReferenceDVB serviceRef = cp->getServiceRef();

	ePlaylistEntry* p=0;
	gPixmap *timerPic = NULL;
	if ( (p = eTimerManager::getInstance()->findEvent( &serviceRef, &tempEvent )) ) {
		if ( p->type & ePlaylistEntry::SwitchTimerEntry )
			timerPic = inTimer;
		else if ( p->type & ePlaylistEntry::RecTimerEntry )
			timerPic = inTimerRec;

		if ( timerPic != NULL ) {
			int ypos = centreImage( rect.height(), timerPic->y, rect.top() );
			rc->blit( *timerPic, ePoint( xpos, ypos ), eRect(), gPixmap::blitAlphaTest);
		}
		xpos += inTimer->x + TIMER_TITLE_GAP;
	}

	int titleWidth = rect.width() - xpos;
	if ( flags & programListEntryFlagShowChannelIcon )
		titleWidth -= CHANNEL_ICON_MAX_WIDTH;
	if ( flags & programListEntryFlagShowChannelName )
		titleWidth -= CHANNEL_NAME_WIDTH;
	if ( 
		( flags & programListEntryFlagShowChannelName ) ||
		( flags & programListEntryFlagShowChannelIcon )
	)
		titleWidth -= TITLE_CHANNEL_GAP;

	// Just in case of crash...
	if ( titleWidth < 0 )
		titleWidth = 10;

	// DRAW TITLE
        if ( ! paraTitle ) {
		paraTitle = makeNewTextPara( 
			pp->getTitle(),
                	0 ,0, titleWidth, rect.height(),
			titleFont
		);
                titleYOffset = ((rect.height() - paraTitle->getBoundBox().height()) / 2 ) - paraTitle->getBoundBox().top();
        }
	rc->clip( eRect( xpos, rect.top(), titleWidth, rect.height()) );
        rc->renderPara( *paraTitle, ePoint( xpos, rect.top() + titleYOffset ) );
	rc->clippop();

	xpos += titleWidth + TITLE_CHANNEL_GAP;
	
	// CHANNEL ICON AND OR NAME
	if ( 
		( flags & programListEntryFlagRepeatChannelName  ) ||
		( last_channel != cp->getName() )
	) {
		// CHANNEL ICON
		if ( flags & programListEntryFlagShowChannelIcon ) {
			gPixmap * iconP = cp->getIconP();
			if ( iconP != NULL ) {
				int ypos = centreImage( rect.height(), iconP->y, rect.top() );
				rc->blit( *iconP, ePoint( xpos, ypos ), eRect(), 0);
				xpos += CHANNEL_ICON_MAX_WIDTH + 1;
			}
		}

		// DRAW CHANNEL NAME
		if ( flags & programListEntryFlagShowChannelName ) {
	        	if ( ! paraChannel ) {
				paraChannel = makeNewTextPara( 
					cp->getName(), 
					0, 0, rect.width() - xpos, rect.height(),
					channelFont
				);
				channelYOffset = ((rect.height() - paraChannel->getBoundBox().height()) / 2 ) - paraChannel->getBoundBox().top();
			}

			ePoint chanTopLeft = ePoint( xpos, rect.top() + channelYOffset );
			rc->renderPara( *paraChannel, chanTopLeft );
		}

		// DRAW DIVIDING LINE
		if ( 
			( ! ( flags & programListEntryFlagRepeatChannelName ) ) &&
			( last_channel != "" ) 
		) {
			rc->setForegroundColor( getNamedColour( PROGLISTDIVIDELINE_COLOUR ) );
       	 		rc->line( ePoint( startX, rect.top()), ePoint( rect.right() - 5, rect.top()));
		}

		last_channel = cp->getName();
	}

	return pp->getTitle();
}

ProgramListEntry::~ProgramListEntry( )
{
	if ( paraTime )
		paraTime->destroy();
	if ( paraTitle )
		paraTitle->destroy();
	if ( paraChannel )
		paraChannel->destroy();
}

Program *ProgramListEntry::getProgramP( void )
{
	return pp;
}

Channel *ProgramListEntry::getChannelP (void )
{
	return cp;
}
