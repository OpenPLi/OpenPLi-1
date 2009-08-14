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

#ifndef __MENU_H__
#define __MENU_H__

#include <lib/gui/listbox.h>

#include "util.h"
#include "text.h"

#define MENU_ITEM_NONE				0
#define MENU_ITEM_EDIT_VIEW_GEOM		1
#define MENU_ITEM_EDIT_VIEW_PRESENTATION	2
#define MENU_ITEM_EDIT_VIEW_FEATURES		3
#define MENU_ITEM_EDIT_INPUTS			4
#define MENU_ITEM_ALIAS_MANAGER			5
#define MENU_ITEM_FAVOURITES_MANAGER		6
#define MENU_ITEM_MISC				7
#define MENU_ITEM_ABOUT 			8	
#define MENU_ITEM_TIPS 				9	
#define MENU_ITEM_FEEDBACK			10
#define MENU_ITEM_RESET				11
#define MENU_NO_ITEMS				11

#define MENU_TOPLEFTX		190
#define MENU_TOPLEFTY		100
#define MENU_WIDTH		310
#define MENU_HEIGHT		400


class MainMenu : public eListBoxWindow<eListBoxEntryText> {
        void listSelected( eListBoxEntryText* t );
public:
        MainMenu();
};

#endif
