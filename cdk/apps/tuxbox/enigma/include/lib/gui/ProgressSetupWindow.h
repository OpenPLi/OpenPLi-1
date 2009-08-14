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

#ifndef __progresssetupwindow_h
#define __progresssetupwindow_h

#include <src/setup_window.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/elabel.h>

class ProgressSetupWindow:public eSetupWindow
{
   public:
   static const int maxNrOfBars = 3;
   
   ProgressSetupWindow(const char *titlemm, int entries, int width);
   void setNrOfBars(int bars);
   void setProgressLabel(eString label, int barNr);
   void setProgressBar(int percent, int barNr);

   protected:
   eLabel *progressLabel[maxNrOfBars];
   eProgress *progressBar[maxNrOfBars];
   int nrOfBars;
};

#endif // __progresssetupwindow_h
