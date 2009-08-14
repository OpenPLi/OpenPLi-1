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

#include "files.h"

int fileSize( eString filePath ) {
	struct stat stats;	
	if ( stat( filePath.c_str(), &stats ) == -1 ) {
		return -1;
	}
	else {
		return stats.st_size;
	}
}

int moveFileSameFS( eString from, eString to ) {
	return ( rename( from.c_str(), to.c_str() ) != -1 );
}

int deleteFile( eString &filePath ) {
	return ( unlink( filePath.c_str() ) != -1 );
}

int makeDirectory( eString path )
{
	return ( mkdir( path.c_str(), S_IRUSR | S_IWRITE | S_IXUSR ) != -1 );
}

int pathExists( eString path )
{
	 return ( fileSize( path ) != -1 );
}

bool isDirectory( const eString &path )
{
	struct stat stats;	
	if ( stat( path.c_str(), &stats ) == -1 ) {
		return 0;
	}
	else {
		return( S_ISDIR(stats.st_mode) != 0 );
	}
}

int setMtime( eString path, time_t toSet )
{
	struct utimbuf tb = { toSet, toSet };

	return ( utime( path.c_str(), &tb ) == 0 );
}

int makeDirIfNotExists( eString path )
{
	if ( 
		pathExists( path ) &&
		isDirectory( path )
	)
		return 1;
	else
                return ( makeDirectory( path ) );
}

eString getNextFileInDir( eString &dir, bool fromStart )
{
	static DIR *dirP = NULL;

	if ( fromStart )
        	dirP = opendir( dir.c_str() );

	if ( dirP == NULL )
		return "";

	eString pathToReturn;

	struct direct *dirEntryP;
        struct stat statStruct;

	while ( ( dirEntryP  = readdir( dirP )) != NULL ) {

		eString filenameString( dirEntryP->d_name );

		if (
			( filenameString == eString( "." ) ) ||
			( filenameString == eString( ".." ) )
		)
			continue;

		pathToReturn = dir + eString("/") + filenameString;

		// Stop with this one on error
		if ( stat( pathToReturn.c_str(), &statStruct ) != 0 )
			continue;

             	if ( S_ISDIR( statStruct.st_mode ) )
			continue;

		break;
	}

	if ( dirEntryP == NULL ) {
		closedir( dirP );
		dirP = NULL;
		return "";
	}
	else {
		return pathToReturn;
	}
}

void deleteOldFilesInDir( eString path, time_t oldestAge )
{
	eString nextFile = getNextFileInDir( path, true );

	while ( nextFile.length() != 0 ) {
		if ( 
			( fileSize( nextFile ) == 0 ) ||
			( fileMtime( nextFile ) < oldestAge )
		)
			deleteFile( nextFile );

		nextFile = getNextFileInDir( path, false );
	}
}

time_t fileMtime( eString filePath )
{
	struct stat stats;	
	if ( stat( filePath.c_str(), &stats ) == -1 )
		return -1;
	else
		return stats.st_mtime;
}

bool writeStringToFile( eString &path, char *fill )
{
	FILE *f = fopen( path.c_str(), "w" );
	if ( f ) {
		fprintf( f, fill );
		fclose( f );
		return true;
	}
	else {
		return false;
	}
}


/*
if(rename(old, new) < 0) {
 if(ERRNO != EXDEV){
 --- unexpected problem
perror(..);
...
} else {
 outside the device
char buffer
if(fopen(old, ...
if(fopen(new, ...
while(fread(...
fwrite(...
unlink(old);
...
case 0:
 -- everything's fine...
 ...
 break;
case
}
*/

