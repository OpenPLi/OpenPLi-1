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

#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/listbox.h>
#include <lib/gui/emessage.h>
#include <lib/gui/enumber.h>
#include <lib/gui/elabel.h>
#include <lib/gui/echeckbox.h>

class camxScreen:public eWindow
{
   private:
   void showAbout(void);
//   void gotoNext(void);

   eButton *bt_ok;
   eButton *bt_cancel;
   eButton *bt_about;
   eButton *bt_next;
   eLabel *lb_cardinfo;
   eStatusBar *sb_help;

   public:
   camxScreen();
   ~camxScreen();
};
/*
class serverScreen:public eWindow
{
   private:
   void gotoPrev(void);
   void gotoNext(void);

   eLabel *lb_port;
   eNumber *nb_port;
   eLabel *lb_timeout;
   eNumber *nb_timeout;
   eLabel *lb_backlog;
   eNumber *nb_backlog;
   eLabel *lb_ecmCache;
   eNumber *nb_ecmCache;

   eButton *bt_prev;
   eButton *bt_cancel;
   eButton *bt_next;
   eStatusBar *sb_help;

   public:
   serverScreen();
   ~serverScreen();
};

class clientScreen:public eWindow
{
   private:
   void gotoPrev(void);
   void gotoNext(void);

   eCheckbox *cb_netEcm;
   eButton *bt_prev;
   eButton *bt_cancel;
   eButton *bt_next;
   eStatusBar *sb_help;

   public:
   clientScreen();
   ~clientScreen();
};

class miscScreen:public eWindow
{
   private:
   void gotoPrev(void);

   eButton *bt_prev;
   eButton *bt_cancel;
   eStatusBar *sb_help;

   public:
   miscScreen();
   ~miscScreen();
};

class camxConfig
{
   public:
   camxConfig();
   ~camxConfig();

   void readFile(const char *fileName);
   void writeFile(const char *fileName);

   // Server settings
   int port;
   int net_timeout_server;
   int backlog;
   int ecm_cache;
   char user[16][32];
   char user_password[16][32];
   bool net_ecm;

   // Miscellaneous
   int sc_read;
   int smartcard_write_delay;
   int smartcard_atr_delay;
   bool debug_ecm;
   bool debug_net;
   int update[4];
   int cw_delay;
   bool show_pmt;
   bool raw_ecm;
   bool display_cw;
   bool ecm_debug;
   bool emm_debug;
   bool emu;
   bool timing;
   bool block_seca_ppua;
   bool block_seca_ua;
   bool emm_monitor;
   bool emm_known;

   // Client

   bool prefer_emu;
   int net_timeout_network;
   int tries;
};


   */

