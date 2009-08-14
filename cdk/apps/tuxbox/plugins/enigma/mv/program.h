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

#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include <lib/base/estring.h>
#include "util.h"
#include "featuremenu.h"

#define MIN_PROGRAM_LENGTH_SECONDS      	110

// NewD/OldD should be more than this
#define MAX_FRACTION_DURATION_ADJUST    	0.95
// startDiff + endDiff / dur1 + dur2
#define MIN_DIFF_FRACTION_DIFFERENT_PROGRAMS	0.25

enum {
	isVisibleFlag = 1,
	isFilmFlag = 2,
	isFavouriteFlag = 4,
	isFromCacheFlag = 8
};

class Program 
{
	unsigned int id;
	eString name;
	time_t startTime;
	time_t endTime;

	char flags;
	short idxFic;
	long offset;
public:
	Program();
	void reset( unsigned int id, eString name, time_t startTime, time_t duration, bool isFilm, bool isFavourite, bool isFromCache ,short idxFic,long offset);
	~Program() {}

	bool isSimilar( time_t startTime, time_t endTime );
	void setEndTime( time_t toSet );
	time_t getStartTime( void ) { return startTime; }
	time_t getEndTime( void ) { return endTime; }
	time_t getDuration( void ) { return endTime - startTime; }

	bool timeOverlaps( time_t otherTime );

	int getDurationMinutes( void );
	eString getDurationMinutesText( void );
	eString getStatusBarText( int flags );
	const eString & getTitle( void );
	eString getStartEndTimeText( bool startTime );
	eString getStartTimeDiffText( void );
	void setVisible( bool toSet );
	bool isVisible( void );
	bool isFavourite( void );
	bool isFromCache( void );
	unsigned int getID( void );
	bool isFilm( void );
	short getIdxFic() {return idxFic;}
	long getOffset() {return offset;}
//	int getStartHour( void );
};

#endif
