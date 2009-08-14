#ifdef WIFI
/*
    Copyright (C) 2004-2005  dAF2000
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

27 Sep 2004 dAF2000
Creation of version v1.0
02 Oct 2004 dAF2000
Version v1.1: enable/disable wifi runtime, shared/open key selection, new screens
29 Nov 2004 dAF2000
Version v1.2: ad hoc connection and link/signal/noise added
12 Feb 2005 dAF2000
Version v1.2.1: compiled for 1.09 images
28 May 2005 dAF2000
Version v1.2.2: 1.09.1b compatible, SSID fix, wifi default off, small GUI improvement
29 Oct 2005 dAF2000
Version v1.3: latest kernel modules, logging
*/

#include "wifi.h"

#include <plugin.h>
#include <stdio.h>
#include <unistd.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/textinput.h>
#include <lib/gui/elabel.h>
#include <lib/gui/enumber.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/combobox.h>

#define PLUGIN_VERSION "1.3"
#define CONFIG_FILENAME (CONFIGDIR "/wifiplugin.cfg")

//#define COMPILEASPLUGIN

#ifdef OE
#define BINPATH "/usr/bin"
#else
#define BINPATH "/var/wifi"
#endif

wifiConfig config;

#ifdef COMPILEASPLUGIN
extern "C" int plugin_exec(PluginParam *par);

int plugin_exec(PluginParam *par)
{
   // Read configuration
   config.readFile(CONFIG_FILENAME);

   wifiScreen screen;
   screen.show();
   screen.exec();
   screen.hide();

   return(0);
}
#endif

