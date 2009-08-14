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

01 Oct 2004 David van der Vliet
Creation of version v1.0
16 Oct 2004 David van der Vliet
Version v2.0 includes all settings
*/

#include "camx.h"

#include <plugin.h>
#include <stdio.h>
#include <unistd.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/listbox.h>
#include <lib/gui/emessage.h>

#define PLUGIN_VERSION "2.0"
#define CONFIG_FILENAME ("/var/bin/config")

camxScreen::camxScreen():eWindow(0)
{
   int y = 20;
	FILE *fp;
   int i = 0;
   char screen[10000];

	cmove(ePoint(100, 100));
	cresize(eSize(520, 400));
	setText(dgettext("plisetup", "Camx settings plugin v" PLUGIN_VERSION));

   lb_cardinfo = new eLabel(this);
   lb_cardinfo->move(ePoint(0, 0));
   lb_cardinfo->resize(eSize(clientrect.width(), clientrect.height()-70));

	// create all buttons
   y = clientrect.height()-70;
	bt_ok = new eButton(this);
	bt_ok->move(ePoint(10, y));
	bt_ok->resize(eSize(100, 30));
	bt_ok->setShortcut("green");
	bt_ok->setShortcutPixmap("green");
	bt_ok->loadDeco();
	bt_ok->setText("Save");
   bt_ok->setHelpText(dgettext("plisetup", "Save changes"));
   CONNECT(bt_ok->selected, eWidget::accept);

	bt_cancel = new eButton(this);
	bt_cancel->move(ePoint(143, y));
	bt_cancel->resize(eSize(100, 30));
	bt_cancel->setShortcut("red");
	bt_cancel->setShortcutPixmap("red");
	bt_cancel->loadDeco();
	bt_cancel->setText(dgettext("plisetup", "Quit"));
   bt_cancel->setHelpText(dgettext("plisetup", "Exit plugin (don't forget to save)"));
   CONNECT(bt_cancel->selected, eWidget::reject);
/*
	bt_next = new eButton(this);
	bt_next->move(ePoint(277, y));
	bt_next->resize(eSize(100, 30));
	bt_next->setShortcut("yellow");
	bt_next->setShortcutPixmap("yellow");
	bt_next->loadDeco();
	bt_next->setText(dgettext("plisetup", "Next"));
   bt_next->setHelpText(dgettext("plisetup", "Go to next screen"));
	CONNECT(bt_next->selected, camxScreen::gotoNext);
*/
	bt_about = new eButton(this);
	bt_about->move(ePoint(410, y));
	bt_about->resize(eSize(100, 30));
	bt_about->setShortcut("blue");
	bt_about->setShortcutPixmap("blue");
	bt_about->loadDeco();
	bt_about->setText(dgettext("plisetup", "About"));
   bt_about->setHelpText(dgettext("plisetup", "About this plugin"));
	CONNECT(bt_about->selected, camxScreen::showAbout);

   // Create statusbar
   sb_help = new eStatusBar(this);
   sb_help->move(ePoint(0, clientrect.height()-30));
   sb_help->resize(eSize(clientrect.width(), 30));
   sb_help->loadDeco();
   setHelpID(82);

   setFocus(bt_cancel);

   fp = fopen("/tmp/sc-info", "r");
   if(!fp)
   {
      lb_cardinfo->setText(dgettext("plisetup", "Could not find cardinfo"));
   }
   else
   {
      while((i = fread(screen, 1, 9999, fp)) > 0)
      {
         screen[i] = '\0';
      }

      lb_cardinfo->setText(eString().sprintf("%s", screen));

      fclose(fp);
   }
}

camxScreen::~camxScreen()
{
   delete lb_cardinfo;
   delete bt_ok;
   delete bt_cancel;
   delete bt_next;
   delete bt_about;
}

