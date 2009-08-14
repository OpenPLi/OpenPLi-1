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

#ifndef __DOWNLOADCACHE_H__
#define __DOWNLOADCACHE_H__

#include "downloader.h"
#include "util.h"

#define MAX_NO_CACHES	10

// WARNING: don't reuse refresh, code wasn't finished for
// that 

enum {
	dailyRotationCacheType,
	fixedFileCacheType
};

struct CacheDetails {
	int type;
	eString cacheDir;
	union {
		int daysAhead;
	};
	eString baseURL;
	eString filePrefix;
	eString fileAffix;
};

class DownloadState {
        int status;
        int downloadNo;
	Downloader *downP;
public:
        DownloadState( int st, int dno ) : status(st), downloadNo(dno), downP( NULL )
 {}
        ~DownloadState() { if ( downP != NULL ) delete downP; }
	void setDownloader( Downloader *dl ) { downP = dl; }
	void abortDownload( void ) { if ( downP != NULL ) { downP->abort(); delete downP; downP = NULL; } }
	void setStatus( int toSet ) { status = toSet; }
        int getStatus() { return status; }
        int getDownloadNo() { return downloadNo; }
	void resetDownloadNo( void ) { downloadNo = 0; }
	void incDownloadNo( void ) { downloadNo++; }
};


class DownloadCache : public Object
{
	int 			noCaches;
	struct CacheDetails 	details[MAX_NO_CACHES];
	class DownloadState *	state[MAX_NO_CACHES];
	int cachesFinishedCount;
	int flags;
	int successfullDownloads;
	
	eString makeFilename( int cacheType, eString prefix, eString affix, int downloadNo );

	void startNextDownload( int cacheNo );
	void downloadDone( int cacheNo, int err );
	int lastDownloadNo( int cacheNo );
	void doIncDownload( int cacheNo );
	void doCacheFinished( int cacheNo );
public:
	DownloadCache();
	~DownloadCache();
	int addCache( struct CacheDetails *details );
	void refresh( void );
	void tidy ( time_t maxAge );
	void abortRefresh( void );
	void setParallelDownload( void );
	bool allDone( void );
	void setPostDownloadScript( const char * script );

	Signal0<void,void> downloadStarting;
	Signal1<void,int> allDoneSignal;
};

#endif

