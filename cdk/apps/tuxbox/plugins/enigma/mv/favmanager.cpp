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

#include "favmanager.h"

FavManager::FavManager( eString path )
	: eWindow( 1 ) , filePath( path ), dirtyFlag( false ), matchesInitialisedFlag( false ) 
{
	setText( getStr( strFavManagerTitle ) );
	setWidgetGeom( this,
		FAV_MANAGER_X, FAV_MANAGER_Y,
		FAV_MANAGER_WIDTH, FAV_MANAGER_HEIGHT
	);

	// RED, DELETE
        eButton *bt = makeNewButton(
                this, strButtonDelete,
                FAV_RED_BUTTON_X, BUTTON_ROW_Y,
                BUTTON_WIDTH, BUTTON_HEIGHT,
                "red"
        );
	CONNECT(bt->selected, FavManager::deleteHandle );

        // GREEN, DONE
        bt = makeNewButton(
                this, strButtonDone,
                FAV_GREEN_BUTTON_X, BUTTON_ROW_Y,
                BUTTON_WIDTH, BUTTON_HEIGHT,
                "green"
        );
	CONNECT(bt->selected, eWidget::accept );

        // YELLOW, EDIT
        bt = makeNewButton(
                this, strButtonEdit,
                FAV_YELLOW_BUTTON_X, BUTTON_ROW_Y,
                BUTTON_WIDTH, BUTTON_HEIGHT,
                "yellow"
        );
	CONNECT(bt->selected, FavManager::editHandle );

        // BLUE, ADD
        bt = makeNewButton(
                this, strButtonAdd,
                FAV_BLUE_BUTTON_X, BUTTON_ROW_Y,
                BUTTON_WIDTH, BUTTON_HEIGHT,
                "blue"
        );
	CONNECT( bt->selected, FavManager::addHandle );

	theList = new eListBox<FavManEntry>(this);
	setWidgetGeom( theList,
		FAV_LISTBOX_X, FAV_LISTBOX_Y,
		FAV_LISTBOX_WIDTH, FAV_LISTBOX_HEIGHT
	);
	theList->loadDeco();
	theList->setColumns(3);
	CONNECT( theList->selected, FavManager::selected );

	readFile();
}


struct writeToFileFunc: public std::unary_function<FavManEntry &, void> 
{
	FILE *fileP;

	writeToFileFunc( FILE * fileP) : fileP(fileP) { }

	bool operator() ( FavManEntry &entry ) {
		struct Favourite *fp = entry.getFavouriteP();
		if ( fp->title.length() == 0 )
			fp->title = "-";
		if ( fp->descr.length() == 0 )
			fp->descr = "-";
		if ( fp->channel.length() == 0 )
			fp->channel = "-";
		fprintf( fileP, "%s %d %s %d %s %d %d %d %d\n",
			fp->title.c_str(), fp->titleBool,
			fp->descr.c_str(), fp->descrBool,
			fp->channel.c_str(),
			fp->hourA, fp->hourB, fp->hourCombine,
			fp->notifyFlag
		);
		return 0;
	}
};

void FavManager::addNameAndChannelEntry( eString pName, eString cName )
{
	addHandleOne(
		new struct Favourite(
			pName, "", cName,
			0, 24,
			favAndOrderedVal, favAndVal,
			favHourCombineInside,
			true	
		)
	);
}

void FavManager::writeFile( void )
{
	FILE *fp = fopen( filePath.c_str(), "w" );

        if ( fp ) {
		fprintf( fp, "# MV favourite definitions\n# See http://mv.dinglisch.net/userguide.html#fav\n#\n" );
		theList->forEachEntry( writeToFileFunc( fp ) );
		fclose(fp);
        }
	else {
		dmsg( getStr( strErrWritingFavourites ), filePath.c_str() );
	}
}

