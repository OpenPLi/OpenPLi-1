/***************************************************************/
/*
    * Copyright (C) 2004 Lee Wilmot <lee@dinglisch.net>

    This file is part of MV.

    MV is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    MV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You can find a copy of the GNU General Public License in the
    file gpl.txt distributed with this file.
*/

#include "timepicker.h"

eString labelStrings[NO_LABELS] = {
        "9am", "12am", "3pm", "6pm", "9pm", "11pm"
};

time_t secondOffsets[NO_LABELS] = {
        ( 3600 * 9 ), ( 3600 * 12 ), ( 3600 * 15 ),
        ( 3600 * 18 ), ( 3600 * 21 ), ( 3600 * 23 )
};

TimePicker::TimePicker( eWidget * parent )
	: eWidget( parent, 0 ), currentLabelNo( 0 ), cursorLabelNo(-1)
{

	for ( int labelNo = 0; labelNo < NO_LABELS; labelNo++ ) {
                labels[labelNo] =  new eLabel( this, eLabel::flagVCenter );
		labels[labelNo]->setAlign( eTextPara::dirCenter );
		labels[labelNo]->setFont( getNamedFont( TIMEPICKER_FONT ) );
		labels[labelNo]->setText( labelStrings[labelNo] );
                setLabel( labelNo );
	}
}

void TimePicker::redoGeom( int width, int height )
{
	resize( eSize( width, height ) );

        int ypos = 0;
        int labelWidth = clientrect.width();
        int labelHeight = clientrect.height() / NO_LABELS;

	for ( int labelNo = 0; labelNo < NO_LABELS; labelNo++ ) {
		labels[labelNo]->move( ePoint( 0, ypos ) );
		labels[labelNo]->resize( eSize( labelWidth, labelHeight ) );
		ypos += labelHeight;
	}
}

void TimePicker::shiftCursorLabel( int posNeg )
{
	int oldCursorLabelNo = cursorLabelNo;
	if ( posNeg > 0 )
		cursorLabelNo = ( cursorLabelNo + 1 ) % NO_LABELS;
	else if ( cursorLabelNo == 0 )
		cursorLabelNo = NO_LABELS - 1;
	else
		cursorLabelNo--;
	
        setLabel( oldCursorLabelNo );
        setLabel( cursorLabelNo );
}

void TimePicker::activate( void )
{
	cursorLabelNo = currentLabelNo;

	setLabel( cursorLabelNo );
}

time_t TimePicker::accept( void )
{
	int oldCurrentLabelNo = currentLabelNo;
	currentLabelNo = cursorLabelNo;
	cursorLabelNo = -1;
	setLabel( oldCurrentLabelNo );
	setLabel( currentLabelNo );

	return secondOffsets[currentLabelNo];
}

void TimePicker::reject( void )
{
	int oldCursorLabelNo = cursorLabelNo;
	cursorLabelNo = -1;
	setLabel( oldCursorLabelNo );
}

void TimePicker::setLabel( int labelNo )
{
	int newColourOption;
        if ( labelNo == cursorLabelNo )
                newColourOption = PM_COLOUR_STD_DGREEN;
        else if ( labelNo == currentLabelNo )
                newColourOption = PM_COLOUR_STD_DRED;
        else
                newColourOption = PM_COLOUR_CONTENT;

	gColor newColour = colourOptionToColour( newColourOption );

	labels[labelNo]->setBackgroundColor( newColour );
}

void TimePicker::reset( time_t nowOffset )
{
	int oldCurrentLabelNo = currentLabelNo;
        currentLabelNo = 0;

	// Set current to first before now offset

	for ( int labelNo = ( NO_LABELS - 1 ); labelNo >= 0; labelNo-- ) {
		if ( nowOffset > secondOffsets[labelNo] ) {
			currentLabelNo = labelNo;
			break;
		}
	}

        setLabel( oldCurrentLabelNo );
        setLabel( currentLabelNo );

}
