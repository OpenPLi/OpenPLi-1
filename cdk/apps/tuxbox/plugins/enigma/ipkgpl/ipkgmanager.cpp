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

#include "ipkgmanager.h"
#include <plugin.h>

extern "C" {
int plugin_exec( PluginParam *par );
};

const eString &eListBoxEntryIpkg::redraw(gPainter *rc, const eRect &rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state)
{
	gPixmap *pm1, *pm2;

	switch(check1)
	{
	case 1:
		pm1 = eSkin::getActive()->queryImage("shortcut.green");
		break;
	case 2:
		pm1 = eSkin::getActive()->queryImage("shortcut.blue");
		break;
	case 3:
		pm1 = eSkin::getActive()->queryImage("shortcut.yellow");
		break;
	default:
		pm1 = eSkin::getActive()->queryImage("shortcut.red");
		break;
	}

	pm2 = eSkin::getActive()->queryImage("eListBoxEntryCheck");

	drawEntryRect(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );

	int pm1XOffs=0;
	int pm2XOffs=0;
	int nameYOffs=0;
	int ypos=0;

	switch(menu)
	{
		case CIPKG::INST: case CIPKG::TEMP:
			pm2XOffs = pm2->x + 20;
			ypos = (rect.height() - pm2->y) / 2;
			if(check2)
				rc->blit( *pm2, ePoint( 10, rect.top()+ypos ), eRect(), gPixmap::blitAlphaTest);

			break;
		case CIPKG::AVAI:
			pm1XOffs = pm1->x + 20;
			ypos = (rect.height() - pm1->y) / 2;
			rc->blit( *pm1, ePoint(10, rect.top()+ypos ), eRect(), gPixmap::blitAlphaTest);
			pm2XOffs = pm2->x + 20;
			ypos = (rect.height() - pm2->y) / 2;
			if(check2)
				rc->blit( *pm2, ePoint( rect.left() + pm2XOffs, rect.top()+ypos ), eRect(), gPixmap::blitAlphaTest);

			break;
	}

	if (!namePara)
	{
		eString sname;
		if (name.length())
			sname=name;

		namePara = new eTextPara( eRect( 0, 0, rect.width()-pm1XOffs-pm2XOffs, rect.height() ) );
		namePara->setFont( eSkin::getActive()->queryFont("eServiceSelector.singleColumn.Entry.Name") );
		namePara->renderString( sname );
		if(sname == _("[GO UP]"))
			namePara->realign(eTextPara::dirCenter);

		nameYOffs = ((rect.height() - namePara->getBoundBox().height()) / 2 ) - namePara->getBoundBox().top();
	}
	rc->renderPara(*namePara, ePoint( rect.left() + pm1XOffs + pm2XOffs, rect.top() + nameYOffs) );

	return text;
}

eListBoxEntryIpkg::~eListBoxEntryIpkg()
{
	invalidate();
}
void eListBoxEntryIpkg::invalidate()
{
	if (namePara)
	{
		namePara->destroy();
		namePara=0;
	}
}

struct moveFirstChar
{
	char c;

	moveFirstChar(char c): c(c)
	{
	}

	bool operator()(const eListBoxEntryIpkg& s)
	{
		if (s.name[0] == c)
		{
			( (eListBox<eListBoxEntryIpkg>*) s.listbox)->setCurrent(&s);
			return 1;
		}
		return 0;
	}
};


IPKG::IPKG()
{
	addActionMap(&i_shortcutActions->map);
	menu=CIPKG::MAIN;
	commCont=0;

	cmove(ePoint(120, 100)); cresize(eSize(480, 380));

	t_liste=new eListBox<eListBoxEntryIpkg>(this);
	t_liste->move(ePoint(5, 5));
	t_liste->resize(eSize(clientrect.width()-10,clientrect.height()-90));
	//t_liste->loadDeco();
	CONNECT( t_liste->selected, IPKG::T_Listeselected);
	CONNECT( t_liste->selchanged, IPKG::T_Listeselchanged);

	l_a=new eLabel(this);
	l_a->setProperty("align","center");
	l_a->setProperty("vcenter","");

	l_b=new eLabel(this);
	l_b->setProperty("align","center");
	l_b->setProperty("vcenter","");

	l_c=new eLabel(this);
	l_c->setProperty("align","center");
	l_c->setProperty("vcenter","");

	l_d=new eLabel(this);
	l_d->setProperty("align","center");
	l_d->setProperty("vcenter","");

	status = new eStatusBar(this);
	status->move( ePoint(5, clientrect.height()-45) );status->resize( eSize( clientrect.width()-10, 45) );
	status->loadDeco();

	m_ipkg = new CIPKG;
	m_ipkg->mainPackages();
	m_ipkg->installedPackages();

	setLabel();
	liste_fill();
}

