#ifndef DISABLE_HDD
#ifndef DISABLE_FILE
/*
 * setup_harddisk.cpp
 *
 * Copyright (C) 2002 Felix Domke <tmbinc@tuxbox.org>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * $Id: setup_harddisk.cpp,v 1.29 2009/04/28 06:43:06 rhabarber1848 Exp $
 */

#include <setup_harddisk.h>
#include <enigma.h>
#include <enigma_main.h>
#include <lib/gui/emessage.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/combobox.h>
#include <lib/gui/emessage.h>
#include <lib/gui/statusbar.h>
#include <lib/dvb/epgcache.h>
#include <lib/system/info.h>
#include <sys/vfs.h> // for statfs
#include <unistd.h>
#include <signal.h>
#include <lib/system/info.h>

#define MENUNAME N_("Harddisk")

class eHarddiskSetupFactory : public eCallableMenuFactory
{
public:
	eHarddiskSetupFactory() : eCallableMenuFactory("eHarddiskSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new eHarddiskSetup;
	}

	bool isAvailable()
	{
		return eSystemInfo::getInstance()->hasHDD();
	}
};

eHarddiskSetupFactory eHarddiskSetup_factory;

// Returns capacity in bytes, or -1 in case of an error
static long long getCapacity(int dev)
{
	FILE *f = fopen(eString().sprintf("/proc/ide/hd%c/capacity", 'a' + dev).c_str(), "r");
	if(!f)
	{
		return -1;
	}
		
	int capacity = -1;
	fscanf(f, "%d", &capacity);
	fclose(f);
	
	if(capacity != -1)
	{
		return (long long)capacity * 512LL;
	}
	else
	{
		return -1;
	}
}

static eString getCapacityString(long long cap)
{
	// Convert capacity to MB
	int capacity = cap / 1000000;
	eString retString;
	
	if(capacity >= 1000)
	{
		// Use GB notation
		if(capacity % 1000 >= 500)
		{
			// Prevent 1.999 GB for example gets printed as 1 GB
			capacity += 500;
		}
					
		retString.sprintf("%d GB", capacity / 1000);
	}
	else if(capacity > 0)
	{
		// Use MB notation (small CF cards)
		retString.sprintf("%d MB", capacity);
	}

	return retString;
}

static eString getModel(int dev)
{
	int c='a'+dev;
	char line[1024];

	FILE *f=fopen(eString().sprintf("/proc/ide/hd%c/model", c).c_str(), "r");
	if (!f)
		return "";
	*line=0;
	fgets(line, 1024, f);
	fclose(f);
	if (!*line)
		return "";
	line[strlen(line)-1]=0;
	return line;
}

int freeDiskspace(int dev, eString mp="")
{
	FILE *f=fopen("/proc/mounts", "rb");
	if (!f)
		return -1;
	eString path;
	int host=!!(dev&2);
	int bus=0;
	int target=!!(dev&1);
	path.sprintf("/dev/ide/host%d/bus%d/target%d/lun0/", host, bus, target);

	while (1)
	{
		char line[1024];
		if (!fgets(line, 1024, f))
			break;
		if (!strncmp(line, path.c_str(), path.size()))
		{
			eString mountpoint=line;
			mountpoint=mountpoint.mid(mountpoint.find(' ')+1);
			mountpoint=mountpoint.left(mountpoint.find(' '));
			//eDebug("mountpoint: %s", mountpoint.c_str());
			if ( mp && mountpoint != mp )
				return -1;
			struct statfs s;
			int free;
			if (statfs(mountpoint.c_str(), &s)<0)
				free=-1;
			else
				free=s.f_bfree/1000*s.f_bsize/1000;
			fclose(f);
			return free;
		}
	}
	fclose(f);
	return -1;
}

static int numPartitions(int dev)
{
	eString path;
	int host=!!(dev&2);
	int bus=0;
	int target=!!(dev&1);

	path.sprintf("ls /dev/ide/host%d/bus%d/target%d/lun0/ > /tmp/tmp.out", host, bus, target);
	system( path.c_str() );

	FILE *f=fopen("/tmp/tmp.out", "rb");
	if (!f)
	{
		eDebug("fopen failed");
		return -1;
	}

	int numpart=-1;		// account for "disc"
	while (1)
	{
		char line[1024];
		if (!fgets(line, 1024, f))
			break;
		if ( !strncmp(line, "disc", 4) )
			numpart++;
		if ( !strncmp(line, "part", 4) )
			numpart++;
	}
	fclose(f);
	system("rm /tmp/tmp.out");
	return numpart;
}

