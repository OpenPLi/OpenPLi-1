#include "programlist.h"

gFont ProgramListEntry::timeFont;
gFont ProgramListEntry::titleFont;
gFont ProgramListEntry::channelFont;

int ProgramListEntry::timeXSize = 0;
int ProgramListEntry::dayXSize = 0;

enum {
	showDescrStatusBar = 0,
	showDescrWindow
};

ProgramList::ProgramList( 
	struct ViewConf &conf,
	const time_t &timerPreOffset, const time_t &timerPostOffset, 
	eStatusBar *sBarP,
	eLabel *timeLabelP,
	eLabel *hiddenIndP, 
	eLabel *editIndP
) : eWindow(1) , conf( conf ), 
	mode( MV_MODE_VIEW ), 
	editIndP( editIndP ), 
	timerPreOffset( timerPreOffset ), timerPostOffset( timerPostOffset ), sBarP( sBarP ), timeLabelP( timeLabelP ), hiddenIndP(hiddenIndP),
	hiddenFlag( false )
{
	setListEntryFonts();

	theList = new eListBox<ProgramListEntry>(this);
	theList->loadDeco();
	theList->hide();

	CONNECT( theList->selected, ProgramList::selected );
	CONNECT( theList->selchanged, ProgramList::selectionChanged );

	setSbarGeom();
}

void ProgramList::setListEntryFonts( void )
{
	ProgramListEntry::timeFont = makeFontFromOptions( 
		conf.pres.programTitleFontFamily, 
		conf.pres.programTitleFontSize
	);
        ProgramListEntry::titleFont = makeFontFromOptions( 
		conf.pres.programTitleFontFamily, 
		conf.pres.programTitleFontSize
	);
        ProgramListEntry::channelFont = makeFontFromOptions( 
		conf.pres.channelHeaderFontFamily, 
		conf.pres.channelHeaderFontSize 
	);
	ProgramListEntry::dayXSize = stringWidthPixels( ProgramListEntry::timeFont, "___ " );
        ProgramListEntry::timeXSize = stringWidthPixels( ProgramListEntry::timeFont, "__:__ " );
}

void ProgramList::reset( const eString &titleSpec )
{
	theList->hide();
	theList->clearList();
	setText( eString( getStr( strProgramListWindowTitlePrefix ) ) + titleSpec );
	setSbarGeom();

	setListEntryFonts();
}

int ProgramList::noEntries( void )
{
	return theList->getCount();
}

void ProgramList::setMode( int newMode )
{
	mode = newMode;
}

void ProgramList::addFavouriteHandle( Channel *cp, Program *pp )
{
	showHideAll( false, true );
	FavManager *favP = new FavManager( prefixDir( eString( CONFIG_DIR ),  eString( FAVOURITES_FILENAME )));
	favP->addNameAndChannelEntry( pp->getTitle(), cp->getName() );
	delete favP;
	showHideAll( true, true );
}

void ProgramList::showHideSbar( bool showFlag )
{
	showHideWidget( sBarP, 
		showFlag &&
		conf.feat.showStatusBar &&
		( noEntries() > 0 )
	);
}

void ProgramList::showHideAll( bool showFlag, bool emitFlag )
{
	showHideWidget( this, showFlag );
	showHideSbar( showFlag );
//	if ( emitFlag )
//		/*emit*/ hiding( ! showFlag );
}

void ProgramList::selectionChanged( ProgramListEntry *entryP )
{
	if ( conf.feat.showStatusBar )
		showDescr( showDescrStatusBar );
}

void ProgramList::selected( ProgramListEntry *entryP )
{
	if ( entryP ) {
		ProgramListEntry *ep = theList->getCurrent();
		if ( ep != NULL ) {
			Channel *cp = ep->getChannelP();
			cp->play();
			// Otherwise the colour of the 
			// playing channel isn't updated
			theList->invalidate();
		}
	}
}

void ProgramList::addEntry( Channel *cp, Program *toAdd )
{
	int flags = 0;

	if ( isCronSorted() )
		flags = programListEntryFlagShowDate | programListEntryFlagRepeatChannelName;

	if ( conf.feat.channelHeaderFlags & channelHeaderFlagShowIcon )
		flags |= programListEntryFlagShowChannelIcon;

	if ( conf.feat.channelHeaderFlags & channelHeaderFlagShowName )
		flags |= programListEntryFlagShowChannelName;

	new ProgramListEntry( 
		theList, cp, toAdd, 
		flags, 
		colourOptionToColour( conf.pres.playingBackColour ),
		colourOptionToColour( conf.pres.favouriteBackColour ),
		colourOptionToColour( conf.pres.filmBackColour )
	);
}

