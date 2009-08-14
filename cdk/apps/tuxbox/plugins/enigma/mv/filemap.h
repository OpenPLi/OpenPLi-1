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

#ifndef __FILEMAP_H__
#define __FILEMAP_H__

#define FILEMAP_MAX_NAME_LENGTH 2048

#include "util.h"

class FileMap
{
	eString mapPath;
	bool needWriteFlag;
protected:
	std::set<eString> inputs;
	std::multimap<eString,eString> map;
public:
	FileMap( eString path );
	~FileMap();

	bool readMapFile( void );
	bool writeMapFile( void );

	void addMappedName( char * from, char * to, bool needStore );
	void addMappedName( const eString &from, const eString &to, bool needStore );

	bool haveUnknownEntries( void );

	eString getMappedName( const eString &original );
	bool isMapped( const char * original , const char * mapped );
};

#endif