eString resolvSymlinks(const eString &path)
{
	char buffer[128];
	eString tmpPath;
	char *tok, *str, *org;
	str=org=strdup( path ? path.c_str() : "");
// simple string tokenizer
	if ( *str == '/' )
		str++;
	while(1)
	{
		tmpPath+='/';
		tok=strchr(str, '/');

		if ( tok )
			*tok=0;

		tmpPath+=str;

		while(1)
		{
			struct stat s;
			lstat(tmpPath.c_str(), &s);
			if (S_ISLNK(s.st_mode))						// is this a sym link ?
			{
				int count = readlink(tmpPath.c_str(), buffer, 255);
				if (buffer[0] == '/')			// is absolute path?
				{
					tmpPath.assign(buffer,count);	// this is our new path
//					eDebug("new realPath is %s", tmpPath.c_str());
				}
				else
				{
					// add resolved substr
					tmpPath.replace(
						tmpPath.rfind('/')+1,
						sizeof(str),
						eString(buffer,count));
//					eDebug("after add dest realPath is %s", tmpPath.c_str());
				}
			}
			else
				break;
		}
		if (tok)
		{
			str=tok;
			str++;
		}
		else
			break;
	}
//	eDebug("rp is %s", tmpPath.c_str() );
	free(org);											// we have used strdup.. must free

	return tmpPath;
}

eString getPartFS(int dev, eString mp="")
{
	FILE *f=fopen("/proc/mounts", "rb");
	if (!f)
		return "";
	eString path;
	int host=!!(dev&2);
	int bus=0;
	int target=!!(dev&1);
	path.sprintf("/dev/ide/host%d/bus%d/target%d/lun0/", host, bus, target);

	eString tmp=resolvSymlinks(mp);

	while (1)
	{
		char line[1024];
		if (!fgets(line, 1024, f))
			break;

		if (!strncmp(line, path.c_str(), path.size()))
		{
			eString mountpoint=line;
			mountpoint=mountpoint.mid(mountpoint.find(' ')+1);
			mountpoint=mountpoint.left(mountpoint.find(' '));
//			eDebug("mountpoint: %s", mountpoint.c_str());
			if ( tmp && mountpoint != tmp )
				continue;

			eString fs=line;
			fs=fs.mid(fs.find(' ')+1);
			fs=fs.mid(fs.find(' ')+1);
			fs=fs.left(fs.find(' '));
			eString mpath=line;
			mpath=mpath.left(mpath.find(' '));
			mpath=mpath.mid(mpath.rfind('/')+1);
			fclose(f);
			return fs+','+mpath;
		}
	}
	fclose(f);
	return "";
}

eHarddiskSetup::eHarddiskSetup()
	:eSetupWindow(_(MENUNAME), 2, 350)
{
	init_eHarddiskSetup();
}

void eHarddiskSetup::init_eHarddiskSetup()
{
	int entry = 0;
	valign();
	
	CONNECT((new eListBoxEntryMenu(&list, _("General settings"), eString().sprintf("(%d) %s", ++entry, 
		_("General settings for all internal harddisks, like sleep time")) ))->selected, eHarddiskSetup::generalSettings);
	CONNECT((new eListBoxEntryMenu(&list, _("Specific settings"), eString().sprintf("(%d) %s", ++entry, 
		_("Specific settings for a selectable harddisk, like formatting")) ))->selected, eHarddiskSetup::specificSettings);
}

void eHarddiskSetup::generalSettings()
{
	hide();
	eGeneralHarddiskSetup setup;
#ifndef DISABLE_LCD
	setup.setLCD(LCDTitle, LCDElement);
#endif
	setup.show();
	setup.exec();
	setup.hide();
	show();
}

void eHarddiskSetup::specificSettings()
{
	hide();
	eSpecificHarddiskSetup setup;
#ifndef DISABLE_LCD
	setup.setLCD(LCDTitle, LCDElement);
#endif
	if(!setup.getNr())
	{
		eMessageBox::ShowBox(_("sorry, no harddisks found!"), _("Specific harddisk settings..."));
	} 
	else
	{
		setup.show();
		setup.exec();
		setup.hide();
	}
	show();
}

void eHarddiskSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

eGeneralHarddiskSetup::eGeneralHarddiskSetup()
	:ePLiWindow(_("General harddisk settings"), 530),
	iHDSleepTime(60), iHDAcoustics(128),
	hd_sleeptime_label(0), hd_sleeptime(0),
	hd_acoustics_label(0), hd_acoustics(0)
{
	eConfig::getInstance()->getKey("/extras/hdparm-s", iHDSleepTime);
	if (iHDSleepTime <= 240)
		iHDSleepTime /= 12;
	else if (iHDSleepTime == 252)
		iHDSleepTime = 21;
	else if (iHDSleepTime <= 251)
		iHDSleepTime = (iHDSleepTime - 240) * 30;
		
	eConfig::getInstance()->getKey("/extras/hdparm-m", iHDAcoustics);

	hd_sleeptime_label = new eLabel (this);
	hd_sleeptime_label->setText (_("Harddisk sleep time"));
	hd_sleeptime_label->move (ePoint (10, yPos()));
	hd_sleeptime_label->resize (eSize (250, widgetHeight()));

	hd_sleeptime = new eNumber (this, 1, 0, 330, 3, &iHDSleepTime, 0, hd_sleeptime_label);
	hd_sleeptime->move (ePoint (210, yPos()));
	hd_sleeptime->resize (eSize (50, widgetHeight()));
	hd_sleeptime->setHelpText (_("Number of minutes after the harddisk will spin down. 0 to 330 minutes. 0 is no spin down"));
	hd_sleeptime->loadDeco ();

	hd_acoustics_label = new eLabel (this);
	hd_acoustics_label->setText (_("Harddisk acoustics"));
	hd_acoustics_label->move (ePoint (270, yPos()));
	hd_acoustics_label->resize (eSize (250, widgetHeight()));

	hd_acoustics = new eNumber (this, 1, 0, 254, 128, &iHDAcoustics, 0, hd_acoustics_label);
	hd_acoustics->move (ePoint (470, yPos()));
	hd_acoustics->resize (eSize (50, widgetHeight()));
	hd_acoustics->setHelpText (_("Sets the acoustics of the drive, 128=quiet/slow 254=fast/loud"));
	hd_acoustics->loadDeco ();

	buildWindow();
	CONNECT (bOK->selected, eGeneralHarddiskSetup::okPressed);
}

void eGeneralHarddiskSetup::okPressed()
{
	if (hd_sleeptime && hd_acoustics)
	{
		int iAccoustics = hd_acoustics->getNumber();
		if (iAccoustics < 128) iAccoustics = 128;
		int isleeptime = hd_sleeptime->getNumber();
		// Calculate time to sleeptime byte code. 
		// round to first lower time step. E.g. 22 to 29 minutues maps to 21 minutes
		// 31 to 59 minutes map to 30 minutes. etc.
		if (isleeptime <= 20)
		{
			isleeptime *= 12; // range from 5 secs to 20 minutes.
		}
		else if (isleeptime <= 29)
		{
			isleeptime = 252; // 21 minutes
		}
		else if (isleeptime <= 330)
		{
			// 30 to 330 minutes
			isleeptime /= 30; // scale to stepsize
			isleeptime += 240; // bring in range
		}
		else 
			isleeptime = 251;  // max value
			
		eConfig::getInstance()->setKey("/extras/hdparm-s", isleeptime);
		eConfig::getInstance()->setKey("/extras/hdparm-m", iAccoustics);
		eString cmd;
		cmd.sprintf("hdparm -S %d -M %d /dev/ide/host0/bus0/target0/lun0/disc", isleeptime, hd_acoustics->getNumber());
		system(cmd.c_str());
		// if second channel exist there is a second HDD active, also set sleep time and acoustics
		if (access("/proc/ide/hdb", F_OK) == 0)
		{
			cmd.sprintf("hdparm -S %d -M %d /dev/ide/host0/bus0/target1/lun0/disc", isleeptime, hd_acoustics->getNumber());
			system(cmd.c_str());
		}
	}
	
	close(0);
}

