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

#ifndef __DAYBAR_H__
#define __DAYBAR_H__

#include <lib/gui/elabel.h>
#include <lib/base/estring.h>

#include "util.h"
#include "presentationmenu.h"
#include "timepicker.h"
#include "text.h"

class DayBar : public eWidget
{
	eLabel *headerLabelP;
	eLabel *dayLabels[7];
	time_t windowRange;
	int currentLabelNo;
	int cursorLabelNo;

	bool activeFlag;

	time_t offsetFromDayStart;
	time_t secondsStartOfFirstDay;

	TimePicker *timePickerP;

	void setLabel( int labelNo );
	void moveTimePicker( int labelNo );
	void recalcTimes( void );
	void setHeaderLabel( void );
public:
	DayBar( eWidget * parent );

	void shiftCursorLabel( int leftRight );
	time_t getStartTime( void );
	void doTimePickerFunc( int func );

	bool isActive( void );
	bool timePickerIsActive( void );
	void activate( void );
	time_t accept( bool hideFlag );
	void reset( void );
	void reject( bool hideFlag );
	void redoGeom( int x, int y, int width, int height, int headerWidth, int timePickerHeight );
};

#endif
