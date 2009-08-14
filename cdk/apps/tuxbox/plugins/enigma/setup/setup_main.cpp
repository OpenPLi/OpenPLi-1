/*
 * Ronald's setup plugin for dreambox
 * Copyright (c) 2004 Ronaldd <Ronaldd@sat4all.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "setup_main.h"
#include "emuconfig.h"
#include "ppanel.h"

#include <lib/system/econfig.h>
#include <lib/system/info.h>

const char* TITLE = "Setup tool PLi®";

#include <lib/gui/listbox.h>
#include <enigma_plugins.h>
#include <enigma_setup.h>
#include <backup.h>
#include <plugin.h>

#include <parentallock.h>

extern "C" int plugin_exec (PluginParam * par);

int plugin_exec (PluginParam * par)
{
	/*
	 * TODO: custom dir?
	 * bindtextdomain("plisetup", LOCALEDIR);
	 *
	 */
	bind_textdomain_codeset("plisetup", "UTF-8");

	eMySettings setup;
	setup.show ();
	setup.exec ();
	setup.hide ();
	return 0;
}

eMySettings::eMySettings () : eSetupWindow (TITLE, 4, 350)
{
}

eMySettings::~eMySettings()
{
	// write enigma config file
	eConfig::getInstance()->flush();
}