eSpecificHarddiskSetup::eSpecificHarddiskSetup()
	:eListBoxWindow<eListBoxEntryText>(_("Select your harddisk"), 5, 420)
{
	nr=0;
	valign();
	
	for (int host=0; host<2; host++)
		for (int target=0; target<2; target++)
		{
			int num=target+host*2;

			int c='a'+num;

			// check for presence
			char line[1024];
			int ok=1;
			FILE *f=fopen(eString().sprintf("/proc/ide/hd%c/media", c).c_str(), "r");
			if (!f)
				continue;
			if ((!fgets(line, 1024, f)) || strcmp(line, "disk\n"))
				ok=0;
			fclose(f);

			if (ok)
			{
				long long capacity = getCapacity(num);
				if (capacity < 0)
					continue;

				eString sharddisks;
				sharddisks=getModel(num);
				sharddisks+=" (";
				if (c&1)
					sharddisks+="master, ";
				else
					sharddisks+="slave, ";

				sharddisks += getCapacityString(capacity);
				sharddisks += ")";
				
				nr++;
				
				new eListBoxEntryText(&list, sharddisks, (void*)num);
			}
	}
	
	CONNECT(list.selected, eSpecificHarddiskSetup::selectedHarddisk);
}

void eSpecificHarddiskSetup::selectedHarddisk(eListBoxEntryText *t)
{
	if ((!t) || (((int)t->getKey())==-1))
	{
		close(0);
		return;
	}
	int dev=(int)t->getKey();
	
	eHarddiskMenu menu(dev);
	
	hide();
	menu.show();
	menu.exec();
	menu.hide();
	show();
}

void eHarddiskMenu::check()
{
	hide();
	ePartitionCheck check(dev);
	check.show();
	check.exec();
	check.hide();
	show();
	restartNet=true;
}

void eHarddiskMenu::extPressed()
{
	if ( visible )
	{
		gPixmap *pm = eSkin::getActive()->queryImage("arrow_down");
		if (pm)
			ext->setPixmap( pm );
		fs->hide();
		sbar->hide();
		resize( getSize()-eSize( 0, 45) );
		sbar->move( sbar->getPosition()-ePoint(0,45) );
		sbar->show();
		eZap::getInstance()->getDesktop(eZap::desktopFB)->invalidate( eRect( getAbsolutePosition()+ePoint( 0, height() ), eSize( width(), 45 ) ));
		visible=0;
	}
	else
	{
		gPixmap *pm = eSkin::getActive()->queryImage("arrow_up");
		if (pm)
			ext->setPixmap( pm );
		sbar->hide();
		sbar->move( sbar->getPosition()+ePoint(0,45) );
		resize( getSize()+eSize( 0, 45) );
		sbar->show();
		fs->show();
		visible=1;
	}
}

