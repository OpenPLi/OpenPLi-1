/*
 * $Id: lcdmenu.h,v 1.14.2.1 2003/02/18 19:33:02 zwen Exp $
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

#ifndef __LCDMENU_H__
#define __LCDMENU_H__

#include <crypt.h>
#include <dbox/fp.h>
#include <signal.h>
#include <unistd.h>

#include <string>
#include <vector>

#ifdef HAVE_CONFIG_H
#include <config.h>
#else
#define CONFIGDIR "/var/tuxbox/config"
#endif

#include <configfile.h>
#include <lcddisplay/fontrenderer.h>
#include <lcddisplay/lcddisplay.h>

#include "rcinput.h"

#define LEFTALIGNED	0
#define CENTERED	1

class CLCDMenu : public CLCDDisplay
{
	public:
		static CLCDMenu *getInstance()
		{
			if (instance == NULL)
				instance = new CLCDMenu(CONFIGDIR "/lcdmenu.conf");
			return instance;
		}

		void addEntry (std::string);
		bool selectEntry (int);
		int getDefaultEntry () { return defaultEntry; }
		int getSelectedEntry () { return selectedEntry; }
		void addNumberPrefix ();

		bool drawMenu ();
		bool drawString (std::string, int, int, int);
		int getTextAlign () { return textAlign; } /* 0=left, 1=centered */

		CRCInput *getRc ()  { return rc; }
		bool rcLoop ();

		std::string pinScreen (std::string, bool);
		bool changePin ();
		bool checkPin (std::string);
		bool isPinProtected (int);
		void addPinProtection (int);

		const char *getCurrentSalt ();
		char *getNewSalt ();
	
		void poweroff ();

		CConfigFile *getConfig () { return config; }

	protected:
		CLCDMenu (std::string); /* configuration filename */
		~CLCDMenu ();
		
	private:
		static CLCDMenu *instance;
		static void timeout (int);
	
		CConfigFile *config;
		CRCInput *rc;
		LcdFontRenderClass *fontRenderer;
		LcdFont *menuFont;

		int upperRow;
		int fontSize;
		int lineSpacing;
		int textAlign;
		bool showNumbers;
		int timeoutValue;

		int selectedEntry;
		int entryCount;
		int defaultEntry;
		int visibleEntries;

		std::vector <std::string> entries;
		std::vector <int> pinEntries;

		int pinFailures;
		std::string cryptedPin;
		char *newSalt;
};

#endif /* __LCDMENU_H__ */
