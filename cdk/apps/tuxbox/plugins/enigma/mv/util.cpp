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

#include <string>
#include "util.h"
#include "myiso639.h"

// This is just a dummy class to let us set the
// font size of list boxes
class myListBoxEntryText : public eListBoxEntryText
{
        int ps;
public:
        myListBoxEntryText( int pointSize ) : eListBoxEntryText( NULL
) , ps( pointSize ) { font.pointSize = ps; }
        int getEntryHeight() { return ( ps  ); }
};

// MUST RESET TO 0 AFTER USE
void setListboxFontSize( int toset )
{
	myListBoxEntryText bob( toset );
}

void changeFlagValue( int &toChange, int mask, int newValue )
{
	if ( newValue == 0 )
		toChange &= ( ~mask );
	else
		toChange = toChange | mask;
}

int getEnigmaIntKey( const char *keyName ) {
	int value = -1;
	eConfig::getInstance()->getKey( keyName, value );
	return value;
;
}

eButton * makeOKButton( eWidget *parent )
{
        const eRect &cr = parent->getClientRect();

        return makeNewButton( parent, strButtonDone,
                cr.width() - 80, cr.height() -30, 78, 28,
                "green"
        );
}



eTextInputField *makeNewTextInput( 
	eWidget *parent, eString text,
	int x, int y, int width, int height,
	int maxChars 
) {
	eTextInputField *tmp = new eTextInputField( parent ); 
	tmp->setMaxChars( maxChars ); 
	setWidgetGeom( tmp, x, y, width, height );
	tmp->loadDeco(); 
	tmp->setText( text );
	return tmp;	
}
	

eNumber *makeNewNumber( 
	eWidget *parent, int *initP,
	int x, int y, int width, int height,
	int maxDigits, int minValue, int maxValue
) {
	eNumber *tmp = new eNumber( parent, maxDigits, minValue, maxValue, 1, initP );
        tmp->setNumber( *initP );
	setWidgetGeom( tmp, x, y, width, height );
        tmp->loadDeco();
        tmp->setFlags( 0 );
        tmp->show();
	return tmp;
}

gFont getNamedFontAndSize( const char *font, int size )
{
	gFont f = getNamedFont( font );
	f.pointSize = size;
	return f;
}

eListBox<eListBoxEntryText> * makeNewListbox( 
	eWidget * parent,
	int x, int y, int width, int height, 
	int flags
) {
	eListBox<eListBoxEntryText> * tmp = new eListBox<eListBoxEntryText>(parent);
	if ( flags != 0 )
	        tmp->setFlags( flags );
	setWidgetGeom( tmp, x, y, width, height );
        tmp->loadDeco();
	return tmp;
}

eCheckbox * makeNewCheckbox( 
	eWidget * parent, int textIndex,
	int x, int y, int width, int height, 
	int value
) {
	eCheckbox *tmp = new eCheckbox( parent, value );
	setWidgetGeom( tmp, x, y, width, height );
	if ( textIndex != strNoStrings )
	        tmp->setText( getStr( textIndex ) );
        tmp->show();
	return tmp;
}

eComboBox *makeNewComboBox(
	eWidget * parent,
	int x, int y, int width, int height, 
	int visItems
) {
        eComboBox *noColP = new eComboBox( parent, visItems, 0 );
	setWidgetGeom( noColP, x, y, width, height );
        noColP->loadDeco();
	return noColP;
}

eLabel * makeNewLabel( 
	eWidget * parent, int textIndex,
	int x, int y, int width, int height, 
	int flags, gColor *backP, gColor *textP 
) {
	return makeNewLabel( 
		parent, getStr( textIndex ), 
		x, y, width, height, flags, backP, textP
	);
}

eLabel * makeNewLabel( 
	eWidget * parent, eString text,
	int x, int y, int width, int height, 
	int flags, gColor *backP, gColor *textP 
) {
        eLabel * tmp = new eLabel( parent );
        tmp->setText( text );
	setWidgetGeom( tmp, x, y, width, height );
	if ( backP != NULL )
		tmp->setBackgroundColor( *backP);
	if ( textP != NULL )
		tmp->setForegroundColor( *textP);
        tmp->setFlags( flags );
	if ( parent != NULL )
	        tmp->show();
        return tmp;
}

