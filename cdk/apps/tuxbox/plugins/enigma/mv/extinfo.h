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

#ifndef __EXTINFO_H__
#define __EXTINFO_H__

#include <lib/gui/ewindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/eprogress.h>
#include <lib/gdi/gpixmap.h>

#include "imdb.h"
#include "util.h"
#include "keys.h"
#include "conf.h"
#include "program.h"

#define SCROLLBAR_WIDTH		20
#define BOTTOM_ROW_HEIGHT	30

#define MARGIN			5
#define PAGE_WIDTH		clientrect.width() - ( MARGIN * 2 )
#define STAR_GAP		4
#define START_Y			5
#define STARS_Y			START_Y
#define DIRECTOR_WIDTH		PAGE_WIDTH - ( ( noStars > 0 ) ? ( 8 + ( noStars * ( starImageP->x + STAR_GAP  ) ) ) : 0 )
#define DIRECTOR_HEIGHT		( EXTINFO_FONTSIZE + 6 )
#define GENRE_HEIGHT		DIRECTOR_HEIGHT
#define GENRE_WIDTH		PAGE_WIDTH

#define DIRECTOR_BG_COLOUR	"blue_nontrans"

void doExtInfo( Program *pp, char *descr, gColor filmColour );

class ExtInfo : public eWindow
{
	int eventHandler(const eWidgetEvent &event);
	eLabel *descrLabel;
	eWidget *descrWidget;
	int total;
	gPixmap * starImageP;
	int noStars;

	eProgress *scrollbar;
	void updateScrollbar( void );
	void handleDown( void );
	void handleUp( void );

	void redrawWidget(gPainter *d, const eRect &area);
public: 
	ExtInfo( 
		const eString &title, char *desc, 
		struct IMDBRecord *imdbRec = NULL,
		gPixmap *starImageP = NULL,
		gColor filmColour = 0
	);
};

#endif
