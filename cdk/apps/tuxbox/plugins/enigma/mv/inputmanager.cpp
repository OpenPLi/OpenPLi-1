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

#include "inputmanager.h"

#include "epgui.cpp"

void InputManager::loadOneChannelCacheData( eServiceReferenceDVB ref )
{
	time_t curTime = getLoadTime();
	ecacheP->readOneChannelProgramData( false, ref, curTime - 86400, curTime + (SECONDS_IN_A_DAY * 100), false ); 
}

InputManager::~InputManager()
{
 int i;
 	for(i=0 ; i < nbDataFiles ; i++)
		fclose(pDataFiles[i]);
	free(pDataFiles);
	free(FileNames);
	free(encodings);
	if ( channelMgrP )
		delete channelMgrP;

	if ( ecacheP )
		delete ecacheP;

}

bool InputManager::haveCacheInput( struct Inputs &inputs )
{
        InputDefReader defs;

        for ( int dno = 0; dno < MAX_NO_INPUTS; dno++ ) {
                if ( ! inputs.confs[dno].enabledFlag )
                        continue;
                struct inputDef *defP = defs.getDefP( inputs.confs[dno].name );

                if (
                        ( defP != NULL ) &&
                        ( defP->format == CACHE_INPUT_FORMAT )
                )
                        return true;
        }
        return false;
}

void InputManager::checkXMLTVNotConverting( bool showMessage )
{
	while ( XMLTVThread != NULL ) {
		if ( showMessage )
			flashMessage( getStr( strXMLTVConvertMessage ) );
		microSleep( 50000 );
	}
}

void InputManager::receiveOneChannelCacheData( struct ProgramData &p )
{
	/*emit*/ gotOneChannelCacheData( p );
}

void InputManager::receiveData( struct ProgramData &p )
{
	if ( dataMode == dataModeLoad )
		/*emit*/ gotData( p );
	else {
		// Here dataModeSave or dataModeTransferOldCache

		// IMPORTANT: ALL TIMES IN p.startTime HERE ARE LOCALTIME
		// Cache times are stored in localtime, 
		// and we write them locatime too, so
		// and they aren't converted coming back
		// (special call to readDatafile)

		// If we're copying an old cache file to a new one, we
		// don't want to include programs that are in the
		// enigma cache, because we'll be writing those next
		// We also want to drop old records

		if ( dataMode == dataModeTransferOldCache ) {
			if ( 
				( ecacheP->eventIsInCache( p.channelName, p.startTime ) ) ||
				( ( p.startTime + p.duration ) < getLoadTime() )
			)
				return;
		}

                // Channel end and start
                if ( p.channelName != lastReceivedChannelName ) {
                        if ( lastReceivedChannelName.length() > 0 )
                                fprintf( cacheSaveFP, "c\n" );

                        fprintf( cacheSaveFP, "C 0000 %s\n", p.channelName.c_str() );
		
			lastReceivedChannelName = p.channelName;
                }

		writeEPGUIProgramToFileHandle( cacheSaveFP, p );
        }

}

eServiceReferenceDVB *InputManager::nextBouquetRef( bool fromStartFlag )
{
	return ecacheP->nextBouquetRef( fromStartFlag );
}

eString InputManager::nextBouquetName( bool fromStartFlag )
{
	return ecacheP->nextBouquetName( fromStartFlag );
}

bool InputManager::haveUnknownChannels( void )
{
	return ( channelMgrP && channelMgrP->haveUnknownEntries() );
}

