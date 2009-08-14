/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


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


#ifndef __setting_helpers__
#define __setting_helpers__

#include <gui/widget/menue.h>
#include <gui/widget/stringinput_ext.h>


unsigned long long getcurrenttime();

class CSatDiseqcNotifier : public CChangeObserver
{
	private:
		CMenuItem* satMenu;
		CMenuItem* extMenu;
		CMenuItem* repeatMenu;
	protected:
		CSatDiseqcNotifier( ) : CChangeObserver(){};  // prevent calling constructor without data we need
	public:
		CSatDiseqcNotifier( CMenuItem* SatMenu, CMenuItem* ExtMenu, CMenuItem* RepeatMenu) : CChangeObserver()
		{ satMenu = SatMenu; extMenu = ExtMenu; repeatMenu = RepeatMenu;};
		bool changeNotify(string OptionName, void*);
};

class CDHCPNotifier : public CChangeObserver
{
	private:
		int dhcppid;
		CMenuForwarder* toDisable[6];
	public:
		CDHCPNotifier( CMenuForwarder*, CMenuForwarder*, CMenuForwarder*, CMenuForwarder*, CMenuForwarder*, CMenuForwarder* );
		bool changeNotify(string OptionName, void*);
		void startStopDhcp();
};

class CRecordingNotifier : public CChangeObserver
{
	private:
		CMenuForwarder* toDisable;
	public:
	CRecordingNotifier( CMenuForwarder*);
	bool changeNotify(string OptionName, void*);
};
/*
class CCableSpectalInversionNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};
*/
class CLcdNotifier : public CChangeObserver
{
	private:
		int *LcdPowerSetting, *LcdInverseSetting;
	public:
		CLcdNotifier(int *lcdPowerSetting,int *lcdInverseSetting);
		bool changeNotify(string OptionName, void*);
};

class CStartNeutrinoDirectNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CColorSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CAudioSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CVideoSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CLanguageSetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CKeySetupNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CIPChangeNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CConsoleDestChangeNotifier : public CChangeObserver
{
	public:
		bool changeNotify(string OptionName, void*);
};

class CAPIDChangeExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, string actionKey);
};

void showSubchan(string subChannelName);
class CNVODChangeExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, string actionKey);
};

class CStreamFeaturesChangeExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, string actionKey);
};

class CUCodeCheckExec : public CMenuTarget
{
	public:
		int exec(CMenuTarget* parent, string actionKey);
};

void setDefaultGateway(char* ip);
void setNetworkAddress(char* ip, char* netmask, char* broadcast);
void setNameServer(char* ip);
void testNetworkSettings(char* ip, char* netmask, char* broadcast, char* gateway, char* nameserver, int dhcp);
void showCurrentNetworkSettings();


#endif
