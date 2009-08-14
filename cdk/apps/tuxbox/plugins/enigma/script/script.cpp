/*
	Script - Enigma Plugin

	Plugin that calls scripts and displays the output on screen
	
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

#include <plugin.h>
#include <stdio.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gui/eprogress.h>
#include <lib/gdi/font.h>
#include <lib/gui/guiactions.h>
#include <lib/system/info.h>
#include <lib/gui/emessage.h>

#define NUM_SCRIPTS 10

class eScriptWindow: public eWindow
{
	eButton *bt_scripts[10];
	eLabel *label;
	void runScript(int i);
	void initButtons();
public:
	eScriptWindow();
};

class eShowFile: public eWindow
{
	eLabel *label;
	eWidget *visible;
	eProgress *scrollbar;
	int pageHeight;
	int total;
	int eventHandler(const eWidgetEvent &event);
	void updateScrollbar();
public:
	eShowFile();
};

extern "C" int plugin_exec( PluginParam *par )
{
	eScriptWindow dlg;
	dlg.show();
	int result=dlg.exec();
	dlg.hide();
	return result;
}

eScriptWindow::eScriptWindow(): eWindow(1)
{
	cmove(ePoint(100, 100));
	cresize(eSize(520, 376));
	setText((_("Script Plugin")));
	initButtons();
}

void eScriptWindow::initButtons()
{
	bool empty=true;
	int j = 1;
	eString filename;

	for(int i=1; i<NUM_SCRIPTS; i++)
	{
		if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020)
			filename.sprintf("/usr/bin/script%.02d.sh", i);
		else 
			filename.sprintf("/var/bin/script%.02d.sh", i);
		if (access(filename.c_str(), 0) == 0)
		{
			bt_scripts[j-1]=new eButton(this);
			bt_scripts[j-1]->move(ePoint(10, 10+((j-1)*32)));
			bt_scripts[j-1]->resize(eSize(clientrect.width()-20, 30));
			bt_scripts[j-1]->setShortcut(eString().sprintf("%d",i));
			bt_scripts[j-1]->setShortcutPixmap(eString().sprintf("%d",j));
			bt_scripts[j-1]->loadDeco();
			bt_scripts[j-1]->setText(eString().sprintf("Script #%d (%s)", j, filename.c_str()));
			CONNECT_1_0(bt_scripts[j-1]->selected, eScriptWindow::runScript, i);
			if (empty)
				empty = false;
			++j;
		}
	}

	if (empty)
	{	
		label = new eLabel(this);
		label->setFlags(RS_WRAP);
   		label->setText((_("No Scripts Found!\n\nYou can place script01.sh - script09.sh in /var/bin (/usr/bin on the Dreambox 7020)")));
   		label->move(ePoint(10, 10));
   		label->resize(eSize(clientrect.width()-20, 100));
   		label->loadDeco();
	}	
	else 
		setFocus(bt_scripts[0]);
}

void eScriptWindow::runScript(int i)
{
	if (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020)
		system(eString().sprintf("/usr/bin/script%.02d.sh > /var/tmp/script.out 2>&1", i).c_str());
	else
		system(eString().sprintf("/var/bin/script%.02d.sh > /var/tmp/script.out 2>&1", i).c_str());
	hide();
	eShowFile execute;
	execute.show();
	execute.exec();
	execute.hide();
	show();
	setFocus(bt_scripts[0]);
}


eShowFile::eShowFile():
eWindow(1)
{
   cmove(ePoint(70, 85));
   cresize(eSize(595, 450));

   setText((_("Script Output")));

   scrollbar = new eProgress(this);
   scrollbar->setName("scrollbar");
   scrollbar->setStart(0);
   scrollbar->setPerc(100);
   scrollbar->move(ePoint(width() - 30, 5));
   scrollbar->resize(eSize(20, height() - 100));
   scrollbar->setProperty("direction", "1");

   visible = new eWidget(this);
   visible->setName("visible");
   visible->move(ePoint(10, 5));
   visible->resize(eSize(width() - 40, height() - 100));


   eString strview;
   char buf[256];

   FILE *f = fopen("/var/tmp/script.out", "rt");
   if (f)
   {
      int len = 0;
      while (fgets(buf, 256, f))
      {
         len += strlen(buf);
         if (len <= 65536)
            strview += eString().sprintf("%s", buf);
      }
      fclose(f);
   }

   ::unlink("/var/tmp/script.out");


   label = new eLabel(visible);
   label->setFlags(RS_WRAP);
   label->setFont(eSkin::getActive()->queryFont("eStatusBar"));
   float lineheight = fontRenderClass::getInstance()->getLineHeight(label->getFont());
   int lines = (int) (visible->getSize().height() / lineheight);
   pageHeight = (int) (lines * lineheight);
   visible->resize(eSize(visible->getSize().width(), pageHeight + (int) (lineheight / 6)));
   label->resize(eSize(visible->getSize().width(), pageHeight * 16));

   label->hide();
   label->move(ePoint(0, 0));
   label->setText(strview);
   updateScrollbar();
   label->show();
}

int eShowFile::eventHandler(const eWidgetEvent & event)
{
   switch (event.type)
   {
   case eWidgetEvent::evtAction:
      if (total && event.action == &i_cursorActions->up)
      {
         ePoint curPos = label->getPosition();
         if (curPos.y() < 0)
         {
            label->move(ePoint(curPos.x(), curPos.y() + pageHeight));
            updateScrollbar();
         }
      }
      else if (total && event.action == &i_cursorActions->down)
      {
         ePoint curPos = label->getPosition();
         if ((total - pageHeight) >= abs(curPos.y() - pageHeight))
         {
            label->move(ePoint(curPos.x(), curPos.y() - pageHeight));
            updateScrollbar();
         }
      }
      else if (event.action == &i_cursorActions->cancel)
         close(0);
      else
         break;
      return 1;
   default:
      break;
   }
   return eWindow::eventHandler(event);
}

void eShowFile::updateScrollbar()
{
   total = pageHeight;
   int pages = 1;
   while (total < label->getExtend().height())
   {
      total += pageHeight;
      pages++;
   }

   int start = -label->getPosition().y() * 100 / total;
   int vis = pageHeight * 100 / total;
   scrollbar->setParams(start, vis);
   scrollbar->show();
   if (pages == 1)
      total = 0;
}