InputManager::InputManager( eString mapPath, char *userLangPrefP ) : 
	userLangPrefP( userLangPrefP ),
	dataMode( dataModeLoad ),
	channelMgrP( NULL ), 	
	ecacheP( NULL ),
	XMLTVThread( NULL ),
	dcacheP( NULL ),
	allDownloadsDoneFlag( true )
{
	channelMgrP = new ChannelManager( mapPath );

	ecacheP = new EnigmaCache( userLangPrefP, true );
//	ecacheP->convertDataChannels();

	CONNECT( ecacheP->gotData, InputManager::receiveData );
	CONNECT( ecacheP->gotOneChannelData, InputManager::receiveOneChannelCacheData );

	dcacheP = new DownloadCache();
	CONNECT( dcacheP->downloadStarting, InputManager::downloadStarted );
	CONNECT( dcacheP->allDoneSignal, InputManager::allDownloadsFinished );
	nbDataFiles=0;
	maxDataFiles=20;
	pDataFiles=(FILE **)malloc(maxDataFiles*sizeof(FILE *));
	FileNames=(char **)malloc(maxDataFiles*sizeof(char *));
	encodings=(char *)malloc(maxDataFiles*sizeof(char));
}

void InputManager::convertXMLTVFilesToEPGUI( struct Inputs inputs )
{
	// In theory, we could stop the thread, add more files
	// and start it again. But need checking not adding
	// same files already queued for conversion
	if ( XMLTVThread != NULL )
		return;

	InputDefReader defs;

	struct CacheDetails  det;

	for ( int dno = 0; dno < MAX_NO_INPUTS; dno++ ) {
		if ( ! inputs.confs[dno].enabledFlag )
			continue;

		struct inputDef *defP = defs.getDefP( inputs.confs[dno].name );

		if ( 
			( defP == NULL ) ||
			( defP->format != XMLTV_INPUT_FORMAT )
		)	
			continue;

		eString absDir = prefixDir( eString( indexToDir( inputs.storageDir ) ), defP->localDir );

		eString nextFile = getNextFileInDir( absDir, true );

		while ( nextFile.length() != 0 ) {
			eString epguiPath = nextFile + eString( XMLTV_TO_EPGUI_CONVERTED_AFFIX );

			// Need convert if it ends in the right affix, and there's not
			// already a converted file there, or the converted file is
			// more recent than the EPGUI file
			if ( 
				( nextFile.right( strlen( defP->affix ) ) == defP->affix ) &&
				( 
					( ! pathExists( epguiPath ) ) ||
					( fileMtime( epguiPath ) < fileMtime( nextFile ) )
				)
			) {
				if ( XMLTVThread == NULL )
					XMLTVThread = new XMLTVConverter;
	
				XMLTVThread->addFile( nextFile, defP->encoding, epguiPath );
			}
			nextFile = getNextFileInDir( absDir, false );
		}
	}

	if ( XMLTVThread != NULL ) {
		CONNECT( XMLTVThread->allDone, InputManager::INT_XMLTVConvertDone );
		/*emit*/ XMLTVStatusChange( true );
		XMLTVThread->start();
	}
}

void InputManager::INT_XMLTVConvertDone( int filesConverted )
{
	delete XMLTVThread;
	XMLTVThread = NULL;	
	/*emit*/ XMLTVStatusChange( false );
}

void InputManager::allDownloadsFinished( int count )
{
	dcacheP->tidy( DOWNLOAD_CACHE_MAX_OLD_FILE_AGE );
	allDownloadsDoneFlag = true;
	/* emit */ IMallDownloadsFinished( count );
}

void InputManager::downloadStarted( void )
{
	/* emit */ IMdownloadStarted();
}

// Cache appropriate to the specified inputs

