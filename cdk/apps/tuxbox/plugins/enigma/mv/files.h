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

#ifndef __FILES_H__
#define __FILES_H__

#include <sys/dir.h>
#include <sys/stat.h>
#include <unistd.h>
#include <lib/base/estring.h>
#include <stdio.h>
#include <utime.h>
#include <sys/types.h>

int fileSize( eString filePath );
int moveFileSameFS( eString from, eString to );
int deleteFile( eString & filePath );
int makeDirectory( eString path );
bool isDirectory( const eString &path );
int pathExists( eString path );
void deleteOldFilesInDir( eString path, time_t maxAge );
int setMtime( eString path, time_t toSet );
int makeDirIfNotExists( eString path );
time_t fileMtime( eString filePath );
eString getNextFileInDir( eString &dir, bool fromStart );
bool writeStringToFile( eString &path, char *fill );


#endif
