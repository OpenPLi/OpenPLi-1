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

#include "util.h"
#include <enigma.h>

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
	eWidget * parent, eString text, 
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
        tmp->setText( text );

        tmp->show();
        return tmp;
}

eCheckbox * makeNewCheckbox(eWidget * parent,
                            eString text,
                            int x, int y, int width, int height)
{
    eCheckbox* tmp = new eCheckbox(parent);
    setWidgetGeom(tmp, x, y, width, height );
    tmp->loadDeco();
    tmp->setText( text );
    tmp->show();
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


gPixmap *getNamedPixmapP( const char *name )
{
	return eSkin::getActive()->queryImage( name );
}

gFont getNamedFont( const char *name )
{
	return eSkin::getActive()->queryFont( name );
}



void setWidgetGeom( eWidget *w, int x, int y, int width, int height )
{
        w->move( ePoint( x, y ) );
        w->resize( eSize( width, height ) );
}

// Make sure it's executable
void makeExecutable( const eString &path ) {
	eString sysCom = eString( "chmod ugo+x " ) + path;
	system( sysCom.c_str() );
}

eString getServiceName( const eServiceReferenceDVB &ref )
{
    eString serviceName;
    if ( ref.descr )
        serviceName = ref.descr;
    else {
        eService *sv = eServiceInterface::getInstance()->addRef( ref );
        if ( sv ) {
            eString shortName = buildShortServiceName( sv->service_name );
            serviceName = shortName ? shortName : sv->service_name;
            eServiceInterface::getInstance()->removeRef( ref );
        }
    }
    return serviceName;
}



eString buildShortServiceName( const eString &str )
{
    eString tmp;
    static char stropen[3] = { 0xc2, 0x86, 0x00 };
    static char strclose[3] = { 0xc2, 0x87, 0x00 };
    unsigned int open=eString::npos-1;

    while ( (open = str.find(stropen, open+2)) != eString::npos ) {
        unsigned int close = str.find(strclose, open);
        if ( close != eString::npos )
            tmp += str.mid( open+2, close-(open+2) );
    }
    if ( tmp == "" )
        tmp = str;

    return tmp;
}


