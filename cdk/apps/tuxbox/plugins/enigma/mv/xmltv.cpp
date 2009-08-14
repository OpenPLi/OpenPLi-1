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

#include "xmltv.h"

void XMLTVConverter::start( void )
{
	run();
}

XMLTVConverter::XMLTVConverter() : convertedCount( 0 )
{
}

void XMLTVConverter::thread_finished( void )
{
        /* emit */ allDone( convertedCount );
}

void XMLTVConverter::thread( void )
{
	std::list<struct conversionSpec>::iterator curRec = records.begin();

	while ( curRec != records.end() ) {
		if ( convert( *curRec ) ) {
			if ( ! writeStringToFile( curRec->from, "converted to EPGUI" ) )
				mylog( "failed to clear file after conversion: " + curRec->from );

			// It's important we set XMLTV mtime first, so that the EPGUI
			// conversion is always same as or older than the XMLTV time
			// Otherwise the file will keep being converted!

			if ( ! setMtime( curRec->from, getCurrentTime() ) )
                             mylog( "failed to set modify time after xmltv->epgui conversion: " + curRec->from );
			if ( ! setMtime( curRec->to, getCurrentTime() ) )
                             mylog( "failed to set modify time after xmltv->epgui conversion: " + curRec->to );

			convertedCount++;
		}
		else
			mylog( "conversion xmltv->epgui failed for " + curRec->from );

		curRec++;
	}
}


void XMLTVConverter::addFile( eString from, char encoding, eString to )
{
	struct conversionSpec bob = { from, to, encoding };
	records.push_back( bob );
}

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

bool XMLTVConverter::convert( struct conversionSpec &spec )
{
	eString fromPath = spec.from;
	eString toPath = spec.to;

/* Parser accepts: "ISO-8859-1")) {
    if (streqci(name, "UTF-8")) {
    if (streqci(name, "US-ASCII")) {
    if (!streqci(name, "UTF-16"))
*/
	eString encoding = ( spec.encoding == 'u' ) ? "UTF-8" : "ISO-8859-1";
	spec.encoding = 'u';

	XMLTreeParser *parser = getXMLParser( fromPath.c_str(), encoding.c_str() );

	// This is almost certainly because the file hasn't
	// finished downloading yet, let's just ignore it

	if ( parser == NULL )
		return false;

	bool successFlag = true;

        XMLTreeNode *root=parser->RootNode();
        if ( ! root ) {
                mylog( eString().sprintf( "%s: %s", getStr( strErrXMLNoRootNode ), fromPath.c_str() ) );
		successFlag = false;
	}

        if ( 
		( successFlag == true ) &&
		strcmp( root->GetType(), "tv") 
	) {
                mylog( eString().sprintf( "%s: %s", getStr( strErrXMLBadRootNode ), fromPath.c_str() ) );
		successFlag = false;
        }

	if ( successFlag == true ) {

		char *startTimeString, *endTimeString, *channelID;
		struct ProgramData *p;

		// Collect channels and programs here
		std::map<const char *, std::list<struct ProgramData *>, ltstr > channels;

		for ( XMLTreeNode * prog = root->GetChild(); prog; prog = prog->GetNext() ) {
	
			char *progType = prog->GetType();
	
			// Program: record if OK
	
			if ( 
				( progType != NULL ) &&
				( ! strcmp( progType, "programme" ) )
			) {
				// Start time
				startTimeString = prog->GetAttributeValue( "start" );
				if ( startTimeString == NULL )
					continue;
	
				// End time
				endTimeString = prog->GetAttributeValue( "stop" );
				if ( endTimeString == NULL )
					continue;
	
				// Channel name
				channelID = prog->GetAttributeValue( "channel" );
				if ( channelID == NULL )
					continue;
	
				p = new struct ProgramData;
	
				p->flags = 0;
	
				// Program name/descr
				for ( XMLTreeNode * c = prog->GetChild(); c; c = c->GetNext() ) {
					if ( ! strcmp( c->GetType(), "title" ) ) {
						p->name = convertToUTF( c->GetData(), spec.encoding ) ;
					}
					else if ( ! strcmp( c->GetType(), "category" ) ) {
						char *dataP = c->GetData();
						if ( 
							( dataP != NULL ) && 
							( strcmp( dataP, "Film" ) == 0 )
						)
							p->flags = programDataFlagFilm;
					}
					else if ( ! strcmp( c->GetType(), "desc" ) ) {
						p->descr = convertToUTF( c->GetData(), spec.encoding );
					}
				}
	
				// No name: next please!
				if ( p->name.length() == 0 ) {
					delete p;
					continue;
				}
	
				time_t endTime;
				if ( 
					analyseTime( startTimeString, &p->startTime ) &&
					analyseTime( endTimeString, &endTime )
				) {
					p->duration = endTime - p->startTime;
					(channels[(const char *)channelID]).push_back( p );
				}
			}
		}
	
	
		// Write to temp EPGUI file and delete temp in-memory data
		// as we go
		
		eString tempFile = toPath + ".tmp";

		FILE *out = fopen( tempFile.c_str(), "w" );
	
		if ( out ) {
			std::map<const char *, std::list<struct ProgramData *>, ltstr >::iterator ci = channels.begin();

			while ( ci != channels.end() ) {
				// Start of channel
				fprintf( out, "C 0000 %s\n", ci->first );
	
				ci->second.sort();
	
				// Programs
				std::list<struct ProgramData *>::iterator pi = ci->second.begin();
				while ( pi != ci->second.end() ) {
					writeEPGUIProgramToFileHandle( out, **pi );	
					delete *pi;
					pi++;
				}
	
				// End of channel
				fprintf( out, "c\n" );
				ci++;
			}
			fclose( out );
	
			// Move temp file to final location

			if ( ! moveFileSameFS( tempFile, toPath ) ) {
				dmsg( getStr( strErrXMLTVConversionMovingTempFile ), toPath );
				deleteFile( tempFile );
			}
		}
		else
			successFlag = false;
	}
	
	// Can't delete earlier, the channel names point into it

	delete parser;

	return successFlag;
}

