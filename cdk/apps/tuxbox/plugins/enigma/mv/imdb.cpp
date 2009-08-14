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

#include "imdb.h"

// STARIMAGEP must be deleted by caller!
IMDB::IMDB( const eString &baseURL, const eString & title, int yearHint, gPixmap *starImageP, gColor filmColour ) : eWindow( 0 ) , baseURL( baseURL ), title( title ), downloadDoneFlag( false ), yearHint( yearHint ), starImageP( starImageP ), filmColour( filmColour )
{
	targetPath = prefixDir( eString(DOWNLOAD_DIR), eString(DOWNLOAD_FILENAME) );

	setText( title );
	setWidgetGeom( this,
		EXTINFO_X, EXTINFO_Y,
		EXTINFO_WIDTH, EXTINFO_HEIGHT
	);
	setFont( getNamedFontAndSize( EXTINFO_FONT, EXTINFO_FONTSIZE ));
	initList();

        CONNECT( downloader.downloadDone, IMDB::downloadDone );
}

void IMDB::run( void )
{
	eString queryTitle = stringReplace( title, ' ', '+' );
	eString queryURL = baseURL + queryTitle;

	pleaseWaitMessageP = new eMessageBox( "Please wait...", "Query IMDB", eMessageBox::iconInfo );
	pleaseWaitMessageP->show();

	// 0 = always overwrite, although shouldn't be 
	// a file there in theory

	int err = downloader.start( queryURL, DOWNLOAD_DIR, DOWNLOAD_FILENAME, 0 );

        if ( err != DL_ERR_NONE ) {
		pleaseWaitMessageP->hide();
		flashIntMessage( getStr( strErrDownload ), err );
	}
	// It could be download finished before we get here
	// There's a small race condition 
	else if ( ! downloadDoneFlag )
		exec();
}

// Set the current record of the list to the
// first one with a matching year
struct setCurrentRecord: public std::unary_function<IMDBListEntry &, void>
{
	int yearHint;
	eListBox<IMDBListEntry> *listBoxP;

        setCurrentRecord( int yearHint, eListBox<IMDBListEntry> *lboxP ) : yearHint(yearHint), listBoxP(lboxP) { }

        bool operator() ( IMDBListEntry &entry ) {
		// Match within a year: sometimes they get it
		// wrong by a year!
		if ( yearHint == entry.getYear() ) {
			if ( listBoxP != NULL )
				listBoxP->setCurrent( &entry );
			return 1;
		}
		else
			return 0;
	}
};

// If we don't catch keys we'll get focus problems
// when execing and download not finished and user
// uses remote
//
int IMDB::eventHandler(const eWidgetEvent &event)
{
	int handled = 0;
	switch (event.type) {
		case eWidgetEvent::evtKey:
                	if ( ! downloadDoneFlag ) 
				handled = 1;
                        break;
                default:
                        break;
        }

        return handled ? 1 : eWindow::eventHandler(event);
}

void IMDB::downloadDone( int downloadID, int err )
{
	downloadDoneFlag = true;

	pleaseWaitMessageP->hide();

        if ( err == DL_ERR_NONE ) {
		if ( parseFile() ) {
			// No records: DISAPPOINTED!
			if ( recordListP->getCount() == 0 ) {
				flashMessage( getStr( strIMDBNoRecords ), 1250000 );
				eWidget::accept();
			}

			// One record, or the title only had one
			// word: just show it and exit
			// The one word thing is because a single
			// word is very likely to match SOMETHING,
			else if ( 
				( recordListP->getCount() == 1 ) &&
				( strchr( title.c_str(), '+' ) != NULL ) 
			) { 
				recordSelected( recordListP->getCurrent() );
				eWidget::accept();
			}

			// Multi-records: list them
			else {
            			recordListP->forEachEntry( setCurrentRecord( yearHint, recordListP ) );
				show();
				recordListP->show();
			}
		}
	}
	else {
		flashIntMessage( getStr( strErrDownload ), err );
		eWidget::accept();
	}
}

IMDB::~IMDB()
{
	// I never know whether this is necessary or not :-)
	recordListP->hide();

	deleteFile( targetPath );
}

void IMDB::recordSelected( IMDBListEntry *entryP )
{
	// Show the detail window when user selects
	// film from the list
	if ( entryP != NULL ) {
		hide();
		ExtInfo ei(
			eString().sprintf( "%s (%d)", 
				entryP->getTitle().c_str(),
				entryP->getYear()
			),
			(char *)entryP->getPlot().c_str(),
			entryP->getRecordP(),
			starImageP, 
			filmColour
		);
		showExecHide( &ei );
		show();
	}
}

void IMDB::initList( void )
{
	recordListP = new eListBox<IMDBListEntry>(this);
        recordListP->loadDeco();
	recordListP->hide();
	setWidgetGeom( recordListP,
		0,0,
		clientrect.width(), clientrect.height()
	);
	CONNECT( recordListP->selected, IMDB::recordSelected );
}

bool IMDB::parseFile( void )
{
	XMLTreeParser *parser = getXMLParser( targetPath.c_str(), RECORD_CHARACTER_ENCODING );

	if ( parser == NULL ) {
		dmsg( getStr( strErrXMLParse ), targetPath );
		return false;
	}
	
	bool successFlag = true;

	XMLTreeNode *root = parser->RootNode();

	if ( ! root ) {
                dmsg( getStr( strErrXMLNoRootNode ), targetPath );
                successFlag = false;
        }

        if (
                ( successFlag ) &&
                strcmp( root->GetType(), ROOT_NODE_TAG )
        ) {
                dmsg( getStr( strErrXMLBadRootNode ), targetPath  );
                successFlag = false;
        }

	if ( successFlag ) {
		for ( XMLTreeNode * rec = root->GetChild(); rec; rec = rec->GetNext() ) {
	
			// Still testing 
			struct IMDBRecord tmpRec = { "a", "a", "a", "a", "a", 0, 0 };

			char *recType = rec->GetType();

			if (
				( recType == NULL ) ||
				( strcmp( recType, RECORD_TAG ) )
                        )
				continue;

			for ( XMLTreeNode * c = rec->GetChild(); c; c = c->GetNext() ) {
				const char *typeP = c->GetType();
				if ( typeP == NULL )
					continue;
				const char *dataP = c->GetData();
				if ( dataP == NULL )
					continue;

				eString dataString = eString( dataP );

				if ( ! strcmp( typeP, TITLE_TAG ) )
					tmpRec.title = dataString;
				else if ( ! strcmp( typeP, YEAR_TAG ) ) {
					tmpRec.year = strtol( dataP, (char **)NULL, 10);
					if ( 
						( tmpRec.year < 1900 ) ||
						( tmpRec.year > 2030 )
					)
						tmpRec.year = 0;
				}
				else if ( ! strcmp( typeP, PLOT_TAG ) )
					tmpRec.plot = dataString;
				else if ( ! strcmp( typeP, CAST_TAG ) )
					tmpRec.cast = dataString;
				else if ( ! strcmp( typeP, DIRECTOR_TAG ) )
					tmpRec.director = dataString;
				else if ( ! strcmp( typeP, GENRE_TAG ) )
					tmpRec.genre = dataString;
				else if ( ! strcmp( typeP, RATING_TAG ) )
					tmpRec.rating = strtof( dataP, (char **)NULL );
			}
			new IMDBListEntry( recordListP, tmpRec );
		}
	}

	delete parser;

	return successFlag;
}
