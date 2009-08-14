/* ExecuteOutput
 * Opens a window for script output with a small font.
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

#include <lib/base/estring.h>
#include <lib/gui/listbox.h>
#include <lib/gui/ebutton.h>
#include <lib/base/console.h>
#include <lib/gui/emessage.h>
#include <lib/gdi/font.h>

#include <lib/gui/ExecuteOutput.h>
#include <lib/gui/myListBoxEntryText.h>

ExecuteOutput::ExecuteOutput(const eString &title, const eString &target)
   :eWindow(0)
{
   unsigned int v_tvsystem;
   
   eConfig::getInstance()->getKey("/elitedvb/video/tvsystem", v_tvsystem);
   int deltaY = (v_tvsystem == 2) ? -48 : 0;
	eRCInput::getInstance()->lock();
   setText(title);

   cresize(eSize(560, 420 + deltaY));
   valign();

   setListboxFont(eSkin::getActive()->queryFont("global.nonprop.small"));

   lbx = new eListBox<eListBoxEntryText>(this);
   lbx->move(ePoint(10, 10));
   lbx->loadDeco();
   lbx->resize(eSize(clientrect.width()-20, 360 + deltaY));

   eButton* bClose = new eButton(this);
   bClose->setText(_("Close"));
   bClose->setShortcut("red");
   bClose->setShortcutPixmap("red");
   bClose->move(ePoint(10, 370 + deltaY));
   bClose->resize(eSize(120, 40));
   bClose->loadDeco();
   bClose->hide();
   
   eButton* bScrollLeft = new eButton(this);
   bScrollLeft->setText(_("<< Scroll left"));
   bScrollLeft->setShortcut("green");
   bScrollLeft->setShortcutPixmap("green");
   bScrollLeft->move(ePoint(140, 370 + deltaY));
   bScrollLeft->resize(eSize(200, 40));
   bScrollLeft->loadDeco();
   bScrollLeft->hide();
   
   eButton* bScrollRight = new eButton(this);
   bScrollRight->setText(_("Scroll right >>"));
   bScrollRight->setShortcut("yellow");
   bScrollRight->setShortcutPixmap("yellow");
   bScrollRight->move(ePoint(350, 370 + deltaY));
   bScrollRight->resize(eSize(200, 40));
   bScrollRight->loadDeco();
   bScrollRight->hide();
   
   CONNECT(bClose->selected, ExecuteOutput::accept);
   CONNECT(bScrollLeft->selected, ExecuteOutput::scrollLeft);
   CONNECT(bScrollRight->selected, ExecuteOutput::scrollRight);

   eMessageBox mb(title, _("Please wait"), eMessageBox::iconInfo);
   mb.show();

   lineSize = 48;
   startChar = 0;
   
   FILE *pipe = popen(target.c_str(), "r");
   if(pipe)
   {
      char temp[1024];
      
      while(fgets(temp, 1023, pipe))
      {
         eListBoxEntryText *lbxEntry = new eListBoxEntryText(lbx,
            ((eString)temp).mid(startChar, lineSize), (void *)0);
         textOut.push_back((eString)temp);
         lbxEntries.push_back(lbxEntry);
      }
      pclose(pipe);
   }
   else
   {
      // This is seldom called. Executing a non-existing program doesn't let popen fail
      new eListBoxEntryText(lbx, _("Could not execute script"), (void *)0);
   }

   mb.hide();
   bClose->show();
   bScrollLeft->show();
   bScrollRight->show();
	eRCInput::getInstance()->unlock();
}

ExecuteOutput::~ExecuteOutput()
{
   // Set the eListBox font back to default value
   setListboxFont(eSkin::getActive()->queryFont("eListBox.EntryText.normal"));
}

void ExecuteOutput::scrollLeft()
{
   startChar -= 20;
   if(startChar < 0)
   {
      startChar = 0;
   }
   
   buildListBox();
}

void ExecuteOutput::scrollRight()
{
   startChar += 20;
   
   buildListBox();
}

void ExecuteOutput::buildListBox()
{
   for(unsigned int i=0; i<lbxEntries.size(); ++i)
   {
      lbxEntries[i]->SetText(textOut[i].mid(startChar, lineSize));
   }
   
   // Redraw listbox
   lbx->invalidate();
}