extern	InputManager *inputMgrP;
void ProgramList::showDescr( int dmode )
{
	ProgramListEntry *ep = theList->getCurrent();
	if ( ep == NULL )
		return;

	Program *pp = ep->getProgramP();
	char buffer[DC_MAX_DESCR_LENGTH+1];
	if ( 
		( pp == NULL ) ||
		( ! inputMgrP->getDescription( ep->getChannelP(), pp, buffer ) )
	)
		buffer[0] = '\0';
	// Don't bring up an empty info window

	if ( dmode == showDescrWindow ) {
		if ( 
			( strlen( buffer ) > 0 ) ||
			(
				( haveNetwork() ) &&
				( pp != NULL ) &&
				( pp->isFilm() )
			)
		) {
			showHideAll( false, true );
			doExtInfo( pp, buffer, colourOptionToColour( conf.pres.filmBackColour ) );
			showHideAll( true, true );
		}
	}
	// Always set text for status bar, to make sure we clear
	// previous entry's text
	else {
		 sBarP->setText( pp->getStatusBarText( conf.feat.statusBarFlags ) + eString( "\n" ) + eString( buffer ) );
	}
}

void ProgramList::doHideShow( void )
{
	static ePoint lastPos;

	hide();
	/*emit*/ hiding( ! hiddenFlag );

	if ( hiddenFlag ) {
		hiddenIndP->hide();
        	move( lastPos );
		showHideSbar( true );
	}
	else { 
		lastPos = getPosition();
		move( ePoint( 900, 900 ) );
		hiddenIndP->show();
		showHideSbar( false );
	}
	show();

	hiddenFlag = 1 - hiddenFlag;
}

int ProgramList::eventHandler(const eWidgetEvent &event)
{
	int handled = 0;
        switch (event.type) {
		case eWidgetEvent::execBegin:
			setGeom();
			setFocus( theList );
			if ( conf.feat.sortType == sortTypeListStartTime )
				theList->sort();
			theList->moveSelection( eListBoxBase::dirFirst );
			theList->show();
			break;
                case eWidgetEvent::evtKey:
		{
			if ( mode == MV_MODE_VIEW )
				handled = eventHandleViewMode( (event.key)->code, (event.key)->flags );
			else
				handled = eventHandleEditMode( (event.key)->code, (event.key)->flags );
		}
		default:
			break;
	}

	return handled ? 1 : eWindow::eventHandler(event);
}

#define POS_SHIFT_AMOUNT	5
#define SIZE_SHIFT_AMOUNT	5


int ProgramList::eventHandleEditMode( int rc, int keyState )
{
        if ( keyState == KEY_STATE_UP )
		return 0;
	int handled = 1;
	switch ( rc ) {
		case EDIT_SWAP_FOCUS:
			if ( mode == MV_MODE_EDIT )
				mode = MV_MODE_EDIT_IND;
			else if ( mode == MV_MODE_EDIT_IND ) {
				conf.feat.showStatusBar = true;
				setSbarGeom();
				sBarP->show();
				mode = MV_MODE_EDIT_SBAR;
			}
			else
				mode = MV_MODE_EDIT;
			break;
		case EDIT_SHOW_HELP:
                        showInfoBox( this, strProgramListEditHelpTitle, strProgramListEditHelp );
                        break;
		case EDIT_FINISHED:
			if ( keyState == KEY_STATE_DOWN ) {
				mode = MV_MODE_VIEW;
				editIndP->hide();
				showHideAll( true );
			}
                        break;
		case EDIT_EXIT:
			close(rc);
                        break;
                case EDIT_SHOW_MENU:
			close( VIEW_SHOW_MENU ); 
                        break;
		case EDIT_DEC_X:
			changeValue( &(conf.geom.topLeftX), &(conf.geom.statusBarX), &(conf.geom.timeLabelX), -POS_SHIFT_AMOUNT );
			break;
		case EDIT_INC_X:
			changeValue( &(conf.geom.topLeftX), &(conf.geom.statusBarX), &(conf.geom.timeLabelX), +POS_SHIFT_AMOUNT );
			break;
		case EDIT_DEC_Y:
			changeValue( &(conf.geom.topLeftY), &(conf.geom.statusBarY), &(conf.geom.timeLabelY), -POS_SHIFT_AMOUNT );
			break;
		case EDIT_INC_Y:
			changeValue( &(conf.geom.topLeftY), &(conf.geom.statusBarY), &(conf.geom.timeLabelY), +POS_SHIFT_AMOUNT );
			break;
		case EDIT_DEC_WIDTH:
			changeValue( &(conf.geom.widthPixels), &(conf.geom.statusBarWidthPixels), NULL, -POS_SHIFT_AMOUNT );
			break;
		case EDIT_INC_WIDTH:
			changeValue( &(conf.geom.widthPixels), &(conf.geom.statusBarWidthPixels), NULL, +POS_SHIFT_AMOUNT );
			break;
		case EDIT_DEC_HEIGHT:
			changeValue( &(conf.geom.heightPixels), &(conf.geom.statusBarHeightPixels), NULL, -POS_SHIFT_AMOUNT );
			break;
		case EDIT_INC_HEIGHT:
			changeValue( &(conf.geom.heightPixels), &(conf.geom.statusBarHeightPixels), NULL, +POS_SHIFT_AMOUNT );
			break;
	 	default:
			handled = 0;
	}
	return handled;
}

