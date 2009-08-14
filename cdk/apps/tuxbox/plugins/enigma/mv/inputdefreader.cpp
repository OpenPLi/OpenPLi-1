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

#include "inputdefreader.h"

int remoteType( eString spec )
{
	if ( spec.length() == 0 )
		return inputDefRemoteTypeNone;
	else {
		eString prot = spec.left(5);
		if ( prot == "http:" )
			return inputDefRemoteTypeHTTP;
		else if ( prot == "otid:" ) 
			return inputDefRemoteTypeOTV;
		else
			return inputDefRemoteTypeNone;
	}
}

void makeInputDefFileIfNotExist( void )
{
	char *path = INPUTDEF_PATH;
	if ( ! pathExists( path ) ) {
		FILE *fp = fopen( path, "w" );
		if ( fp ) {	
			fputs( "\
# MV input definitions.\n\
# http://mv.dinglisch.net/userguide.html#id\n\
#\n\
TPS(FR) e l f  0 otv-fr 8 epg .dat otid:2e18h-1:0:1:5dd:2e18:b0:820000:0:0:0:\n\
EnigmaCache c u f 0 - 0 - - -\n\
SavedCache s u f 7200 ec-save 5 epg .dat -\n\
NordicTEAM e l r -7200 nordicteam 7 tv- .dat http://mv.nordicteam.com/\n\
Dinglisch(EU) e l r 0 dinglisch-eu 3 tv- .dat http://mv.dinglisch.net/data/e\n\
KULICHIKI e u r -18000 kul-rus 2 kul_ .dat http://www.twinline.net/tv/\n\
Wolf(NL) x u r 0 wolf-nl 4 tv- .xmltv http://wolf.xs4all.nl/xmltv/\n\
MyXMLTV x u f 0 myxmltv 4 tv .xmltv -\n", fp );

// WARNING: if you change MyXMLTV name, need to change
// the warning text in inputdefreader.cpp too

			fclose( fp );
		}
		else {
			dmsg( getStr( strErrWriteOpenInputDefs ), path );
		}
	}
}

bool isDST( void )
{
	time_t nowLocaltime = time(NULL);
	struct tm nowStruct = *localtime( &nowLocaltime );

	return ( nowStruct.tm_isdst > 0 );
}


InputDefReader::InputDefReader()
{
        FILE *fp = fopen( INPUTDEF_PATH, "r" );
        if ( fp ) {
        	char line[MAX_STRING_LENGTH+1];
		struct inputDef *temp;
                while( fgets( line, MAX_STRING_LENGTH, fp ) ) {
                        if ( *line == '#' )
                                continue;

			temp = new( struct inputDef );
			int matched = sscanf( line, "%s %c %c %c %d %s %d %s %s %s", temp->name, &temp->format, &temp->encoding, &temp->type, &temp->offsetToGMT, temp->localDir, &temp->days, temp->prefix, temp->affix, temp->remoteDir );
			if (matched !=10)
			{
			  matched = sscanf( line, "%s %c %c %c %dDST %s %d %s %s %s", temp->name, &temp->format, &temp->encoding, &temp->type, &temp->offsetToGMT, temp->localDir, &temp->days, temp->prefix, temp->affix, temp->remoteDir );
			  if( isDST()) temp->offsetToGMT+=3600;
			}

			if ( matched == 10 ) {
				if ( strcmp( temp->remoteDir, "-" ) == 0 ) 
					strcpy( temp->remoteDir, "" );
				defList.push_back( temp );
			}
			else {
				dmsg( getStr( strErrBadInputDefsLine) , line );
				delete temp;
			}
		}
		fclose( fp );
	}
	else {
		dmsg( getStr( strErrReadOpenInputDefs ), INPUTDEF_PATH );
	}
}

InputDefReader::~InputDefReader()
{
	std::list<struct inputDef *>::iterator cur = defList.begin();
	while ( cur != defList.end() ) {
		delete *cur;
		cur++;
	}
}

int InputDefReader::noDefs( void )
{
	return defList.size();
}

struct inputDef *InputDefReader::getDefP( char *name )
{
	std::list<struct inputDef *>::iterator curr = defList.begin();
        while ( curr != defList.end() ) {
		if ( strcmp( (*curr)->name, name ) == 0 )
			return *curr;
		curr++;
	}
	return NULL;
}

struct inputDef *InputDefReader::getNextDefP( bool fromStart )
{
	static std::list<struct inputDef *>::iterator cur;

	if ( fromStart )
		cur = defList.begin();
	if ( cur == defList.end() )
		return NULL;

	struct inputDef *toReturn = *cur;
	cur++;
	return toReturn;
}
