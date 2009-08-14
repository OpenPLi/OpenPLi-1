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

#include "program.h"

Program::Program()
{
}

void Program::reset( 
	unsigned int newID, eString name, 
	time_t newStartTime, time_t duration,
	bool isFilm, bool isFavourite, bool isFromCache,short newIdxFic,long newOffset
) {
	id = newID;
	startTime = newStartTime;
	endTime = startTime + duration;
	idxFic=newIdxFic;
	offset=newOffset;

	// Kiosk: remove annoying 5 digits and '-' prefix

	int length = name.length();
	if ( 
		( length > 6 ) &&
		( isNumericString( (char *) name.left(5).c_str() ) ) &&
		( name.mid( 5, 1 ) == eString( "-" ) )
	)
		this->name = name.right( length - 6 );
	else
		this->name = name;

	flags = isFilm ? isFilmFlag : 0;
	if ( isFavourite )
		flags |= isFavouriteFlag;

	if ( isFromCache )
		flags |= isFromCacheFlag;
}

bool Program::timeOverlaps( time_t otherTime )
{
	return (
		( getStartTime() <= otherTime ) &&
		( getEndTime() >= otherTime )
	);
}

bool Program::isSimilar( time_t otherStart, time_t otherEnd)
{
	time_t otherDuration = otherEnd - otherStart;

	time_t startDiff = abs( getStartTime() - otherStart );

	time_t endDiff = abs( getEndTime() - otherEnd );

	float totalDiff = (float) ( startDiff + endDiff );
	float totalDuration = (float) ( getDuration() + otherDuration );

        return ( ( totalDiff / totalDuration ) < MIN_DIFF_FRACTION_DIFFERENT_PROGRAMS );
}

void Program::setEndTime( time_t newEndTime )
{
	endTime = newEndTime;
}

/*
int Program::getStartHour( void )
{
	tm *begin = localtime( &startTime );

        return begin->tm_hour;
}
*/

void Program::setVisible( bool toSet ) {
	if ( toSet )
		flags = ( flags | isVisibleFlag );	
	else {
		char mask = isVisibleFlag;
		flags = ( flags & ~mask );
	}
}

bool Program::isVisible( void )
{
	return ( flags & isVisibleFlag );
}

eString Program::getStatusBarText( int flags )
{
	tm *begin = localtime( &startTime );

	eString toSet;

        if ( flags & sBarFlagDate )
                toSet = toSet + eString().sprintf( "%02d/%02d ", begin->tm_mday, begin->tm_mon+1 );

        if ( flags & sBarFlagTime )
                toSet = toSet + eString().sprintf( "%02d:%02d ", begin->tm_hour, begin->tm_min );

        if ( flags & sBarFlagEndTime ) {
		tm *end = localtime( &endTime );
                toSet = toSet + eString().sprintf( "- %02d:%02d ", end->tm_hour, end->tm_min );
	}

        if ( flags & sBarFlagDuration )
		toSet = toSet + eString().sprintf( "(%dm) ", getDurationMinutes() );

        if ( flags & sBarFlagProgramName )
		toSet = toSet + name;

	return toSet;
}

eString Program::getStartTimeDiffText( void )
{
	time_t diff = startTime - getLoadTime();
	return eString().sprintf( "%dm", diff / 60 );
}

eString Program::getStartEndTimeText( bool startTimeFlag )
{
	tm *tp;

	if ( startTimeFlag ) 
		tp = localtime( &startTime );
	else
		tp = localtime( &endTime );

	return eString().sprintf( "%02d:%02d", tp->tm_hour, tp->tm_min );
}

const eString & Program::getTitle( void )
{
	return name;
}

unsigned int Program::getID( void )
{
	return id;
}

int Program::getDurationMinutes( void )
{
	return(  (int) ( getDuration() / 60 ) );
}

eString Program::getDurationMinutesText( void )
{
	return(  eString().sprintf( "%d", getDurationMinutes() ) );
}

bool Program::isFavourite( void )
{
	return flags & isFavouriteFlag;
}

bool Program::isFromCache( void )
{
	return flags & isFromCacheFlag;
}

bool Program::isFilm( void )
{
	return flags & isFilmFlag;
}

