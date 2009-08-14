/*
 * $Id: dreamflash.h,v 1.3 2006/02/05 23:45:10 pieterg Exp $
 *
 * (C) 2005 by mechatron, digi_casi
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
 
#include <plugin.h>
#include <stdio.h>
#include <sys/mount.h> //for mount
#include <sys/stat.h> //for stat
#include <sys/vfs.h> //for statfs
#include <dirent.h> //for Directory
#include <fcntl.h>

#include <lib/gui/ewindow.h>
#include <lib/gui/emessage.h>
#include <lib/gui/elabel.h>
#include <lib/gui/textinput.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/listbox.h>
#include <lib/gui/enumber.h>
#include <lib/gdi/epng.h>//for loadPNG
#include <lib/gdi/gfbdc.h>//for loadPNG
#include <lib/system/econfig.h> //for video-format
#include <lib/gui/statusbar.h>

class setup_df: public eWindow
{
	eListBox<eListBoxEntryText> *mliste, *sliste;
	eStatusBar *status;
	eNumber *starttimer;
	eCheckbox *ch_inetd;
	eTextInputField *ed_skin_path;
	void okselected();
	void load_sliste();
public:
	setup_df();
};

class info_df: public eWindow
{
	eLabel *x;
	eString ver;
	void Listeselchanged(eListBoxEntryText *item);
public:
	info_df();
};

class image_df: public eWindow
{
	eTextInputField *Iname;
	eButton *ok;
	eStatusBar *status;
	
	int was1, free_space;
	void oksel();
	void Listeselchanged(eListBoxEntryText *item);
public:
	image_df(int was);
};

class df_main: public eWindow
{
	eListBox<eListBoxEntryText> *textlist;
	void Listeselected(eListBoxEntryText *item);
public:
	df_main();
};

