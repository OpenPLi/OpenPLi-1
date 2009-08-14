#ifndef __PROGLISTENTRY_H__
#define __PROGLISTENTRY_H__

#include <lib/gui/listbox.h>
#include <lib/base/estring.h>
#include <timer.h>

#include "util.h"
#include "text.h"
#include "program.h"
#include "channel.h"
#include "conf.h"

#define FILM_FAV_WIDTH 8
#define CHANNEL_NAME_WIDTH 100
#define TIME_TIMER_GAP 10
#define TIMER_TITLE_GAP 10
#define TITLE_CHANNEL_GAP 10

enum {
	programListEntryFlagShowDate = 1,
	programListEntryFlagRepeatChannelName = 2,
	programListEntryFlagShowChannelIcon = 4,
	programListEntryFlagShowChannelName = 8
};

class Channel;
class ProgramListEntry : public eListBoxEntry
{
	friend class eListBox<ProgramListEntry>;
	Program *pp;
	Channel *cp;
	int flags;
	gColor playingBackColour, favouriteBackColour, filmBackColour;

	eTextPara *paraTime, *paraTitle, *paraChannel;

	int timeYOffset, titleYOffset, channelYOffset;

	static gPixmap *inTimer, *inTimerRec;
protected:
	static int getEntryHeight( void );
	
	const eString& ProgramListEntry::redraw(
		gPainter *rc, const eRect& rect,
		gColor coActiveB, gColor coActiveF,
		gColor coNormalB, gColor coNormalF,
		int state
	);
public:
	ProgramListEntry( 
		eListBox<ProgramListEntry> *listBoxP, 
		Channel *channel,
		Program *pp,
		int flags,
		gColor playingBackColour,
		gColor favouriteBackColour,
		gColor filmBackColour
	);
	~ProgramListEntry();

	// Comparison for sort
	bool operator < ( const eListBoxEntry &ref ) const {
		return ( pp->getStartTime() < (((const ProgramListEntry &)ref).pp)->getStartTime() );
	}
	Program *getProgramP( void );
	Channel *getChannelP (void );

	static gFont timeFont, titleFont, channelFont;
	static int timeXSize, dayXSize;
};

#endif