void InputManager::startCache( struct Inputs &inputs )
{
	// Can't download if there's no downloader or network...
	if ( 
		( ! dcacheP ) ||
		( ! haveNetwork() )
	) {
		/* emit */ IMallDownloadsFinished( 0 );
		return;
	}

	allDownloadsDoneFlag = true;

	InputDefReader defs;

	struct CacheDetails  det;

	for ( int dno = 0; dno < MAX_NO_INPUTS; dno++ ) {
		if ( ! inputs.confs[dno].enabledFlag )
			continue;

		struct inputDef *defP = defs.getDefP( inputs.confs[dno].name );

		if ( defP == NULL )
			continue;

		// If there's a remote dir (starting with 'http:')
		// need pull

		if ( remoteType( defP->remoteDir ) == inputDefRemoteTypeHTTP ) {
			det.cacheDir = prefixDir( eString( indexToDir( inputs.storageDir ) ), defP->localDir );

			det.baseURL = defP->remoteDir;
			det.filePrefix = defP->prefix;
			det.fileAffix = defP->affix;
			det.daysAhead = defP->days;
			det.type = ( defP->type == 'r' ) ? dailyRotationCacheType : fixedFileCacheType;

			if ( ! makeDirIfNotExists( det.cacheDir ) )  {
				dmsg( getStr( strErrMakingCacheDir ), (char *)det.cacheDir.c_str() );
				continue;
			}
			dcacheP->addCache( &det );
			allDownloadsDoneFlag = false;
		}
	}

	if ( allDownloadsDoneFlag )
		/* emit */ IMallDownloadsFinished( 0 );
		
	dcacheP->refresh();
}

void InputManager::readProgramData( time_t startingFrom, struct Inputs &inputs )
{
	InputDefReader defs;

	for ( int dno = 0; dno < MAX_NO_INPUTS; dno++ ) {
		if ( ! inputs.confs[dno].enabledFlag )
			continue;
		struct inputDef *defP = defs.getDefP( inputs.confs[dno].name );

		if ( defP == NULL ) {
			dmsg( getStr( strErrInputNameHasNoDefinition ), inputs.confs[dno].name );
			continue;
		}

		time_t pre = startingFrom - inputs.pre;
		time_t post = startingFrom + inputs.post;
		eString path = prefixDir( eString( indexToDir( inputs.storageDir ) ), defP->getFixedLocalPath() );

		switch ( defP->type ) {
		case FIXED_INPUT_TYPE:
			switch ( defP->format ) {
			case CACHE_INPUT_FORMAT:
				ecacheP->readProgramData( pre, post, inputs.maxChannelsPerInput, false );
				break;
			case SAVED_CACHE_INPUT_FORMAT:
				readDatafile( 
					(char*)path.c_str(), 
					defP->format, 
					defP->encoding,
					0,
					pre, post, 
					inputs.maxChannelsPerInput
				);
				break;
			default:
				readDatafile( 
					(char*)path.c_str(), 
					defP->format, 
					defP->encoding,
					defP->offsetToGMT , 
					pre, post, 
					inputs.maxChannelsPerInput
				);
				break;
			}
			break;
		case ROTATING_INPUT_TYPE:
			readRotatingProgramData( eString( indexToDir( inputs.storageDir ) ), defP, pre, post, inputs.maxChannelsPerInput );
			break;
		default:
			dmsg( getStr( strErrBadInputType ), defP->name );
		}
	}
}

eString InputManager::secondsToDateString( time_t seconds )
{
	tm timeStruct = *localtime( &seconds );	

	int year = timeStruct.tm_year + 1900;

	int month = timeStruct.tm_mon + 1;

	return eString().sprintf( "%.4d%.2d%.2d", year, month, timeStruct.tm_mday );	
}
	

void InputManager::readRotatingProgramData( eString storageBase, struct inputDef *defP, time_t from, time_t to, int maxChannels )
{
	eString lastDate = secondsToDateString( to );

	time_t currentSeconds = from;

	eString currentDate;
	do {
		currentDate = secondsToDateString( currentSeconds );

		currentSeconds += SECONDS_IN_A_DAY;

		eString filepath = prefixDir( storageBase, defP->getLocalPath( currentDate ) );

		readDatafile( 
			(char *) filepath.c_str(), 
			defP->format, 
			defP->encoding,
			defP->offsetToGMT , 
			from, to, maxChannels 
		);
	}
	while ( currentDate != lastDate );
}

void InputManager::readDatafile( char *filepath, char format, char encoding, int offset, time_t from, time_t to, int maxChannels )