IPKG::~IPKG()
{
	delete m_ipkg;
	delete t_liste;

	if(FILE *f=fopen(IPKG_LOG_FILE,"r"))
	{
		fclose(f);
		unlink(IPKG_LOG_FILE);
	}

	delete commCont;
}

void IPKG::liste_fill()
{
	t_liste->beginAtomic();
	t_liste->clearList();
	int current = 0;

	switch(menu)
	{
		case CIPKG::MAIN:
			new eListBoxEntryIpkg(t_liste,m_ipkg->mainliste[0].name,0,0,0,0);
			for(MainList::iterator p=m_ipkg->mainliste.begin()+1; p!=m_ipkg->mainliste.end() ;p++)
				new eListBoxEntryIpkg(t_liste,(*p).name + " [Feed: " + (*p).feed + "]", menu, 0, 0, ++current);
			break;
		case CIPKG::INST:
			new eListBoxEntryIpkg(t_liste,m_ipkg->instliste[0].name,0,0,0,0);
			for(InstList::iterator p=m_ipkg->instliste.begin()+1; p!=m_ipkg->instliste.end() ;p++)
				new eListBoxEntryIpkg(t_liste, (*p).name, menu, 0, (*p).select, ++current);
			break;
		case CIPKG::AVAI:
			new eListBoxEntryIpkg(t_liste,m_ipkg->availiste[0].name,0,0,0,0);
			for(AvaiList::iterator p=m_ipkg->availiste.begin()+1; p!=m_ipkg->availiste.end() ;p++)
				new eListBoxEntryIpkg(t_liste, (*p).name, menu, (*p).stat, (*p).select, ++current);
			break;
		case CIPKG::TEMP:
			new eListBoxEntryIpkg(t_liste,m_ipkg->templiste[0].name,0,0,0,0);
			for(TempList::iterator p=m_ipkg->templiste.begin()+1; p!=m_ipkg->templiste.end() ;p++)
				new eListBoxEntryIpkg(t_liste, (*p).name, menu, 0, (*p).select, ++current);
			break;
	}

	t_liste->endAtomic();

	setStatus(0);
}

void IPKG::T_Listeselchanged(eListBoxEntryIpkg *item)
{
	if (item) setStatus(item->count);
}

void IPKG::T_Listeselected(eListBoxEntryIpkg *item)
{
	if(item)
	{
		int val = item->count;
		int old_menu = menu;

		switch(menu)
		{
			case CIPKG::MAIN:
				if(!val) menu = CIPKG::INST;
				else if (val==1)
				{
					m_ipkg->tempPackages();
					menu = CIPKG::TEMP;
				}
				else
				{
					t_liste->setHelpText("One moment please... Reading available packages");
					m_ipkg->availablePackages(m_ipkg->mainliste[val].feed);
					menu = CIPKG::AVAI;
				}
				break;
			case CIPKG::INST:
				if(!val) menu = CIPKG::MAIN;
				if(item->check2)
				{
					m_ipkg->instliste[val].select = 0;
					item->check2 = 0;
				}
				else
				{
					m_ipkg->instliste[val].select = 1;
					item->check2 = 1;
				}
				t_liste->invalidateCurrent();
				break;
			case CIPKG::AVAI:
				if(!val) menu = CIPKG::MAIN;
				if(item->check2)
				{
					m_ipkg->availiste[val].select = 0;
					item->check2 = 0;
				}
				else
				{
					m_ipkg->availiste[val].select = 1;
					item->check2 = 1;
				}
				t_liste->invalidateCurrent();
				break;
			case CIPKG::TEMP:
				if(!val) menu = CIPKG::MAIN;
				if(item->check2)
				{
					m_ipkg->templiste[val].select = 0;
					item->check2 = 0;
				}
				else
				{
					m_ipkg->templiste[val].select = 1;
					item->check2 = 1;
				}
				t_liste->invalidateCurrent();
				break;
		}

		if(old_menu != menu)
		{
			setLabel();
			liste_fill();
		}
	}
}