void camxScreen::showAbout(void)
{
   eMessageBox about(
      eString().sprintf(
      "Plugin user interface by dAF2000\n"
      "http://talk.to/dAF\n\n"
      "Camx cam by radxnl"),
      "About",
      eMessageBox::iconInfo|eMessageBox::btOK);

		about.show();
      about.exec();
      about.hide();
}
/*
void camxScreen::gotoNext(void)
{
   serverScreen screen;

   hide();
   screen.show();
   screen.exec();
   screen.hide();
   show();
   setFocus(bt_ok);
}
*/
/******************************************************************************/
/*
serverScreen::serverScreen():eWindow(0)
{
   int x = 20;
   int y = 20;

	cmove(ePoint(100, 100));
	cresize(eSize(520, 400));
	setText(dgettext("plisetup", "Server settings"));

   // Port
   lb_port = new eLabel(this);
   lb_port->move(ePoint(x, y));
   lb_port->resize(eSize(140, 30));
   lb_port->loadDeco();
   lb_port->setText(dgettext("plisetup", "Port"));

   nb_port = new eNumber(this, 1, 0, 65535, 5, &config.port, 0, lb_port);
   nb_port->move(ePoint(x+160, y));
   nb_port->resize(eSize(100, 30));
   nb_port->setHelpText(dgettext("plisetup", "Server port (PORT)"));
   nb_port->loadDeco();

   // Timeout
   y+=40;
   lb_timeout = new eLabel(this);
   lb_timeout->move(ePoint(x, y));
   lb_timeout->resize(eSize(140, 30));
   lb_timeout->loadDeco();
   lb_timeout->setText(dgettext("plisetup", "Net timeout"));

   nb_timeout = new eNumber(this, 1, 0, 9999, 4, &config.net_timeout_server, 0, lb_timeout);
   nb_timeout->move(ePoint(x+160, y));
   nb_timeout->resize(eSize(100, 30));
   nb_timeout->setHelpText(dgettext("plisetup", "Net timeout (NET_TIMEOUT)"));
   nb_timeout->loadDeco();

   // Backlog
   y+=40;
   lb_backlog = new eLabel(this);
   lb_backlog->move(ePoint(x, y));
   lb_backlog->resize(eSize(140, 30));
   lb_backlog->loadDeco();
   lb_backlog->setText(dgettext("plisetup", "Backlog"));

   nb_backlog = new eNumber(this, 1, 0, 9999, 4, &config.backlog, 0, lb_backlog);
   nb_backlog->move(ePoint(x+160, y));
   nb_backlog->resize(eSize(100, 30));
   nb_backlog->setHelpText(dgettext("plisetup", "Backlog connections (BACKLOG)"));
   nb_backlog->loadDeco();

   // ECM cache
   y+=40;
   lb_ecmCache = new eLabel(this);
   lb_ecmCache->move(ePoint(x, y));
   lb_ecmCache->resize(eSize(140, 30));
   lb_ecmCache->loadDeco();
   lb_ecmCache->setText(dgettext("plisetup", "ECM cache"));

   nb_ecmCache = new eNumber(this, 1, 0, 32, 2, &config.ecm_cache, 0, lb_ecmCache);
   nb_ecmCache->move(ePoint(x+160, y));
   nb_ecmCache->resize(eSize(100, 30));
   nb_ecmCache->setHelpText(dgettext("plisetup", "Number of ECM to keep in cache (ECM CACHE)"));
   nb_ecmCache->loadDeco();

	// create all buttons
   y = clientrect.height()-70;
	bt_cancel = new eButton(this);
	bt_cancel->move(ePoint(10, y));
	bt_cancel->resize(eSize(100, 30));
	bt_cancel->setShortcut("red");
	bt_cancel->setShortcutPixmap("red");
	bt_cancel->loadDeco();
	bt_cancel->setText(dgettext("plisetup", "Cancel"));
   bt_cancel->setHelpText(dgettext("plisetup", "Discard changes and return to previous screen"));
   CONNECT(bt_cancel->selected, eWidget::reject);

	bt_prev = new eButton(this);
	bt_prev->move(ePoint(210, y));
	bt_prev->resize(eSize(100, 30));
	bt_prev->setShortcut("green");
	bt_prev->setShortcutPixmap("green");
	bt_prev->loadDeco();
	bt_prev->setText(dgettext("plisetup", "Prev"));
   bt_prev->setHelpText(dgettext("plisetup", "Return to previous screen"));
	CONNECT(bt_prev->selected, serverScreen::gotoPrev);

	bt_next = new eButton(this);
	bt_next->move(ePoint(410, y));
	bt_next->resize(eSize(100, 30));
	bt_next->setShortcut("yellow");
	bt_next->setShortcutPixmap("yellow");
	bt_next->loadDeco();
	bt_next->setText(dgettext("plisetup", "Next"));
   bt_next->setHelpText(dgettext("plisetup", "Go to next screen"));
	CONNECT(bt_next->selected, serverScreen::gotoNext);

   // Create statusbar
   sb_help = new eStatusBar(this);
   sb_help->move(ePoint(0, clientrect.height()-30));
   sb_help->resize(eSize(clientrect.width(), 30));
   sb_help->loadDeco();
   setHelpID(82);

   setFocus(bt_next);
}

void serverScreen::gotoPrev(void)
{
   // Store settings

   close(0);
}

void serverScreen::gotoNext(void)
{
   clientScreen screen;

   hide();
   screen.show();
   screen.exec();
   screen.hide();
   show();
   setFocus(bt_prev);
}

serverScreen::~serverScreen()
{
   delete lb_port;
   delete nb_port;
   delete lb_timeout;
   delete nb_timeout;
   delete lb_backlog;
   delete nb_backlog;
   delete lb_ecmCache;
   delete nb_ecmCache;
   delete bt_prev;
   delete bt_cancel;
   delete bt_next;
   delete sb_help;
}
*/

