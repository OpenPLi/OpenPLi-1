#include "favmanentry.h"

FavManEntry::FavManEntry( eListBox<FavManEntry> *listBoxP,
                struct Favourite *favP
        ) : 
	eListBoxEntry( (eListBox<eListBoxEntry> *) listBoxP), favP(favP), para(NULL)
{
	font = getNamedFont( "epg.title" );
}

int FavManEntry::getEntryHeight( void )
{
	return 30;
}

const eString &FavManEntry::redraw(
	gPainter *rc, const eRect& rect, 
	gColor coActiveB, gColor coActiveF, 
	gColor coNormalB, gColor coNormalF, 
	int state 
) {
	drawEntryRect( rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );

	if ( ! para ) {
		para = makeNewTextPara(
			(
				((favP->title).length() > 0 )  ?
				favP->title : favP->descr
			),
			rect.left(), 0, rect.width(), rect.height(),
			font,
			eTextPara::dirCenter
		);
		yOffs = ((rect.height() - para->getBoundBox().height()) / 2) - para->getBoundBox().top();
        }
        rc->renderPara(*para, ePoint(0, rect.top() + yOffs ) );

	return favP->title;
}

FavManEntry::~FavManEntry( )
{
	delete favP;
	if ( para )
		para->destroy();
}

struct Favourite *FavManEntry::getFavouriteP( void )
{
	return favP;
}

