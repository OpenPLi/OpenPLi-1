/*
 * Ronald's setup plugin for dreambox
 * Copyright (c) 2004 Ronaldd <Ronaldd@sat4all.com>
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

#include <setup_runapp.h>

#define DEBUG 1

#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


#include <lib/base/i18n.h>

//#include <lib/gui/ewindow.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/eaudio.h>
#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/echeckbox.h>
#include <lib/gui/emessage.h>
#include <lib/gui/eskin.h>
#include <lib/driver/rc.h>
#include <lib/system/econfig.h>
#include <lib/gui/combobox.h>
#include <lib/gui/listbox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/textinput.h>
#include <lib/driver/rc.h>


RunApp::RunApp (const char* szExecute, const char* szMessage): eWindow(0), lState(0), bClose(0)
{
	eRCInput::getInstance()->lock();
	if (szExecute) {
		strncpy(m_szApplication, szExecute, 255);
	}
	if (szMessage) {
		strncpy(m_szMessage, szMessage, 127);
	}
	setText (eString ().sprintf ("%s", m_szApplication));
	cmove (ePoint (100, 100));
	cresize (eSize (550, 400));

	bClose = new eButton (this);
	bClose->setText (dgettext("plisetup", "Close"));
	bClose->setShortcut ("green");
	bClose->setShortcutPixmap ("green");
	bClose->move (ePoint (20, 320));
	bClose->resize (eSize (170, 40));
	bClose->loadDeco ();
	bClose->hide ();
	CONNECT (bClose->selected, RunApp::CloseApp);

	lState = new eLabel (this);
	lState->setName ("state");
	lState->move (ePoint (0, 0));
	lState->resize (eSize (630, 400));
}
//-----------------------------------------------------------------------------

RunApp::~RunApp()
{
	if (lState) delete lState;
	if (bClose) delete bClose;
}

int RunApp::eventHandler (const eWidgetEvent & e)
{
	FILE *file;
	char buf[1024];
	int i;

	switch (e.type)
	{
		case eWidgetEvent::execBegin:
		{
			bool fError(false);
			if (m_szApplication) {
				getData (eString ().sprintf (m_szMessage));
				file = popen (m_szApplication, "r");
				if (file) {
					while ((i = fread (buf, 1, 1023, file)) > 0) {
						buf[i] = 0;
						getData (eString ().sprintf ("%s", buf));
					}
					pclose (file);
				} else {
					fError = true;
				}
			} else {
				fError = true;
			}
			if (fError) {
				eMessageBox msg (dgettext("plisetup", "cannot execute script."), dgettext("plisetup", "cannot execute script."), eMessageBox::btOK | eMessageBox::iconError);
				msg.show ();
				msg.exec ();
				msg.hide ();
				eRCInput::getInstance()->unlock();
				close (-1);
			}
			//RunApp::appClosed (0);
			eRCInput::getInstance()->unlock();
			bClose->show ();
		}
		break;

		default:
			return eWindow::eventHandler (e);
	}
		return 1;
}
//-----------------------------------------------------------------------------

void RunApp::getData (eString str)
{
	// printf( "RunApp::getData\n");
	lState->setText (str);
}
//-----------------------------------------------------------------------------

void RunApp::onCancel ()
{
	close (1);
}
//-----------------------------------------------------------------------------

void RunApp::appClosed (int i)
{
	// printf( "RunApp::appClosed i=%i\n", i);
	if (i != 0) {
		lState->setText (dgettext("plisetup", "An error occured during exucution"));
	}
	bClose->show ();
}
//-----------------------------------------------------------------------------

void RunApp::CloseApp()
{
	close(0);
}
//-----------------------------------------------------------------------------