/******************************************************************************/
/*
clientScreen::clientScreen():eWindow(0)
{
   int y = 20;

	cmove(ePoint(100, 100));
	cresize(eSize(520, 400));
	setText(dgettext("plisetup", "Client settings"));

	// create all buttons
   y = clientrect.height()-70;
	bt_cancel = new eButton(this);
	bt_cancel->move(ePoint(10, y));
	bt_cancel->resize(eSize(100, 30));
	bt_cancel->setShortcut("red");
	bt_cancel->setShortcutPixmap("red");
	bt_cancel->loadDeco();
	bt_cancel->setText(dgettext("plisetup", "Cancel"));
   bt_cancel->setHelpText(dgettext("plisetup", "Discard changes and return to previous screen"));
   CONNECT(bt_cancel->selected, eWidget::reject);

	bt_prev = new eButton(this);
	bt_prev->move(ePoint(210, y));
	bt_prev->resize(eSize(100, 30));
	bt_prev->setShortcut("green");
	bt_prev->setShortcutPixmap("green");
	bt_prev->loadDeco();
	bt_prev->setText(dgettext("plisetup", "Prev"));
   bt_prev->setHelpText(dgettext("plisetup", "Return to previous screen"));
	CONNECT(bt_prev->selected, clientScreen::gotoPrev);

	bt_next = new eButton(this);
	bt_next->move(ePoint(410, y));
	bt_next->resize(eSize(100, 30));
	bt_next->setShortcut("yellow");
	bt_next->setShortcutPixmap("yellow");
	bt_next->loadDeco();
	bt_next->setText(dgettext("plisetup", "Next"));
   bt_next->setHelpText(dgettext("plisetup", "Go to next screen"));
	CONNECT(bt_next->selected, clientScreen::gotoNext);

   // Create statusbar
   sb_help = new eStatusBar(this);
   sb_help->move(ePoint(0, clientrect.height()-30));
   sb_help->resize(eSize(clientrect.width(), 30));
   sb_help->loadDeco();
   setHelpID(82);

   setFocus(bt_next);
}

void clientScreen::gotoPrev(void)
{
   // Store settings

   close(0);
}

void clientScreen::gotoNext(void)
{
   miscScreen screen;

   hide();
   screen.show();
   screen.exec();
   screen.hide();
   show();
   setFocus(bt_prev);
}

clientScreen::~clientScreen()
{
   delete bt_prev;
   delete bt_cancel;
   delete bt_next;
   delete sb_help;
}
*/
/******************************************************************************/
/*
miscScreen::miscScreen():eWindow(0)
{
   int y = 20;

	cmove(ePoint(100, 100));
	cresize(eSize(520, 400));
	setText(dgettext("plisetup", "Miscellaneous settings"));

	// create all buttons
   y = clientrect.height()-70;
	bt_cancel = new eButton(this);
	bt_cancel->move(ePoint(10, y));
	bt_cancel->resize(eSize(100, 30));
	bt_cancel->setShortcut("red");
	bt_cancel->setShortcutPixmap("red");
	bt_cancel->loadDeco();
	bt_cancel->setText(dgettext("plisetup", "Cancel"));
   bt_cancel->setHelpText(dgettext("plisetup", "Discard changes and return to previous screen"));
   CONNECT(bt_cancel->selected, eWidget::reject);

	bt_prev = new eButton(this);
	bt_prev->move(ePoint(410, y));
	bt_prev->resize(eSize(100, 30));
	bt_prev->setShortcut("green");
	bt_prev->setShortcutPixmap("green");
	bt_prev->loadDeco();
	bt_prev->setText(dgettext("plisetup", "Prev"));
   bt_prev->setHelpText(dgettext("plisetup", "Return to previous screen"));
	CONNECT(bt_prev->selected, miscScreen::gotoPrev);

   // Create statusbar
   sb_help = new eStatusBar(this);
   sb_help->move(ePoint(0, clientrect.height()-30));
   sb_help->resize(eSize(clientrect.width(), 30));
   sb_help->loadDeco();
   setHelpID(82);

   setFocus(bt_prev);
}

void miscScreen::gotoPrev(void)
{
   // Store settings

   close(0);
}

miscScreen::~miscScreen()
{
   delete bt_prev;
   delete bt_cancel;
   delete sb_help;
}
*/
/******************************************************************************/
/*
camxConfig::camxConfig()
{
}

void camxConfig::readFile(const char *fileName)
{
}

void camxConfig::writeFile(const char *fileName)
{
}

camxConfig::~camxConfig()
{
}
*/