eButton * makeNewButton( 
	eWidget * parent, int textIndex, 
	int x, int y, int width, int height, 
	char *shortcut
) {
        eButton * tmp = new eButton( parent );
	setWidgetGeom( tmp, x, y, width, height );
	if ( shortcut != NULL ) {
		tmp->setShortcut( shortcut );
		tmp->setShortcutPixmap( shortcut );
	}
	tmp->loadDeco();
        tmp->setText( getStr( textIndex ) );

        tmp->show();
        return tmp;
}

eString makeEightDigitDate( time_t t )
{
	tm tStruct = *localtime( &t );
	return eString().sprintf( "%.4d%.2d%.2d", tStruct.tm_year + 1900, tStruct.tm_mon + 1, tStruct.tm_mday);
}

time_t absTimeDiff( time_t one, time_t two )
{
	if ( one > two ) 
		return one - two;
	else
		return two - one;
}

// Could be > 1...
int reverseBooleanInt( int toDo )
{
	if ( toDo == 0 )
		toDo = 1;
	else
		toDo = 0;
	return toDo;
}

gPixmap *getNamedPixmapP( const char *name )
{
	return eSkin::getActive()->queryImage( name );
}

int getNamedValue( const char *name )
{
	return eSkin::getActive()->queryValue( name, -1 );
}

gFont getNamedFont( const char *name )
{
	return eSkin::getActive()->queryFont( name );
}

void flashIntMessage( eString mess, int no, long sleepTime )
{
	eString num;
	num.setNum( no );
	eString fmess = mess + " " + num;
	flashMessage( fmess, sleepTime );
}

void flashMessage( eString mess, long sleepTime )
{
	eMessageBox msg( mess, "Info", eMessageBox::iconInfo );
	msg.show();
	microSleep( sleepTime );
	msg.hide();
}

gColor getNamedColour( const char *name )
{
	return eSkin::getActive()->queryColor( name );
}


time_t getCurrentTime( void )
{
	return ( time(0) + eDVB::getInstance()->time_difference );
}


static time_t loadTime = 0;
void setLoadTime( void )
{
	loadTime = getCurrentTime();
}

time_t getLoadTime( void )
{
	return loadTime;
}

int stringWidthPixels( const gFont &font, eString st )
{
	eTextPara *tmp = makeNewTextPara(
		st, 
		0,0, 400,30,
		font
	);
	int width = tmp->getBoundBox().width();
	tmp->destroy();
	return width;
}

void microSleep( unsigned long time )
{
	struct timeval  tv;

        tv.tv_sec = 0;
        tv.tv_usec = time;
        select( 0, 0, 0, 0, &tv );
}

bool isLetterChar( char c )
{
	return (
		( 
			( c >= 'a' && c <= 'z' )
		) ||
		( 
			( c >= 'A' && c <= 'Z' )
		) 
	);
}

bool isNumericChar( char c )
{
	return ( c >= '0' && c <= '9' );
}

bool timeOverlap( time_t start1, time_t end1, time_t start2, time_t end2 )
{
	return ( 
		(
			( start1 >= start2 ) &&
			( start1 <= end2 )
		) ||	
		(
			( end1 >= start2 ) &&
			( end1 <= end2 )
		)
	);
}
bool isNumericString( char * st )
{
        int length = strlen( st );
        int count = 1;
        char * p = st;
        while ( count < length ) {
                if ( ! isNumericChar( *p ) )
			if( (count >1) && (*p==' '||*p=='\t'||*p=='\n'))
				return true;
			else
                        	return false;
		count++; p++;
        }
        return true;
}

eString buildShortName( const eString &str )
{
        eString tmp;
        static char stropen[3] = { 0xc2, 0x86, 0x00 };
        static char strclose[3] = { 0xc2, 0x87, 0x00 };
        unsigned int open=eString::npos-1;

        while ( (open = str.find(stropen, open+2)) != eString::npos )
        {
                unsigned int close = str.find(strclose, open);
                if ( close != eString::npos )
                        tmp+=str.mid( open+2, close-(open+2) );
        }
        return tmp;
}