wifiScreen::wifiScreen():eWindow(0)
{
   int x = 40;
   int y = 20;

   cmove(ePoint(105, 100));
   cresize(eSize(510, 360));
   setText(dgettext("plisetup", "Wifi plugin v"PLUGIN_VERSION" - network settings"));

   // Wifi on/off checkbox
   cb_onoff = new eCheckbox(this, config.wifiOn);
   cb_onoff->move(ePoint(x, y));
   cb_onoff->resize(eSize(270, 30));
   cb_onoff->loadDeco();
   cb_onoff->setText(dgettext("plisetup", "Enable Wifi"));
   cb_onoff->setHelpText(dgettext("plisetup", "Enable or disable Wifi"));

   // Ip label
   y+=40;
   lb_ip = new eLabel(this);
   lb_ip->move(ePoint(x, y));
   lb_ip->resize(eSize(80, 30));
   lb_ip->loadDeco();
   lb_ip->setText(dgettext("plisetup", "IP"));

   // Ip eNumber
   nb_ip = new eNumber(this, 4, 0, 255, 3, config.ip, 0, lb_ip);
   nb_ip->move(ePoint(x+100, y));
   nb_ip->resize(eSize(170, 30));
   nb_ip->setFlags(eNumber::flagDrawPoints);
   nb_ip->setHelpText(dgettext("plisetup", "Enter your IP address"));
   nb_ip->loadDeco();

   // Netmask label
   y+=40;
   lb_netmask = new eLabel(this);
   lb_netmask->move(ePoint(x, y));
   lb_netmask->resize(eSize(80, 30));
   lb_netmask->loadDeco();
   lb_netmask->setText(dgettext("plisetup", "Netmask"));

   // Netmask eNumber
   nb_netmask = new eNumber(this, 4, 0, 255, 3, config.nm, 0, lb_netmask);
   nb_netmask->move(ePoint(x+100, y));
   nb_netmask->resize(eSize(170, 30));
   nb_netmask->setFlags(eNumber::flagDrawPoints);
   nb_netmask->setHelpText(dgettext("plisetup", "Enter your netmask"));
   nb_netmask->loadDeco();

   // Gateway label
   y+=40;
   lb_gateway = new eLabel(this);
   lb_gateway->move(ePoint(x, y));
   lb_gateway->resize(eSize(80, 30));
   lb_gateway->loadDeco();
   lb_gateway->setText(dgettext("plisetup", "Gateway"));

   // Gateway eNumber
   nb_gateway = new eNumber(this, 4, 0, 255, 3, config.gw, 0, lb_gateway);
   nb_gateway->move(ePoint(x+100, y));
   nb_gateway->resize(eSize(170, 30));
   nb_gateway->setFlags(eNumber::flagDrawPoints);
   nb_gateway->setHelpText(dgettext("plisetup", "Enter your gateway IP address"));
   nb_gateway->loadDeco();

   // DNS label
   y+=40;
   lb_dns = new eLabel(this);
   lb_dns->move(ePoint(x, y));
   lb_dns->resize(eSize(80, 30));
   lb_dns->loadDeco();
   lb_dns->setText(dgettext("plisetup", "DNS"));

   // DNS eNumber
   nb_dns = new eNumber(this, 4, 0, 255, 3, config.dns, 0, lb_dns);
   nb_dns->move(ePoint(x+100, y));
   nb_dns->resize(eSize(170, 30));
   nb_dns->setFlags(eNumber::flagDrawPoints);
   nb_dns->setHelpText(dgettext("plisetup", "Enter your Dynamic Name Server IP address"));
   nb_dns->loadDeco();

   // link, level and noise labels
   y+=40;
   lb_link = new eLabel(this);
   lb_link->move(ePoint(x, y));
   lb_link->resize(eSize(100, 30));
   lb_link->loadDeco();
   lb_link->setText(dgettext("plisetup", "Link: 0"));

   lb_level = new eLabel(this);
   lb_level->move(ePoint(x+120, y));
   lb_level->resize(eSize(100, 30));
   lb_level->loadDeco();
   lb_level->setText(dgettext("plisetup", "Level: 0"));

   lb_noise = new eLabel(this);
   lb_noise->move(ePoint(x+240, y));
   lb_noise->resize(eSize(100, 30));
   lb_noise->loadDeco();
   lb_noise->setText(dgettext("plisetup", "Noise: 0"));

   // Create all buttons
   y = clientrect.height()-90;
   bt_ok = new eButton(this);
   bt_ok->move(ePoint(10, y));
   bt_ok->resize(eSize(100, 30));
   bt_ok->setShortcut("green");
   bt_ok->setShortcutPixmap("green");
   bt_ok->loadDeco();
   bt_ok->setText(dgettext("plisetup", "Save"));
   bt_ok->setHelpText(dgettext("plisetup", "Save changes"));
   CONNECT(bt_ok->selected, wifiScreen::saveChanges);

   bt_cancel = new eButton(this);
   bt_cancel->move(ePoint(140, y));
   bt_cancel->resize(eSize(100, 30));
   bt_cancel->setShortcut("red");
   bt_cancel->setShortcutPixmap("red");
   bt_cancel->loadDeco();
   bt_cancel->setText(dgettext("plisetup", "Exit"));
   bt_cancel->setHelpText(dgettext("plisetup", "Exit this plugin, don't forget to save!"));
   CONNECT(bt_cancel->selected, eWidget::reject);

   bt_next = new eButton(this);
   bt_next->move(ePoint(270, y));
   bt_next->resize(eSize(100, 30));
   bt_next->setShortcut("blue");
   bt_next->setShortcutPixmap("blue");
   bt_next->loadDeco();
   bt_next->setText(dgettext("plisetup", "Next"));
   bt_next->setHelpText(dgettext("plisetup", "Set security settings"));
   CONNECT(bt_next->selected, wifiScreen::nextPressed);

   bt_about = new eButton(this);
   bt_about->move(ePoint(400, y));
   bt_about->resize(eSize(100, 30));
   bt_about->setShortcut("yellow");
   bt_about->setShortcutPixmap("yellow");
   bt_about->loadDeco();
   bt_about->setText(dgettext("plisetup", "About"));
   bt_about->setHelpText(dgettext("plisetup", "About the Wifi plugin"));
   CONNECT(bt_about->selected, wifiScreen::showAbout);

   // Create statusbar
   sb_help = new eStatusBar(this);
   sb_help->move(ePoint(0, clientrect.height()-50));
   sb_help->resize(eSize(clientrect.width(), 50));
   sb_help->loadDeco();
   setHelpID(82);

   // Update timer
   et_timer = new eTimer(eApp);
   et_timer->start(1000, true);
   CONNECT(et_timer->timeout, wifiScreen::update);
}

