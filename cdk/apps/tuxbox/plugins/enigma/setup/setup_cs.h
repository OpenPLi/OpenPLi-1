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

#ifndef __setup_cs_h
#define __setup_cs_h

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <lib/base/console.h>

#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/combobox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/textinput.h>

#define MAX_IP 10

class SetupScam_cs:public eWindow
{
  eButton *ok, *abort, *defaults;
  eNumber *cs_srvport, *cs_clntport[MAX_IP], *cs_server_ip[MAX_IP];
  eTextInputField *cs_server[MAX_IP], *cs_p_dev;
  eStatusBar *statusbar;
  eCheckbox *cs_au_in, *cs_num;
  eComboBox *ServerN;
  //eListBox < eListBoxEntryText > *ServerN;
private:
  void okPressed ();
  void abortPressed ();
  void defaultsPressed ();
  void GetCFG ();
  void WriteCFG ();
  void NumChanged (int);
  void SelectServerN (eListBoxEntryText * item);
  eString c_server[MAX_IP];
  eString c_port[MAX_IP];
  eString c_mode;
  eString c_serial;
  eString c_au_internet;
  eString s_port;
  eString s_mode;
  eString s_phoenix_dev;
  int v_cs_server_ip[MAX_IP][4];
  int v_cs_au_in;
  int numeric;
  int server;
public:
    SetupScam_cs ();
};

#endif