/*
void warnMsg( char *msg1 )
{
	char buffer[MAX_STRING_LENGTH+1]; 
	strcpy( buffer, msg1 );
	buffer[MAX_STRING_LENGTH] = '\0';
	showMessageBox( "Warning", buffer, eMessageBox::iconWarning|eMessageBox::btOK );
}
void warnMsg2( char *msg1, char * msg2 )
{
	char buffer[MAX_STRING_LENGTH+1]; 
	strcpy( buffer, msg1 );
	buffer[MAX_STRING_LENGTH] = '\0';
	strcat( buffer, msg2 );
	buffer[MAX_STRING_LENGTH] = '\0';
	showMessageBox( "Warning", buffer, eMessageBox::iconWarning|eMessageBox::btOK );
}
*/

void dmsg( eString stt )
{
	dmsg( (char*)stt.c_str() );
}
void dmsg( char *stt, eString stt2 )
{
	dmsg( stt, (char*)stt2.c_str() );
}
void dmsg( eString stt, eString stt2 )
{
	dmsg( (char*)stt.c_str(), (char*)stt2.c_str() );
}

void dmsg( char *stt )
{
	showMessageBox( "Diagnostic", stt, eMessageBox::iconWarning|eMessageBox::btOK );
}

//btOK=1, btCancel=2, btYes=4, btNo=8, btMax};
//iconInfo=16, iconWarning=32, iconQuestion=64, iconError

int showMessageBox( char *header, char *stt, int mode )
{
        eMessageBox msg( stt, header, mode );
	int ret = showExecHide( &msg );
	return ret;
}

void dmsg( char * msg, char msg2 )
{
        char buffer[200];
	strcpy( buffer, msg );
	unsigned int last = strlen(buffer);
	buffer[last] = msg2;
	buffer[last+1] = '\0';
	dmsg( buffer );
}

void dmsg( char * msg, char *msg2 )
{
        char buffer[200];
	strcpy( buffer, msg );
	strcat( buffer, msg2 );
	dmsg( buffer );
}

void dmsg( char * msg, int toShow, int toShow2 )
{
        char buffer[200];
        char nbuffer[20];
	strcpy( buffer, msg );
        sprintf( nbuffer, "%i", toShow );
	strcat( buffer, nbuffer );
	strcat( buffer, ", " );
        sprintf( nbuffer, "%i", toShow2 );
	strcat( buffer, nbuffer );
        dmsg( buffer );
}
void dmsg( char * msg, int toShow )
{
        char buffer[200];
        char nbuffer[20];
	strcpy( buffer, msg );
        sprintf( nbuffer, "%i", toShow );
	strcat( buffer, nbuffer );
        dmsg( buffer );
}

void dmsg( unsigned int toShow )
{
        char buffer[200];
        sprintf( buffer, "%d", toShow );
        dmsg( buffer );
}

/*
eSize rectToSize( eRect &theRect )
{
	ePoint tl = theRect.topLeft();
	ePoint br = theRect.bottomRight();

	eSize size( br.x() - tl.x(), br.y() - tl.y() );

	return size;
}
*/


#define FIRST_REASONABLE_TIME   1078415416
#define LAST_REASONABLE_TIME    1420070400	// 1-1-2015  00:00:00

bool checkSensibleSystemTime( void )
{
	time_t now = getCurrentTime();
        return (
                ( now > FIRST_REASONABLE_TIME ) &&
                ( now < LAST_REASONABLE_TIME )
        );
}

XMLTreeParser * getXMLParser( const char * filename, const char *encoding )
{
        XMLTreeParser *parser = NULL;

        FILE *in = fopen( filename, "rt" );

        if ( in ) {
//                parser = new XMLTreeParser("ISO-8859-1");
                parser = new XMLTreeParser( encoding );
                char buf[2048];

                int done;
                do {
                        unsigned int len=fread(buf, 1, sizeof(buf), in);
                        done = ( len < sizeof(buf) );
                        if ( ! parser->Parse( buf, len, done ) ) {
                                mylog( 
					eString().sprintf(
				              	"%s: XMLTV parse error: %s at line %d",
						filename, 
				              	parser->ErrorString(parser->GetErrorCode()),
       	         				parser->GetCurrentLineNumber()
                              		)
				);
                                delete parser;
                                parser = NULL;
                                done = true;
                        }
                } while ( ! done );

                fclose(in);
        }

        return parser;
}

