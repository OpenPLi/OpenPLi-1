/*
 * $Id: main.cpp,v 1.3.2.1 2003/11/25 14:24:35 obi Exp $
 *
 * A startup menu for the d-box 2 linux project
 *
 * Copyright (C) 2001, 2002 Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "main.h"

int main (int argc, char **argv)
{
    /* create menu instance */
    CLCDMenu *menu = CLCDMenu::getInstance();

    /* don't continue if there is no lcd */
    if (!menu->isAvailable())
	    return 0;

    /* draw the menu */
    menu->drawMenu();

    /* select default entry */
    menu->selectEntry(menu->getDefaultEntry());

    /* get command from remote control */
    menu->rcLoop();

    /* remember last selection */
    if (menu->getSelectedEntry() != menu->getDefaultEntry())
    {
	menu->getConfig()->setInt32("default_entry", menu->getSelectedEntry());
	menu->getConfig()->setModifiedFlag(true);
    }

    if (menu->getConfig()->getModifiedFlag())
    {
	/* save configuraion */
	menu->getConfig()->saveConfig(CONFIGFILE);
    }

    /* clear screen before exit */
    menu->draw_fill_rect(0, 0, 119, 63, CLCDDisplay::PIXEL_OFF);
    menu->update();

    return menu->getSelectedEntry();
}
