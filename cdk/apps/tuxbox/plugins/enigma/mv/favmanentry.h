#ifndef __FAVMANENTRY_H__
#define __FAVMANENTRY_H__

#include <lib/gui/listbox.h>
//#include <lib/gdi/font.h>

#include "favedit.h"
#include "util.h"
#include "presentationmenu.h"

class FavManEntry : public eListBoxEntry
{
	friend class eListBox<FavManEntry>;
	struct Favourite *favP;
	eTextPara *para;
	gFont font;
	int yOffs;
protected:
	static int getEntryHeight( void );
	
	const eString & redraw(
		gPainter *rc, const eRect& rect,
		gColor coActiveB, gColor coActiveF,
		gColor coNormalB, gColor coNormalF,
		int state
	);
public:
	FavManEntry( 
		eListBox<FavManEntry> *listBoxP, 
		struct Favourite *favP
	);
	~FavManEntry();
	struct Favourite *getFavouriteP( void );
};

#endif
