/*
    Copyright (C) 2004  David van der Vliet
    http://talk.to/dAF

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

History:

See .cpp file
*/

#ifdef WIFI

#include <lib/gui/echeckbox.h>
#include <lib/gui/textinput.h>
#include <lib/gui/enumber.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/combobox.h>

class wifiScreen: public eWindow
{
   private:
   void showAbout(void);
   void saveChanges(void);
   void nextPressed(void);
   void update(void);

	eTimer *et_timer;
   eCheckbox *cb_onoff;
   eLabel *lb_ip;
   eLabel *lb_netmask;
   eLabel *lb_gateway;
   eLabel *lb_dns;
   eLabel *lb_link;
   eLabel *lb_level;
   eLabel *lb_noise;
   eNumber *nb_ip;
   eNumber *nb_netmask;
   eNumber *nb_gateway;
   eNumber *nb_dns;
   eButton *bt_ok;
   eButton *bt_cancel;
   eButton *bt_next;
   eButton *bt_about;
   eStatusBar *sb_help;

   public:
   wifiScreen();
   ~wifiScreen();
};

class securityScreen: public eWindow
{
   private:
   void prevPressed(void);
   void nextPressed(void);
   void saveSettings(void);
   void wepStateChanged(int state);
   void securityStateChanged(eListBoxEntryText *le);
   
   eCheckbox *cb_wep;
   eLabel *lb_mode;
   eComboBox *co_mode;
   eListBoxEntryText *lt_mode[3];
   eLabel *lb_wep0;
   eTextInputField *if_wep0;
   eLabel *lb_ssid;
   eTextInputField *if_ssid;
   eButton *bt_cancel;
   eButton *bt_prev;
   eStatusBar *sb_help;
   
   public:
   securityScreen();
   ~securityScreen();
};

class loggingScreen: public eWindow
{
   private:
   void prevPressed(void);
   
   eLabel *screenOutput;
   eButton *bt_prev;
   
   public:
   loggingScreen();
   ~loggingScreen();
};

class wifiConfig
{
   public:
   wifiConfig();
   ~wifiConfig();

   void readFile(const char *fileName);
   void writeFile(const char *fileName);
   
   int wifiOn;
   int wepOn;
   int secMode;
   char ssid[17];
   char wep0[27];
   int ip[4];
   int nm[4];
   int gw[4];
   int dns[4];
};

#endif