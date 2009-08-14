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

#ifndef __IMDB_H__
#define __IMDB_H__

#include <lib/gui/ewindow.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>

#include "util.h"
#include "conf.h"
#include "downloader.h"
#include "imdblistentry.h"
#include "extinfo.h"

#define DOWNLOAD_DIR	"/var/tmp"
#define DOWNLOAD_FILENAME "imdb.download"

#define ROOT_NODE_TAG	"imdb"
#define RECORD_TAG	"imdb-record"
#define TITLE_TAG	"title"
#define PLOT_TAG	"plot"
#define CAST_TAG	"cast"
#define GENRE_TAG	"genre"
#define DIRECTOR_TAG	"director"
#define RATING_TAG	"rating"
#define YEAR_TAG	"year"

#define RECORD_CHARACTER_ENCODING "UTF-8"

class IMDB : public eWindow
{
	Downloader downloader;
	const eString &baseURL;
	const eString &title;
	bool downloadDoneFlag;
	eMessageBox *pleaseWaitMessageP;

	int yearHint;
	gPixmap *starImageP;
	gColor filmColour;
	eString targetPath;

	eListBox<IMDBListEntry> *recordListP;

	bool parseFile( void );
	
	void downloadDone( int downloadID, int err );

	void initList( void );
	void recordSelected( IMDBListEntry *entryP );
	int eventHandler(const eWidgetEvent &event);
public: 
	// PIXMAP MUST BE DELETED BY CALLER
	IMDB( const eString &baseURL, const eString &title, int yearHint, gPixmap *starImageP, gColor filmColour );
	~IMDB();
	void run( void );
};

#endif
