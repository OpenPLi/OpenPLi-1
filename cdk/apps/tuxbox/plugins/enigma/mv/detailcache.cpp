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

#include "detailcache.h"

struct DCHeader
{
	long    startOffset;
	size_t  dataLength;
};


DetailCache::DetailCache( char *userLangPref )  : dataP( NULL ), headP( NULL ), finalizedFlag( false )
{
	cacheP = new EnigmaCache( userLangPref );
}

bool DetailCache::init( char * dirPath )
{
	currentDataOffset = 0;
	finalizedFlag = false;

	this->headPath = eString( dirPath ) + eString( "/dc.head" );
	this->dataPath = eString( dirPath ) + eString( "/dc.data" );

	if ( ! (
                (
                        pathExists( dirPath ) &&
                        isDirectory( dirPath )
                ) ||
                makeDirectory( dirPath )
        ) )
                return false;

	return ( openFiles( "w" ) );
}

bool DetailCache::openFiles( const char *mode )
{
	// Open files
  
	headP = fopen( headPath.c_str(), mode );
  
	if ( ! headP )
		return false;
  
	dataP = fopen( dataPath.c_str(), mode );

	if ( ! dataP ) {
		fclose( headP );
		return false;
	}
  
	return true;
}

void DetailCache::empty( void )
{
	if ( headP ) {
		fclose( headP );
		headP = NULL;
		deleteFile( headPath );
	}

	if ( dataP ) {
		fclose( dataP );
		dataP = NULL;
		deleteFile( dataPath );
	}
}

bool DetailCache::finalize( void )
{
	if ( finalizedFlag )
		return false;
	else {
		bool okFlag = true;

		if ( headP ) 
			fclose( headP );
		else
			okFlag = false;

		if ( dataP )
			fclose( dataP );
		else
			okFlag = false;

		if ( okFlag && openFiles( "r" ) )
			finalizedFlag = true;
	}

	return finalizedFlag;
}

bool DetailCache::add( const char *descr )
{
	bool okFlag = true;

	struct DCHeader headerStruct = { currentDataOffset, strlen( descr ) };

	// Filter out self-evident descriptions
	// Then the cursor will flash nicely

	if ( eString( descr ).left(15) == "Geen informatie" )
		headerStruct.dataLength = 0;

	if ( headerStruct.dataLength >= DC_MAX_DESCR_LENGTH )
		headerStruct.dataLength = DC_MAX_DESCR_LENGTH - 1;

	// First write header to header file

	if ( fwrite( &headerStruct, sizeof( struct DCHeader ), 1, headP ) == 1 ) {

		// Now the data, if it's non-zero

		if ( headerStruct.dataLength > 0 ) {	

			if ( fwrite( descr, headerStruct.dataLength, 1, dataP ) == 1 ) {
				currentDataOffset = currentDataOffset + (long) headerStruct.dataLength;
			}
			else  {
				okFlag = false;
			}
		}
	}
	else 
		okFlag = false;

	return okFlag;
}

bool DetailCache::getDescription( 
	Channel *cp, Program *pp,
	char *buffer
) {
	buffer[0] = '\0';
	bool okFlag;

	if ( pp->isFromCache() ) {
		okFlag = cacheP->getProgramDescription( 
				cp->getServiceRef(),
				pp->getStartTime(),
				buffer	
		);
	}
	else {
		okFlag = true;

		// In case anything goes wrong

		if ( finalizedFlag ) {

			long headerOffset = ( (long) pp->getID() * sizeof( struct DCHeader ) );

			// Read the header file, to see where in the
			// data file the data for index starts

			fseek( headP, headerOffset, SEEK_SET );

			struct DCHeader headerStruct;

			if ( fread( &headerStruct, sizeof( struct DCHeader ), 1, headP ) == 1 ) {
				// Read the data

				if ( headerStruct.dataLength > 0 ) {
					if ( headerStruct.dataLength >= DC_MAX_DESCR_LENGTH )
						okFlag = false;
					else {
						fseek( dataP, headerStruct.startOffset, SEEK_SET );

						if ( fread( buffer, headerStruct.dataLength, 1, dataP ) == 1 ) {
							buffer[headerStruct.dataLength] = '\0';
						}
						else {
							buffer[0] = '\0';
							okFlag = false;
						}
					}
				}
			}
			else
				okFlag = false;
		}
		else
			okFlag = false;
	}

	return okFlag;
} 
		      

DetailCache::~DetailCache()
{
	delete cacheP;
	empty();
}