// 3chrabuffer Buffer must be 4 characters
// 2chrabuffer Buffer must be 3 characters
bool getUserISOLangPref( char *charbuffer2, char *charbuffer3 )
{
	charbuffer2[0] = '\0';
	charbuffer3[0] = '\0';

	char *userPrefP;
        if ( eConfig::getInstance()->getKey( LANGUAGE_KEY, userPrefP ) ) {
                userPrefP = NULL;
		return false;
	}

	// Turn it into a two-letter county code

	if ( strcmp( userPrefP, "C" ) == 0 ) 
		strcpy( charbuffer2, "en" );
	else {
		charbuffer2[0] = userPrefP[0];
		charbuffer2[1] = userPrefP[1];
		charbuffer2[2] = '\0';
	}

        for ( unsigned int i=0; i < sizeof( myIso639 ) / sizeof(*myIso639); ++i) {
                if ( strcmp( myIso639[i].twoletterm, charbuffer2 ) == 0 ) {
			strcpy( charbuffer3, myIso639[i].iso639int );
			break;
		}
        }

	if ( userPrefP != NULL ) 
		free( userPrefP );

	return ( 
		( charbuffer3[0] != '\0' ) &&
		( charbuffer2[0] != '\0' )
	);
}

void safeStrncpy( char * to, char *from, int max )
{
	strncpy( to, from, max );
	to[max] = '\0';
}

time_t my_timegm (struct tm *tm) {
                  time_t ret;
                  char *tz;

                  tz = getenv("TZ");
                  setenv("TZ", "", 1);
                  tzset();
                  ret = mktime(tm);
                  if (tz)
                      setenv("TZ", tz, 1);
                  else
                      unsetenv("TZ");
                  tzset();
                  return ret;
              }


bool isPseudoEmptyString( char * theString )
{
        return ( ( strlen( theString ) == 1 ) && ( theString[0] == '-' ) );
}


time_t offsetGMTToLocaltime( void )
{
	static time_t timeDiff = 99999;

	// Only calculate it once...

	if ( timeDiff == 99999 ) {
		time_t nowLocaltime = getCurrentTime();

		struct tm nowStruct = *localtime( &nowLocaltime );

		time_t nowUTC = my_timegm( &nowStruct );	

		timeDiff =  nowUTC - nowLocaltime;
	}
	
	return timeDiff;
}

eString stringReplace( const eString &toReplace, char from, char to )
{
        char buffer[MAX_STRING_LENGTH+1];
        safeStrncpy( buffer, (char *)toReplace.c_str(), MAX_STRING_LENGTH );
        char *p = buffer;
        while ( *p != '\0' ) {
                if ( *p == from )
                        *p = to;
		p++;
        }
        return eString( buffer );
}

void removeTrailingNewlines( char *toRemove )
{
	// Empty string silly case

	if ( *toRemove == '\0' )
		return;

	// Find end-of-string

        char *p = toRemove;
	while ( *p != '\0' )
		p++;

	// Character before end-of-string
	p--;

	// Go back, replacing \n with \0
        while ( *p == '\n' ) {
		*p = '\0';

		// Don't go past start of string
		if ( p == toRemove )
			break;
		p--;
	}
}

/*
#define eq(x,y) ((!(x^y)))  || (((x^y) == 0x20) && (((x|y) > 0x60) && ((x|y) < 0x7b)))

char *myStrcasestr (char *hay, char *needle)
{
    char *h, *n, *hMax, *hMin;

    if (hay[0])
    {
        if (needle[0])
        {
            hMax = h = hay; n = needle;
            while (*hMax++);
            while (*n++) hMax--;
            n = needle;
            if (hMax > h)
            {
                while ((h < hMax) && (!(eq(*h,*n)))) h++;
                if (h < hMax)
                {
                    if (n [1])
                    {
                        hMin = h;
                        while (*(h = hMin))
                        {
                            n = needle;
                            while ((h < hMax) && (!(eq(*h,*n)))) h++;
                            if ((h < hMax))
                            {
                                hMin = h + 1;
                                while ((*h) && (eq(*h,*n))) {h++; n++;}
                                if (!*n) return --hMin;
                            }
                            else break;
                        }
                    }
                    else return h;
                }
            }
        }
        else return hay;
    }
    return NULL;
}
*/

// IF CHANGE THESE, NEED TO CHANGE DEFAULT
// SELECTION in MV : setConfigDefaults
// AND mvccat.cpp version
// TODO (dAF2000): Ugly coded, remove /var/mnt, see also mv.cpp
static eString dirs[7] = {
	"/var/mnt/usb/mv",
	"/media/usb/mv",
	"/var/mnt/cf/mv",
	"/media/cf/mv",
	"/media/hdd/mv",
	"/var/tuxbox/config/mv",
	"/var/tmp/mv"
};	

