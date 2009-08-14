/*
 * $Id: dreamflash.cpp,v 1.7 2007/07/26 21:34:09 pieterg Exp $
 *
 * (C) 2005 by mechatron, digi_casi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
 
#include "dreamflash.h"
#define INSTIMAGESUPPORT
#include <bootmenue/bmimage.h>
#include <bootmenue/bmconfig.h>
#include <bootmenue/bmboot.h>

#define VDF "Version 2.8"

//#define TMPFILE "/tmp/.dftemp"

extern "C" int plugin_exec(PluginParam *par);

int fd;
eListBox<eListBoxEntryText> *liste;
bmconfig cfg;

static std::string resolvSymlinks(const char *path)
{
	char buffer[128];
	std::string tmpPath;
	char *tok, *str, *org;
	str = org = strdup( path ? path : "");
	if (*str == '/') 
		str++;
	while (1)
	{
		tmpPath += '/';
		tok = strchr(str, '/');
		if (tok) 
			*tok = 0;
		tmpPath += str;
		while (1)
		{
			struct stat s;
			lstat(tmpPath.c_str(), &s);
			if (S_ISLNK(s.st_mode))
			{
				int count = readlink(tmpPath.c_str(), buffer, 255);
				if (buffer[0] == '/') 
					tmpPath.assign(buffer, count);
				else 
					tmpPath.replace(tmpPath.rfind('/') + 1, sizeof(str), std::string(buffer, count));
			}
			else 
				break;
		}
		if (tok) 
		{
			str = tok;
			str++;
		}
		else break;
	}
	free(org);
	return tmpPath;
}

bool ismounted(std::string mountpoint)
{
	char buffer[200+1], mountDev[100], mountOn[100], mountType[20];
	eString realPath = resolvSymlinks(mountpoint.c_str());
	bool mounted = false;
	if (FILE *mounts = fopen("/proc/mounts", "rt"))
	{
		while (fgets(buffer, 200, mounts))
		{
			mountDev[0] = mountOn[0] = mountType[0] = 0;
			sscanf(buffer, "%s %s %s ", mountDev, mountOn, mountType);
			if (realPath == mountOn)
				mounted = true;
		}
		fclose(mounts);
	}
	return mounted;
}

void msgerror(eString errorstring)
{
	eMessageBox msg(errorstring, _("Error"), eMessageBox::btOK|eMessageBox::iconError); 
	msg.show(); 
	msg.exec(); 
	msg.hide();
}

void msgok(eString okstring)
{
	eMessageBox msg(okstring, _("Information"), eMessageBox::btOK); 
	msg.show(); 
	msg.exec(); 
	msg.hide();
}

bool image_liste(int func)
{
	bool get = false;
	bmimages imgs;
	
	if (func)
	{
		imgs.load(cfg.mpoint, true);
	
		for (unsigned int i = 0; i < imgs.imageList.size(); i++)
		{
			eString name = imgs.imageList[i].name;
			eString location = imgs.imageList[i].location;
			new eListBoxEntryText(liste, name, (void *)new eString(location));
		}
		
		get = imgs.imageList.size() > 1;
	}
	else
	{
		if (DIR *p = opendir(cfg.mpoint.c_str()))
		{
			while (struct dirent *e = readdir(p))
			{
				eString name = cfg.mpoint + "/" + e->d_name;
				eString tmp = name; 
				tmp.upper();
				if (tmp.find(".IMG") != eString::npos)
				{
					new eListBoxEntryText(liste, e->d_name, (void *)new eString(name));
					get = true;
				}
			}
			closedir(p);
		}
	}
	return get;
}

setup_df::setup_df()
{
	cmove(ePoint(200, 120));
	cresize(eSize(330, 370));
	setText(_("Setup"));

	eLabel *a = new eLabel(this);
	a->move(ePoint(10, 15));
	a->resize(eSize(90, fd + 10));
	a->setText("Medium:");

	mliste = new eListBox<eListBoxEntryText>(this, a);
	mliste->move(ePoint(100, 10));
	mliste->resize(eSize(220, fd + 15));
	mliste->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	mliste->setHelpText(_("Press left or right to change"));
	mliste->loadDeco();

	eLabel *b = new eLabel(this);
	b->move(ePoint(10, 60));
	b->resize(eSize(260, fd + 10));
	b->setText("Startmenu timeout:");

	int timeout = atoi(cfg.timeoutValue.c_str());
	starttimer = new eNumber(this, 1, 1, 19, 2, &timeout, 0, b);
	starttimer->move(ePoint(270, 55));
	starttimer->resize(eSize(50, fd + 15));
	starttimer->setFlags(eNumber::flagDrawPoints);
	starttimer->setHelpText(_("Enter startmenu timeout in seconds"));
	starttimer->loadDeco();

	ch_inetd=new eCheckbox(this, atoi(cfg.randomSkin.c_str()));
	ch_inetd->move(ePoint(10, 100));
	ch_inetd->resize(eSize(clientrect.width() - 20, fd + 10));
	ch_inetd->setText("Select random skin");
	ch_inetd->setHelpText("Default is disabled");

	ed_skin_path = new eTextInputField(this);
	ed_skin_path->move(ePoint(10, clientrect.height() - 220));
	ed_skin_path->resize(eSize(clientrect.width() - 20, fd + 15));
	ed_skin_path->setHelpText("Press OK to enter the skin path");
	ed_skin_path->setEditHelpText("Enter path (Yellow a>A)");
	ed_skin_path->loadDeco();
	CONNECT(ed_skin_path->selected, setup_df::load_sliste);

	eLabel *c = new eLabel(this);
	c->move(ePoint(10, clientrect.height() - 170));
	c->resize(eSize(90, fd + 10));
	c->setText(_("Skins:"));

	sliste=new eListBox<eListBoxEntryText>(this,a);
	sliste->move(ePoint(100, clientrect.height() - 170));
	sliste->resize(eSize(220, fd + 15));
	sliste->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	sliste->setHelpText(_("Press left or right to change"));
	sliste->loadDeco();

	eButton *ok=new eButton(this);
	ok->move(ePoint((clientrect.width() / 2) - 75, clientrect.height() - 105));
	ok->resize(eSize(150, fd + 15));
	ok->loadDeco();
	ok->setText(_("Save"));
	ok->setHelpText(_("Save changes and return"));
	ok->hide();
	CONNECT(ok->selected, setup_df::okselected);

	status = new eStatusBar(this);
	status->move(ePoint(0, clientrect.height() - 50));
	status->resize(eSize(clientrect.width(), 50));
	status->loadDeco();

	if (ismounted("/media/usb")) 
	{
		new eListBoxEntryText(mliste, _("USB Stick"), (void *)new eString("/media/usb")); 
		ok->show(); 
	}
	if (ismounted("/media/cf"))  
	{
		new eListBoxEntryText(mliste, _("Compact Flash"), (void *)new eString("/media/cf")); 
		ok->show(); 
	}
	if (ismounted("/media/hdd"))
	{
		new eListBoxEntryText(mliste, _("Harddisk"), (void *)new eString("/media/hdd")); 
		ok->show(); 
	}
	mliste->setCurrent(0);

	ed_skin_path->setText(cfg.skinPath);
	load_sliste();
}

void setup_df::load_sliste()
{
	bmboot bmgr;
	eListBoxEntryText* selection = 0;
	sliste->beginAtomic(); 
	sliste->clearList();

	bmgr.getSkins(cfg.skinPath, cfg.mpoint);
	for (unsigned int i = 0; i < bmgr.skinList.size(); i++)
	{
		eString name = bmgr.skinList[i];
		unsigned int pos = name.find_last_of('/');
		name = name.right(name.length() - pos - 1);
		eString tmp = name.left(name.length() - 5);
		eListBoxEntryText *s = new eListBoxEntryText(sliste, tmp, (void *)new eString(name));
		if (cfg.skinName == name) 
			selection = s;
	}

	if (selection) 
		sliste->setCurrent(selection);

	sliste->endAtomic();
}

void setup_df::okselected()
{
	bmboot bmgr;
	bmgr.activateMenu("BM");

	cfg.mpoint = ((eString *)mliste->getCurrent()->getKey())->c_str();
	cfg.timeoutValue = eString().sprintf("%d", starttimer->getNumber());
	cfg.skinPath = ed_skin_path->getText();
	cfg.skinName = sliste->getCount() ? ((eString *)sliste->getCurrent()->getKey())->c_str() : "";
	cfg.randomSkin = ch_inetd->isChecked() ? "1" : "0";
	//video norm
	unsigned int colorformat;
	if (eConfig::getInstance()->getKey("/elitedvb/video/colorformat", colorformat)) 
		colorformat = 1;
	switch (colorformat)
	{
		case 1: cfg.videoFormat = "1"; break;
		case 2: cfg.videoFormat = "0"; break;
		case 3: cfg.videoFormat = "2"; break;
		case 4: cfg.videoFormat = "3"; break;
	}
	
	cfg.save();

	close(0);
}

info_df::info_df()
{
	cmove(ePoint(140, 100)); 
	cresize(eSize(440, 320)); 
	setText(_("Information"));

	eLabel *l = new eLabel(this);
	l->move(ePoint(10, 10));
	l->resize(eSize(clientrect.width() - 20, fd + 4));
	if (cfg.mpoint == "/media/usb")
		l->setText("USB Stick on \"media/usb\"");
	else 
	if (cfg.mpoint == "/media/cf")
		l->setText("Compact Flash on \"/media/cf\"");
	else 
	if (cfg.mpoint == "/media/hdd")
		l->setText("Harddisk on \"/media/hdd\"");
	else
		l->setText("No mount point available.");

	liste = new eListBox<eListBoxEntryText>(this);
	liste->move(ePoint(10, 45));
	liste->resize(eSize(clientrect.width() - 20, 170));
	image_liste(4);
	liste->loadDeco();
	CONNECT( liste->selchanged, info_df::Listeselchanged);

	x = new eLabel(this); 
	x->move(ePoint(10, clientrect.height() - 100));
	x->resize(eSize(clientrect.width() - 20, 100));
	ver = "DreamFlash "VDF;
	x->setText(ver);
}

void info_df::Listeselchanged(eListBoxEntryText *item)
{
	if (item)
	{
		eString loc = ((eString *) liste->getCurrent()->getKey())->c_str();
#if 0
		if (loc)
		{
			system(eString().sprintf("/bin/du -s \"%s\" > %s", loc.c_str(), TMPFILE).c_str());
			if (FILE *d = fopen(TMPFILE, "r"))
			{
				char duinfo[256];
				fgets(duinfo, 256, d);
				fclose(d);
				unlink(TMPFILE);

				eString duwert = duinfo; duwert = duwert.left(duwert.find('/') - 1);
				eString tmp = "Image-Size: " + duwert + " KB";
				x->setText(tmp);
			}
		}
		else 
#endif
			x->setText(ver);
	}
}

image_df::image_df(int was)
{
	was1 = was;

	struct statfs s;
	if (statfs(cfg.mpoint.c_str(), &s) >= 0) 
		free_space = (s.f_bavail * (s.f_bsize / 1024));

	cmove(ePoint(210, 150));

	switch (was)
	{
		case 0: cresize(eSize(300, 270)); setText(_("Add Image")); break;
		case 1: cresize(eSize(300, 270)); setText(_("Rename Image")); break;
		case 2: cresize(eSize(300, 200)); setText(_("Delete Image")); break;
	}

	liste=new eListBox<eListBoxEntryText>(this);
	liste->move(ePoint(10, 10));
	liste->resize(eSize(clientrect.width() - 20, fd + 15));
	liste->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	liste->setHelpText(_("Press left or right to change"));
	bool is=image_liste(was);
	liste->setCurrent(0);
	liste->loadDeco();
	CONNECT(liste->selchanged, image_df::Listeselchanged);

	eLabel *l=new eLabel(this);
	l->setText(_("Enter new name:"));
	l->move(ePoint(10, 80));
	l->resize(eSize(clientrect.width() - 20, fd + 4));

	Iname=new eTextInputField(this, l);
	Iname->move(ePoint(10, 110));
	Iname->resize(eSize(clientrect.width()-20, fd + 15));
	Iname->setMaxChars(15);
	//Iname->setUseableChars("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890-+:.");
	Iname->setHelpText(_("Press OK to enter name"));
	Iname->setEditHelpText("Enter name (Yellow a>A)");
	Iname->loadDeco();

	ok=new eButton(this);
	ok->move(ePoint((clientrect.width()-140)/2, clientrect.height() - 95));
	ok->resize(eSize(140, fd + 15));
	ok->setText(_("OK"));
	ok->loadDeco();
	CONNECT(ok->selected, image_df::oksel);

	status = new eStatusBar(this);
	status->move( ePoint(0, clientrect.height() - 45) );
	status->resize( eSize( clientrect.width(), 45) );
	status->loadDeco();

	if (was == 2) 
	{
		l->hide(); 
		Iname->hide();
	}

	if (!is) 
		ok->hide();
	else 
	{
		liste->goNext(); 
		liste->goPrev(); 
	}

	if (was)
		ok->setHelpText(_("Press OK to change"));
	else
		ok->setHelpText(eString().sprintf("Free memory is %d KB", free_space).c_str());
}

void image_df::Listeselchanged(eListBoxEntryText *item)
{
	if (item)
	{
		eString tmp = item->getText();
		if (!was1) 
			tmp = tmp.left(tmp.length() - 4);
		Iname->setText(tmp);
	}
	else
		close(0);
}

void image_df::oksel()
{
	eString loc = ((eString *) liste->getCurrent()->getKey())->c_str();
	eString namet = Iname->getText();
	eString path = cfg.mpoint + "/fwpro/" + namet;

	switch (was1)
	{
		case 0: // add
		{
			bmimages imgs;
			int rc = 0;
			
			eMessageBox box(_("Image installation is about to start...\nEnigma will shut down and restart automatically when installation is complete.\nPress YES to continue, NO to abort."), _("Install Image?"), eMessageBox::btYes|eMessageBox::btNo|eMessageBox::iconWarning, eMessageBox::btNo);
			box.show();
			int button = box.exec();
			box.hide();
			if (button == eMessageBox::btYes)
			{
				eMessageBox box2(_("Please wait while image is being installed..."), _("Image installation in progress"), eMessageBox::iconInfo);
				box2.show();
				rc = imgs.add(loc, namet, cfg.mpoint);
				box2.hide();
			}
			msgerror(eString("Error: " + eString().sprintf("%d", rc)));
			break;
		}
		case 1: // rename
		{
			if (loc != path)
			{
				if(system(eString().sprintf("mv \"%s\" \"%s\"", loc.c_str(), path.c_str()).c_str()) >> 8)
				{
					hide();
					msgerror("Rename Error!"); 
					show();
				}
				else 
				{ 
					hide(); 
					msgok(_("Done.")); 
					show(); 
				}
			}
			break;
		}
		case 2: // delete
		{
			if (system(eString().sprintf("rm -rf \"%s\"", loc.c_str()).c_str()) >> 8)
			{
				hide(); 
				msgerror("Delete Error!"); 
				show();
			}
			else 
			{ 
				hide(); 
				msgok(_("Done."));
				show(); 
			}
			break;
		}
	}
	close(0);
}

df_main::df_main()
{
	gPixmap *img = loadPNG("/tmp/flash.png");

	cmove(ePoint(200, 150));

	if (img)
	{
		cresize(eSize(280, 310));

		gPixmap *mp = &gFBDC::getInstance()->getPixmap();
		gPixmapDC mydc(img);
		gPainter p(mydc);
		p.mergePalette(*mp);

		eLabel *logo = new eLabel(this);
		logo->move(ePoint(70, clientrect.height() - 90));
		logo->resize(eSize(142, 75));
		logo->setName("logo");
		logo->setPixmap(img);
	}
	else cresize(eSize(280, 220));

	setText("Functions");

	textlist = new eListBox<eListBoxEntryText>(this);
	textlist->move(ePoint(10, 10));
	textlist->resize(eSize(clientrect.width() - 20, 200));
	textlist->setFlags(eListBoxBase::flagNoPageMovement);
	new eListBoxEntryText(textlist, _("Add Image"), (void *)0);
	new eListBoxEntryText(textlist, _("Rename Image"), (void *)1);
	new eListBoxEntryText(textlist, _("Delete Image"), (void *)2);
	new eListBoxEntryTextSeparator(textlist, eSkin::getActive()->queryImage("listbox.separator"), 0, true );
	new eListBoxEntryText(textlist, _("Setup"), (void *)3);
	new eListBoxEntryText(textlist, _("Information"), (void *)4);
	CONNECT(textlist->selected, df_main::Listeselected);
}

void df_main::Listeselected(eListBoxEntryText *item)
{
	if (item)
	{
		if ((int)item->getKey() < 3)
		{
			hide(); 
			image_df dlg((int)item->getKey());
			dlg.show();
			dlg.exec();
			dlg.hide();
			show();
		}
		else 
		if ((int)item->getKey() == 3)
		{
			hide(); 
			setup_df dlg; 
			dlg.show(); 
			dlg.exec(); 
			dlg.hide();
			show(); 
		}
		else
		{
			hide();
			info_df dlg;
			dlg.show();
			dlg.exec();
			dlg.hide();
			show();
		}
	}
	else
		close(0);
}

int plugin_exec(PluginParam *par)
{
	bmboot bmgr;
	fd = eSkin::getActive()->queryValue("fontsize", 20);

	static unsigned char logo[] = {
	#include "flash_pic.h"
	};
	int in = open ("/tmp/flash.png", O_CREAT | O_WRONLY);
	write(in, logo, sizeof(logo));
	close(in);

	bmgr.mountJFFS2();
	cfg.load();
	bmgr.unmountJFFS2();

	df_main dlg; 
	dlg.show(); 
	dlg.exec(); 
	dlg.hide();

	unlink("/tmp/flash.png");
	return 0;
}