void FavManager::readFile( void )
{
	FILE * fp = fopen( filePath.c_str(), "r" );
	if ( fp ) {
                char line[MAX_STRING_LENGTH+1];
                struct Favourite *tempFavP;
                while( fgets( line, MAX_STRING_LENGTH, fp ) ) {
                        if ( *line == '#' )
                                continue;
			int tempNotifyFlag;
			char titleString[MAX_STRING_LENGTH+1], descrString[MAX_STRING_LENGTH+1], channelString[MAX_STRING_LENGTH+1];
                        tempFavP = new( struct Favourite );
                        int matched = sscanf( line,
				"%s %d %s %d %s %d %d %d %d",
				titleString, &(tempFavP->titleBool),
				descrString, &(tempFavP->descrBool),
				channelString, 
				&(tempFavP->hourA), &(tempFavP->hourB), &(tempFavP->hourCombine),
				&(tempNotifyFlag)
			);

                        if ( matched == 9 ) {
				tempFavP->notifyFlag = ( tempNotifyFlag < 1 ) ? 0 : 1;
				if ( ! isPseudoEmptyString( titleString ) )
					tempFavP->title = eString( titleString );
				else
					tempFavP->title=eString("");
				if ( ! isPseudoEmptyString( descrString ) )
					tempFavP->descr = eString( descrString );
				else
					tempFavP->descr=eString("");
				if ( ! isPseudoEmptyString( channelString ) )
					tempFavP->channel = eString( channelString );
				else
					tempFavP->channel = eString("" );
				new FavManEntry( theList, tempFavP );
                        }
                        else {
                                dmsg( getStr( strErrBadFavouritesLine ), line );
                                delete tempFavP;
                        }
                }
                fclose( fp );
        }
}


// Return value is whether to stop or not
// 1 = stop

struct matchFunc : public std::unary_function<FavManEntry &, void> 
{
	struct ProgramData *pp;
	bool *notifyFlagP;

	matchFunc(struct ProgramData *p, bool *notifyFlagP ) : pp(p), notifyFlagP(notifyFlagP) {}
	
	bool operator() ( FavManEntry &entry ) {
		struct Favourite *fp = entry.getFavouriteP();
		
		unsigned int noMatches;

		// TITLE CHECK

		if ( fp->titleWordsP != NULL ) {
			noMatches = doMatchCount( 
				fp->titleWordsP, pp->name, 
				( ( fp->titleBool == favOrVal ) || ( fp->titleBool == favNoneVal ) ),
				( fp->titleBool == favAndOrderedVal )
			);
			if ( 
				( 
					( fp->titleBool == favNoneVal ) &&
					( noMatches > 0 )
				) ||
				(	
					( 
						( fp->titleBool == favAndVal ) ||
						( fp->titleBool == favAndOrderedVal ) 
					) &&
					( noMatches < fp->titleWordsP->size() )
				) ||
				(
					( fp->titleBool == favOrVal ) &&
					( noMatches == 0 )
				)
			)
				return 0;
		}

		// DESCR CHECK

		if ( fp->descrWordsP != NULL ) {
			noMatches = doMatchCount( 
				fp->descrWordsP, pp->descr, 
				( ( fp->descrBool == favOrVal ) || ( fp->descrBool == favNoneVal ) ),
				( fp->descrBool == favAndOrderedVal )
			);
			if ( 
				( 
					( fp->descrBool == favNoneVal ) &&
					( noMatches > 0 )
				) ||
				(	
					(
						( fp->descrBool == favAndVal ) ||
						( fp->descrBool == favAndOrderedVal ) 
					) &&
					( noMatches < fp->descrWordsP->size() )
				) ||
				(
					( fp->descrBool == favOrVal ) &&
					( noMatches == 0 )
				)
			)
				return 0;
		}

		// CHANNEL CHECK

		// If any channel words specified, channel must match one of

		if ( fp->channelWordsP != NULL ) {
			noMatches = doMatchCount( 
				fp->channelWordsP, pp->channelName, true, false
			);
			if ( noMatches == 0 )
				return 0;
		}

		// TIME CHECK
		
		int hour = pp->getStartHour();
	
		if ( fp->hourCombine == favHourCombineInside ) {
			if ( 
				( hour < fp->hourA ) ||
				( hour > fp->hourB )
			)
				return 0;
		}
		// outside
		else if ( 
			( hour >= fp->hourA ) &&
			( hour <= fp->hourB )
		)
			return 0;

		// Found: stop here, don't need to check
		// any more entries

		*notifyFlagP = fp->notifyFlag;

		return 1;
	}

