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

#include <lib/gui/ePLiWindow.h>

ePLiWindow::ePLiWindow(const char *title, int width, int helpWindowHeight):
	eWindow(0), bOK(0), bCancel(0), helpWindowHeight(helpWindowHeight)
{
	windowWidth = width;
	if(windowWidth < 270)
	{
		windowWidth = 270;
	}
	
	yPosition = 10;
	fd = eSkin::getActive ()->queryValue ("fontsize", 16) + 8;
	setText(title);
}

ePLiWindow::~ePLiWindow()
{
}

void ePLiWindow::buildOKButton()
{
	yPosition+=50;
	
	bOK = new eButton(this);
	bOK->setText(_("Save"));
	bOK->setShortcut("green");
	bOK->setShortcutPixmap("green");
	bOK->move(ePoint (10, yPosition));
	bOK->resize(eSize (120, 40));
	bOK->setHelpText(_("Save changes and return"));
	bOK->loadDeco();
}

void ePLiWindow::buildCancelButton()
{
	bCancel = new eButton(this);
	bCancel->setText(_("Cancel"));
	bCancel->setShortcut("red");
	bCancel->setShortcutPixmap("red");
	bCancel->move(ePoint (windowWidth - 130, yPosition));
	bCancel->resize(eSize (120, 40));
	bCancel->setHelpText(_("Discard changes and return"));
	bCancel->loadDeco();
	CONNECT(bCancel->selected, eWidget::reject);
}

void ePLiWindow::buildWindow()
{
	if(bOK == 0) buildOKButton();
	if(bCancel == 0) buildCancelButton();

	cresize(eSize(windowWidth, yPosition + 50 + helpWindowHeight));
	valign();
	
	sStatusbar = new eStatusBar(this);
	sStatusbar->move(ePoint (0, clientrect.height() - helpWindowHeight));
	sStatusbar->resize(eSize(clientrect.width(), helpWindowHeight));
	sStatusbar->loadDeco();
}
