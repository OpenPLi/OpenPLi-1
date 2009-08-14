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

#ifndef __FAVEDIT_H__
#define __FAVEDIT_H__

#include <lib/base/estring.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/textinput.h>
#include <lib/gui/enumber.h>
#include <lib/gui/combobox.h>

#include "defs.h"
#include "util.h"

enum {
	favOrVal = 0,
	favAndVal,
	favAndOrderedVal,
	favNoneVal,
	favNoBools
};

enum {
	favHourCombineInside = 0,
	favHourCombineOutside,
};

#define FAV_EDIT_X           	60
#define FAV_EDIT_Y           	170
#define FAV_EDIT_WIDTH       	580
#define FAV_EDIT_HEIGHT      	270

#define FE_LABEL_WIDTH		130
#define FE_BOOL_WIDTH		140
#define FE_BOOL_HEIGHT		35

#define FE_LABEL_X		5
#define FE_TEXT_X		100
#define FE_TEXT_WIDTH		( FAV_EDIT_WIDTH - FE_LABEL_WIDTH + FE_LABEL_X - FE_BOOL_WIDTH - 20 )
#define FE_TEXT_HEIGHT		35


#define FE_BOOL_X		FE_TEXT_X + FE_TEXT_WIDTH + 10

#define FE_TITLE_Y		5
#define FE_DESCR_Y		60
#define FE_CHANNEL_Y		120
#define FE_NUMBERS_Y		180

#define FE_NUMBERS_WIDTH	40
#define FE_NUMBERS_HEIGHT	30

#define FE_NOTIFY_WIDTH		100

#define FE_FROM_LABEL_WIDTH	100
#define FE_FROM_LABEL_HEIGHT	30

#define FE_FROM_NUMBER_X	150
#define FE_FROM_LABEL_X		193
#define FE_TO_NUMBER_X		240
#define FE_NOTIFY_X		300

struct Favourite {
	eString title, descr, channel;
	int 	hourA, hourB;
	int 	titleBool, descrBool, hourCombine;
	bool	notifyFlag;
	std::list<char *>	*titleWordsP;
	std::list<char *>	*descrWordsP;
	std::list<char *>	*channelWordsP;
public:
	Favourite() : titleWordsP( NULL), descrWordsP(NULL), channelWordsP(NULL) {}
	Favourite( eString t, eString d, eString c, int hrA, int hrB, int tb, int db, int hc, bool notifyFlag ) : title(t), descr(d), channel(c), hourA(hrA), hourB(hrB), titleBool(tb), descrBool(db), hourCombine(hc), notifyFlag(notifyFlag), titleWordsP( NULL), descrWordsP(NULL), channelWordsP(NULL)  {}
	~Favourite() {
		deleteWords( titleWordsP );
		deleteWords( channelWordsP );
		deleteWords( descrWordsP );
	}

	void deleteWords( std::list<char *> *listP ) {
		if ( listP != NULL ) {
			std::list<char *>::iterator cur = listP->begin();

			while ( cur != listP->end() ) {
				delete (*cur);
				cur++;
			}
			delete listP;
		}
	}
	void initWords( void ) {
		if ( title.length() > 0 ) {
			titleWordsP = new std::list<char *>;
			initWordsAux( title, titleWordsP );
		}
		if ( descr.length() > 0 ) {
			descrWordsP = new std::list<char *>;
			initWordsAux( descr, descrWordsP );
		}
		if ( channel.length() > 0 ) {
			channelWordsP = new std::list<char *>;
			initWordsAux( channel, channelWordsP );
		}
	}

	void initWordsAux( eString &s, std::list<char *> *listP ) 
	{
                char regex[MAX_STRING_LENGTH+1];
                safeStrncpy( regex, (char *)s.c_str(), MAX_STRING_LENGTH );
                char *sp = regex;
                char *ep = regex;

                bool doneFlag = false;

                while ( ! doneFlag ) {

                        // Go to end of ','s
                        while ( *sp == ',' )
                                sp++;
                        ep = sp;

                        // Move end ptr to end of word
                        while (
                                ( *ep != ',' )  &&
                                ( *ep != '\0' )
                        )
                                ep++;

                        if ( *ep == '\0' )
                                doneFlag = true;

                        if ( ep != sp ) {
                                // Mark end of word
                                *ep = '\0';

				char *tmp = new char[strlen(sp)+1];
				strcpy( tmp, sp );
				listP->push_back( tmp );

                                ep++;
                                sp = ep;
                        }
                        else
                                doneFlag = true;
                }
	}
};

class FavEdit : public eWindow
{
	struct Favourite *favP;

	eTextInputField *titleTextP, *descrTextP, *channelTextP;
	eNumber *fromNumberP, *toNumberP;

	int fromInt, toInt;

	void titleBoolChanged( eListBoxEntryText *ep );
	void descrBoolChanged( eListBoxEntryText *ep );
	void hourCombineChanged( eListBoxEntryText *ep );

	void fromNumberChanged( void );
	void toNumberChanged( void );

	void notifyChanged( int newVal );

public:
	FavEdit( struct Favourite *toEdit );
	~FavEdit();
};

#endif