void IPKG::setStatus(int val)
{
	switch(menu)
	{
		case CIPKG::MAIN:
			if(!val)	t_liste->setHelpText("list all " + m_ipkg->mainliste[val].name);
			else		t_liste->setHelpText(m_ipkg->mainliste[val].path);
			break;
		case CIPKG::INST:
		case CIPKG::AVAI:
			if(!val)	t_liste->setHelpText(_("go up a directory"));
			else		t_liste->setHelpText(_("press ok for mark selected entry or press 0-9 for sort"));
			break;
		case CIPKG::TEMP:
			if(!val)	t_liste->setHelpText(_("go up a directory"));
			else		t_liste->setHelpText(_("press ok for mark selected entry"));
			break;
	}
}

void IPKG::setLabel()
{
	int fd=eSkin::getActive()->queryValue("fontsize", 20);
	l_a->hide();l_b->hide();l_c->hide();l_d->hide();

	switch(menu)
	{
		case CIPKG::MAIN:
			setText(_("PLi Package Management"));

			l_c->move(ePoint(5, clientrect.height()-90));
			l_c->resize(eSize(150, fd+10));
			l_c->setProperty("backgroundColor","std_dgreen");
			l_c->setText(_("Upgrade (PLi)"));
			l_c->show();
			
			l_a->move(ePoint(165, clientrect.height()-90));
			l_a->resize(eSize(150, fd+10));
			l_a->setProperty("backgroundColor","std_dyellow");
			l_a->setText(_("Setup"));
			l_a->show();

			l_b->move(ePoint(325, clientrect.height()-90));
			l_b->resize(eSize(150, fd+10));
			l_b->setProperty("backgroundColor","std_dblue");
			l_b->setText(_("Info"));
			l_b->show();

			break;
		case CIPKG::INST:
			setText(_("IPKG - installed Packages"));

			l_a->move(ePoint(5, clientrect.height()-90));
			l_a->resize(eSize(230, fd+10));
			l_a->setProperty("backgroundColor","std_dred");
			l_a->setText(_("Delete"));
			l_a->show();

			l_b->move(ePoint(245, clientrect.height()-90));
			l_b->resize(eSize(230, fd+10));
			l_b->setProperty("backgroundColor","std_dblue");
			l_b->setText(_("Info"));
			l_b->show();

			break;
		case CIPKG::AVAI:
			setText(eString(_("Packages from Feed: ")) + m_ipkg->availiste[0].feed);

			l_b->move(ePoint(120, clientrect.height()-90));
			l_b->resize(eSize(110, fd+10));
			l_b->setProperty("backgroundColor","std_dgreen");
			l_b->setText(_("Refresh"));
			l_b->show();

			if(m_ipkg->availiste.size()>1)
			{
				l_a->move(ePoint(5, clientrect.height()-90));
				l_a->resize(eSize(110, fd+10));
				l_a->setProperty("backgroundColor","std_dred");
				l_a->setText(_("Install"));
				l_a->show();

				l_c->move(ePoint(235, clientrect.height()-90));
				l_c->resize(eSize(110, fd+10));
				l_c->setProperty("backgroundColor","std_dyellow");
				l_c->setText(_("Upgrade"));
				l_c->show();

				l_d->move(ePoint(350, clientrect.height()-90));
				l_d->resize(eSize(125, fd+10));
				l_d->setProperty("backgroundColor","std_dblue");
				l_d->setText(_("Info"));
				l_d->show();
			}
			break;
		case CIPKG::TEMP:
			setText(_("Packages in TEMP Directory"));

			if(m_ipkg->templiste.size()>1)
			{
				l_a->move(ePoint(5, clientrect.height()-90));
				l_a->resize(eSize(200, fd+10));
				l_a->setProperty("backgroundColor","std_dgreen");
				l_a->setText(_("Install"));
				l_a->show();
			}
			break;
	}
}

