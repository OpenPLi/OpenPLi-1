#ifndef __IMDBLISTENTRY_H__
#define __IMDBLISTENTRY_H__

#include <lib/gui/listbox.h>
#include <lib/base/estring.h>

#include "util.h"
#include "text.h"

struct IMDBRecord {
        eString title, plot, director, cast, genre;
	float rating;
	int year;
};

#define IMDBLE_TITLE_X		0
#define IMDBLE_YEAR_WIDTH	50
#define IMDBLE_YEAR_X		rect.width() - IMDBLE_YEAR_WIDTH
#define IMDBLE_DIRECTOR_WIDTH	150
#define IMDBLE_DIRECTOR_X	IMDBLE_YEAR_X - IMDBLE_DIRECTOR_WIDTH - 5
#define IMDBLE_TITLE_WIDTH	IMDBLE_DIRECTOR_X - 5
#define IMDBLE_DIRECTOR_COLOUR	"blue_nontrans"

class IMDBListEntry : public eListBoxEntry
{
	friend class eListBox<IMDBListEntry>;
	struct IMDBRecord rec;
	eTextPara *paraTitle;
	eTextPara *paraDirector;
	eTextPara *paraYear;
	static gFont titleFont;
protected:
	static int getEntryHeight( void );
	
	const eString& IMDBListEntry::redraw(
		gPainter *rc, const eRect& rect,
		gColor coActiveB, gColor coActiveF,
		gColor coNormalB, gColor coNormalF,
		int state
	);
public:
	~IMDBListEntry();
	IMDBListEntry( 
		eListBox<IMDBListEntry> *listBoxP, 
		struct IMDBRecord rec
	);

	const eString & getTitle( void );
	const eString & getPlot( void );
	int getYear( void );
	struct IMDBRecord *getRecordP( void );
};

#endif