wifiScreen::~wifiScreen()
{
   et_timer->stop();
   if(cb_onoff)   delete cb_onoff;
   if(lb_ip)      delete lb_ip;
   if(lb_netmask) delete lb_netmask;
   if(lb_gateway) delete lb_gateway;
   if(lb_dns)     delete lb_dns;
   if(lb_link)    delete lb_link;
   if(lb_level)   delete lb_level;
   if(lb_noise)   delete lb_noise;
   if(nb_ip)      delete nb_ip;
   if(nb_netmask) delete nb_netmask;
   if(nb_gateway) delete nb_gateway;
   if(nb_dns)     delete nb_dns;
   if(bt_ok)      delete bt_ok;
   if(bt_cancel)  delete bt_cancel;
   if(bt_next)    delete bt_next;
   if(bt_about)   delete bt_about;
   if(sb_help)    delete sb_help;
   if(et_timer)   delete et_timer;
}

void wifiScreen::showAbout(void)
{
   eMessageBox about(
      dgettext("plisetup", "Plugin by dAF2000\n"
      "http://talk.to/dAF\n\n"
      "Wifi drivers released under GPL\n"
      "by AbsoluteValue Systems, Inc.\n\n"
      "Thanks to Rongogo, djmastera2000,\n"
      "PLi image team, Gemini image testers\n"
      "and others"),
      dgettext("plisetup", "About"),
      eMessageBox::iconInfo|eMessageBox::btOK);

   about.show();
   about.exec();
   about.hide();
}

void wifiScreen::nextPressed(void)
{
   // Hide wifiscreen
   hide();

   // Open security screen
   securityScreen screen;
   screen.show();
   screen.exec();
   screen.hide();

   // Show wifiscreen again
   show();
}

void wifiScreen::saveChanges(void)
{
   unsigned int i;

   // Store settings
   config.wifiOn = cb_onoff->isChecked();

   for(i=0; i<4; ++i)
   {
      config.ip[i] = nb_ip->getNumber(i);
   }

   for(i=0; i<4; ++i)
   {
      config.nm[i] = nb_netmask->getNumber(i);
   }

   for(i=0; i<4; ++i)
   {
      config.gw[i] = nb_gateway->getNumber(i);
   }

   for(i=0; i<4; ++i)
   {
      config.dns[i] = nb_dns->getNumber(i);
   }

   setFocus(bt_ok);
   bt_ok->setHelpText(eString().sprintf("Changes saved, Wifi should be %s now",
      config.wifiOn ? "enabled":"disabled"));

   // Save settings
   config.writeFile(CONFIG_FILENAME);

   // Run wifistart of wifistop
   if(config.wifiOn)
   {
      system(BINPATH"/wifistart.sh");
   }
   else
   {
      system(BINPATH"/wifistop.sh");
   }
}

void wifiScreen::update(void)
{
   FILE *pipe;
   int link = 0;
   int level = 0;
   int noise = 0;
   char temp[256];

   pipe = popen(BINPATH"/wlanctl-ng wlan0 lnxreq_commsquality", "r");
   if(pipe)
   {
      while(fscanf(pipe, "%s", temp) != EOF)
      {
         if(!strncmp(temp, "link=", 5)) link=atoi(&temp[5]);
         if(!strncmp(temp, "level=", 6)) level=atoi(&temp[6]);
         if(!strncmp(temp, "noise=", 6)) noise=atoi(&temp[6]);
      }
      pclose(pipe);
   }

   lb_link->setText(eString().sprintf("Link: %d", link));
   lb_level->setText(eString().sprintf("Level: %d", level));
   lb_noise->setText(eString().sprintf("Noise: %d", noise));

   et_timer->start(1000, true);
}

