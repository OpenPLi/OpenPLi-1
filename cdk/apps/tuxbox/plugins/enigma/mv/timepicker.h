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

#ifndef __TIMEPICKER_H__
#define __TIMEPICKER_H__

#include <lib/base/estring.h>
#include <lib/gui/elabel.h>

#include "presentationmenu.h"

#define NO_LABELS  6

enum pickerFunc {
	pfuncShow = 1,
	pfuncShiftUp,
	pfuncShiftDown,
	pfuncRedraw
};

class TimePicker : public eWidget
{
	eLabel *labels[NO_LABELS];
	int currentLabelNo;
	int cursorLabelNo;
public:
	TimePicker( eWidget * parent );

	void setLabel( int labelNo );
	void shiftCursorLabel( int posNeg );
	time_t accept( void );
	void reject( void );
	void activate( void );
	void reset( time_t nowTimeDayOffset );
	void redoGeom( int width, int height );
};


#endif