void eHarddiskMenu::s_format()
{
	eString errMsg = "";
	hide();
	do
	{
		{
			int res = eMessageBox::ShowBox(
				 _("Are you SURE that you want to format this disk?\n"),
				 _("formatting harddisk..."),
				 eMessageBox::btYes|eMessageBox::btCancel, eMessageBox::btCancel);
			if (res != eMessageBox::btYes)
				break;
		}
		if (numpart)
		{
			int res = eMessageBox::ShowBox(
				 _("There's data on this harddisk.\n"
				 "You will lose that data. Proceed?"),
				 _("formatting harddisk..."),
				 eMessageBox::btYes|eMessageBox::btNo, eMessageBox::btNo);
			if (res != eMessageBox::btYes)
				break;
		}
		int host=!!(dev&2);
		int bus=0;
		int target=!!(dev&1);


		eDebug("HDDformat: killing samba and umount disk partitions");
// kill samba server... (exporting /media/hdd)
#ifdef HAVE_DREAMBOX_HARDWARE
		system("killall -9 smbd");
#else
		system("killall -9 smbd nmbd");
#endif
		// Unmount /media/hdd
		// For some strange reason unmounting /dev/ide/... or
		// /dev/discs/... causes an "Invalid argument".
		system("/bin/umount /media/hdd");
		
		restartNet=true;

		system(eString().sprintf(
#ifdef HAVE_DREAMBOX_HARDWARE
				"/bin/umount /dev/ide/host%d/bus%d/target%d/lun0/part*"
#else
				"/sbin/swapoff /dev/ide/host%d/bus%d/target%d/lun0/part1"
#endif
				, host, bus, target).c_str());

		eMessageBox::ShowBox(
			_("Please wait while formatting the harddisk.\nThis might take some minutes.\n"),
			_("formatting harddisk..."), 0);

		eDebug("HDDformat: running sfdisk to partition the disk");
		FILE *f=popen(eString().sprintf(
#ifdef HAVE_DREAMBOX_HARDWARE
				"/sbin/sfdisk -f /dev/ide/host%d/bus%d/target%d/lun0/disc"
#else
				"/sbin/sfdisk -f -uM /dev/ide/host%d/bus%d/target%d/lun0/disc"
#endif
				, host, bus, target).c_str(), "w");
		if (!f)
		{
			eDebug("HDDformat: sfdisk not found");
			eMessageBox::ShowBox(
				_("sorry, couldn't find sfdisk utility to partition harddisk."),
				_("formatting harddisk..."),
				 eMessageBox::btOK|eMessageBox::iconError);
			break;
		}
#ifdef HAVE_DREAMBOX_HARDWARE
		fprintf(f, "0,\n;\n;\n;\ny\n");
#else
		fprintf(f, "0,100,82\n,,,*\n;\n;\ny\n");
#endif
		fclose(f);
/*Set up Swapspace*/
#ifdef HAVE_DBOX_HARDWARE
		system(eString().sprintf("/sbin/mkswap /dev/ide/host%d/bus%d/target%d/lun0/part1", host, bus, target).c_str());
#endif

		int rc;
		::sync();
#if ENABLE_REISERFS
		if ( !fs->getCurrent()->getKey() )  // reiserfs
		{
			eDebug("HDDformat: create reiserfs filesystem");
			rc = system( eString().sprintf(
#ifdef HAVE_DREAMBOX_HARDWARE
					"/sbin/mkreiserfs -f -f /dev/ide/host%d/bus%d/target%d/lun0/part1"
#else
					"/sbin/mkreiserfs -f -f /dev/ide/host%d/bus%d/target%d/lun0/part2"
#endif
					, host, bus, target).c_str());
		}
		else  // ext3
#endif
		{
			eString extraParm;
			// if size of disc larger then 2Gb use largefile support, not very funny on a small disc like a CF card :-)
			if (getCapacity(dev) > 2 * 1024 * 1024 * 1024)
			{
				extraParm = "-T largefile ";	
			}
			eDebug("HDDformat: create ext3 filesystem");
			rc = system( eString().sprintf(
#ifdef HAVE_DREAMBOX_HARDWARE
				"/sbin/mkfs.ext3 %s-m0 /dev/ide/host%d/bus%d/target%d/lun0/part1"
#else
				"/sbin/mkfs.ext3 %s-m0 /dev/ide/host%d/bus%d/target%d/lun0/part2"
#endif
				, extraParm.c_str(), host, bus, target).c_str());
		}

		if (rc >> 8)
		{
			errMsg.sprintf("%s (rc=%d)", _("creating filesystem failed"), rc >> 8);
			goto err;
		}
		::sync();

		eDebug("HDDformat: mount filesystem");
		switch( eSystemInfo::getInstance()->getHwType() )
		{
			case eSystemInfo::DM7020:
			case eSystemInfo::DM600PVR:
				// Only try to mount the just formatted partition from fstab
				rc = system(eString().sprintf("mount /dev/ide/host%d/bus%d/target%d/lun0/part1", host, bus, target).c_str());
				break;
			default:
				rc = system("/bin/mount_hdd_cf.sh");
		}
		if (rc >> 8)
		{
			errMsg.sprintf("%s (rc=%d)", _("mounting harddisk failed"), rc >> 8);
			goto err;
		}
		::sync();
		eDebug("HDDformat: create /media/hdd/movie");
		rc = system("mkdir /media/hdd/movie");
		if (rc >> 8)
		{
			errMsg.sprintf("%s (rc=%d)", _("creating /media/hdd/movie failed"), rc >> 8);
			goto err;
		}
		::sync();
		goto noerr;

err:
		{
			eMessageBox::ShowBox(
				errMsg,
				_("formatting harddisk..."),
				 eMessageBox::btOK|eMessageBox::iconError);
			break;
		}

noerr:
		{
			eZapMain::getInstance()->clearRecordings();
			eMessageBox::ShowBox(
				_("successfully formatted your disk!"),
				_("formatting harddisk..."),
				 eMessageBox::btOK|eMessageBox::iconInfo);
		}
		readStatus();
	} while (0);
	show();
}

