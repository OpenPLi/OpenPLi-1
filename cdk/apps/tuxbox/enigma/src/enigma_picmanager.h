/*
 * $Id: enigma_picmanager.h,v 1.6 2009/02/03 18:54:33 dbluelle Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
 
#ifndef __picmanager__
#define __picmanager__

#include <lib/gui/ewindow.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/listbox.h>
#include <lib/gui/echeckbox.h>

class ePicViewerSettings: public eWindow
{
private:

	eButton *ok, *abort;
	eStatusBar *statusbar;
	eCheckbox *subdirs, *busy, *format_169;
	eListBox<eListBoxEntryText> *timeout;

	void fieldSelected(int *number);
	void okPressed();
	void abortPressed();
	void init_ePicViewerSettings();
public:
	ePicViewerSettings();
	~ePicViewerSettings();
};
#endif