{
	switch ( format ) {
		case EPGUI_INPUT_FORMAT:
		case SAVED_CACHE_INPUT_FORMAT:
			readEPGUIProgramData( filepath, encoding, offset, from, to, maxChannels );
			break;
		case XMLTV_INPUT_FORMAT:
		{
			eString convertedFilepath = eString( filepath ) + eString( XMLTV_TO_EPGUI_CONVERTED_AFFIX );
			readEPGUIProgramData( (char *) convertedFilepath.c_str(), 'u', offset, from, to, maxChannels );
		}
			break;
		default:
			dmsg( getStr( strErrBadInputFormat ), filepath );
	}
}

bool InputManager::getServiceRef( eString &channelName, eServiceReferenceDVB *eRef )
{
	return ecacheP->getServiceRef( channelName, eRef );
}


// NOTE NOTE NOTE: this function mangles the original buffer!

int InputManager::genreDescrToGenreFlags( char * descr, time_t programLengthSeconds )
{
	int flags = 0;	
/*
	char * wordStart = descr;
	char * wordEnd = wordStart;
	bool doneFlag = false;

	// The MHW ones are more complicated

	while ( ! doneFlag ) {
		while ( 
			( *wordEnd != '\n' ) &&
			( *wordEnd != '\0' ) &&
			( *wordEnd != ' ' )
		)	
			wordEnd++;

		if ( *wordEnd != ' ' )
			doneFlag = true;

		*wordEnd = '\0';

		if ( 	
			( strcmp( wordStart, "FILM" ) == 0 ) ||
			( 
				( programLengthSeconds > ( 60 * 85 ) ) &&
				(
					( strcmp( wordStart, "KOMEDIE" ) == 0 ) ||
					( strcmp( wordStart, "ANDERE" ) == 0 ) ||
					( strcmp( wordStart, "MISDAAD" ) == 0 )
				)
			)
		) {
			flags |= programDataFlagFilm;
			doneFlag = true;
		}
		else {
			wordEnd++;
			wordStart = wordEnd;
		}
	}
*/

	// Why the hell did I do all that complicated stuff ?

	if ( 
		strstr( descr, "FILM" ) ||
		strstr( descr, "Film" ) ||
		strstr( descr, "film" ) ||
		strstr( descr, "CINE" ) ||
		strstr( descr, "Cine" ) ||
		strstr( descr, "CINEMA" ) ||
		( 
			( programLengthSeconds > ( 60 * 85 ) ) &&
			(
				strstr( descr, "KOMEDIE" )  ||
				strstr( descr, "ANDERE" ) ||
				strstr( descr, "MISDAAD" ) ||
				strstr( descr, "COMMEDIA" ) ||
				strstr( descr, "DRAMMATICO" ) ||
				strstr( descr, "AZIONE" ) ||
				strstr( descr, "ROMANTICO" ) ||
				strstr( descr, "FICTION" )
			)
		)
	)
		flags |= programDataFlagFilm;
	
	return flags;
}



bool InputManager::downloading( void ) {
        return( ! allDownloadsDoneFlag );
}

// Method: move old to old.tmp
// If old exists: read old.tmp, writing ones not already in cache; delete old.tmp
// Read cache, write everything	
// If we only save the cache everytime, then after
// a reboot we only see the old date once before it is lost

