/*	Ipkgpl - Ipkg Enigma Plugin

	Copyright (C) 2005 'mechatron' (mechatron@gmx.net)

	Homepage: http://mechatron.6x.to/

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

#include "ipkgsetup.h"

static char * HELP_INST[]={
_("red Button:"),
_("remove selected packages"),
_("blue button:"),
_("show extended package information"),
"---"
};

static char * HELP_AVAI[]={
_("red button:"),
_("install selected packages"),
_("green button:"),
_("Update list of available packages"),
_("yellow button:"),
_("Upgrade all installed packages to latest version"),
_("blue button:"),
_("show extended package information"),
" ",
_("red dot:"),
_("not installed packages"),
_("green dot:"),
_("installed packages, version is current"),
_("yellow point:"),
_("installed packages, version is new"),
_("blue dot:"),
_("installed packages, version is old"),
"---"
};


message::message( CIPKG *m_ipkg, int menu , int val)
{
	cmove(ePoint(70, 140)); cresize(eSize(580, 300)); setText(_("information"));
	gFont fontsmall = eSkin::getActive()->queryFont("epg.title");
	eString str="";

	eListBox<eListBoxEntryText> *liste = new eListBox<eListBoxEntryText>(this);
	liste->move(ePoint(5, 5));
	liste->resize(eSize(clientrect.width()-10, clientrect.height()-10));

	eLabel *l = new eLabel(this);
	l->setFont(fontsmall);
	l->move(ePoint(5, 5));
	l->resize(eSize(clientrect.width()-10, clientrect.height()-10));
	l->hide();

	switch(menu)
	{
		case CIPKG::INST:
			if(val >=0)
			{
				str  = "Package: " + m_ipkg->instliste[val].name;
				str += "\nVersion: " + m_ipkg->instliste[val].version;
				str += "\nStatus: " + m_ipkg->instliste[val].status;
				str += "\nArchitecture: " + m_ipkg->instliste[val].arch;
				str += "\nProvides: " + m_ipkg->instliste[val].prov;
				str += "\nDepends: " + m_ipkg->instliste[val].depends;
				str += "\nPath: " +  m_ipkg->instliste[val].feed;
				if(m_ipkg->instliste[val].Time > 0)
				{
					char timestring[32]; *timestring=0;
					strftime(timestring, 32, "\nDate: %d.%m.%Y  Time: %H:%M", gmtime(&m_ipkg->instliste[val].Time));
					str += timestring;
				}
				l->setText(str);
				l->show();
			}
			else
			{
				bool doexit=false; int i=-1;
				while(!doexit)
				{
					if(HELP_INST[++i] == "---") doexit=true;
					else new eListBoxEntryText(liste,HELP_INST[i]);
				}
			}
			break;

		case CIPKG::AVAI:
			if(val >=0)
			{
				str  = "Package: " + m_ipkg->availiste[val].name;
				str += "\nDepends: " + m_ipkg->availiste[val].depends;
				str += "\nVersion: " + m_ipkg->availiste[val].version;
				str += "\nSection: " + m_ipkg->availiste[val].section;
				str += "\nArchitecture: " + m_ipkg->availiste[val].arch;
				str += "\nMaintainer: " + m_ipkg->availiste[val].maintainer;
				str += "\nMD5Sum: " + m_ipkg->availiste[val].md5_sum;
				str += eString().sprintf("\nSize: %d", m_ipkg->availiste[val].size).c_str();
				str += "\nFilename: " + m_ipkg->availiste[val].filename;
				str += "\nDescription: " + m_ipkg->availiste[val].desc;
				if(m_ipkg->availiste[val].stat > 1)
				{
					for(InstList::iterator p=m_ipkg->instliste.begin()+1; p!=m_ipkg->instliste.end() ;p++)
						if( m_ipkg->availiste[val].name == (*p).name)
							str += "\n\ninstalled Version: " + (*p).version;
				}
				l->setText(str);
				l->show();
			}
			else
			{
				bool doexit=false; int i=-1;
				while(!doexit)
				{
					if(HELP_AVAI[++i] == "---") doexit=true;
					else new eListBoxEntryText(liste,HELP_AVAI[i]);
				}
			}
			break;
		case CIPKG::MAIN:
			char line[1024]; *line=0;
			if(FILE *f=fopen(IPKG_LOG_FILE, "r"))
			{
				while((fgets(line,1024, f)!=NULL))
					new eListBoxEntryText(liste, line);
				fclose(f);
			}
			break;
	}
}


ipkgsetup::ipkgsetup(CIPKG *m_ipkg)
{
	int fd=eSkin::getActive()->queryValue("fontsize", 20);
	cmove(ePoint(140, 90)); cresize(eSize(440, 400)); setText(_("Setup"));
	ms_ipkg = m_ipkg;

	del_inst=new eCheckbox(this);
	del_inst->move(ePoint(5, clientrect.height()-390));
	del_inst->resize(eSize(clientrect.width()-10, fd+10));
	del_inst->setText(_("Remove after Install"));
	del_inst->setHelpText(_("enable or disable, deleted packages in Temp"));

	m_optForceDepends=new eCheckbox(this);
	m_optForceDepends->move(ePoint(5, clientrect.height()-350));
	m_optForceDepends->resize(eSize(clientrect.width()-10, fd+10));
	m_optForceDepends->setText(_("Force Depends"));
	m_optForceDepends->setHelpText(_("enable or disable the '-force-depends' option"));

	m_optForceReinstall=new eCheckbox(this);
	m_optForceReinstall->move(ePoint(5, clientrect.height()-310));
	m_optForceReinstall->resize(eSize(clientrect.width()-10, fd+10));
	m_optForceReinstall->setText(_("Force Reinstall"));
	m_optForceReinstall->setHelpText(_("enable or disable the '-force-reinstall' option"));

	m_optForceRemove=new eCheckbox(this);
	m_optForceRemove->move(ePoint(5, clientrect.height()-270));
	m_optForceRemove->resize(eSize(clientrect.width()-10, fd+10));
	m_optForceRemove->setText(_("Force Remove"));
	m_optForceRemove->setHelpText(_("\"Toto\"enable or disable the '-force-removal-of-dependent-packages' option"));

	m_optForceOverwrite=new eCheckbox(this);
	m_optForceOverwrite->move(ePoint(5, clientrect.height()-230));
	m_optForceOverwrite->resize(eSize(clientrect.width()-10, fd+10));
	m_optForceOverwrite->setText(_("Force Overwrite"));
	m_optForceOverwrite->setHelpText(_("enable or disable the '-force-overwrite' option"));

	eLabel *a = new eLabel(this);
	a->move(ePoint(5, clientrect.height()-175));
	a->resize(eSize(180, fd+10));
	a->setText(_("information level:"));

	m_optVerboseIpkg = new eComboBox(this, 3);
	m_optVerboseIpkg->move(ePoint(200, clientrect.height()-190));
	m_optVerboseIpkg->resize(eSize(230, fd+10));
	m_optVerboseIpkg->setHelpText(_("Select information level for Ipkg."));
	m_optVerboseIpkg->loadDeco();
	new eListBoxEntryText( *m_optVerboseIpkg,_("Errors only"),(void*)0);
	new eListBoxEntryText( *m_optVerboseIpkg,_("Normal"),(void*)1);
	new eListBoxEntryText( *m_optVerboseIpkg,_("Informative"),(void*)2);
	new eListBoxEntryText( *m_optVerboseIpkg,_("Troubleshooting"),(void*)3);

	eLabel *l = new eLabel(this);
	l->move(ePoint(5, clientrect.height()-145));
	l->resize(eSize(180, fd+10));
	l->setText(_("install packages to:"));

	c_dest = new eComboBox(this, 3);
	c_dest->move(ePoint(200, clientrect.height()-150));
	c_dest->resize(eSize(230, fd+10));
	c_dest->setHelpText(_("press ok to change"));
	c_dest->loadDeco();

	eButton *ok = new eButton(this);
	ok->move(ePoint((clientrect.width()/2)-90, clientrect.height()-95));
	ok->resize(eSize(180, fd+15));
	ok->loadDeco();
	ok->setShortcut("green");
	ok->setShortcutPixmap("green");
	ok->setText(_("save"));
	ok->setHelpText(_("press ok to save"));
	CONNECT(ok->selected, ipkgsetup::okselected);

	status = new eStatusBar(this);
	status->move( ePoint(5, clientrect.height()-45) ); status->resize( eSize( clientrect.width()-10, 45) );
	status->loadDeco();

	loaddata();
}
void ipkgsetup::loaddata()
{
	eListBoxEntryText *cur=0;
	eString dest_str = ms_ipkg->m_ipkgExecDestPath;

	for(ConfList::iterator p=ms_ipkg->confliste.begin(); p!=ms_ipkg->confliste.end() ;p++)
	{
		if((*p).type == CIPKG::DESTINATION)
		{
			eListBoxEntryText *entrys=new eListBoxEntryText( *c_dest, (*p).name);
			if((*p).name == dest_str) cur=entrys;
		}
	}
	c_dest->setCurrent(cur);

	if ( ms_ipkg->m_ipkgExecOptions & FORCE_DEPENDS )	m_optForceDepends->setCheck( true );
	if ( ms_ipkg->m_ipkgExecOptions & FORCE_REINSTALL )	m_optForceReinstall->setCheck( true );
	if ( ms_ipkg->m_ipkgExecOptions & FORCE_REMOVE )	m_optForceRemove->setCheck( true );
	if ( ms_ipkg->m_ipkgExecOptions & FORCE_OVERWRITE )	m_optForceOverwrite->setCheck( true );

	m_optVerboseIpkg->setCurrent(ms_ipkg->m_ipkgExecVerbosity);

	if ( ms_ipkg->m_ipkgExecDelete ) del_inst->setCheck( true );
}

void ipkgsetup::okselected()
{
	// Save options configuration
	int options = 0;
	int del = 0;
	if(m_optForceDepends->isChecked())	options |= FORCE_DEPENDS;
	if(m_optForceReinstall->isChecked())	options |= FORCE_REINSTALL;
	if(m_optForceRemove->isChecked())	options |= FORCE_REMOVE;
	if(m_optForceOverwrite->isChecked())	options |= FORCE_OVERWRITE;
	if(del_inst->isChecked()) del = 1;

	int verbosity = (int)m_optVerboseIpkg->getCurrent()->getKey();

	ms_ipkg->m_ipkgExecOptions = options;
	ms_ipkg->m_ipkgExecVerbosity = verbosity;
	ms_ipkg->m_ipkgExecDelete = del;
	ms_ipkg->m_ipkgExecDestPath = c_dest->getText();

	eConfig::getInstance()->setKey("/enigma/plugins/ipkg/ExecOptions", options);
	eConfig::getInstance()->setKey("/enigma/plugins/ipkg/Verbosity", verbosity);
	eConfig::getInstance()->setKey("/enigma/plugins/ipkg/Delete", del);

	close(0);
}

