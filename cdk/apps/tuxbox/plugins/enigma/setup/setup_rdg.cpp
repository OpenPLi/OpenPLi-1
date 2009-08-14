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

#include <setup_rdg.h>

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#include <lib/base/i18n.h>

#include <lib/dvb/edvb.h>
#include <lib/dvb/eaudio.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <lib/driver/rc.h>
#include <lib/system/econfig.h>
#include <lib/gui/combobox.h>
#include <lib/gui/listbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/textinput.h>

void
SetupRdg::GetCFG ()
{
  // char section[32] = "";
  // char line[256];
  // char one[64];
  // char two[64];
  // char *ptr;
  // FILE *fp;
}

void
SetupRdg::WriteCFG ()
{
  // FILE *fp;
}

SetupRdg::SetupRdg ():
eWindow (0)
{
  SetupRdg::GetCFG ();

  int fd = eSkin::getActive ()->queryValue ("fontsize", 16);
  int s_y = 130, h_x = 250;
  eLabel *l;

  setText (dgettext("plisetup", "Radegast setup"));
  cmove (ePoint (720 / 2 - h_x, 120));
  cresize (eSize (h_x * 2, 380));

  ok = new eButton (this);
  ok->setText (dgettext("plisetup", "Save"));
  ok->setShortcut ("green");
  ok->setShortcutPixmap ("green");
  ok->move (ePoint (10, 270));
  ok->resize (eSize (150, 40));
  ok->setHelpText (dgettext("plisetup", "Save changes and return"));
  ok->loadDeco ();
  CONNECT (ok->selected, SetupRdg::okPressed);

  abort = new eButton (this);
  abort->setShortcut ("red");
  abort->setShortcutPixmap ("red");
  abort->loadDeco ();
  abort->setText (dgettext("plisetup", "Abort"));
  abort->move (ePoint (180, 270));
  abort->resize (eSize (150, 40));
  abort->setHelpText (dgettext("plisetup", "Ignore changes and return"));
  CONNECT (abort->selected, SetupRdg::abortPressed);

  defaults = new eButton (this);
  defaults->setShortcut ("blue");
  defaults->setShortcutPixmap ("blue");
  defaults->loadDeco ();
  defaults->setText (dgettext("plisetup", "defaults"));
  defaults->move (ePoint (180 + 170, 270));
  defaults->resize (eSize (150, 40));
  defaults->setHelpText (dgettext("plisetup", "Load good defaults for local CS"));
  CONNECT (defaults->selected, SetupRdg::defaultsPressed);

  statusbar = new eStatusBar (this);
  statusbar->move (ePoint (0, clientrect.height () - 60));
  statusbar->resize (eSize (clientrect.width (), 50));
  statusbar->loadDeco ();
}

void
SetupRdg::okPressed ()
{
  SetupRdg::WriteCFG ();
  close (1);
}

void
SetupRdg::abortPressed ()
{
  close (1);
}

void
SetupRdg::defaultsPressed ()
{
}
#endif //SETUP_RDG