bool InputManager::saveEnigmaCache( bool testOnlyFlag, struct Inputs &inputs )
{
	InputDefReader defs;

	time_t now = getLoadTime();

	for ( int dno = 0; dno < MAX_NO_INPUTS; dno++ ) { 
		if ( inputs.confs[dno].enabledFlag ) {
			struct inputDef *defP = defs.getDefP( inputs.confs[dno].name );

			if ( 
				( defP->format != SAVED_CACHE_INPUT_FORMAT ) ||
				( defP->days == 0 )
			)
				continue;

			eString saveDir = prefixDir( indexToDir( inputs.storageDir ), defP->localDir );

			if ( ! makeDirIfNotExists( saveDir ) ) {
				dmsg( getStr( strErrMakingDir ), saveDir );
				continue;
			}

			eString path = prefixDir( saveDir, defP->getFixedFilename() );

			// Slight abuse of offsetToGMT field to mean 'only refresh
			// the file from the cache every x seconds

			if ( fileNeedsRefreshing( path, defP->offsetToGMT ) ) {
				if ( testOnlyFlag )
					return true;
			}
			else
				continue;

			cacheSaveFP = NULL;

			// If it already exists,
			// move it to .tmp, read back only keeping ones that aren't
			// in the cache already

			if ( pathExists( path ) ) {
				eString tempPath = path + ".tmp";

				if ( ! moveFileSameFS( path, tempPath ) ) {
					dmsg( getStr( strErrMovingOldCacheToTempFile) , tempPath );
					continue;
				}
			
				cacheSaveFP = fopen( path.c_str(), "w" );

				if ( cacheSaveFP == NULL ) {
					dmsg( getStr( strErrOpeningCacheBackupFile ), path );
					continue;
				}

				dataMode = dataModeTransferOldCache;
				lastReceivedChannelName = "";
	
				readDatafile( 
					(char*) tempPath.c_str(), 
					SAVED_CACHE_INPUT_FORMAT, 
					'u',
					offsetGMTToLocaltime(), 
					now, now + (SECONDS_IN_A_DAY * 20 ),
					inputs.maxChannelsPerInput
				);

				// Terminate the last channel, not a problem if none read
                                fprintf( cacheSaveFP, "c\n" );

				if ( ! deleteFile( tempPath ) )
					dmsg( getStr( strErrDeletingTempCacheBackupFile ), tempPath );
			}

			dataMode = dataModeSave;
			lastReceivedChannelName = "";

			// Open it, if it wasn't already for the read-back

			if ( cacheSaveFP == NULL )
				cacheSaveFP = fopen( path.c_str(), "w" );

			if ( cacheSaveFP == NULL )
				dmsg( getStr( strErrOpeningCacheBackupFile ), path );
		        else {
				ecacheP->readProgramData( 
					now, ( now + ( (time_t) defP->days * (time_t) SECONDS_IN_A_DAY ) ), 
					inputs.maxChannelsPerInput,
					true
				);

				// Terminate the last channel, not a problem if none read
				fprintf( cacheSaveFP, "c\n" );
				fclose( cacheSaveFP );
				if ( ! setMtime( path, getLoadTime() ) )
					dmsg( getStr( strErrCouldntSetModifyTime ), path );
			}

			dataMode = dataModeLoad;
		}
	}

	return false;
}

bool InputManager::getDescription(Channel *cp,Program *pp,char *buffer)
{
  if (pp->isFromCache() )
  {
    return ecacheP->getProgramDescription(
                          cp->getServiceRef(),
                          pp->getStartTime(),
                          buffer);
  }
  if(pp->getIdxFic() <0 ) return false;
 FILE *fp=pDataFiles[pp->getIdxFic()];
  fseek(fp,pp->getOffset()+2,SEEK_SET);
  if(fgets(buffer,MAX_PROGRAM_DESC_LENGTH,fp))
  {
	//eString  descr=convertToUTF( buffer, encodings[pp->getIdxFic()] );
	safeStrncpy(buffer,(char*)
			convertToUTF( buffer, encodings[pp->getIdxFic()] ).c_str(),MAX_PROGRAM_DESC_LENGTH);
	int i;
	for(i=0 ; i<MAX_PROGRAM_DESC_LENGTH ; i++)
	{
	  if(buffer[i]=='|')buffer[i]='\n';
	  else if(!buffer[i]) break;
	}
    return true;
  }
  return false;
}

// ------------------- LITE FUNCTIONS ----------------- //

#ifndef MVLITE
void InputManager::runChannelManager( void )
{
	setListboxFontSize( CHANNEL_MANAGER_LISTBOX_FONT_SIZE );
	channelMgrP->build( ecacheP->getServiceListP() );
	showExecHide( channelMgrP );
	setListboxFontSize( 0 );
}
#endif
