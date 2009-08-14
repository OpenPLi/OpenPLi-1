/*	Ipkgpl - Ipkg Enigma Plugin

	Copyright (C) 2005 'mechatron' (mechatron@gmx.net)

	Homepage: http://mechatron.6x.to/

	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA
*/

#ifndef __setup__ipkg__
#define __setup__ipkg__

#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/eskin.h>
#include <lib/system/econfig.h>

#include "ipkg.h"

class message: public eWindow
{
public:
	message( CIPKG *m_ipkg, int menu , int val=0);
};

class ipkgsetup: public eWindow
{
	CIPKG *ms_ipkg;

	eCheckbox *del_inst;
	eCheckbox *m_optForceDepends;
	eCheckbox *m_optForceReinstall;
	eCheckbox *m_optForceRemove;
	eCheckbox *m_optForceOverwrite;
	eComboBox *m_optVerboseIpkg;
	eComboBox *c_dest;
	eStatusBar *status;

	void okselected();
	void loaddata();
public:
	ipkgsetup(CIPKG *m_ipkg);
};

#endif