bool XMLTVConverter::analyseTime( eString t,  time_t *result )
{
	tm tStruct;

	eString timePart = t.left( 12 );

	if ( 
		( timePart.length() != 12 ) ||
		( ! isNumericString( (char *)timePart.c_str() ) )
	)
		return false;
	
	tStruct.tm_year = atoi( timePart.left( 4 ).c_str() ) - 1900;	
	tStruct.tm_mon = atoi( timePart.mid( 4,2 ).c_str() ) - 1;
	tStruct.tm_mday = atoi( timePart.mid( 6,2 ).c_str() );

	tStruct.tm_hour = atoi( timePart.mid( 8,2 ).c_str() );
	tStruct.tm_min = atoi( timePart.mid( 10,2 ).c_str() );
	tStruct.tm_sec = 0;

	*result = mktime( &tStruct );

	// Analyse offset if it looks like there is one
	// Test 14 not 12 because some formats include 
	// 2 digits for seconds

	if ( t.length() > 14 ) {

		// Apply shift field

		eString shiftPart = t.right( 5 );
		eString shiftTime = shiftPart.right(4);

		if ( 
			( shiftTime.length() != 4 ) ||
			( ! isNumericString( (char *)shiftTime.c_str() ) )
		) {
			return false;
		}

		time_t shiftHour = atoi( shiftTime.left(2).c_str() );
		time_t shiftMin = atoi( shiftTime.right(2).c_str() ); 

		time_t shiftTimeT = ( shiftHour * 3600 ) + ( shiftMin * 60 );

		if ( shiftPart.left(1) == "+" )
			*result -= shiftTimeT;	
		else if ( shiftPart.left(1) == "-" )
			*result += shiftTimeT;	
		else
			return false;
	}

	return true;
}