void eHarddiskMenu::readStatus()
{
	if (!(dev & 1))
		bus->setText("master");
	else
		bus->setText("slave");

	eString mod=getModel(dev);
	setText(mod);
	model->setText(mod);
	long long cap = getCapacity(dev);
	
	if (cap != -1)
		capacity->setText(getCapacityString(cap));
		
	numpart=numPartitions(dev);
	int fds;
	
	if (numpart == -1)
		status->setText(_("(error reading information)"));
	else if (!numpart)
		status->setText(_("uninitialized - format it to use!"));
	else if ((fds=freeDiskspace(dev)) != -1)
		status->setText(eString().sprintf(_("in use, %d.%04d GB (about %d minutes) free"), fds/1024, fds%1024, fds/33 ));
	else
		status->setText(_("initialized, but unknown filesystem"));
}

eHarddiskMenu::eHarddiskMenu(int dev): dev(dev), restartNet(false)
{
	init_eHarddiskMenu();
}
void eHarddiskMenu::init_eHarddiskMenu()
{
	visible=0;
	status=new eLabel(this); status->setName("status");
	model=new eLabel(this); model->setName("model");
	capacity=new eLabel(this); capacity->setName("capacity");
	bus=new eLabel(this); bus->setName("bus");
	
	format=new eButton(this); format->setName("format");
	bcheck=new eButton(this); bcheck->setName("check");
	ext=new eButton(this); ext->setName("ext");

	fs=new eComboBox(this,2); fs->setName("fs"); fs->hide();

	sbar = new eStatusBar(this); sbar->setName("statusbar");

	new eListBoxEntryText( *fs, ("ext3"), (void*) 1 );
#ifdef ENABLE_REISERFS
	new eListBoxEntryText( *fs, ("reiserfs"), (void*) 0 );
#endif
	fs->setCurrent((void*)1);
  
	if (eSkin::getActive()->build(this, "eHarddiskMenu"))
		eFatal("skin load of \"eHarddiskMenu\" failed");

	gPixmap *pm = eSkin::getActive()->queryImage("arrow_down");
	if (pm)
	{
		eSize s = ext->getSize();
		ext->setPixmap( pm );
		ext->setPixmapPosition( ePoint(s.width()/2 - pm->x/2, s.height()/2 - pm->y/2) );
	}

	readStatus();

	CONNECT(ext->selected, eHarddiskMenu::extPressed);
	CONNECT(format->selected, eHarddiskMenu::s_format);
	CONNECT(bcheck->selected, eHarddiskMenu::check);
}

ePartitionCheck::ePartitionCheck( int dev )
:eWindow(1), dev(dev), fsck(0)
{
	init_ePartitionCheck();
}
void ePartitionCheck::init_ePartitionCheck()
{
	lState = new eLabel(this);
	lState->setName("state");
	bClose = new eButton(this);
	bClose->setName("close");
	CONNECT( bClose->selected, ePartitionCheck::accept );
	if (eSkin::getActive()->build(this, "ePartitionCheck"))
		eFatal("skin load of \"ePartitionCheck\" failed");
	bClose->hide();
}

