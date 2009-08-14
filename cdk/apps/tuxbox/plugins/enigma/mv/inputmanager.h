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

#ifndef __INPUTMANAGER_H__
#define __INPUTMANAGER_H__

#include "defs.h"
#include "conf.h"
#include "util.h"
#include "channelmanager.h"
#include "enigmacache.h"
#include "text.h"
#include "inputdefreader.h"
#include "xmltv.h"
#include "epgui.h"
#include "program.h"
#include "channel.h"

#include <lib/dvb/dvb.h>
#include "downloadcache.h"

extern	struct Inputs inputs;

class EnigmaCache;
class ChannelManager;
class Channel;

enum {
        dataModeLoad = 0,
        dataModeSave,
        dataModeTransferOldCache
};

class InputManager : public Object {
	char *userLangPrefP;
	int dataMode;
	FILE *cacheSaveFP;
	
	ChannelManager *channelMgrP;
	EnigmaCache *ecacheP;

	XMLTVConverter *XMLTVThread;

	int inputFlags;

	DownloadCache *dcacheP;
	bool allDownloadsDoneFlag;

	// Only used when saving enigma cache
	// so we know when to start a fresh channel
	// in the output file
	eString lastReceivedChannelName;

	short nbDataFiles;
	short maxDataFiles;
	FILE ** pDataFiles;
	char ** FileNames;
	char *encodings;

	void receiveData( struct ProgramData &p );
	void receiveOneChannelCacheData( struct ProgramData &p );

	void readEPGUIProgramData( char *path, char encoding, int offset, time_t from, time_t to, int maxChannels );
	bool analyseTimeLine( char *line, time_t *timeP );
	bool analyseChannelLine( char * line, unsigned int *id, char *channelName );

	void readRotatingProgramData( eString baseDir, struct inputDef *defP, time_t from, time_t to, int maxChannels );
	void readDatafile( char *filepath, char format, char encoding, int offset, time_t from, time_t to, int maxChannels );

	eString secondsToDateString( time_t seconds );

	int genreDescrToGenreFlags( char * descr ,time_t programLengthSeconds );
	void INT_XMLTVConvertDone( int noConverted );
public:
	InputManager( eString mapPath, char * langPref );
	~InputManager();
	void checkXMLTVNotConverting( bool showMessage );

	bool saveEnigmaCache( bool testOnly, struct Inputs &inputs );
	bool downloading( void );
	void startCache( struct Inputs &inputs );
	void readProgramData( time_t startingFrom, struct Inputs &inputs );
	bool haveCacheInput( struct Inputs &inputs );
	bool getServiceRef( eString &channelName, eServiceReferenceDVB *eRef );
	eString nextBouquetName( bool fromStartFlag );
	eServiceReferenceDVB *nextBouquetRef( bool fromStartFlag );

	bool haveUnknownChannels( void );

	void downloadStarted( void );
	void allDownloadsFinished( int count );

	void convertXMLTVFilesToEPGUI( struct Inputs inputs );


	void loadOneChannelCacheData( eServiceReferenceDVB ref );

	Signal1<void, struct ProgramData &>	gotData;
	Signal1<void, struct ProgramData &>	gotOneChannelCacheData;
	Signal1<void, bool>			MHWStatusChange;
	Signal1<void, bool>			XMLTVStatusChange;
	Signal0<void, void>			IMdownloadStarted;
	Signal1<void, int>			IMallDownloadsFinished;
	bool getDescription(Channel *cp, Program *pp, char *buffer);

#ifndef MVLITE
	void runChannelManager( void );
#endif
};

#endif