void ProgramList::changeValue( int *mainValue, int *sBarValue, int *indValue, int amount )
{
	if ( mode == MV_MODE_EDIT ) {
		*mainValue += amount;
		hide();
		setGeom();
		show();
	}
	else if ( mode == MV_MODE_EDIT_SBAR ) {
		*sBarValue += amount;
		sBarP->hide();
		setSbarGeom();
		sBarP->show();
	}
	else if ( indValue != NULL ) {
		timeLabelP->hide();
		*indValue += amount;
		timeLabelP->move( ePoint( conf.geom.timeLabelX, conf.geom.timeLabelY ) );
		timeLabelP->show();
	}
}

void ProgramList::setGeom( void )
{
	setWidgetGeom( this, 
		conf.geom.topLeftX, conf.geom.topLeftY,
		conf.geom.widthPixels, conf.geom.heightPixels
	);

	setWidgetGeom( theList,
		0, 0,
		clientrect.width(), clientrect.height() 
	);
}

void ProgramList::setSbarGeom( void )
{
	setWidgetGeom( sBarP, 
		conf.geom.statusBarX, conf.geom.statusBarY,
		conf.geom.statusBarWidthPixels, conf.geom.statusBarHeightPixels
	);
}


int ProgramList::eventHandleViewMode( int rc, int keyState )
{
	int handled = 0;
	if ( 
		hiddenFlag &&
		( keyState == KEY_STATE_DOWN ) &&
		( rc != RC_VOLUP ) &&
		( rc != RC_MUTE ) &&
		( rc != RC_VOLDN )
	) {
		if ( rc == VIEW_SHOW_INFO )
			/*emit*/ showInfoHidden();
/*		if (  
			( rc == RC_BOUQUP ) ||
              		( rc == RC_UP ) ||
                        ( rc == RC_BOUQDN ) ||
                        ( rc == RC_DN ) ||
        		( rc == RC_LEFT ) ||
        		( rc == RC_RIGHT )
		)
			emit gotHiddenKey( rc );
		else
*/
			doHideShow();
		handled = 1;
	}
        else if ( keyState == KEY_STATE_DOWN ) {
		handled = 1;
		switch ( rc ) {
			case VIEW_SHOW_HELP:
				showInfoBox( this, strProgramListHelpTitle, strProgramListHelp );
				break;
			case VIEW_SHOW_TIMER_LIST:
				showHideAll( false, true );
				showTimerListWindow();
				showHideAll( true, true );
				break;
			case VIEW_HIDE_WINDOW:
				doHideShow();
				break;
			case VIEW_DAY_LEFT:
			case VIEW_DAY_RIGHT:
			case VIEW_EXIT:
			case VIEW_SWITCH_1:
			case VIEW_SWITCH_2:
			case VIEW_SWITCH_3:
			case VIEW_SWITCH_4:
			case VIEW_SWITCH_5:
			case VIEW_SWITCH_6:
			case VIEW_SWITCH_7:
			case VIEW_SWITCH_8:
			case VIEW_SWITCH_9:
			case VIEW_SHOW_MENU:
			case VIEW_BOUQUET_SELECTOR:
			case VIEW_DAY_CENTRE:
			case VIEW_PICKER_UP:
			case VIEW_PICKER_DN:
				close( rc ); 
				break;
			default:
				handled = 0;
		}

		// Events that rely on a valid current list entry

		if ( ! handled ) {
			ProgramListEntry *ep = theList->getCurrent();
			if ( ep ) {
				Channel *cp = ep->getChannelP();
				Program *pp = ep->getProgramP();
				int timerMode = -1;
				switch ( rc ) {
					case VIEW_SHOW_INFO:
						showDescr( showDescrWindow );
						handled = 1;
						break;
					case VIEW_SHOW_TIMER_EDIT_REC:
						timerMode = timerEditRecord;
						break;
					case VIEW_SHOW_TIMER_EDIT_NGRAB:
						timerMode = timerEditNGRAB;
						break;
					case VIEW_SHOW_TIMER_EDIT_DELETE:
						timerMode = timerEditDelete;
						break;
					case VIEW_SHOW_TIMER_EDIT_SWITCH:
						timerMode = timerEditSwitch;
						break;
					case VIEW_ADD_FAVOURITE:
						addFavouriteHandle( cp, pp );
						break;
				}
				if ( timerMode != -1 ) {
					handled = 1;
					showHideAll( false, true );
					showTimerEditWindow( 
						this, timerMode,
						cp, pp,
						timerPreOffset,
						timerPostOffset
					);
					showHideAll( true, true );
				}
			}
		}
	}
	return handled;
}
