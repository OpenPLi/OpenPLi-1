/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

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
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __window__
#define __window__

#include <string>

#include "component.h"


using namespace std;

class CWindow : public CComponent
{
	protected:
		int		iWindowMode;
		CPoint		cWindowOrigin;
		CPoint		cClientWindowOrigin;
		CDimension	cWindowDimension;
		CDimension	cClientWindowDimension;
		string		sWindowTitle;
		string		sWindowIcon;
		int		iTitleHeight;

		//intern zum berechnen der Fenstergrössen usw
		virtual void calculateWindow();

		//interne Paintfunktionen
		virtual void paintHeader();
		virtual void paintContent();

	public:

		enum windowMode
		{
			WINDOW_CENTER=1,
			WINDOW_NOTITLEBAR=2
		};


		CWindow(string title="", string icon="", CDimension clientWindowDimension = CDimension(200,60), 
				int mode = WINDOW_CENTER, CPoint windowPosition = CPoint(100,50));

		virtual ~CWindow();

		void setClientWindowDimension(CDimension cientWindowSize);
		void setWindowOrigin(CPoint windowPosition);

		void setWindowTitle(string title);
		void setWindowIcon(string icon);

		string getWindowTitle();
		string getWindowIcon();

		//Paint-Funktionen
		virtual void paint();
		virtual void hide();
};


#endif
