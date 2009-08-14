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

#include "text.h"

static std::vector<eString> texts;

bool initStrings( char * langCode, bool moanFlag )
{
	eString textsPath = prefixDir( CONFIG_DIR, TEXTS_FILENAME ) + eString( "." ) + eString( langCode );

	FILE *fp = fopen( textsPath.c_str(), "r" );
	if ( fp == NULL ) {
		if ( moanFlag )
			dmsg( "Couldn't open language file: ", textsPath );
	}
	else {
		if ( ! texts.empty() )
			texts.clear();

		char buffer[MAX_STRING_LENGTH+1];
		while ( fgets( buffer, MAX_STRING_LENGTH, fp ) ) {
			if ( 
				( buffer[0] != '#' ) &&
				( buffer[0] != '\0' ) &&
				( buffer[0] != '\n' ) &&
				// Request of Alex Shtol
				( buffer[0] != '\r' ) 
			) {
				buffer[MAX_STRING_LENGTH] = '\0';
				buffer[strlen(buffer)-1] = '\0';
				texts.push_back( buffer );
			}
		}
               	fclose( fp );
		if ( texts.size() != strNoStrings )
			dmsg( eString().sprintf( "Bad language file: %s\nNeeded %d texts, got %d", textsPath.c_str(), strNoStrings, texts.size() ));
	}
        
	return ( 
		texts.size() == strNoStrings
	);
}

char * getStr( unsigned int strNo ) {
	static char badString[2] = "-";
	if ( texts.size() != strNoStrings )
		return badString;
	else
		return (char*) texts[strNo].c_str();	
}