securityScreen::securityScreen():eWindow(0)
{
   int x = 40;
   int y = 20;

   cmove(ePoint(105, 100));
   cresize(eSize(510, 360));
   setText(dgettext("plisetup", "Wifi plugin v"PLUGIN_VERSION" - security settings"));

   // SSID label
   lb_ssid = new eLabel(this);
   lb_ssid->move(ePoint(x, y));
   lb_ssid->resize(eSize(80, 30));
   lb_ssid->loadDeco();
   lb_ssid->setText(dgettext("plisetup", "SSID"));

   // SSID textinputfield
   if_ssid = new eTextInputField(this);
   if_ssid->setMaxChars(16);
   if_ssid->move(ePoint(x+100, y));
   if_ssid->resize(eSize(clientrect.width()-180, 30));
   if_ssid->loadDeco();
   if_ssid->setHelpText(dgettext("plisetup", "Enter SSID"));
   if_ssid->setText(eString().sprintf("%s", config.ssid));

   // Security mode label
   y+=40;
   lb_mode = new eLabel(this);
   lb_mode->move(ePoint(x, y));
   lb_mode->resize(eSize(80, 30));
   lb_mode->loadDeco();
   lb_mode->setText(dgettext("plisetup", "Mode"));

   // Security mode combobox
   co_mode = new eComboBox(this, 3);
   co_mode->move(ePoint(x+100, y));
   co_mode->resize(eSize(140, 35));
   co_mode->loadDeco();
   co_mode->setHelpText(dgettext("plisetup", "Choose between open system, shared key or ad hoc connection"));

   lt_mode[0] = new eListBoxEntryText(*co_mode, dgettext("plisetup", "open system"), (void*)0);
   lt_mode[1] = new eListBoxEntryText(*co_mode, dgettext("plisetup", "shared key"), (void*)1);
   lt_mode[2] = new eListBoxEntryText(*co_mode, dgettext("plisetup", "ad hoc"), (void*)2);

   co_mode->setCurrent(lt_mode[config.secMode]);
   CONNECT(co_mode->selchanged, securityScreen::securityStateChanged);

   // Wep on/off checkbox
   y+=40;
   cb_wep = new eCheckbox(this, config.wepOn);
   cb_wep->move(ePoint(x, y));
   cb_wep->resize(eSize(240, 30));
   cb_wep->loadDeco();
   cb_wep->setText(dgettext("plisetup", "Enable wep key"));
   cb_wep->setHelpText(dgettext("plisetup", "Enable or disable wep key"));
   CONNECT(cb_wep->checked, securityScreen::wepStateChanged);

   // Wep0 label
   y+=40;
   lb_wep0 = new eLabel(this);
   lb_wep0->move(ePoint(x, y));
   lb_wep0->resize(eSize(80, 30));
   lb_wep0->loadDeco();
   lb_wep0->setText(dgettext("plisetup", "Wep key"));

   // Wep0 textinputfield
   if_wep0 = new eTextInputField(this);
   if_wep0->setMaxChars(26);
   if_wep0->setUseableChars("0123456789abcdefABCDEF");
   if_wep0->move(ePoint(x+100, y));
   if_wep0->resize(eSize(clientrect.width()-180, 30));
   if_wep0->loadDeco();
   if_wep0->setHelpText(dgettext("plisetup", "Enter wep key, 10 or 26 hexadecimal digits"));
   if_wep0->setEditHelpText(dgettext("plisetup", "Enter 0..9, a..f, ok to save (help is available)"));
   if_wep0->setText(eString().sprintf("%s", config.wep0));

   wepStateChanged((config.secMode == 2) ? 0 : config.wepOn);
   securityStateChanged(lt_mode[config.secMode]);

   // Create all buttons
   y = clientrect.height()-90;
   bt_prev = new eButton(this);
   bt_prev->move(ePoint(140, y));
   bt_prev->resize(eSize(100, 30));
   bt_prev->setShortcut("red");
   bt_prev->setShortcutPixmap("red");
   bt_prev->loadDeco();
   bt_prev->setText(dgettext("plisetup", "Prev"));
   bt_prev->setHelpText(dgettext("plisetup", "Return to network settings"));
   CONNECT(bt_prev->selected, securityScreen::prevPressed);

   bt_cancel = new eButton(this);
   bt_cancel->move(ePoint(270, y));
   bt_cancel->resize(eSize(100, 30));
   bt_cancel->setShortcut("blue");
   bt_cancel->setShortcutPixmap("blue");
   bt_cancel->loadDeco();
   bt_cancel->setText(dgettext("plisetup", "Next"));
   bt_cancel->setHelpText(dgettext("plisetup", "Show logging screen"));
   CONNECT(bt_cancel->selected, securityScreen::nextPressed);

   // Create statusbar
   sb_help = new eStatusBar(this);
   sb_help->move(ePoint(0, clientrect.height()-50));
   sb_help->resize(eSize(clientrect.width(), 50));
   sb_help->loadDeco();
   setHelpID(82);
}

securityScreen::~securityScreen()
{
   if(cb_wep)     delete cb_wep;
   if(lb_mode)    delete lb_mode;
   if(lt_mode[0]) delete lt_mode[0];
   if(lt_mode[1]) delete lt_mode[1];
   if(lt_mode[2]) delete lt_mode[2];
   if(co_mode)    delete co_mode;
   if(lb_wep0)    delete lb_wep0;
   if(if_wep0)    delete if_wep0;
   if(lb_ssid)    delete lb_ssid;
   if(if_ssid)    delete if_ssid;
   if(bt_cancel)  delete bt_cancel;
   if(bt_prev)    delete bt_prev;
   if(sb_help)    delete sb_help;
}