eString & indexToDir( int index )
{
	return dirs[index];
}

int noDirs( void )
{
	return 7;
}

int dirToIndex( eString &dir )
{
	int index = 0;
	while ( index < 7 ) {
		if ( dir == dirs[index] ) 
			break;
		index++;
	}

	if ( index == 7 )
		dmsg( "Failed to get dir index for ", (char *)dir.c_str() );

	return index;
}


eString prefixDir( eString baseDir, eString relPath )
{
        return baseDir + eString( "/" ) + relPath;
}

bool dirIsMountPoint( const char *dir )
{
	bool mountedFlag = false;

	// Make sure space after dir, avoid spotting
	// /mnt/cfs as /mnt/cf

	eString dirString = eString( dir ) + eString( " " );

	FILE *fp = fopen( "/proc/mounts", "r" );
	if ( fp ) {
		char buffer[MAX_STRING_LENGTH+1];
		while ( fgets( buffer, MAX_STRING_LENGTH, fp ) ) {
			buffer[MAX_STRING_LENGTH] = '\0';
			if ( strstr( buffer, dirString.c_str() ) ) {
				mountedFlag = true;
				break;
			}
		} 
		fclose( fp );
	}

	return mountedFlag;
}

bool fileNeedsRefreshing( const eString &path, time_t maxAge )
{
        return (
                ( ! pathExists( path ) ) ||
                ( fileSize( path ) == 0 ) ||
                ( ( getCurrentTime() - fileMtime( path ) ) > maxAge )
        );
}

void showInfoBox( eWidget*parent, int titleIndex, int messageIndex )
{
	if ( parent != NULL )
	        parent->hide();
        showMessageBox( getStr( titleIndex), getStr( messageIndex ), eMessageBox::iconInfo | eMessageBox::btOK );

	if ( parent != NULL )
        	parent->show();
}

int showExecHide( eWidget *it )
{
	it->show();
	int result = it->exec();
	it->hide();
	return result;
}


bool isGraphicalView( int viewNo )
{
        return ( viewNo < FIRST_LIST_VIEW );
}

// We don't want to rely on the keys for views
// having consecutive key codes

static int keyNos[NO_VIEWS] = { 
	0,
	VIEW_SWITCH_1, VIEW_SWITCH_2, VIEW_SWITCH_3,
	VIEW_SWITCH_4, VIEW_SWITCH_5, VIEW_SWITCH_6,
	VIEW_SWITCH_7, VIEW_SWITCH_8, VIEW_SWITCH_9
};

int viewNoToKeyNo( int viewNo )
{
	if ( ( viewNo < 1 ) || ( viewNo >= NO_VIEWS ) )
		dmsg("viewNoToKeyNo: bad view passed: ", viewNo );

	return keyNos[viewNo];
}

int keyNoToViewNo( int keyNo )
{
	int viewNo = 1;
	while ( 
		( viewNo < NO_VIEWS ) &&
		( keyNos[viewNo] != keyNo )
	)
		viewNo++;

	if ( viewNo == NO_VIEWS )
		dmsg( "keyNoToViewNo: bad key passed: ", keyNo );	

	return viewNo;
}

// Make sure all words start with An Upper Case, and
// rest are lower case
// Only use when you have to, you'll ruin acronyms
// etc
eString mungeWordsToLowerCase( eString toDo )
{
        char buffer[MAX_STRING_LENGTH+1];
        safeStrncpy( buffer, (char *)toDo.c_str(), MAX_STRING_LENGTH );

	char *cp = buffer;

	bool inWord = false;

	while ( *cp != '\0' ) {
		if ( *cp > 48 ) {
			if ( inWord ) {
				*cp = tolower( *cp );
			}
			else {
				*cp = toupper( *cp );
				inWord = true;
			}
		}
		else
			inWord = false;
		cp++;
	}

	return eString( buffer );
}
/*
		
	while ( ( cp = strchr( toMunge, ' ' ) ) != NULL ) {
		cp++;
		if ( *cp == '\0' )
			break;

		if ( 
			( *cp < 'A' ) ||
			( *cp > 'Z' )
		)
		  	continue;
			
		cp++;

		while ( 
			isUpperCaseChar( *cp )
			( *cp >= 'A' ) &&
			( *cp <= 'Z' )
		)
			*cp++;
	}
}
*/


