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

#include "daybar.h"

DayBar::DayBar( eWidget * parent ) :
	eWidget( parent, 0 ), currentLabelNo( 0 ), cursorLabelNo(-1), activeFlag( false )
{
	time_t nowTime = getLoadTime();
	struct tm startTimeStruct = *localtime( &nowTime );

	recalcTimes();

	int labelDay = startTimeStruct.tm_wday;
	int labelNo = 0;

	while ( labelNo < 7 ) {
		dayLabels[labelNo] =  new eLabel( this, eLabel::flagVCenter );
		dayLabels[labelNo]->setText( getStr( labelDay + strDaybarFirst ) );
		dayLabels[labelNo]->setAlign( eTextPara::dirCenter );
		dayLabels[labelNo]->setFont( getNamedFont( DAYBAR_FONT ) );
		setLabel( labelNo );

		labelDay = ( labelDay + 1 ) % 7;
		labelNo++;
	}
	
	headerLabelP = makeNewLabel( 
		this, 
		"",
		0, 0, 0, 0, 
		eLabel::flagVCenter
	);
	headerLabelP->setFont( getNamedFont( DAYBAR_FONT ) );
	headerLabelP->setAlign( eTextPara::dirLeft );

	timePickerP = new TimePicker( this );
	timePickerP->reset( offsetFromDayStart );
	timePickerP->hide();

	setHeaderLabel();
}

void DayBar::recalcTimes( void )
{
	time_t nowTime = getLoadTime();
	struct tm startTimeStruct = *localtime( &nowTime );

	offsetFromDayStart = 
		( startTimeStruct.tm_hour * 3600 ) +
		( startTimeStruct.tm_min * 60 ) +
		startTimeStruct.tm_sec;

	startTimeStruct.tm_hour = 0;
	startTimeStruct.tm_min = 0;
	startTimeStruct.tm_sec = 0;

	secondsStartOfFirstDay = mktime( &startTimeStruct );
}

void DayBar::redoGeom( int x, int y, int width, int height, int headerWidth, int timePickerHeight )
{
	setWidgetGeom( this, x, y, width, (height+timePickerHeight ) );

	int labelWidth;
	if ( headerWidth == 0 ) {
		labelWidth = ( clientrect.width() / 8 );
		headerWidth = labelWidth;
	}
	else
		labelWidth = ( ( clientrect.width() - headerWidth )/ 7 );

	headerLabelP->resize( eSize( headerWidth, height ) );

	int xpos = headerWidth; 

	for ( int labelNo = 0; labelNo < 7; labelNo++ ) {
		dayLabels[labelNo]->move( ePoint( xpos, 0 ) );
		dayLabels[labelNo]->resize( eSize( labelWidth, height ) );
		xpos += labelWidth;
	}
	timePickerP->redoGeom( labelWidth, timePickerHeight );
}

void DayBar::moveTimePicker( int toLabelNo )
{
	timePickerP->hide();
	timePickerP->move( 
		ePoint( 
			dayLabels[toLabelNo]->getPosition().x(),
			dayLabels[toLabelNo]->height()
		) 
	);
	timePickerP->show();
}

void DayBar::setLabel( int labelNo )
{
	int newColourOption;
	if ( labelNo == cursorLabelNo )
		newColourOption = PM_COLOUR_STD_DGREEN;
	else if ( labelNo == currentLabelNo )
		newColourOption = PM_COLOUR_STD_DRED;
	else
		newColourOption = PM_COLOUR_CONTENT;

	gColor newColour = colourOptionToColour( newColourOption );

	dayLabels[labelNo]->setBackgroundColor( newColour );
}

void DayBar::doTimePickerFunc( int func )
{
	switch ( func ) {
		case pfuncShow:
			timePickerP->activate();
			moveTimePicker( cursorLabelNo );
			break;
		case pfuncShiftUp:
			timePickerP->shiftCursorLabel( -1 );
			break;
		case pfuncShiftDown:
			timePickerP->shiftCursorLabel( +1 );
			break;
		case pfuncRedraw:
			timePickerP->redraw();
			break;
		default:
			break;
	}
}

void DayBar::shiftCursorLabel( int leftRight )
{
	int oldCursorLabelNo = cursorLabelNo;
	if ( leftRight > 0 )
		cursorLabelNo = ( cursorLabelNo + 1 ) % 7;
	else if ( cursorLabelNo == 0 )
		cursorLabelNo = 6;
	else
		cursorLabelNo--;

	setLabel( oldCursorLabelNo );
	if ( timePickerP->isVisible() )
		moveTimePicker( cursorLabelNo );
	setLabel( cursorLabelNo );
	setHeaderLabel();
}

void DayBar::setHeaderLabel( void )
{
	int lno = ( cursorLabelNo == -1 ) ? currentLabelNo : cursorLabelNo;
	time_t selectedTime = secondsStartOfFirstDay + ( (time_t) SECONDS_IN_A_DAY * (time_t) lno );

	struct tm startTimeStruct = *localtime( &selectedTime );
	headerLabelP->setText( eString().sprintf( "%.2d/%.2d", startTimeStruct.tm_mday, startTimeStruct.tm_mon + 1));
}

void DayBar::reset( void )
{
	int oldCurrentLabelNo = currentLabelNo;
	currentLabelNo = 0;

	setLabel( oldCurrentLabelNo );
	setLabel( currentLabelNo );

	recalcTimes();

	timePickerP->reset( offsetFromDayStart );
}

void DayBar::activate( void )
{
	activeFlag = true;
	cursorLabelNo = currentLabelNo;
	setLabel( cursorLabelNo );

	if ( ! isVisible() )
		show();
}

time_t DayBar::accept( bool hideFlag )
{
	activeFlag = false;

	int oldCurrentLabelNo = currentLabelNo;
	currentLabelNo = cursorLabelNo;
	setLabel( oldCurrentLabelNo );
	cursorLabelNo = -1;
	setLabel( currentLabelNo );

	time_t selectedTime = secondsStartOfFirstDay + ( (time_t) SECONDS_IN_A_DAY * (time_t) currentLabelNo );

	if ( timePickerP->isVisible() ) {
		offsetFromDayStart = timePickerP->accept();
		timePickerP->hide();
	}

	selectedTime += offsetFromDayStart;

	if ( hideFlag )
		hide();

	return selectedTime;
}

void DayBar::reject( bool hideFlag )
{
	int oldCursorLabelNo = cursorLabelNo;
	cursorLabelNo = -1;
	setLabel( oldCursorLabelNo );
	activeFlag = false;
	setHeaderLabel();

	if ( timePickerP->isVisible() ) {
		timePickerP->reject();
		timePickerP->hide();
	}

	if ( hideFlag )
		hide();
}

bool DayBar::isActive( void )
{
	return activeFlag;
}

bool DayBar::timePickerIsActive( void )
{
	return timePickerP->isVisible();
}

time_t DayBar::getStartTime( void )
{
	return 0;
}
