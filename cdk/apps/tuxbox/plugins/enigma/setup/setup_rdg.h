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

#ifdef SETUP_RDG

#ifndef __setup_rdg_h
#define __setup_rdg_h

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <lib/base/console.h>

#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/textinput.h>


class SetupRdg:public eWindow
{
  eButton *ok, *abort, *defaults;
  eStatusBar *statusbar;
private:
  void okPressed ();
  void abortPressed ();
  void defaultsPressed ();
  void GetCFG ();
  void WriteCFG ();
public:
    SetupRdg ();
};

#endif
#endif // SETUP_RDG