int IPKG::eventHandler( const eWidgetEvent &e )
{
	switch( e.type )
	{
	case eWidgetEvent::evtAction:
		if ( e.action == &i_cursorActions->cancel)
		{
			if(commCont)	stop_cont(0);
			else		close(0);
		}
		else if ( e.action == &i_shortcutActions->blue)
		{
			if(menu == CIPKG::MAIN) //log
			{
				hide(); message dlg(m_ipkg, menu); dlg.show(); dlg.exec(); dlg.hide(); show();//Log
			}
			else //info
			{
				eListBoxEntryIpkg *safe = t_liste->getCurrent();
				if(safe->count)
				{
					hide(); message dlg(m_ipkg, menu, safe->count); dlg.show(); dlg.exec(); dlg.hide();show();
				}
			}
		}
		else if ( e.action == &i_shortcutActions->red)
		{
			switch(menu)
			{
				case CIPKG::INST: //remove
					m_ipkg->startremove();
					if(m_ipkg->commliste.size()) comm_exe();
					break;
				case CIPKG::AVAI: //install - selected
					m_ipkg->install_avai(false);
					if(m_ipkg->commliste.size()) comm_exe();
					break;
			}
		}
		else if ( e.action == &i_shortcutActions->green)
		{
			switch(menu)
			{
				case CIPKG::MAIN://quickupgrade
					quickupgrade();
					break;
				case CIPKG::AVAI://update
					hide();
					m_ipkg->startupdate();
					liste_fill();
					setLabel();
					show();
					break;
				case CIPKG::TEMP://install templiste
					m_ipkg->install_temp(false);
					if(m_ipkg->commliste.size()) comm_exe();
					break;
			}
		}
		else if ( e.action == &i_shortcutActions->yellow)
		{
			switch(menu)
			{
				case CIPKG::AVAI://upgrade
					m_ipkg->install_avai(true);
					if(m_ipkg->commliste.size()) comm_exe();
					break;
				case CIPKG::MAIN://setup
					hide(); ipkgsetup dlg(m_ipkg); dlg.show(); dlg.exec(); dlg.hide(); show();
					break;
			}
		}
		else if ( e.action == &i_shortcutActions->number0)
		{
			if(menu == CIPKG::INST || menu == CIPKG::AVAI)
			{
				m_ipkg->sort_list(menu);
				liste_fill();
			}
		}
		else if ( e.action == &i_shortcutActions->number2) gotoChar(2);
		else if ( e.action == &i_shortcutActions->number3) gotoChar(3);
		else if ( e.action == &i_shortcutActions->number4) gotoChar(4);
		else if ( e.action == &i_shortcutActions->number5) gotoChar(5);
		else if ( e.action == &i_shortcutActions->number6) gotoChar(6);
		else if ( e.action == &i_shortcutActions->number7) gotoChar(7);
		else if ( e.action == &i_shortcutActions->number8) gotoChar(8);
		else if ( e.action == &i_shortcutActions->number9) gotoChar(9);
		else if ( e.action == &i_cursorActions->help)
		{
			if(menu == CIPKG::INST || menu == CIPKG::AVAI)
			{
				hide(); message dlg(m_ipkg, menu, -1); dlg.show(); dlg.exec(); dlg.hide(); show();
			}
		}

		else break;

		return 1;
	default: break;
	}
	return eWindow::eventHandler(e);
}

