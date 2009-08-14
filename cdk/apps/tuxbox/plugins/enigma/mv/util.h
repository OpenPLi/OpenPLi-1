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

#ifndef __UTIL_H__
#define __UTIL_H__

#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/font.h>
#include <lib/dvb/dvbservice.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/enumber.h>
#include <lib/gui/listbox.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/textinput.h>
#include <lib/gdi/gfbdc.h>
#include <xmltree.h>
#include <string.h>
#include <lib/driver/streamwd.h>
#include <lib/driver/eavswitch.h>


#include "defs.h"
#include "files.h"
#include "text.h"
#include "keys.h"


#define SECONDS_IN_A_DAY 86400
#define SECONDS_IN_A_WEEK (SECONDS_IN_A_DAY * 7)

#define MYMAX(a,b) (( a < b ) ? b : a)
#define MYMIN(a,b) (( a > b ) ? b : a)

class KeyCatcher : public eWindow
{
	eTimer *timerP;
        int KeyCatcher::eventHandler(const eWidgetEvent &event) {
		if ( event.type == eWidgetEvent::execBegin )  {
			timerP = new eTimer( eApp );
			CONNECT( timerP->timeout, KeyCatcher::timeout );
			timerP->start( 50 );
			return 1;
		}
                else if ( event.type == eWidgetEvent::evtKey ) {
			timerP->stop();
                        close(0);
			return 1;
		}
		else {
                return eWindow::eventHandler(event);
		}
	}
public:
	void KeyCatcher::timeout( void ) {
		close(0);
	}

        KeyCatcher() : eWindow(1) { timerP = NULL; }
	~KeyCatcher() { if ( timerP != NULL ) delete timerP; }
};



enum {
        encodingUTF = 'u',
        encodingLatin = 'l'
};

/*
enum {
	dockModeTopRight = 0,
	dockModeTopRightInside
};
void dockWidget( eWidget *dock, eWidget *docker, int mode );
*/

void setListboxFontSize( int toset );

time_t my_timegm (struct tm *tm);

gPixmap *getNamedPixmapP( const char *name );
int getNamedValue( const char *name );


time_t absTimeDiff( time_t one, time_t two );

eTextInputField *makeNewTextInput(
        eWidget *parent, eString text,
        int x, int y, int width, int height,
        int maxChars = 30
);

void setRestoreAspectRatio( bool setFlag, int mode );

bool isPseudoEmptyString( char * );

eButton * makeNewButton(
        eWidget * parent, int textIndex,
        int x, int y, int width, int height,
        char *shortcut = NULL
);

eButton * makeOKButton( eWidget *parent );

void changeFlagValue( int &toChange, int mask, int newValue );

void setWidgetGeom( eWidget *w, int x, int y, int width, int height );

int getEnigmaIntKey( const char *key );

eComboBox *makeNewComboBox(
        eWidget * parent,
        int x, int y, int width, int height,
        int visItems = 3
);

eCheckbox * makeNewCheckbox(
        eWidget * parent, int textIndex,
        int x, int y, int width, int height,
        int value = 0
);

eListBox<eListBoxEntryText> * makeNewListbox(
        eWidget * parent, 
        int x, int y, int width, int height,
        int flags = 0
);

eLabel * makeNewLabel( 
	eWidget *parent, eString text,
	int x, int y, int width, int height,
	int flags=0,
	gColor *backP=NULL, gColor *foreP=NULL
);
eLabel * makeNewLabel( 
	eWidget *parent, int textIndex, 
	int x, int y, int width, int height,
	int flags=0,
	gColor *backP=NULL, gColor *foreP=NULL
);

int showExecHide( eWidget *it );
void microSleep( unsigned long time );
time_t getLoadTime( void );
time_t getCurrentTime( void );
void setLoadTime( void );
eString makeEightDigitDate( time_t t );

int stringWidthPixels( const gFont &font, eString st );
void waitOnFlag( bool &flag, bool waitingFor, long int delayPeriodMicroSeconds );

int reverseBooleanInt( int todo );
bool isNumericString( char * st );
bool timeOverlap( time_t start1, time_t end1, time_t start2, time_t end2 );
bool isLetterChar( char c );
bool isNumericChar( char c );

gColor getNamedColour( const char *name );
gFont getNamedFont( const char *name );
gFont getNamedFontAndSize( const char *font, int size );


//void showTimerListWindow( void );
//void showTimerEditWindow( int type, Channel *cp, Program *pp, time_t preOffset, time_t postOffset );

void dmsg( char *msg );
void dmsg( eString msg );
void dmsg( eString msg, eString msg2 );
void dmsg( char *msg, int toShow );
void dmsg( char *msg, eString msg2 );
void dmsg( char *msg, int toShow, int toShow2 );
void dmsg( char *msg, char *msg2 );
void dmsg( char *msg, char msg2 );
void dmsg( char * msg, uint toShow );
int showMessageBox( char *header, char *stt, int mode );
//void warnMsg2( char *m1, char *m2 );
//void warnMsg( char *m1 );
//eSize rectToSize( eRect &theRect );
eString buildShortName( const eString &str );
bool checkSensibleSystemTime( void );
void flashIntMessage( eString mess, int colNo, long sleepTime = 2000000 );
void flashMessage( eString mess, long sleepTime = 2000000 );

XMLTreeParser * getXMLParser( const char * filename, const char * encoding );


eNumber *makeNewNumber(
        eWidget *parent, int *initP,
        int x, int y, int width, int height,
        int maxDigits, int minValue, int maxValue
);



void safeStrncpy( char * to, char *from, int max );
time_t offsetGMTToLocaltime( void );
bool getUserISOLangPref( char * charbuffer2, char * charbuffer3 );

eString stringReplace( const eString &toReplace, char from, char to );
void removeTrailingNewlines( char *toRemove );

//char *myStrcasestr (char *hay, char *needle);

eString & indexToDir( int index );
int dirToIndex( eString &dir );
int noDirs( void );

void showHideWidget( eWidget *w, bool showFlag );

eString mungeWordsToLowerCase( eString toDo );

eString prefixDir( eString baseDir, eString relPath );

eString timeToHourMinuteString( time_t t );


gColor getColourFromRGB( int r, int g ,int b, int a );
gColor getColourFromRGB( const struct gRGB &tmp  );
struct gRGB getRGBFromColour( const gColor & col );
gColor adjustedColour( const gColor &toAdjust, int dr, int dg, int db, int da = 0);

bool userConfirms( int confirmTextIndex );

bool dirIsMountPoint( const char *dir );

bool fileNeedsRefreshing( const eString &path, time_t maxAge );

void showInfoBox( eWidget *parent, int title, int message );

bool isGraphicalView( int viewNo );
int viewNoToKeyNo( int viewNo );
int keyNoToViewNo( int keyNo );
char * mystrcasestr( char *s, char *d );


void convertToUpper( eString &str);
eString convertToUTF( const char *toConvert, char encoding );


eTextPara * makeNewTextPara(
	const eString &text,
        int x, int y, int width, int height,
        const gFont & font,
	int realignFlags = 0
);


int centreImage( int space, int imageSize, int startCoord = 0 );


void mylogReset( void );
void mylog( eString towrite );
void mylogWrite( const char *towrite, const char *mode );

int findYearInString( const char *haystackP );

bool loadSkin( eString path, const char *flagName, bool forceFlag );

bool haveNetwork( void );

void makeExecutable( const eString &path );


#endif
