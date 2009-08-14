/*
 * ePLiWindow, an eWindow to have the same look and feel in all PLi windows
 * Copyright (c) 2006 dAF2000 daf2000@ditadres.nl
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

#ifndef __epliwindow_h
#define __epliwindow_h

#include <lib/gui/ewidget.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/eskin.h>

class ePLiWindow : public eWindow
{
	public:
		ePLiWindow(const char *title, int width = 270, int helpWindowHeight = 50);
		~ePLiWindow();
		
		int yPos() { return yPosition; };
		int widgetHeight() { return fd; };
		void nextYPos(int increment = 30) { yPosition += increment; };
		void setYPos(int yPos = 10) { yPosition = yPos; };
		void buildWindow();
		void buildOKButton();
		void buildCancelButton();
		
		eButton *bOK;
		eButton *bCancel;
		
	private:
		eStatusBar *sStatusbar;
		int windowWidth;
		int yPosition;
		int fd;
		int helpWindowHeight;
};

#endif