void securityScreen::prevPressed(void)
{
   saveSettings();
   close(0);
}

void securityScreen::nextPressed(void)
{
   saveSettings();

   // Hide securityScreen
   hide();

   // Open logging screen
   loggingScreen screen;
   screen.show();
   screen.exec();
   screen.hide();

   // Show securityScreen again
   show();
}

void securityScreen::saveSettings(void)
{
   // Store settings
   config.wepOn = cb_wep->isChecked();
   config.secMode = (int)co_mode->getCurrent()->getKey();
   strcpy(config.ssid, if_ssid->getText().c_str());
   strcpy(config.wep0, if_wep0->getText().c_str());
}

void securityScreen::wepStateChanged(int state)
{
   if(state)
   {
      if_wep0->show();
      lb_wep0->show();
   }
   else
   {
      if_wep0->hide();
      lb_wep0->hide();
   }
}

void securityScreen::securityStateChanged(eListBoxEntryText *le)
{
   if(le)
   {
      switch((int)le->getKey())
      {
         case 0:
         case 1:
            // Open system or shared key
            cb_wep->show();
            wepStateChanged(config.wepOn);
         break;

         case 2:
            // Adhoc
            cb_wep->hide();
            if_wep0->hide();
            lb_wep0->hide();
         break;

         default:
         break;
      }
   }
}

loggingScreen::loggingScreen():eWindow(0)
{
   int x = 5;
   int y = 5;

   cmove(ePoint(105, 100));
   cresize(eSize(510, 360));
   setText(dgettext("plisetup", "Wifi plugin v"PLUGIN_VERSION" - logging"));

   screenOutput = new eLabel(this);
   screenOutput->move(ePoint(x, y));
   screenOutput->resize(eSize(500, 300));
   screenOutput->loadDeco();

   FILE *fp = fopen("/tmp/wifi.log", "r");
   char outputString[1024];

   if(fp)
   {
      int nrChars = fread(outputString, sizeof(char), 1023, fp);
      outputString[nrChars] = '\0';
      screenOutput->setText(outputString);
      fclose(fp);
   }
   else
   {
      screenOutput->setText(dgettext("plisetup", "No wifi logging found"));
   }

   // Create all buttons
   y = clientrect.height()-90;
   bt_prev = new eButton(this);
   bt_prev->move(ePoint(205, 320));
   bt_prev->resize(eSize(100, 30));
   bt_prev->setShortcut("red");
   bt_prev->setShortcutPixmap("red");
   bt_prev->loadDeco();
   bt_prev->setText(dgettext("plisetup", "Prev"));
   CONNECT(bt_prev->selected, eWidget::reject);
}

loggingScreen::~loggingScreen()
{
   if(screenOutput) delete screenOutput;
   if(bt_prev) delete bt_prev;
}

wifiConfig::wifiConfig()
{
   wifiOn = 0;
   wepOn = 1;
   secMode = 1;
   strcpy(ssid, "Enter_SSID");
   strcpy(wep0, "0000000000");
   ip[0] = 192; ip[1] = 168; ip[2] = 1; ip[3] = 10;
   nm[0] = 255; nm[1] = 255; nm[2] = 255; nm[3] = 0;
   gw[0] = 192; gw[1] = 168; gw[2] = 1; gw[3] = 1;
   dns[0] = 192; dns[1] = 168; dns[2] = 1; dns[3] = 1;
}

wifiConfig::~wifiConfig()
{
}

