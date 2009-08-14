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

#include "downloader.h"

Downloader::Downloader( int did ) : 
	connectionP(NULL), dataSinkP(NULL), flags( 0 ), id( did )
{
}

Downloader::~Downloader()
{
	if ( connectionP != NULL )
		delete connectionP;
}

void Downloader::setTempDir( eString toSet )
{
	flags |= useTempDirFlag;
	tempDir = toSet;

}

int Downloader::start( eString theURL, eString theTargetDir, eString theFilename, time_t overwriteAge )
{
	filename = theFilename;
	sourceURL = theURL;
	destDir = theTargetDir;

	eString targetPath = destDir + eString( "/" ) + filename;

	// Stop if target dir didn't exist and we couldn;t
	// create it

	if ( ! makeDirIfNotExists( destDir ) ) 
		return DL_ERR_CANT_MAKE_DIR;

	// Stop if target file already exists and it's
	// younger than the max overwrite age

	if ( ! fileNeedsRefreshing( targetPath, overwriteAge ) )
		return DL_ERR_FILE_EXISTS;

	// Stop if temp dir don't exist and we
	// can't make it

	if ( flags & useTempDirFlag ) {
		if ( ! (
			( 
				pathExists( tempDir ) &&
				isDirectory( tempDir )
			) ||
			makeDirectory( tempDir )
		) ) 
			return DL_ERR_CANT_MAKE_DIR;
	}

        int error = DL_ERR_NONE;
        connectionP = eHTTPConnection::doRequest( sourceURL.c_str(), eApp, &error);

        if ( ! connectionP || error ) {
		error = DL_ERR_CONNECTION;
        }
        else {
                CONNECT( connectionP->transferDone, Downloader::transferDone);
                CONNECT( connectionP->createDataSource, Downloader::createDownloadSink );

                connectionP->local_header["User-Agent"]=USER_AGENT_STRING;
                connectionP->start();
        }
	return error;
}

void Downloader::abort( void )
{
        if ( connectionP != NULL )
        {
                delete connectionP;
                connectionP = NULL;
        }
}

void Downloader::transferDone( int err )
{
	eString tempPath = tempDir + eString("/") + filename;
	eString destPath = destDir + eString( "/") + filename;

	eString sourcePath;
	if ( flags & useTempDirFlag )
		sourcePath = tempPath;
	else 
		sourcePath = destPath;

        if ( err ) {
		mylog( ( eString( "Error: failed download from ") + sourceURL + eString( " with code: " )) + eString().sprintf( "%d", err ) );
		err = DL_ERR_SUB;
	}
	else if ( ! connectionP ) {
		mylog( eString( "Error: failed download (no connection) from ") + sourceURL );
		err = DL_ERR_CONNECTION;
	}
	else if ( connectionP->code != 200 ) {
		mylog( eString( "Error: failed download from ") + sourceURL + eString( ", response code: " ) + eString().sprintf( "%d", (int)connectionP->code ) );
		err = DL_ERR_RESPONSE_CODE;
		deleteFile( destPath );
	}
	else {
		err = DL_ERR_NONE;

		int filesize = fileSize( sourcePath );

		if ( filesize > 0 ) {
			if ( flags & useTempDirFlag ) {
				if ( moveFileSameFS( sourcePath, destPath ) == -1 ) {
					flashMessage( getStr( strErrDownloadFailedRenameTempfile ), sourcePath );
					err = DL_ERR_RENAME;
				}
			}
		}
		else if ( filesize == 0 ) {
			flashMessage( getStr( strErrDownloadFilesizeZero ), sourcePath );
			deleteFile( sourcePath );	
			err = DL_ERR_ZERO_BYTES;
		}
		else if ( filesize == -1 ) {
			flashMessage( getStr( strErrDownloadNoFile ), sourcePath );
			err = DL_ERR_FILE_DOESNT_EXIST;
		}
	}

	// Apparently it gets deleted somewhere else unless
	// the enigma code is incorrect

	connectionP = NULL;

	if ( flags & useTempDirFlag )
		deleteFile( tempPath );

	// Execute post-download script
	if ( err == DL_ERR_NONE ) {
		eString script = prefixDir( CONFIG_DIR, POST_DOWNLOAD_SCRIPT_NAME );
		if ( pathExists( script ) ) {
			makeExecutable( script );
			eString com = script + " " + destPath + " >> " + eString( LOG_PATH ) + " 2>&1";
			mylog( com );
			system( com.c_str() );
		}

		// Set mtime
		if ( ! setMtime( destPath, getCurrentTime() ) == -1 ) 
			flashMessage( getStr( strErrCouldntSetModifyTime ), destPath );
	}

	/*emit*/ downloadDone( id, err );
}

eHTTPDataSource * Downloader::createDownloadSink( eHTTPConnection *conn ) {
	eString downloadPath;

	if ( flags & useTempDirFlag )
		downloadPath = tempDir + "/" + filename;
	else
		downloadPath = destDir + "/" + filename;

       	dataSinkP = new eHTTPDownload( connectionP, (char *)downloadPath.c_str() );

        return dataSinkP;
}



