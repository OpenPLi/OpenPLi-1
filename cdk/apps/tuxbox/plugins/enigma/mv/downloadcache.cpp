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

#include "downloadcache.h"

#define DOWNLOAD_STATUS_NOT_STARTED 	0
#define DOWNLOAD_STATUS_RUNNING		1
#define DOWNLOAD_STATUS_FINISHED	2

enum {
	parallelDownloadFlag = 1
};

DownloadCache::DownloadCache() : noCaches(0), cachesFinishedCount(0), flags(0), successfullDownloads(0)
{
}

DownloadCache::~DownloadCache()
{
	for ( int cacheNo = 0; cacheNo < noCaches; cacheNo++ )
		delete state[cacheNo];
}

void DownloadCache::setParallelDownload( void )
{
	flags |= parallelDownloadFlag;
}

int DownloadCache::addCache( struct CacheDetails *d )
{
	if ( noCaches < MAX_NO_CACHES ) {
		details[noCaches] = *d;
		state[noCaches] = new DownloadState( DOWNLOAD_STATUS_NOT_STARTED, 0 );
		noCaches++;
		return 1;
	}
	else
		return 0;
}

void DownloadCache::tidy( time_t maxAgeSeconds )
{
	time_t now = getCurrentTime();
	for ( int cacheNo = 0; cacheNo < noCaches; cacheNo++ )
		deleteOldFilesInDir( details[cacheNo].cacheDir, ( now - maxAgeSeconds ) );
}

void DownloadCache::refresh( void )
{
	if ( noCaches > 0 ) {
		if ( flags & parallelDownloadFlag ) {
			for ( int cacheNo = 0; cacheNo < noCaches; cacheNo++ )
				startNextDownload( cacheNo );
		}
		else {
			startNextDownload( 0 );
		}
	}
}

void DownloadCache::startNextDownload( int cacheNo )
{
	struct DownloadState *stateP = state[cacheNo];
	struct CacheDetails det = details[cacheNo];

	eString theFilename = makeFilename( det.type, det.filePrefix, det.fileAffix, stateP->getDownloadNo() );

	stateP->setStatus( DOWNLOAD_STATUS_RUNNING );

	eString tempDir = det.cacheDir + eString("/tmp");

	Downloader *dl = new Downloader( cacheNo );

	stateP->setDownloader( dl );

        dl->setTempDir( tempDir );

	CONNECT( dl->downloadDone, DownloadCache::downloadDone );

	// Fixed file: we want to overwrite if it's older than specified age
	int startErr;
	eString baseURL = prefixDir( det.baseURL, theFilename );
	if ( det.type == fixedFileCacheType )
        	startErr = dl->start( baseURL, det.cacheDir, theFilename, det.daysAhead * SECONDS_IN_A_DAY );
	else
        	startErr = dl->start( baseURL, det.cacheDir, theFilename );

	if ( startErr == DL_ERR_FILE_EXISTS ) {
		doIncDownload( cacheNo );
	}
	else if ( startErr != DL_ERR_NONE ) {
		flashIntMessage( getStr( strErrDownload ), startErr );
		doCacheFinished( cacheNo );
	}
	else {
		/* emit */ downloadStarting();
	}
}

eString DownloadCache::makeFilename(
	int cacheType, eString prefix, eString affix, int downloadNo
) {
	eString filename;
	if ( cacheType == dailyRotationCacheType ) {
		filename = prefix + makeEightDigitDate( getCurrentTime() + ( 86400 * downloadNo ) ) + affix;
	}
	else {
		filename = prefix + affix;
	}
	return filename;
}

void DownloadCache::downloadDone( int cacheNo, int err )
{
	state[cacheNo]->setStatus( DOWNLOAD_STATUS_FINISHED );

	// Stop further from this source on err

	if ( err == DL_ERR_NONE ) {
		successfullDownloads++;
		doIncDownload( cacheNo );
	}
	else
		doCacheFinished( cacheNo );
}

void DownloadCache::doIncDownload( int cacheNo )
{
	state[cacheNo]->incDownloadNo();
	if ( state[cacheNo]->getDownloadNo() >= lastDownloadNo( cacheNo )  )
		doCacheFinished( cacheNo );
	else
		startNextDownload( cacheNo );
}

void DownloadCache::doCacheFinished( int cacheNo )
{
	state[cacheNo]->resetDownloadNo();
	cachesFinishedCount++;

	if ( cachesFinishedCount == noCaches ) {
		/*emit*/ allDoneSignal( successfullDownloads );
	}
	else if ( ! ( flags & parallelDownloadFlag ) ) {
		cacheNo++;
		if ( cacheNo < noCaches )
			startNextDownload( cacheNo++ );
	}
}

int DownloadCache::lastDownloadNo( int cacheNo )
{
	if ( details[cacheNo].type == dailyRotationCacheType )
		return details[cacheNo].daysAhead;
	else
		return 1;
}

void DownloadCache::abortRefresh( void )
{
	for ( int cacheNo = 0; cacheNo < noCaches; cacheNo++ )
		state[cacheNo]->abortDownload();
}

bool DownloadCache::allDone( void )
{
	return ( cachesFinishedCount == noCaches );
}