void wifiConfig::readFile(
   const char *fileName)
{
   FILE *fp;
   char line[128];
   char *p;

   char wifiOnStr[128] = "";
   char wepStr[128] = "";
   char secModeStr[128] = "";
   char ssidStr[128] = "";
   char wep0Str[128] = "";
   char ipStr[128] = "";
   char netmaskStr[128] = "";
   char gatewayStr[128] = "";
   char dnsStr[128] = "";

   fp = fopen(fileName, "r");
   if(!fp)
   {
      // No file found, leave out parameters unchanged
   }
   else
   {
      while(fgets(line, 128, fp))
      {
         if(*line == '#')
         {
            // comment
            continue;
         }

         // Search for an assignment
         p = strchr(line, '=');

         if(!p)
         {
            // no "=" found
            continue;
         }

         *p=0;
         p++;

         if(!strcmp(line, "wifi_on"))
         {
            strcpy(wifiOnStr, p);
         }
         else if(!strcmp(line, "wep"))
         {
            strcpy(wepStr, p);
         }
         else if(!strcmp(line, "authtype"))
         {
            strcpy(secModeStr, p);
         }
         else if(!strcmp(line, "ssid"))
         {
            strcpy(ssidStr, p);
         }
         else if(!strcmp(line, "wep0"))
         {
            strcpy(wep0Str, p);
         }
         else if(!strcmp(line, "ip"))
         {
            strcpy(ipStr, p);
         }
         else if(!strcmp(line, "nm"))
         {
            strcpy(netmaskStr, p);
         }
         else if(!strcmp(line, "gw"))
         {
            strcpy(gatewayStr, p);
         }
         else if(!strcmp(line, "dns"))
         {
            strcpy(dnsStr, p);
         }
         else
         {
            // Another assignment
         }
      }

      fclose(fp);

      // Now parse strings read
      if(*wifiOnStr)
      {
         wifiOn = atoi(wifiOnStr);
      }

      if(*wepStr)
      {
         wepOn = atoi(wepStr);
      }

      if(*secModeStr)
      {
         if(!strncmp(secModeStr, "adhoc", size_t(5)))
         {
            secMode = 2;
         }
         else if(!strncmp(secModeStr, "sharedkey", size_t(9)))
         {
            secMode = 1;
         }
         else
         {
            secMode = 0;
         }
      }

      if(*ssidStr)
      {
      	sscanf(ssidStr, "%s", ssid);
      }

      if(*wep0Str)
      {
         int i = 0;
         p = wep0Str;
         while(*p)
         {
            if(isxdigit(*p))
            {
               wep0[i] = *p;
               ++i;
            }
            ++p;
         }
         wep0[i] = '\0';
      }

      if(*ipStr)
      {
         p = ipStr;
         for(int i=0; i<=3; ++i)
         {
            if(p)
            {
               ip[i] = atoi(p);
               p = strchr(p, '.');
               if(p) ++p;
            }
         }
      }

      if(*netmaskStr)
      {
         p = netmaskStr;
         for(int i=0; i<=3; ++i)
         {
            if(p)
            {
               nm[i] = atoi(p);
               p = strchr(p, '.');
               if(p) ++p;
            }
         }
      }

      if(*gatewayStr)
      {
         p = gatewayStr;
         for(int i=0; i<=3; ++i)
         {
            if(p)
            {
               gw[i] = atoi(p);
               p = strchr(p, '.');
               if(p) ++p;
            }
         }
      }

      if(*dnsStr)
      {
         p = dnsStr;
         for(int i=0; i<=3; ++i)
         {
            if(p)
            {
               dns[i] = atoi(p);
               p = strchr(p, '.');
               if(p) ++p;
            }
         }
      }
   }
}

void wifiConfig::writeFile(
   const char *fileName)
{
   FILE *fp;
   char wep0Key[50] = "";

   fp = fopen(fileName, "w");
   if(!fp)
   {
      // File cannot be opened. Error message needed here.
   }
   else
   {
      fprintf(fp, "# Configuration created by wifi plugin\n");
      fprintf(fp, "wifi_on=%d\n", wifiOn);
      fprintf(fp, "wep=%d\n", wepOn);
      fprintf(fp, "authtype=%s\n",
         (secMode==0) ? "opensystem" :
         (secMode==1) ? "sharedkey" : "adhoc");
      fprintf(fp, "ssid=%s\n", ssid);

      for(unsigned int i=0; i<strlen(wep0); i+=2)
      {
         strncat(wep0Key, wep0+i, 2);
         strcat(wep0Key, ":");
      }
      wep0Key[strlen(wep0Key)-1] = '\0';

      fprintf(fp, "wep0=%s\n", wep0Key);
      fprintf(fp, "ip=%d.%d.%d.%d\n",
         ip[0], ip[1], ip[2], ip[3]);
      fprintf(fp, "nm=%d.%d.%d.%d\n",
         nm[0], nm[1], nm[2], nm[3]);
      fprintf(fp, "gw=%d.%d.%d.%d\n",
         gw[0], gw[1], gw[2], gw[3]);
      fprintf(fp, "dns=%d.%d.%d.%d\n",
         dns[0], dns[1], dns[2], dns[3]);
      fclose(fp);
   }
}

#endif
