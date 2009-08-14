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

#include "menu.h"


MainMenu::MainMenu() : eListBoxWindow<eListBoxEntryText>( getStr( strMainMenuTitle ), MENU_NO_ITEMS + 3, MENU_WIDTH, false )
{
        for ( int i = 1; i <= MENU_NO_ITEMS; i++ ) {
                if ( 
			( i == MENU_ITEM_EDIT_INPUTS ) ||
			( i == MENU_ITEM_RESET ) ||
			( i == MENU_ITEM_ABOUT )
		)
                        new eListBoxEntryTextSeparator((&(list)), getNamedPixmapP( "listbox.separator" ), 0, true );

                new eListBoxEntryText( &list, getStr( strMainMenuItemFirst + i - 1 ), (void*)i);
        }

        move( ePoint( MENU_TOPLEFTX, MENU_TOPLEFTY ) );

        CONNECT( list.selected, MainMenu::listSelected );
}

void MainMenu::listSelected( eListBoxEntryText* t )
{
        if ( t == NULL )
                close( -1 );
        else
                close( (int) ( t->getKey() )  );
}