	unsigned int doMatchCount( std::list<char *> *regexP, eString haystack, bool onlyOneNeededFlag, bool needOrderFlag )
	{
		unsigned int matchCount = 0;
	
		std::list<char *>::iterator cur = regexP->begin();
		bool doneFlag = false;

		char *startOfHaystack = (char *) haystack.c_str();
		char *nextMatchStart = startOfHaystack;

		while ( 
			( ! doneFlag ) &&
			( cur != regexP->end() )
		) {
			char *needleLoc = mystrcasestr( nextMatchStart, *cur );
			if ( needleLoc != NULL ) {
				matchCount++;
				if ( onlyOneNeededFlag ) 
					doneFlag = true;
			}
			else if ( needOrderFlag )
				doneFlag = true;

			if ( needOrderFlag )
				nextMatchStart = needleLoc;
			else
				nextMatchStart = startOfHaystack;
			cur++;
		}
		
		return matchCount;
	}
};

struct initFunc : public std::unary_function<FavManEntry &, void> 
{
	bool operator() ( FavManEntry &entry ) {
		entry.getFavouriteP()->initWords();
		return 0;	
	}
};

void FavManager::checkFavourite( struct ProgramData *pp, bool *isFavouriteFlagP, bool *notifyFlagP )
{
	if ( ! matchesInitialisedFlag ) {
		theList->forEachEntry( initFunc() );
		matchesInitialisedFlag = true;
	}

	*isFavouriteFlagP = ( ! theList->forEachEntry( matchFunc( pp, notifyFlagP ) ) );
}

FavManager::~FavManager()
{
	if ( dirtyFlag )
		writeFile();
}

void FavManager::editHandle( void )
{
	FavManEntry *entryP = theList->getCurrent();
	if ( entryP ) {
		doEdit( entryP->getFavouriteP() );
		dirtyFlag = true;
	}
}

void FavManager::addHandle( void )
{
	addHandleOne( 
		new struct Favourite(
		DEFAULT_FE_TITLE, DEFAULT_FE_DESCR, DEFAULT_FE_CHANNEL,
		DEFAULT_FE_HOUR_A, DEFAULT_FE_HOUR_B,
		DEFAULT_FE_TITLE_BOOL, DEFAULT_FE_DESCR_BOOL,
		DEFAULT_FE_HOUR_COMBINE,
		DEFAULT_FE_NOTIFY
		)
	);
}

void FavManager::addHandleOne( struct Favourite *newFavP )
{	
	// Need a title or descr to add
	if ( 
		doEdit( newFavP ) &&
		(
			( newFavP->title.length() > 0 ) ||
			( newFavP->descr.length() > 0 )
		)
	) {
		dirtyFlag = true;
		if ( isVisible() )
			theList->hide();

		FavManEntry *newP = new FavManEntry( theList, newFavP );
		theList->setCurrent( newP );

		if ( isVisible() )
			theList->show();
	}
	else
		delete newFavP;
}

void FavManager::selected( FavManEntry *entry )
{
	if ( entry ) {
		doEdit( entry->getFavouriteP() );
		dirtyFlag = true;
	}
}

bool FavManager::doEdit( struct Favourite *toEditP )
{
	FavEdit *edit = new FavEdit( toEditP );
	hide();
	int ret = showExecHide( edit );
	delete edit;
	show();
	return ( ret != -1 );
}

void FavManager::deleteHandle( void )
{
	FavManEntry *entryP = theList->getCurrent();
	if ( entryP ) {
		theList->remove( entryP );
		dirtyFlag = true;
	}
}
