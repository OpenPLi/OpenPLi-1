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

#include "filemap.h"

FileMap::~FileMap()
{
	writeMapFile();
}

FileMap::FileMap( eString path ) : 
	 mapPath( path ), 
	needWriteFlag( false )
{
	readMapFile();
}

bool FileMap::readMapFile( void )
{
	char fromBuffer[FILEMAP_MAX_NAME_LENGTH+1];
	char toBuffer[FILEMAP_MAX_NAME_LENGTH+1];

	FILE *fp = fopen( mapPath.c_str(), "r" );

	if ( ! fp ) {
		mylog( eString().sprintf( "FileMap: failed to read %s", mapPath.c_str() ) );
		return false;
	}

	bool doneFlag = false;

	while ( ! doneFlag ) {
		if ( 
			fgets( fromBuffer, FILEMAP_MAX_NAME_LENGTH, fp ) &&
			fgets( toBuffer, FILEMAP_MAX_NAME_LENGTH, fp ) 
		) {
			removeTrailingNewlines( fromBuffer );
			removeTrailingNewlines( toBuffer );

			addMappedName( fromBuffer, toBuffer, false );
		}
		else
			doneFlag = true;
	}

	fclose( fp );

	return true;
}

bool FileMap::writeMapFile( void )
{
	if ( ! needWriteFlag )
		return true;

	FILE *fp = fopen( mapPath.c_str(), "w" );

	if ( ! fp ) {
		mylog( eString().sprintf( "FileMap: failed to write %s", mapPath.c_str() ) );
		return false;
	}

	std::multimap<eString, eString>::iterator it = map.begin();
	while ( it != map.end() ) {
		// Don't store the unknown ones nor identicals nor ...+1
		if ( it->first != eString("UNKNOWN") 
		     && strcasecmp(it->first.c_str(),it->second.c_str())
		     && (  it->first[it->first.length()-2]!='+'
			|| it->first[it->first.length()-1]!='1'
			|| it->first.length() !=it->second.length()+2
			|| strncasecmp(it->first.c_str(),it->second.c_str(),it->second.length()))
			)
		{
			fputs( it->second.c_str(), fp );
			fputc( '\n', fp );
			fputs( it->first.c_str(), fp );
			fputc( '\n', fp );
		}
		it++;
	}
	fclose( fp );

	return true;
}

eString FileMap::getMappedName( const eString & channel )
{
	eString orig=eString(channel);
	convertToUpper(orig);
	std::multimap<eString, eString>::iterator it = map.find( orig);

	if ( it == map.end() )
		return eString("");
	else 
		return it->second;
}

bool FileMap::isMapped( const char *Input , const char * mappedChannel )
{
	eString orig=eString(mappedChannel);
	convertToUpper(orig);
	std::multimap<eString, eString>::iterator it = map.lower_bound( orig);
	for ( ; it != map.end() && it->first==orig ; it++)
	{
		if(it->first.compare(orig))
		{
			return false;
		}
		if(!it->second.icompare(Input)) return true;
	}
	return false;
}

void FileMap::addMappedName( const eString &Input, const eString &Channel, bool needStore )
{
  eString inp = Input;
  eString cha =Channel;
  convertToUpper(inp);
  convertToUpper(cha);
	map.insert(std::pair<eString,eString>(cha,inp));
	if ( needStore )
		needWriteFlag = true;
}

void FileMap::addMappedName( char *input, char *channel, bool needStore )
{
	addMappedName( eString( input ), eString( channel ), needStore );
}

bool FileMap::haveUnknownEntries( void )
{
	bool gotOneFlag = false;

	std::multimap<eString, eString>::iterator it = map.begin();
        while ( 
		( ! gotOneFlag ) &&
		( it != map.end() )
	) {
		if ( it->second == eString( "UNKNOWN" ) )
			gotOneFlag = true;
		it++;
	}
	return gotOneFlag;
}
