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

#ifndef __DETAILCACHE_H__
#define __DETAILCACHE_H__

#include <stdio.h>
#include "files.h"
#include "util.h"
#include "enigmacache.h"
#include "channel.h"

class Channel;

class DetailCache {

	EnigmaCache 	*cacheP;
	FILE           *dataP, *headP;

	eString        dataPath, headPath;

	long           currentDataOffset;

	bool finalizedFlag;

	bool openFiles( const char *mode );
public:

	DetailCache( char *userLangPref );
	~DetailCache();

	bool init( char *path );
	bool finalize( void );
	bool getDescription( Channel *cp, Program *pp, char *buffer );
	bool add( const char * desc );
	void empty( void );
};

#endif
