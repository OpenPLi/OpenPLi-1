/*
 *  Inadyn configuration plugin 
 *  Original by SDT team
 *  Updated and made PLI specific by PLi team
 *
 *  Copyright (C) 2006 SDT team
 *  Copyright (C) 2007 dAF2000 <PLi@daf2000.nl>
 *  Copyright (C) 2007 PLi team <http://www.pli-images.org>
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

#include "ipdyn.h"

extern "C" int plugin_exec( PluginParam *par );

static void errorMessage(const eString message, int type=0)
{
	int flags;
	if(type==1)
	{
		flags = eMessageBox::iconInfo|eMessageBox::btOK;
	}
	else
	{	
		flags = eMessageBox::iconWarning|eMessageBox::btOK;
	}

	eMessageBox mb(message, _("Info"), flags);
	mb.show();
	mb.exec();
	mb.hide();
}

int plugin_exec( PluginParam *par )
{
	eInaDyn dlg;
	dlg.show();
	dlg.exec();
	dlg.hide();
	return 0;
}

eInaDyn::eInaDyn(): eWindow(1)
{
	int yPos = 10;
	int fd = eSkin::getActive ()->queryValue ("fontsize", 16) + 8;
	
	dd=getDomain();
	if(dd==-1)
	{
		dd=0;
	}

	setText(_("Inadyn configuration utility v1.1.0"));

	UU=new char[128];
	PP=new char[128];
	AA=new char[128];
	updatePeriodStr = new char[128];

	luser = new eLabel(this);
	luser->move(ePoint(10, yPos));
	luser->resize(eSize(90, fd));
	luser->setText(_("User: "));

	user = new eTextInputField(this,luser);
	user->move(ePoint(100, yPos));
	user->resize(eSize(280, fd));
	user->setUseableChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_@/");
	user->loadDeco();
	user->setHelpText(_("Enter username at dyndns or no-ip"));

	yPos += 35;
	lpass = new eLabel(this);
	lpass->move(ePoint(10, yPos));
	lpass->resize(eSize(90,fd));
	lpass->setText(_("Pass: "));

	pass = new eTextInputField(this,lpass);
	pass->move(ePoint(100, yPos));
	pass->resize(eSize(280, fd));
	pass->setUseableChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_@/");
	pass->loadDeco();
	pass->setHelpText(_("Enter password at dyndns or no-ip"));
	
	yPos += 35;
	ldomain = new eLabel(this);
	ldomain->move(ePoint(10, yPos));
	ldomain->resize(eSize(90,fd));
	ldomain->setText(_("Alias: "));

	domain = new eTextInputField(this,ldomain);
	domain->move(ePoint(100, yPos));
	domain->resize(eSize(280, fd));
	domain->setUseableChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-_@/");
	domain->loadDeco();
	domain->setHelpText(_("Enter redirect URL"));
	
	yPos += 35;
	mode_label=new eLabel(this);
	mode_label->setText(_("Domain :"));
	mode_label->move(ePoint(10, yPos));
	mode_label->resize(eSize(90,fd));
	
	mode=new eComboBox(this, 2, mode_label);
	mode->move(ePoint(100, yPos));
	mode->resize(eSize(280,fd));
	mode->loadDeco();
	mode->setHelpText(_("Select redirect service"));
	eListBoxEntryText* entrys[2];
	entrys[0]=new eListBoxEntryText(*mode, "dyndns.org", (void*)0);
	entrys[1]=new eListBoxEntryText(*mode, "no-ip.com", (void*)1);
	
	mode->setCurrent(entrys[dd]);
	
	CONNECT(mode->selchanged,eInaDyn::domainChanged);
	
	yPos += 35;
	l_update=new eLabel(this);
	l_update->setText(_("Update :"));
	l_update->move(ePoint(10, yPos));
	l_update->resize(eSize(90,fd));
	
	cb_update=new eComboBox(this, 4, l_update);
	cb_update->move(ePoint(100, yPos));
	cb_update->resize(eSize(280,fd));
	cb_update->loadDeco();
	cb_update->setHelpText(_("Select maximal update rate"));
	eListBoxEntryText* updateEntry[4];
	updateEntry[0]=new eListBoxEntryText(*cb_update, _("30 min"), (void*)(30*60));
	updateEntry[1]=new eListBoxEntryText(*cb_update, _("1 hour"), (void*)(60*60));
	updateEntry[2]=new eListBoxEntryText(*cb_update, _("12 hours"), (void*)(12*60*60));
	updateEntry[3]=new eListBoxEntryText(*cb_update, _("24 hours"), (void*)(24*60*60));
	
	yPos += 50;
	bt_ok=new eButton(this);
	bt_ok->move(ePoint(10, yPos)); 
	bt_ok->resize(eSize(120, 40));
	bt_ok->setShortcut("green");
	bt_ok->setShortcutPixmap("green");
	bt_ok->loadDeco();
	bt_ok->setText(_("Save"));
	bt_ok->setHelpText(_("Save settings and (re)start inadyn"));
		
	CONNECT(bt_ok->selected,eInaDyn::ok);
	
	bt_abort=new eButton(this);
	bt_abort->move(ePoint(260, yPos));
	bt_abort->resize(eSize(120, 40));
	bt_abort->setShortcut("red");
	bt_abort->setShortcutPixmap("red");
	bt_abort->loadDeco();
	bt_abort->setText(_("Cancel"));
	bt_abort->setHelpText(_("Discard changes and return"));

	CONNECT(bt_abort->selected, eWidget::reject);
		
	cresize(eSize(390, yPos + 100));
	valign();
	
	sStatusbar = new eStatusBar(this);
	sStatusbar->move(ePoint (0, clientrect.height() - 50));
	sStatusbar->resize(eSize(clientrect.width(), 50));
	sStatusbar->loadDeco();

	int r=loadData();
	if(r)
	{
		user->setText(UU);
		pass->setText(PP);
		domain->setText(AA);
		
		int updatePeriod = atoi(updatePeriodStr);
		if((updatePeriod != 30*60) &&
			(updatePeriod != 60*60) &&
			(updatePeriod != 12*60*60) &&
			(updatePeriod != 24*60*60))
		{
			updatePeriod = 30*60; // Default value
		}
		
		cb_update->setCurrent((void *)updatePeriod);
	}
			
	setFocus(user);
}


void eInaDyn::domainChanged(eListBoxEntryText *item)
{
	dd=(int)item->getKey();
}


void eInaDyn::ok()
{
	string m;
	bool status=true;
	eString opt;
	ofstream Debug;
	
	if(user->getText().length()==0 || pass->getText().length()==0)
	{
		errorMessage(_("Invalid or missing username or password"));
		return;
		status=false;
	}
	if(domain->getText().length()==0)
	{
		errorMessage(_("Invalid or missing domain"));
		return;
		status = false;
	}
	if(status)
	{
		eMessageBox *mmsg=new eMessageBox(
			_("Starting inadyn service...\nOriginal by SDT team\nChanged for PLi image by dAF2000"),
			_("Inadyn configuration utility"),eMessageBox::iconInfo);
		mmsg->show();
		Debug.open(CONFIG);
		system("killall -9 inadyn");
			if(dd==1)
			{
				opt.sprintf("--background --dyndns_system default@no-ip.com --username %s --password %s --update_period_sec %d --alias %s",
					user->getText().c_str(),
					pass->getText().c_str(),
					(int)cb_update->getCurrent()->getKey(),
					domain->getText().c_str());
				m="#1";
			}
			if(dd==0)
			{
				opt.sprintf("--background --dyndns_system dyndns@dyndns.org --username %s --password %s --update_period_sec %d --alias %s",
					user->getText().c_str(),
					pass->getText().c_str(),
					(int)cb_update->getCurrent()->getKey(),
					domain->getText().c_str());
				m="#0";	
			}
		Debug<<opt<<endl;
		Debug<<m<<endl;	
		Debug.close();
		system("/var/bin/inadyn --input_file /var/etc/inadyn.config");
		sleep(2);
		mmsg->hide();
	}
	
	eWidget::accept();
}

int eInaDyn::loadData()
{
	char *buffer;
	char *ptr;
	char *temp;
	ifstream tempfile;

	tempfile.open(CONFIG);
	if(!tempfile.good())
	{
		return 0;
	}

	temp= new char[512];
	buffer=new char[512];

	tempfile.getline(buffer,512,'\n');
	ptr=strstr(buffer,"username");
	strcpy(temp,ptr);
	ptr=strtok(temp," ");
	ptr=strtok(NULL," ");
	strcpy(UU,ptr);
	
	ptr=strstr(buffer,"password");
	strcpy(temp,ptr);
	ptr=strtok(temp," ");
	ptr=strtok(NULL," ");
	strcpy(PP,ptr);
	
	ptr=strstr(buffer,"update_period_sec");
	strcpy(temp,ptr);
	ptr=strtok(temp," ");
	ptr=strtok(NULL," ");
	strcpy(updatePeriodStr,ptr);
	
	ptr=strstr(buffer,"alias");
	strcpy(temp,ptr);
	ptr=strtok(temp," ");
	ptr=strtok(NULL," ");
	strcpy(AA,ptr);
	
	delete [] temp;
	delete [] buffer;
	tempfile.close();
	
	return 1;
}

int eInaDyn::getDomain()
{
	string line,tt;
	ifstream f1(CONFIG);
	if(f1.good())
	{
		getline(f1,line,'\n');
		getline(f1,line,'\n');
		if(line=="#0")
		{
			return 0;
		}
		if(line=="#1")
		{
			return 1;
		}
		else
		{
			return -1;
		}
	}
	return -1;
}

eInaDyn::~eInaDyn()
{
}
