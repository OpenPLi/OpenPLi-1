#include "imdblistentry.h"

gFont IMDBListEntry::titleFont;

IMDBListEntry::IMDBListEntry( eListBox<IMDBListEntry> *listBoxP, struct IMDBRecord rec ) :
	eListBoxEntry( (eListBox<eListBoxEntry> *) listBoxP), 
	rec(rec),
	paraTitle(NULL), paraDirector(NULL), paraYear(NULL)
{
}

IMDBListEntry::~IMDBListEntry()
{
	if ( paraTitle != NULL )
		paraTitle->destroy();
	if ( paraDirector != NULL )
		paraDirector->destroy();
	if ( paraYear != NULL )
		paraYear->destroy();
}

int IMDBListEntry::getEntryHeight( void )
{
	if ( ! titleFont.pointSize )
		titleFont = getNamedFont( "epg.title" );
	return calcFontHeight( titleFont ) + 8;
}

const eString& IMDBListEntry::redraw(
	gPainter *rc, const eRect& rect, 
	gColor coActiveB, gColor coActiveF, 
	gColor coNormalB, gColor coNormalF, 
	int state 
) {
	drawEntryRect(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );
	
        gColor thisForeColour = state ? coActiveF : coNormalF;

	int rowHeight = rect.height() - 4;

	// Do title rect
	if ( paraTitle == NULL )
		paraTitle = makeNewTextPara(
			rec.title,
       	       	        0 ,0, IMDBLE_TITLE_WIDTH, rowHeight,
			titleFont
               	);
	rc->clip( eRect( IMDBLE_TITLE_X, rect.top(), IMDBLE_TITLE_WIDTH, rowHeight ) );
       	rc->renderPara( *paraTitle, ePoint( IMDBLE_TITLE_X, rect.top() ) );
        rc->clippop();

	// Do director rect

	eRect dirRect(  IMDBLE_DIRECTOR_X, rect.top(), IMDBLE_DIRECTOR_WIDTH, rowHeight );

	// Make the background same colour as in extinfo box
	rc->setForegroundColor( getNamedColour( IMDBLE_DIRECTOR_COLOUR ) );
	rc->fill( dirRect );
	rc->setForegroundColor( thisForeColour );

	if ( rec.director.length() > 1 ) {
		if ( paraDirector == NULL ) {
			paraDirector = makeNewTextPara(
				rec.director,
       		       	        0 ,0, IMDBLE_DIRECTOR_WIDTH, rowHeight,
				titleFont
       	        	);
			rc->setBackgroundColor( getNamedColour( IMDBLE_DIRECTOR_COLOUR ) );
		}

		// Clip to avoid overlap with year
		rc->clip( eRect( IMDBLE_DIRECTOR_X, rect.top(), IMDBLE_DIRECTOR_WIDTH, rowHeight ) );
       		rc->renderPara( *paraDirector, ePoint( IMDBLE_DIRECTOR_X, rect.top() ) );
       		rc->clippop();
	}

	// Do year rect
	if ( paraYear == NULL )
		paraYear = makeNewTextPara(
			eString().sprintf( "%d", rec.year ),
       	       	        0 ,0, IMDBLE_YEAR_WIDTH, rowHeight,
			titleFont
               	);
       	rc->renderPara( *paraYear, ePoint( IMDBLE_YEAR_X, rect.top() ) );

	// No idea what gets done with the return value
	return rec.title;
}

const eString & IMDBListEntry::getTitle( void )
{
	return rec.title;
}
const eString & IMDBListEntry::getPlot( void )
{
	return rec.plot;
}
int IMDBListEntry::getYear( void )
{
	return rec.year;
}

struct IMDBRecord * IMDBListEntry::getRecordP( void )
{
	return &rec;
}
