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

#include "extinfo.h"

ExtInfo::ExtInfo( 
	const eString &title, char *desc, 
	struct IMDBRecord *imdbRecP,
	gPixmap *starImageP, gColor filmColour
) : eWindow( 0 ), starImageP( starImageP ), noStars(0)
{
	setText( title );

	setWidgetGeom( this,
		EXTINFO_X, EXTINFO_Y,
		EXTINFO_WIDTH, EXTINFO_HEIGHT
	);
	setFont( getNamedFontAndSize( EXTINFO_FONT, EXTINFO_FONTSIZE ));
	scrollbar = new eProgress(this);
        scrollbar->setStart(0);
        scrollbar->setPerc(100);

	eString descrString( desc );

	int yPos = START_Y;
	
	// All IMDB-specific stuff done here

	if ( imdbRecP != NULL ) {
		if ( 
			( imdbRecP->rating > 0 ) &&
			( starImageP != NULL )
		) {
			noStars = (int) ( ( imdbRecP->rating + 1 ) / 2 );	
			if ( noStars > 5 )
				noStars = 5;
		}

		if ( (imdbRecP->director).length() > 1 ) {
			gColor backColour = getNamedColour( DIRECTOR_BG_COLOUR );
			makeNewLabel(
				this, (eString) imdbRecP->director, 
				MARGIN, yPos, DIRECTOR_WIDTH, DIRECTOR_HEIGHT, 
				eLabel::flagVCenter || RS_FADE,
					&backColour
			);	
			yPos += DIRECTOR_HEIGHT + 5;
		}
		else if ( noStars > 0 )
			yPos += DIRECTOR_HEIGHT + 5;

		// Draw genre
		if ( (imdbRecP->genre).length() > 1 ) {
			makeNewLabel(
				this, imdbRecP->genre,
				MARGIN, yPos, GENRE_WIDTH, GENRE_HEIGHT,
				eLabel::flagVCenter || RS_FADE,
				&filmColour
			);	
			yPos += GENRE_HEIGHT + 5;
		}

		// Draw cast
		// Scrolls with descr
		if ( (imdbRecP->cast).length() > 1 )
			descrString = imdbRecP->cast + "\n\n" + descrString;

		// If we only have stars, don't obscure with description
		if ( yPos == START_Y && noStars > 0 )
			yPos = starImageP->y;

		yPos += 5;
	}

	// Everything below here is the normal description box

	setWidgetGeom( scrollbar,
		clientrect.width() - SCROLLBAR_WIDTH, yPos,
		SCROLLBAR_WIDTH, clientrect.height() - BOTTOM_ROW_HEIGHT - yPos
	);

	descrWidget = makeNewLabel( this, "", 
		MARGIN, yPos,
		clientrect.width() - SCROLLBAR_WIDTH - MARGIN, 
		clientrect.height() - BOTTOM_ROW_HEIGHT - yPos
	);

        descrLabel = makeNewLabel(
			descrWidget, descrString,
                        0,0, 10, 10,
			RS_WRAP
        );

        float lineheight=fontRenderClass::getInstance()->getLineHeight( descrLabel->getFont() );
        int lines = descrWidget->getSize().height() / (int)lineheight;
	int newheight;
	if (lineheight > ((int)lineheight)+.5)
          newheight = lines * (int)lineheight+1 ;
	else
          newheight = lines * (int)lineheight ;
        descrWidget->resize( eSize( descrWidget->getSize().width(), newheight + (int)lineheight/6 ) );
        descrLabel->resize( 
		eSize(
			descrWidget->getSize().width(), 
			descrWidget->getSize().height() * 4
		 )
	);

        eButton *ok = makeOKButton( this );
        CONNECT(ok->selected, eWidget::accept);

	updateScrollbar();
}

void ExtInfo::updateScrollbar()
{
        total = descrWidget->getSize().height();
        int pages=1;
        while( total < descrLabel->getExtend().height() ) {
                total += descrWidget->getSize().height();
                pages++;
        }

        int start =- ( descrLabel->getPosition().y() * 100 ) /total;
        int vis = descrWidget->getSize().height()*100/total;
        scrollbar->setParams(start, vis);
        if (pages == 1)
                total = 0;
}

int ExtInfo::eventHandler(const eWidgetEvent &event)
{
	switch (event.type) {
        	case eWidgetEvent::evtKey:
			if ( (event.key)->flags == KEY_STATE_DOWN ) {
				switch ( (event.key)->code  ) {
					case VIEW_SHOW_INFO:
						close( VIEW_SHOW_INFO );
						break;
					case EXT_INFO_UP:
						handleUp();
						break;
					case EXT_INFO_DN:
						handleDown();
						break;
				}
			}
			break;
		default:
			break;
	}

	return eWindow::eventHandler(event);
}

void ExtInfo::handleDown( void )
{
	ePoint curPos = descrLabel->getPosition();
	if ( 
		total &&
		(total - descrWidget->getSize().height() ) >= abs( curPos.y() - descrWidget->getSize().height() ) 
	) {
		descrLabel->move( 
			ePoint( 
				curPos.x(), 
				curPos.y() - descrWidget->getSize().height() 
			)
		);
		updateScrollbar();
	}
}

void ExtInfo::handleUp( void )
{
	ePoint curPos = descrLabel->getPosition();
	if ( 
		total && 
		( curPos.y() < 0 )
	) {
		descrLabel->move( 
			ePoint( 
				curPos.x(), 
				curPos.y() + descrWidget->getSize().height() 
			) 
		);
		updateScrollbar();
	}
}

void ExtInfo::redrawWidget(gPainter *d, const eRect &area)
{
        eWindow::redrawWidget( d, area );

	// Draw in the stars

	if ( 
		( noStars > 0 ) &&
		( starImageP != NULL )
	) {
		int starWidth = starImageP->x + STAR_GAP;
		int starsWidth = noStars * starWidth;
		int xPos = clientrect.width() - starsWidth + ( STAR_GAP * 2 ) - 5;
		for ( int starNo = 0; starNo < noStars; starNo++ ) {
			d->blit( *starImageP, ePoint( xPos, clientrect.top() + STARS_Y ), eRect(), 0 );	
			xPos += starWidth;
		}
	}
}

void doExtInfo( Program *pp, char *descr, gColor filmColour )
{
        ExtInfo ei( 		
		pp->getStatusBarText( sBarFlagProgramName | sBarFlagTime | sBarFlagEndTime ), 
		descr,
		NULL, NULL, filmColour
	);
        int result = showExecHide( &ei );
        if ( result == VIEW_SHOW_INFO ) {
//		if ( ! pp->isFilm() ) 
//			flashMessage( getStr( strIMDBNotFilm ) );
//		else
		if ( ! haveNetwork() )
			flashMessage( getStr( strErrNoNetwork ) );
		else {
			// Look for a year in title/descr, pass hint to IMDB
			int yearHint = findYearInString( descr );
			//int yearHint = 1981;
			
			eString baseURL( IMDB_DOWNLOAD_BASE_URL );
			gPixmap *iconP = getNamedPixmapP( STAR_ICON_NAME );
			IMDB imdb( baseURL, pp->getTitle(), yearHint, iconP, filmColour);
	        	imdb.run();
		}
        }
}