int ePartitionCheck::eventHandler( const eWidgetEvent &e )
{
	switch(e.type)
	{
		case eWidgetEvent::execBegin:
		{
			system("killall nmbd smbd");
			eEPGCache::getInstance()->messages.send(eEPGCache::Message(eEPGCache::Message::pause));
			eEPGCache::getInstance()->messages.send(eEPGCache::Message(eEPGCache::Message::flush));
			eString fs = getPartFS(dev,"/media/hdd"),
							part = fs.mid( fs.find(",")+1 );

			fs = fs.left( fs.find(",") );

			eDebug("part = %s, fs = %s", part.c_str(), fs.c_str() );

			int host=!!(dev&2);
			int bus=0;
			int target=!!(dev&1);

			// kill samba server... (exporting /media/hdd)
			system("killall -9 smbd");

			if ( system("/bin/umount /media/hdd") >> 8)
			{
				eMessageBox::ShowBox(
				_("could not unmount the filesystem... "),
				_("check filesystem..."),
				 eMessageBox::btOK|eMessageBox::iconError);
				close(-1);
			}
			if ( fs == "ext3" )
			{
				eWindow::globalCancel(eWindow::OFF);
				fsck = new eConsoleAppContainer( eString().sprintf("/sbin/fsck.ext3 -f -y /dev/ide/host%d/bus%d/target%d/lun0/%s", host, bus, target, part.c_str()) );

				if ( !fsck->running() )
				{
					eMessageBox::ShowBox(
						_("sorry, couldn't find fsck.ext3 utility to check the ext3 filesystem."),
						_("check filesystem..."),
						eMessageBox::btOK|eMessageBox::iconError);
					close(-1);
				}
				else
				{
					eDebug("fsck.ext3 opened");
					CONNECT( fsck->dataAvail, ePartitionCheck::getData );
					CONNECT( fsck->appClosed, ePartitionCheck::fsckClosed );
				}
			}
			else if ( fs == "ext2" )
			{
				eWindow::globalCancel(eWindow::OFF);
				fsck = new eConsoleAppContainer( eString().sprintf("/sbin/fsck.ext2 -f -y /dev/ide/host%d/bus%d/target%d/lun0/%s", host, bus, target, part.c_str()) );

				if ( !fsck->running() )
				{
					eMessageBox::ShowBox(
						_("sorry, couldn't find fsck.ext2 utility to check the ext2 filesystem."),
						_("check filesystem..."),
						eMessageBox::btOK|eMessageBox::iconError);
					close(-1);
				}
				else
				{
					eDebug("fsck.ext2 opened");
					CONNECT( fsck->dataAvail, ePartitionCheck::getData );
					CONNECT( fsck->appClosed, ePartitionCheck::fsckClosed );
				}
			}
			else if ( fs == "reiserfs" && eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000 )
			{
				eWindow::globalCancel(eWindow::OFF);
				fsck = new eConsoleAppContainer( eString().sprintf("/sbin/reiserfsck -y --fix-fixable /dev/ide/host%d/bus%d/target%d/lun0/%s", host, bus, target, part.c_str()) );

				if ( !fsck->running() )
				{
					eMessageBox::ShowBox(
						_("sorry, couldn't find reiserfsck utility to check the reiserfs filesystem."),
						_("check filesystem..."),
						eMessageBox::btOK|eMessageBox::iconError);
					close(-1);
				}
				else
				{
					eDebug("reiserfsck opened");
					CONNECT( fsck->dataAvail, ePartitionCheck::getData );
					CONNECT( fsck->appClosed, ePartitionCheck::fsckClosed );
					fsck->write("Yes\n",4);
				}
			}
			else
			{
				eMessageBox::ShowBox(
					_("not supported filesystem for check."),
					_("check filesystem..."),
					eMessageBox::btOK|eMessageBox::iconError);
				close(-1);
			}
		}
		break;

		case eWidgetEvent::execDone:
			eWindow::globalCancel(eWindow::ON);
			if (fsck)
				delete fsck;
			eEPGCache::getInstance()->messages.send(eEPGCache::Message(eEPGCache::Message::restart));
			eDVB::getInstance()->restartSamba();
		break;

		default:
			return eWindow::eventHandler( e );
	}
	return 1;	
}

void ePartitionCheck::onCancel()
{
	if (fsck)
		fsck->kill();
}

void ePartitionCheck::fsckClosed(int state)
{
	int host=!!(dev&2);
	int bus=0;
	int target=!!(dev&1);

#ifdef HAVE_DREAMBOX_HARDWARE
	if ( system( eString().sprintf("/bin/mount /dev/ide/host%d/bus%d/target%d/lun0/part1 /media/hdd", host, bus, target).c_str() ) >> 8 )
		eDebug("mount hdd after check failed");
#else
	if ( system( eString().sprintf("/bin/mount /dev/ide/host%d/bus%d/target%d/lun0/part2 /media/hdd", host, bus, target).c_str() ) >> 8 )
		eDebug("mount hdd after check failed");
#endif

	if (fsck)
	{
		delete fsck;
		fsck=0;
	}

	bClose->show();
}

void ePartitionCheck::getData( eString str )
{
	str.removeChars('\x8');
	if ( str.find("<y>") != eString::npos )
		fsck->write("y",1);
	else if ( str.find("[N/Yes]") != eString::npos )
		fsck->write("Yes",3);
	eString tmp = lState->getText();
	tmp+=str;

	eSize size=lState->getSize();
	int height = size.height();
	size.setHeight(height*2);
	eLabel l(this);
	l.hide();
	l.resize(size);
	l.setText(tmp);
	if ( l.getExtend().height() > height )
		tmp=str;

	lState->setText(tmp);
}

#endif // DISABLE_FILE
#endif // DISABLE_HDD