void showHideWidget( eWidget *w, bool showFlag )
{
        if ( showFlag )
                w->show();
        else
                w->hide();
}


void setWidgetGeom( eWidget *w, int x, int y, int width, int height )
{
        w->move( ePoint( x, y ) );
        w->resize( eSize( width, height ) );
}

bool userConfirms( int confirmTextIndex )
{
	int answer = showMessageBox( getStr( strConfirmTitle ), getStr( confirmTextIndex), eMessageBox::iconQuestion | eMessageBox::btCancel | eMessageBox::btOK );
	return ( answer == 1 );
}

// C++ strcasestr is stripped in dbox

char * mystrcasestr(char *haystack, char *needle)
{
    int needleLen = strlen( needle );
    int matchLen = strlen( haystack ) - needleLen + 1;

    // If needle is bigger than haystack, can't match
    if ( matchLen < 0 )
	return NULL;

    if ( needleLen == 0 )
	return NULL;

    while ( matchLen ) {

	// See if the needle matches the start of the haystack
	int i;
	for ( i = 0; i < needleLen; i++) {
	    if ( tolower( haystack[i] ) != tolower( needle[i] ) )
		break;
	}

	// If it matched along the whole needle length, we found it
	if ( i == needleLen )
	    return haystack;

	matchLen--;
	haystack++;
    }

    return NULL;
}

/*
void dockWidget( eWidget *dockWidget, eWidget *docker, int mode )
{
        const eSize dockSize = dockWidget->getSize();
        const ePoint dockPos = dockWidget->getPosition();

        eSize dockingLabelSize = docker->getSize();

	int y = dockPos.y();

	if ( mode == dockModeTopRight ) 
		y -= dockingLabelSize.height();

        setWidgetGeom(
                docker,
                dockPos.x() + dockSize.width() - dockingLabelSize.width()
,
		y,
                dockingLabelSize.width(), dockingLabelSize.height()
        );
}
*/

void waitOnFlag( bool &flag, bool waitingFor, long int delayPeriodMicroSeconds )
{
	while ( flag != waitingFor )
		microSleep( random() % delayPeriodMicroSeconds );
}

void setRestoreAspectRatio( bool setFlag, int mode )
{
        static bool needRestoreFlag = false;

	bool needReloadFlag = false;

	// Set
	if ( setFlag ) {
		int original = getEnigmaIntKey( PIN8_KEY );

		// We're only interested in the smart mode
		// In other modes, the aspect ration is fixed
		// anyway

		if ( original == 3 ) {
			// Put enigma in always 16:9
			if ( mode == aspectRatioAlways169 ) {
				eAVSwitch::getInstance()->setAspectRatio( r169 );
				needRestoreFlag = true;
			}
			// Put enigma in letterbox
			else if ( mode == aspectRatioAlways43 ) {
				eAVSwitch::getInstance()->setAspectRatio( r43 );
				needRestoreFlag = true;
			}
			else
				needReloadFlag = true;
		}
	}

	// Restore
	else if ( needRestoreFlag )
		needReloadFlag = true;

	if ( needReloadFlag )
		eStreamWatchdog::getInstance()->reloadSettings();
}

eString timeToHourMinuteString( time_t t )
{
	tm *ts= localtime( &t);

	return eString().sprintf( "%.02d:%.02d ", ts->tm_hour, ts->tm_min );
}

void mylogReset( void )
{
	mylogWrite( "", "w" );
}

void mylogWrite( const char *towrite, const char *mode ) {
	FILE *fp = fopen( LOG_PATH, mode );
	if (fp != NULL)
	{
	    fputs( towrite, fp );
	    putc( '\n', fp );
	    fclose( fp );
	}
}

//void mylog( const char *towrite ) {
//	mylogWrite( towrite, "a" );
//}
void mylog( eString towrite ) {
	mylogWrite( towrite.c_str(), "a" );
}


void convertToUpper( eString &str)
{
 unsigned int i;
  for ( i= 0; i <str.length(); i++)
    switch(str[i])
    {
	case 'a' ... 'z' :
	case 160 ... 190 :
		str[i] -= 32;
		break;

	case 'ä' :
		str[i] = 'Ä';
		break;
			
	case 'ü' :
		str[i] = 'Ü';
		break;
			
	case 'ö' :
		str[i] = 'Ö';
		break;
    }
}

