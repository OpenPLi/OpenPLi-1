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



gFont makeFontFromOptions( int familyOpt, int sizeOpt )
{
 	gFont bob = getNamedFont( fontFamilyOptionToFontFamilyName( familyOpt ) );
	bob.pointSize = sizeOpt;
	return bob;
}

char * fontFamilyOptionToFontFamilyName( int familyOpt ) 
{
	static char fontFamilyNames[PM_MAX_NUM_FONT_FAMILIES][PM_MAX_FONT_FAMILY_NAME_LENGTH] =
	{
		"epg.title", "epg.description", "epg.time"
	};

#ifdef DEBUG
	if ( 
		( familyOpt >= PM_MAX_NUM_FONT_FAMILIES ) ||
		( familyOpt < 0 )
	) {
		dmsg( "program error: fontFamilyToFontFamilyName: bad family: ", familyOpt );
		familyOpt = 0;
	}
#endif

	return  fontFamilyNames[familyOpt];
}

//"yellow": for remaining time ?

char * colourOptionToColourName( int colourOption )
{
	static char colourNames[PM_MAX_NUM_COLOURS][PM_MAX_COLOUR_NAME_LENGTH+1] = { 
	"std_black",
	"epg.entry.background", "content", 
	"eStatusBar.background",
	"global.normal.background",
	"eWindow.titleBar",
	"background", 
	"std_blue", "std_dblue",
	 "blue_nontrans", "blue",
	"std_dred", "std_red",
	"global.selected.background",
	"std_dyellow" ,
	"epg.entry.background.selected",
	"std_dgreen", "green" 
	};

#ifdef DEBUG
	if ( 
		( colourOption >= PM_MAX_NUM_COLOURS ) ||
		( colourOption < 0 )
	) {
		dmsg( "program err: colourCoptionToColourName: bad colour: ", colourOption );
		colourOption = 0;
	}
#endif
	return  colourNames[colourOption];
}

gColor colourOptionToColour( int option )
{
	return ( getNamedColour( colourOptionToColourName( option ) ) );
}
