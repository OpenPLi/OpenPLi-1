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

#ifndef __DOWNLOADER_H__
#define __DOWNLOADER_H__

#include <lib/system/httpd.h>
#include <upgrade.h>
#include "util.h"
#include "files.h"

#define DL_ERR_NONE             0
#define DL_ERR_FILE_EXISTS      -1
#define DL_ERR_CANT_MAKE_DIR    -2
#define DL_ERR_CONNECTION       -3
#define DL_ERR_SUB              -4
#define DL_ERR_RESPONSE_CODE            -5
#define DL_ERR_RENAME           -6
#define DL_ERR_ZERO_BYTES               -7
#define DL_ERR_FILE_DOESNT_EXIST                -8


enum {
	useTempDirFlag = 1,
	overwriteFlag = 2
};

class Downloader : public Object {
	eString 		sourceURL;
	eString			filename;
	eString 		destDir;
	eHTTPConnection *	connectionP;
        eHTTPDataSource *	dataSinkP;
	eString			tempDir;
	int 			flags;
	int			id;

	void transferDone( int err );
        eHTTPDataSource *	createDownloadSink( eHTTPConnection *conn );

public:
	Signal2<void,int,int> downloadDone;

	Downloader( int id = 0 );
	~Downloader();
	void setTempDir( eString toSet );
	void setOverwrite( void );
	int start( eString theURL, eString theTargetDir, eString theFilename, time_t overwriteAge = 999999 );
	void abort( void );
	

};

#endif