eString convertToUTF( const char *toConvert, char encoding )
{
        switch ( encoding ) {
                case encodingUTF:
                        return eString( toConvert );
                case encodingLatin:
			{

	unsigned int t=0, i=0, len=strlen(toConvert);

	unsigned char res[2048];

	while (i < len)
	{
		unsigned char code=toConvert[i++];
		if (code < 0x80) 
			res[t++]=code;
		else if (code < 0xc0) // two byte mapping
		{
			res[t++]=194;
			res[t++]=code;
		} else
		{
			res[t++]=195;
			res[t++]=code-64;
		}
	}
	res[t]=0;
	return eString((char*)res);
			}
                default:
                        dmsg( getStr( strErrBadCharacterEncoding ), (int) encoding );
                        return "";
        }

}

gColor getColourFromRGB( int r, int g ,int b, int a )
{
	struct gRGB bob( r, g, b, a);
	return getColourFromRGB( bob );
}

gColor getColourFromRGB( const struct gRGB & rg )
{
	return gFBDC::getInstance()->getPixmap().clut.findColor( rg );
}

struct gRGB getRGBFromColour( const gColor & col )
{
	gPixmap *tmp = &gFBDC::getInstance()->getPixmap();	
	gPixmapDC mydc( tmp );
	return mydc.getRGB( col );
}

gColor adjustedColour( const gColor &toAdjust, int dr, int dg, int db, int da )
{
	struct gRGB tmp= getRGBFromColour( toAdjust );
	tmp.r += dr;
	tmp.g += dg;
	tmp.b += db;
	tmp.a += da;
	return getColourFromRGB( tmp );
}

int centreImage( int space, int imageSize, int startCoord )
{
	return ( startCoord + ( ( space - imageSize ) / 2 ) );
}

eTextPara * makeNewTextPara
( 
	const eString &text,
	int x, int y, int width, int height,
	const gFont & font,
	int realignFlags
) {
	eTextPara *tmp = new eTextPara(
		eRect( x,y, width,height )	
	);
	tmp->setFont( font );

	tmp->renderString( text );

	if ( realignFlags != 0 )
		tmp->realign( realignFlags );

	return tmp;
}

// Pick the first 4-digit year-like thing we find out
// of a string
// This one not good tested
int findYearInString( const char *haystackP ) {
	if ( haystackP == NULL )
		return 0;

        char *firstP = NULL;

        int yearToReturn = 0;
        int foundCount = 0;
        while ( (*haystackP) != '\0' ) {

                if ( isNumericChar( *haystackP ) ) {
                        foundCount++;
                        // Record first byte in case it turns out to be a year
                        if ( foundCount == 1 )
                                firstP = (char *)haystackP;
                        // 5 digits in a row can't be a year
                        else if ( foundCount == 5 )
                                foundCount = 0;
                }
                // Success condition: we've found four numeric, then a non-numeric
                else if ( foundCount == 4 ) {
                        int tmpYear = strtol( eString( firstP ).left(4).c_str(), ( char **)NULL, 10);
                        if (
                                ( tmpYear >= 1900 ) &&
                                ( tmpYear <= 2030 )
                        ) {
                                yearToReturn = tmpYear;
                                break;
                        }
                        else
                                foundCount = 0;
                }
                else
                        foundCount = 0;
                haystackP++;
        }

        return yearToReturn;
}

bool loadSkin( eString path, const char * loadedFlagName, bool forceFlag )
{
	bool loadedFlag = false;

	// Load skin if first time since reboot, or
	// force requested
	if ( 
		( forceFlag ) ||
		( getNamedValue( loadedFlagName ) == -1 )
	) {
		if ( eSkin::getActive()->load( path.c_str() ) == -1 )
			mylog( eString().sprintf( "failed to load skin file %s", path.c_str() ) );
		else
			loadedFlag = true;
	}
	return loadedFlag;
}

bool haveNetwork( void )
{
	return ( getEnigmaIntKey( NETWORK_SETUP_KEY ) != 0 );
}

// Make sure it's executable
void makeExecutable( const eString &path ) {
	eString sysCom = eString( "chmod ugo+x " ) + path;
	system( sysCom.c_str() );
}

