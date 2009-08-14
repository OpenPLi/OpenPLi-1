/*
 * setup_window.cpp
 *
 * Copyright (C) 2003 Andreas Monzner <ghostrider@tuxbox.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: setup_window.cpp,v 1.4 2008/01/05 13:47:16 dbluelle Exp $
 */

#include <setup_window.h>
#include <callablemenu.h>

Signal2<void,eSetupWindow*,int*> eSetupWindow::setupHook;

eSetupWindow::eSetupWindow( const char *title, int entries, int width )
	:eListBoxWindow<eListBoxEntryMenu>(title, entries, width, true)
{
	list.setFlags(eListBoxBase::flagHasShortcuts);
}

int eSetupWindow::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
		if (list.eventHandlerShortcuts(event))
			return 1;
		else if (event.action == &i_cursorActions->cancel)
			close(0);
		else
			break;
		return 1;
	default:
		break;
	}
	return eWindow::eventHandler(event);
}

void eSetupWindow::addCallableMenuEntry(
	const eString& shortcutName,
	const eString& helptext,
	int* entry)
{
	if(shortcutName.length())
	{
		struct eCallableMenuFactory::MenuEntry menuEntry;
		menuEntry = eCallableMenuFactory::getMenuEntry(shortcutName);
		if(menuEntry.factory && menuEntry.factory->isAvailable())
		{
			new eListBoxEntryMenu(&list, _(menuEntry.menuName.c_str()), eString().sprintf("(%d) %s", *entry,
				helptext.c_str()), 0, (void*)menuEntry.factory->shortcutName.c_str());
		}
		
		if(entry)
		{
			++(*entry);
		}
	}
	else
	{
		// Just an empty entry
		new eListBoxEntryMenu(&list, "");
	}
}

