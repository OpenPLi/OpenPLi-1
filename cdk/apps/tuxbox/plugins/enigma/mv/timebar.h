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

#ifndef __TIMEBAR_H__
#define __TIMEBAR_H__

#include <lib/gui/elabel.h>
#include <lib/gui/ewidget.h>
#include <lib/base/estring.h>

#include "util.h"

#define TIMEBAR_MAX_NO_LABELS 10

class TimeBarLabel : public eLabel {
	
	int halfWidth;
	void redrawWidget(gPainter *d, const eRect &area);
public:
	TimeBarLabel( eWidget *parent, int halfWidth);
};

class TimeBar : public eWidget {

	TimeBarLabel *labels[TIMEBAR_MAX_NO_LABELS];
	unsigned int noLabels;
	unsigned int showLabels;
	int timeWidth;
	gFont timeFont;

public:
	TimeBar( eWidget *parent );
	void rebuild( time_t startTime, time_t endTime, time_t markingPeriodSeconds );
};

#endif