void IPKG::comm_exe()
{
	CommList::iterator it = m_ipkg->commliste.begin();
	eDebug("[IPKG] %s",(*it).comm.c_str());
	commCont = new eConsoleAppContainer((*it).comm);

	eString msg_str;
	if(menu == CIPKG::INST)	msg_str = "remove package\n\"";
	else			msg_str = "install package\n\"";
	msg_str += (*it).file + "\"\nin progress...\n\n" + eString(_("One moment please..."));

	if ( !commCont->running() )
	{
		hide(); eMessageBox msg(_("sorry, couldn't find ipkg utility."),_("Error"),eMessageBox::btOK|eMessageBox::iconError);
		msg.show(); msg.exec(); msg.hide(); show();
	}
	else
	{
		CONNECT( commCont->dataAvail, IPKG::get_data );
		CONNECT( commCont->appClosed, IPKG::stop_cont );

		ipkg_msg = new eMessageBox(msg_str,_("Information"),eMessageBox::iconInfo);
		ipkg_msg->show();
	}
}

void IPKG::stop_cont(int)
{
	if (commCont) { delete commCont; commCont=0; }

	if(ipkg_msg)
	{
		ipkg_msg->hide();
		delete ipkg_msg;
		ipkg_msg=0;
	}

	CommList::iterator it = m_ipkg->commliste.begin();
	if(menu != CIPKG::INST && m_ipkg->m_ipkgExecDelete == 1) unlink((*it).file.c_str());
	m_ipkg->commliste.erase(it);

	if(m_ipkg->commliste.size())	comm_exe();
	else
	{
		if(menu != CIPKG::INST)
			if(m_ipkg->m_ipkgExecDestPath != "root")
				m_ipkg->linkPackage_no_root(); //set link

		m_ipkg->installedPackages();
		if(menu == CIPKG::TEMP) m_ipkg->tempPackages();
		else if(menu == CIPKG::AVAI) m_ipkg->availablePackages(m_ipkg->availiste[0].feed);
		liste_fill();
	}
}

void IPKG::get_data( eString str )
{
	str.removeChars('\x8');
	write_ipkg_log_file("%s",str.c_str());
}

void IPKG::gotoChar(char c)
{
	if(menu == CIPKG::INST || menu == CIPKG::AVAI)
	{
		//eDebug("[IPKG] gotoChar %d", c);
		switch(c)
		{
			case 2:// a,b,c
				if (BrowseChar >= 'a' && BrowseChar < 'c')
					BrowseChar++;
				else
					BrowseChar = 'a';
				break;
			case 3:// d,e,f
				if (BrowseChar >= 'd' && BrowseChar < 'f')
					BrowseChar++;
				else
					BrowseChar = 'd';
				break;
			case 4:// g,h,i
				if (BrowseChar >= 'g' && BrowseChar < 'i')
					BrowseChar++;
				else
					BrowseChar = 'g';
				break;
			case 5:// j,k,l
				if (BrowseChar >= 'j' && BrowseChar < 'l')
					BrowseChar++;
				else
					BrowseChar = 'j';
				break;
			case 6:// m,n,o
				if (BrowseChar >= 'm' && BrowseChar < 'o')
					BrowseChar++;
				else
					BrowseChar = 'm';
				break;
			case 7:// p,q,r,s
				if (BrowseChar >= 'p' && BrowseChar < 's')
					BrowseChar++;
				else
					BrowseChar = 'p';
				break;
			case 8:// t,u,v
				if (BrowseChar >= 't' && BrowseChar < 'v')
					BrowseChar++;
				else
					BrowseChar = 't';
				break;
			case 9:// w,x,y,z
				if (BrowseChar >= 'w' && BrowseChar < 'z')
					BrowseChar++;
				else
					BrowseChar = 'w';
				break;
		}

		if (BrowseChar != 0)
		{
			t_liste->beginAtomic();
			t_liste->forEachEntry(moveFirstChar(BrowseChar));
			t_liste->endAtomic();
		}
	}
}

void IPKG::quickupgrade()
{
	if(m_ipkg->availiste.size() < 2)
	{
		m_ipkg->availablePackages("official");
		m_ipkg->startupdate();
	}
	
	m_ipkg->install_avai(true);
	if(m_ipkg->commliste.size()) comm_exe();
}

int plugin_exec( PluginParam *par )
{
	IPKG dlg; dlg.show(); dlg.exec(); dlg.hide(); return 0;
}
