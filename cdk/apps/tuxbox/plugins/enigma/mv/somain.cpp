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

#include "somain.h"

int plugin_exec( PluginParam *par )
{
        MV epg;

	if ( epg.error() == errorCodeNone ) {

		// Stop enigma passing rogue keypress to
		// the EPG on startup

		KeyCatcher kc; showExecHide( &kc );

		epg.run();
	}


	return 0;
}
