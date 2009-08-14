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

#include "timebar.h"

TimeBarLabel::TimeBarLabel( eWidget *parent, int halfWidth ) : eLabel( parent ) , halfWidth( halfWidth )
{
}

void TimeBarLabel::redrawWidget(gPainter *d, const eRect &area)
{
	eLabel::redrawWidget( d, area );

	d->line( 
		ePoint( halfWidth, area.height() - 4 ), 
		ePoint( halfWidth, area.height() ) 
	);
}

TimeBar::TimeBar( eWidget * parent ) : 
	eWidget(parent), noLabels( 0 )
{
//	timeFont = eSkin::getActive()->queryFont("epg.time");
	timeFont = getNamedFont( "epg.time" );
	timeFont.pointSize = 14;
	timeWidth = stringWidthPixels( timeFont, "00:00" );
}

void TimeBar::rebuild( time_t startTime, time_t endTime, time_t markingPeriodSeconds )
{
	showLabels = 0;

	float pixelsPerSecond = (float) size.width() / (float) ( endTime - startTime );

	// Get the hour for the first regular lable

	time_t firstTime = startTime + markingPeriodSeconds;

	tm firstTimeTM = *localtime( &firstTime );

	if ( markingPeriodSeconds < 3600 ) {
		int markingPeriodMinutes = (int) ( markingPeriodSeconds / (time_t) 60);
		int wholeResult = firstTimeTM.tm_min /  markingPeriodMinutes;
		int diff = firstTimeTM.tm_min - ( wholeResult * markingPeriodMinutes );
		firstTimeTM.tm_min -= diff;
	}
	else
		firstTimeTM.tm_min = 0;

	firstTimeTM.tm_sec = 0;

	firstTime = mktime( &firstTimeTM );

	// Off we go..

	time_t labelStartTime = startTime;
	time_t labelEndTime = firstTime;

	int xpos, labelWidth;

	int halfTimeWidth = ( timeWidth / 2 ) + 1;

	while ( 
		( labelStartTime < endTime ) &&
		( showLabels < TIMEBAR_MAX_NO_LABELS )
	) {
		xpos = (int) ( pixelsPerSecond * (float) ( labelStartTime - startTime ) );

		// If 2nd label overlaps with first, redo 2nd
		// as first

		if ( 
			( showLabels == 1 ) &&
			( (xpos - halfTimeWidth - 6 ) < timeWidth  ) 
		)
			showLabels--;

		if ( labelEndTime > endTime )
			labelEndTime = endTime;

		labelWidth = (int) ( pixelsPerSecond * (float) ( labelEndTime - labelStartTime ) ) - 2;

		if ( ( xpos + labelWidth ) > size.width() )
			labelWidth = size.width() - xpos;

		if ( showLabels == noLabels ) {
			labels[showLabels] = new TimeBarLabel( this, halfTimeWidth );
			labels[showLabels]->setFont( timeFont );
			noLabels++;
		}
		else if ( labels[showLabels]->isVisible() )
			labels[showLabels]->hide();

		if ( labelWidth < timeWidth ) {
			labels[showLabels]->setText( "" );
		}
		else {
			tm displayTimeTM = *localtime( &labelStartTime );
			labels[showLabels]->setText( eString().sprintf( "%.2d:%.2d", displayTimeTM.tm_hour, displayTimeTM.tm_min ) );
		}

		int realxpos = xpos - halfTimeWidth;
		if ( realxpos < 0 )
			realxpos = 0;

		labels[showLabels]->move( ePoint( realxpos , 0 ) );
		labels[showLabels]->resize( eSize( labelWidth, clientrect.height() ) );

		if ( labels[showLabels]->isVisible() )
			labels[showLabels]->redraw();
		else	
			labels[showLabels]->show();
			
		labelStartTime = labelEndTime + 1;
		labelEndTime = labelStartTime + markingPeriodSeconds;
		showLabels++;
	}

	// Hide rest of labels
	for ( unsigned int labelNo = showLabels; labelNo < noLabels; labelNo++ )
		labels[labelNo]->hide();
}
