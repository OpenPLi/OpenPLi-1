/*
 *  ProgressSetupWindow
 *
 *  Extension to the eSetupWindow with a progress bar
 *
 *  Copyright 2006 (C) dAF2000 PLi image team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <lib/gui/ProgressSetupWindow.h>

ProgressSetupWindow::ProgressSetupWindow
   (const char *titlemm, int entries, int width)
   : eSetupWindow(titlemm, entries, width)
{
}

void ProgressSetupWindow::setNrOfBars(int bars)
{
   nrOfBars = (bars > maxNrOfBars) ? maxNrOfBars : bars;
   
   // Extend base window and move statusbar
   cresize(eSize(clientrect.width(), clientrect.height()+10+30*nrOfBars));
   statusbar->move(ePoint(0, clientrect.height()-50));

   for(int i=0; i<nrOfBars; ++i)
   {
      progressLabel[i] = new eLabel(this, eLabel::flagVCenter);
      progressLabel[i]->move(ePoint(10, clientrect.height()-90-30*i));
      progressLabel[i]->resize(eSize(clientrect.width(), 40));
      progressLabel[i]->loadDeco();
      progressLabel[i]->show();
   
      progressBar[i] = new eProgress(this);
      progressBar[i]->move(ePoint(150, clientrect.height()-80-30*i));
      progressBar[i]->resize(eSize(clientrect.width()-160, 20));
      progressBar[i]->show();
   }
}

void ProgressSetupWindow::setProgressLabel(eString label, int barNr)
{
   if(barNr >= 0 && barNr < nrOfBars)
   {
      progressLabel[barNr]->setText(label);
   }
}

void ProgressSetupWindow::setProgressBar(int percent, int barNr)
{
   if(barNr >= 0 && barNr < nrOfBars)
   {
      progressBar[barNr]->setPerc(percent);
   }
}

