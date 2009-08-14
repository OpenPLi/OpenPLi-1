#include <plugin.h>
#include <stdio.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/dvb/decoder.h>
#include <lib/dvb/service.h>
#include <lib/driver/eavswitch.h>

extern "C" int plugin_exec( PluginParam *par );

class eListBoxEntryScumm :public eListBoxEntryText
{
public:
	eString name;
	eString fullid;
	eString id;
	int savegame;
	virtual bool operator < ( const eListBoxEntry& e) const
	{
		if (fullid != ((eListBoxEntryScumm&)e).fullid)
			return name < ((eListBoxEntryScumm&)e).name;
		return savegame < ((eListBoxEntryScumm&)e).savegame;
	}
	eListBoxEntryScumm(eListBox<eListBoxEntryScumm>* lb)
		:eListBoxEntryText( (eListBox<eListBoxEntryText>*)lb, "")
	{
	}

};

class eScummVMDialog: public eWindow
{
	eListBox<eListBoxEntryScumm> *listbox;
	void AddGame(eListBoxEntryScumm *pscumm);
	void selectedItem(eListBoxEntryScumm *item);
public:
	eScummVMDialog();
	~eScummVMDialog();
};

	// our entry point.
int plugin_exec( PluginParam *par )
{
	eScummVMDialog dlg;
	dlg.show();
	dlg.exec();
	dlg.hide();
	return 0;
}


eScummVMDialog::eScummVMDialog(): eWindow(1)
{
	cmove(ePoint(100, 100));
	cresize(eSize(520, 376));
	setText("ScummVM Game Selector");
	
	listbox=new eListBox<eListBoxEntryScumm>(this);
	listbox->move(ePoint(10, 10));
	listbox->resize(eSize(clientrect.width()-20, clientrect.height()-60));
	listbox->loadDeco();

	FILE *fp;
	char *p;
	char line[256];
	int validentry = 0;
	eListBoxEntryScumm* pscumm = new eListBoxEntryScumm(listbox);
	pscumm->name = "";
	pscumm->savegame = -1;
	pscumm->SetText("Start ScummVM");
	

	fp = fopen( "/hdd/scummvm/scummvmrc", "r" );
	if ( fp )
	{
		while( fgets( line, 128, fp ) )
		{
			if ( *line == '#' )
				continue;
			if ( *line == ';' )
				continue;
			p=strchr(line,'\n');
			if ( p )
				*p=0;
			if (*line == '[' && strcmp(line,"[scummvm]")) // new game entry
			{
				if (validentry)
					AddGame(pscumm);
				line[strlen(line)-1]=0;
				pscumm = new eListBoxEntryScumm(listbox);
				pscumm->fullid = line+1;
				pscumm->savegame = -1;
				validentry = 1;
				continue;
			}
			p=strchr(line,'=');
			if ( !p )
				continue;
			*p=0;
			p++;
			if ( !strcmp(line,"description") )
				pscumm->name = p;
			else if ( !strcmp(line,"gameid") )
				pscumm->id = p;
		}
		if (validentry)
			AddGame(pscumm);
		fclose(fp);
	}
	listbox->sort();	
	CONNECT(listbox->selected, eScummVMDialog::selectedItem);

	setFocus(listbox);
	
}
void eScummVMDialog::AddGame(eListBoxEntryScumm *pscumm)
{
	pscumm->SetText(pscumm->name);

	eString entry = pscumm->fullid+".s";
	DIR* dp = opendir("/hdd/scummvm/savegames");

	if (dp)
	{
		struct dirent *dentry;
		while((dentry = readdir(dp)) != NULL)
		{
			if (!strncmp(dentry->d_name,entry.c_str(),entry.length()))
			{
				eListBoxEntryScumm *pscummsave;
				pscummsave = new eListBoxEntryScumm(listbox);
				pscummsave->id = pscumm->id;
				pscummsave->fullid = pscumm->fullid;
				pscummsave->savegame = atoi(dentry->d_name+entry.length());
				pscummsave->name = pscumm->name;
				pscummsave->SetText(eString().sprintf("  start savegame %02d",pscummsave->savegame));
			}
		}
		closedir(dp);
	}
}

eScummVMDialog::~eScummVMDialog()
{
}

void eScummVMDialog::selectedItem(eListBoxEntryScumm *item)
{
	if (item)
	{
		char mouseswap[100] = "off";
		FILE *fp=fopen("/proc/stb/ir/mouse/up_down_swap","r");
		if (fp)
		{
			fgets( mouseswap, 99, fp );
			char* p=strchr(mouseswap,'\n');
			if ( p )
				*p=0;
			fclose(fp);
		}
		system("echo on > /proc/stb/ir/mouse/up_down_swap");
		fbClass::getInstance()->lock();
		eRCInput::getInstance()->lock();
		int apid = Decoder::parms.apid;
		Decoder::parms.apid = -1;
		Decoder::Set();
		if (eServiceInterface::getInstance()->getService()->getAspectRatio())
			eAVSwitch::getInstance()->setAspectRatio(r43);
		eString s = "cd /hdd/scummvm/ && ./scummvm -c /hdd/scummvm/scummvmrc --savepath=/hdd/scummvm/savegames ";
		if (item->savegame >= 0)
			s += eString().sprintf("-x %d ",item->savegame);
		s += item->fullid;
		system(s.c_str());
		eRCInput::getInstance()->unlock();
		fbClass::getInstance()->unlock();
		Decoder::parms.apid = apid;
		Decoder::Set();
		if (eServiceInterface::getInstance()->getService()->getAspectRatio())
			eAVSwitch::getInstance()->setAspectRatio(r169);
		s.sprintf("echo %s > /proc/stb/ir/mouse/up_down_swap",mouseswap);
		system(s.c_str());
	} 
	close(0);
}
