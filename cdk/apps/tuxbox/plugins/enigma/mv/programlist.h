#ifndef __PROGRAMLIST_H__
#define __PROGRAMLIST_H__

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>

#include "defs.h"
#include "conf.h"
#include "program.h"
#include "channel.h"
#include "enigmautil.h"
#include "proglistentry.h"
#include "util.h"
#include "proglistdef.h"
#include "keys.h"
#include "extinfo.h"
#include "favmanager.h"

class ProgramList : public eWindow {
	struct ViewConf &conf;
	int mode;
	eLabel *editIndP;

	eListBox<ProgramListEntry> *	theList;
	const time_t &timerPreOffset;
	const time_t &timerPostOffset;

	eStatusBar *sBarP;

	eLabel *timeLabelP;
	eLabel *hiddenIndP;

	bool hiddenFlag;

	void doHideShow();
	void selected( ProgramListEntry *entryP );
	void selectionChanged( ProgramListEntry *entryP );
	int eventHandler(const eWidgetEvent &event);

	int eventHandleViewMode( int rc, int keyState );
	int eventHandleEditMode( int rc, int keyState );
	void changeValue( int *toChange, int *toChangeS, int *toChangeI, int amount );

	void showDescr( int mode );
	void setSbarGeom( void );
	void showHideSbar( bool showFlag );
	void showHideAll( bool showFlag, bool emitFlag = false );
	void addFavouriteHandle( Channel *cp, Program *pp );
	void setGeom( void );
	bool isCronSorted(  void ) { return ( conf.feat.sortType == sortTypeListStartTime ); }
	void setListEntryFonts( void );

public:
	ProgramList( struct ViewConf &conf, const time_t &timerPre, const time_t &timerPost, eStatusBar *sp, eLabel *timeLabelP, eLabel *hiddenIndP
		,eLabel *editIndP
	);
	void setMode( int mode );
	void addEntry( Channel *cp, Program *toAdd );
	int noEntries( void );

	void reset( const eString &titleSpec );
		
	Signal1<void, bool>	hiding;
	Signal0<void>	showInfoHidden;
};

#endif

