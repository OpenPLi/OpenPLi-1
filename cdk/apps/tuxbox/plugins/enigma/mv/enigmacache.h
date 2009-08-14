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

#ifndef __ENIGMACACHE_H__
#define __ENIGMACACHE_H__

#include <lib/dvb/epgcache.h>
#include <enigma.h>

#include "util.h"
#include "inputmanager.h"
#include "debug.h"

eServiceReferenceDVB getPlayingServiceRef( void );
eString getServiceName( const eServiceReferenceDVB &ref );
eString getFullServiceName( const eServiceReferenceDVB &ref );
eString buildShortServiceName( const eString &str );

class EnigmaCache : public Object
{
	char *userLangPref;

	std::list<eServiceReferenceDVB> serviceList;

	unsigned int serviceCount;

	eEPGCache *cacheP;
	
	bool langCodesMatch( char *first, char *second );
	bool parseProgramData( EITEvent *ev, struct ProgramData &p, bool descrFlag, bool contentFlag );
public:
	void addToList( const eServiceReference& ref );

	EnigmaCache( char *userLangPref, bool loadServices = false  );
	bool getProgramDescription( eServiceReferenceDVB ref, time_t startTime, char *buffer );
//	bool getProgramDescription( eString &serviceName, time_t startTime, char *buffer );
	void readProgramData( time_t from, time_t to, int maxChannels, bool includeDescr );
	void readOneChannelProgramData( bool partOfGlobal, eServiceReferenceDVB ref, time_t from, time_t to, bool includeDescr );
	bool eventIsInCache( eString serviceName, time_t startTime );
	bool isInServiceList( char *name );
	std::list<eServiceReferenceDVB> *getServiceListP( void ) { return &serviceList; }
	bool getServiceRef( eString &name, eServiceReferenceDVB *eref );
	eString nextBouquetName( bool fromStartFlag );
	eServiceReferenceDVB *EnigmaCache::nextBouquetRef( bool fromStartFlag );
//	void convertDataChannels( void );


	Signal1<void, struct ProgramData &> gotData;
	Signal1<void, struct ProgramData &> gotOneChannelData;
};

#endif
