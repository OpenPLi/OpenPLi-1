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
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/listbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/echeckbox.h>

#include <lib/dvb/dvb.h>




gPixmap *getNamedPixmapP( const char *name );

eCheckbox * makeNewCheckbox(eWidget * parent,
                            eString text,
                            int x, int y, int width, int height);


eButton * makeNewButton(
        eWidget * parent, eString text,
        int x, int y, int width, int height,
        char *shortcut = NULL
);


void setWidgetGeom( eWidget *w, int x, int y, int width, int height );


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

eNumber *makeNewNumber(
    eWidget *parent, int *initP,
    int x, int y, int width, int height,
    int maxDigits, int minValue, int maxValue
);


gFont getNamedFont( const char *name );

eString buildShortServiceName( const eString &str );
eString getServiceName( const eServiceReferenceDVB &ref );


#endif
