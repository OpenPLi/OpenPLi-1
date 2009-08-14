/* an enigma plugin to download Satellite Settings 
 
 $Id: getset.cpp,v 1.1 2004/12/01 23:05:52 essu Exp $
 
 copyright (c) 2004 by essu@yadi.org. All rights reserved
 aktuelle Versionen: $Source: /cvs/tuxbox/apps/tuxbox/plugins/enigma/getset/getset.cpp,v $

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published
 by the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 675 Mass Ave, Cambridge MA 02139, USA.
*/

#include <getset.h>

#define GET_KEY eConfig::getInstance()->getKey
#define SET_KEY eConfig::getInstance()->setKey

int version=5;
int t_received;
int t_total, s_total;

#define WHERE_TO_LOOK

// entry point.
int plugin_exec( PluginParam *par )
{
	eGetSettings getset;
	getset.show();
	int result=getset.exec();
	getset.hide();	
// if aborted, show warning.
	if (result)
	{	
		eMessageBox msg("You aborted Download Settings, your selections will not be saved", "User Abort", eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
	}
	else
		eConfig::getInstance()->flush();
	return 0;
}

eHTTPDownload::eHTTPDownload(eHTTPConnection *c, const char *filename): eHTTPDataSource(c), filename(filename)
{
	if (c->remote_header.count("Content-Length"))
		total=atoi(c->remote_header["Content-Length"].c_str());
	else
		total=-1;
	received=0;
	s_total +=total;
	fd=::creat(filename, 0777);
	progress(t_received, s_total);
}

void eHTTPDownload::haveData(void *data, int len)
{
	if (len)
	{
		if (fd >= 0)
			::write(fd, data, len);
	}
	received+=len;
	t_received +=len;
	progress(t_received, s_total);
}

eHTTPDownload::~eHTTPDownload()
{
	if (fd >= 0)
		::close(fd);
	if ((total != -1) && (total != received))
		::unlink(filename.c_str());
}

eGetSettings::eGetSettings(): eWindow(1)
{
	for (int i=0; i<32; i++)
	{
		dl_data[i]=0;
		http[i]=0;
	}
	
	cmove(ePoint(100, 100));
	cresize(eSize(520, 376));
	eGetSettings::init();
	setText(eString().sprintf("Download Satellite Settings - getset 0.%d", version));

// Statusbar	
	lb_selected=new eStatusBar(this);
	lb_selected->move(ePoint(0, clientrect.height()-30));
	lb_selected->resize(eSize(clientrect.width(), 30)); // leave space for both buttons
	lb_selected->loadDeco();
	
// create satellite listbox. it's a child of eGetSettings window, thus we give "this" as parent.
	sat_List=new eListBox<eListBoxEntryText>(this);
	sat_List->move(ePoint(10, 10));
	sat_List->resize(eSize(clientrect.width()/2-20, 35));
	sat_List->loadDeco();
	sat_List->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	sat_List->setHelpText(_("left/right: choose, OK: select, up/down change category"));
	
	for (int i=0;i<26;i++)
	{
		new eListBoxEntryText(sat_List, sat[i], (void*)i);
	}
		
	CONNECT(sat_List->selected, eGetSettings::SatSelected);

// Sat-Display
	satcounter=1;
	for (int i=1; i<5; i++)
	{
		char* sat=""; 
		eString conf;
		conf.sprintf("/getset/sat%d", i);
		if ( GET_KEY(conf.c_str(), sat) )
			sat=""; 
		sat_selected[i]=new eLabel(this);
		sat_selected[i]->move(ePoint(10, 15 +i*30));
		sat_selected[i]->resize(eSize(clientrect.width()/2-20, 30)); 
		sat_selected[i]->setText(sat);
		if ( eString().sprintf(sat) != "" )
			satcounter++;
		free(sat);
	}

// delete sat button	
	bt_satdelete=new eButton(this);
	bt_satdelete->move(ePoint(10, 170)); 
	bt_satdelete->resize(eSize(clientrect.width()/2-20, 35));
	bt_satdelete->setShortcut("yellow");
	bt_satdelete->setShortcutPixmap("yellow");
	bt_satdelete->loadDeco();
	if ( satcounter == 1 )
		bt_satdelete->setText("nothing to delete");
	else
		bt_satdelete->setText(eString().sprintf("Delete Sat %d", (int)satcounter-1));
	
	bt_satdelete->setHelpText(_("delete last shown satellite in list"));
	CONNECT(bt_satdelete->selected, eGetSettings::SatDelete);

// create Sort listbox. Positions relative to parent widget.
	sort_List=new eListBox<eListBoxEntryText>(this);
	sort_List->move(ePoint(clientrect.width()/2+10, 10));
	sort_List->resize(eSize(clientrect.width()/2-20, 35));
	sort_List->loadDeco();
	sort_List->setFlags(eListBox<eListBoxEntryText>::flagNoUpDownMovement);
	sort_List->setHelpText(_("left/right: choose, OK: select, up/down change category"));

	for (int i=0;i<7;i++)
	{
		new eListBoxEntryText(sort_List, sort[i], (void*)i);
	}
	CONNECT(sort_List->selected, eGetSettings::SortSelected);

// Sort-Display
	sortcounter=1;
	for (int i=1; i<5; i++)
	{
		char* sort="";
		eString conf;
		conf.sprintf("/getset/sort%d", i);
		if ( GET_KEY(conf.c_str(), sort) )
			sort=""; 
		sort_selected[i]=new eLabel(this);
		sort_selected[i]->move(ePoint(clientrect.width()/2+10, 15 +i*30));
		sort_selected[i]->resize(eSize(clientrect.width()/2-20, 30)); 
		sort_selected[i]->setText(sort);
		if ( eString().sprintf(sort) != "" )
			sortcounter++;
		free(sort);
	}

// create the sort button	
	bt_sortdelete=new eButton(this);
	bt_sortdelete->move(ePoint(clientrect.width()/2+10, 170)); 
	bt_sortdelete->resize(eSize(clientrect.width()/2-20, 35));
	bt_sortdelete->setShortcut("blue");
	bt_sortdelete->setShortcutPixmap("blue");
	bt_sortdelete->loadDeco();
	if ( sortcounter == 1 )
		bt_sortdelete->setText("nothing to delete");
	else
		bt_sortdelete->setText(eString().sprintf("Delete Sort %d", (int)sortcounter-1));
	bt_sortdelete->setHelpText(_("delete last shown sortorder in list"));
	CONNECT(bt_sortdelete->selected, eGetSettings::SortDelete);
	
// Checkboxes
	for (int i=0; i<6;i++)
	{
		eString conf; unsigned int v_key=0;
		conf.sprintf("/getset/%s", opt[i].c_str());
		if ( GET_KEY(conf.c_str(), v_key ) )
			v_key=0;
		Opt_Switching[i]=new eCheckbox(this, v_key, 1);	
		Opt_Switching[i]->move(ePoint(10+(i%4)*clientrect.width()/4+5, clientrect.height()-160+(i/4)*40));
		Opt_Switching[i]->resize(eSize(110, 30));
		Opt_Switching[i]->loadDeco();
		Opt_Switching[i]->setText(_(txt[i].c_str()));
		Opt_Switching[i]->setHelpText(_(help[i].c_str()));
		
		CONNECT_2_1( Opt_Switching[i]->checked, eGetSettings::Opt_Changed, i );
	}

// For Zapit
	FILE *f=fopen("/bin/zapit", "rt");
	if ( !f )
	{	
		unsigned int v_key=0;
		SET_KEY("/getset/neutrino", v_key);
		Opt_Switching[5]->hide();
	}
	else
		fclose(f);

// Listbox keep Bouquets
	bouquet_List=new eListBox<eListBoxEntryChaeck>(this);
	bouquet_List->move(ePoint(clientrect.width()/2+10, clientrect.height()-120));
	bouquet_List->resize(eSize(clientrect.width()/2-20, 35));
	bouquet_List->loadDeco();
	bouquet_List->setFlags(eListBox<eListBoxEntryChaeck>::flagNoUpDownMovement);
	bouquet_List->setHelpText(_("l/r choose, OK keep bouquet, up/down change category"));
	eGetSettings::GetBouquets();
	CONNECT( bouquet_List->selected, eGetSettings::BouquetSelected );

// Progressbar
	progress=new eProgress(this);
	progress->move(ePoint(10, clientrect.height()-80));
	progress->resize(eSize(clientrect.width()/2-20, 35));
	progress->hide();

// Progresstext
	progresstext=new eLabel(this);
	progresstext->move(ePoint(clientrect.width()/2-5, clientrect.height()-70));
	progresstext->resize(eSize(clientrect.width()/4-10, 25));
	progresstext->setText("");
	progresstext->hide();

// install button
	bt_install=new eButton(this);
	bt_install->move(ePoint(10, clientrect.height()-80)); 
	bt_install->resize(eSize(clientrect.width()/2-20, 35));
	bt_install->loadDeco();
	bt_install->setText("Install Settings");
	bt_install->setHelpText(_("install downloaded satellite settings"));
	bt_install->hide();
	CONNECT(bt_install->selected, eGetSettings::installSettings);

// download button	
	bt_getset=new eButton(this);
	bt_getset->move(ePoint(10, clientrect.height()-80)); 
	bt_getset->resize(eSize(clientrect.width()/2-20, 35));
	bt_getset->setShortcut("red");
	bt_getset->setShortcutPixmap("red");
	bt_getset->loadDeco();
	if ( eSystemInfo::getInstance()->getFEType() == eSystemInfo::feSatellite )
	{
		bt_getset->setText("Download Settings");
		bt_getset->setHelpText(_("download selected satellite settings"));
		CONNECT(bt_getset->selected, eGetSettings::GetSet);
	}
	else
	{
		bt_getset->setText("No Download");
		bt_getset->setHelpText(_("This is not a Satellite-Receiver"));
		bt_getset->hide();
		eMessageBox msg("This is not a Satellite-Receiver", "Plugin will not work with Cable- or terrestial Receivers", eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();	
	}

// global ok button
	bt_ok=new eButton(this);
	bt_ok->move(ePoint(clientrect.width()-clientrect.width()/4, clientrect.height()-80));
	bt_ok->resize(eSize(clientrect.width()/4-10, 35));

// shortcut action(green).
	bt_ok->setShortcut("green");
	bt_ok->setShortcutPixmap("green");
	bt_ok->loadDeco();
	bt_ok->setText("OK"); 
	bt_ok->setHelpText(_("close window and save selections"));
// accept is just a close(0); - we will return with zero as return code.
	CONNECT(bt_ok->selected, eWidget::accept);	
// set the focus on Satellites
	setFocus(sat_List);	
}

void eGetSettings::init()
{
// init
	printf("getset 0.%d", version);
	printf("please report bugs to essu@yadi.org");
	eZapMain::getInstance()->saveUserBouquets();
// Checkboxes	
	opt[0]="paytv"; opt[1]="paysign"; opt[2]="presort"; opt[3]="allsats"; opt[4]="tar"; opt[5]="neutrino";
	help[0]="Load PayTV Channels"; help[1]="Mark PayTV Channels with $-Sign"; help[2]="Load presorted UserBouquets"; help[3]="Load all available Satellites(Be careful!)"; help[4]="Save current Services & Bouquets to /tmp/saved_settings.tar"; help[5]="Load Neutrino Services & Bouquets";
	txt[0]="Pay TV"; txt[1]="PaySign"; txt[2]="Presort"; txt[3]="All Sats"; txt[4]="Archive"; txt[5]="for Zapit";
// Satellites
	sat[0]="Astra_19_2"; sat[1]="Hotbird_13"; sat[2]="Amos_1_4_0"; sat[3]="Arab_26_0E";
	sat[4]="AstraEurob"; sat[5]="Atlantic_B"; sat[6]="EuropeStar"; sat[7]="Eutel_2_F3";
	sat[8]="Eutel_W1_1"; sat[9]="Eutel_W3_7"; sat[10]="Eutel_W4_3"; sat[11]="Express_A1";
	sat[12]="Hellas_2_3"; sat[13]="Hispa_30_0"; sat[14]="Intel_901_"; sat[15]="Intel_903_";
	sat[16]="Kopernikus"; sat[17]="Panam_1_45"; sat[18]="Panam_6_43"; sat[19]="Sirius_2_3";
	sat[20]="Telecom_2A"; sat[21]="Telecom_2D"; sat[22]="Telstar_11"; sat[23]="Telstar_12";
	sat[24]="ThorIntel_"; sat[25]="TuerkEuras";
// Sortorders
	sort[0]="Anbieter"; sort[1]="Satellit"; sort[2]="Frequenz"; sort[3]="Sprache"; 
	sort[4]="TV_Radio"; sort[5]="Sendername"; sort[6]="FTA_PayTV"; 	
// uris, local filenames
	//memset(download, 0, sizeof(download));
	for (int i=0; i<32; i++)
	{
		settings_uri[i]="";
		local_file[i]="";
	}	
	unsigned int y=1, n=0;
	eString conf;
	conf.sprintf("/getset/boot0.%d", version);
	if ( GET_KEY(conf.c_str(), v_Getset_Count) )
	{
		SET_KEY("/getset/paytv", y); SET_KEY("/getset/paysign", y); SET_KEY("/getset/presort", y); SET_KEY("/getset/tar", y); SET_KEY("/getset/allsats", n); SET_KEY("/getset/neutrino", n);
		SET_KEY("/getset/sat1", "" ); SET_KEY("/getset/sat2", "" ); SET_KEY("/getset/sat3", "" ); SET_KEY("/getset/sat4", "" );
		SET_KEY("/getset/sort1", "" ); SET_KEY("/getset/sort2", "" ); SET_KEY("/getset/sort3", "" ); SET_KEY("/getset/sort4", "" );
		eConfig::getInstance()->flush();
		eMessageBox msg("First Use: initalized!","System alert",eMessageBox::iconInfo|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
	}
	SET_KEY(conf.c_str(), ++v_Getset_Count);
	
}

void eGetSettings::SatSelected(eListBoxEntryText *item)
{
	if (item)
	{
		if ( satcounter<5 )
		{
			eString selected=sat[(int)item->getKey()];
			sat_selected[satcounter]->setText(selected);
			eString conf;
			conf.sprintf("/getset/sat%d", satcounter);
			SET_KEY(conf.c_str(), selected.c_str() );
			bt_satdelete->setText(eString().sprintf("Delete -%s-", selected.c_str()));
			bt_satdelete->show();
			satcounter++;
		}
		else
		{
			eMessageBox msg("You can select only 4 Satellites, delete one", "Cannot select", eMessageBox::iconInfo|eMessageBox::btOK);
			msg.show(); msg.exec(); msg.hide();
			setFocus(bt_satdelete);
			satcounter=5;
		}
	} else
		reject();
}

void eGetSettings::SatDelete()
{
	if ( satcounter>1 )
	{
		satcounter--;
		sat_selected[satcounter]->setText("");
		eString conf;
		conf.sprintf("/getset/sat%d", satcounter);
		SET_KEY(conf.c_str(), "" );
	}
	if ( satcounter<2 )
	{
		satcounter=1;
		bt_satdelete->setText("nothing to delete");
		bt_satdelete->hide();
		setFocus(sat_List);
	}
	else
		bt_satdelete->setText(eString().sprintf("Delete Sat %d", satcounter-1));
}

void eGetSettings::SortSelected(eListBoxEntryText *item)
{
	if (item)
	{
		if ( sortcounter<5 )
		{
			eString selected=sort[(int)item->getKey()];
			sort_selected[sortcounter]->setText(selected);
			eString conf;
			conf.sprintf("/getset/sort%d", sortcounter);
			SET_KEY(conf.c_str(), selected.c_str() );
			bt_sortdelete->setText(eString().sprintf("Delete -%s-", selected.c_str()));
			bt_sortdelete->show();
			sortcounter++;
		}
		else
		{
			eMessageBox msg("You can select only 4 Sortorders, delete one", "Cannot select", eMessageBox::iconInfo|eMessageBox::btOK);
			msg.show(); msg.exec(); msg.hide();
			setFocus(bt_sortdelete);
			sortcounter=5;
		}
	} else
		reject();
}

void eGetSettings::SortDelete()
{
	if ( sortcounter>1 )
	{
		sortcounter--;
		sort_selected[sortcounter]->setText("");
		eString conf;
		conf.sprintf("/getset/sort%d", sortcounter);
		SET_KEY(conf.c_str(), "" );
	}
	if ( sortcounter<2 )
	{
		bt_sortdelete->setText("nothing to delete");
		bt_sortdelete->hide();
		setFocus(sort_List);
	}
	else
		bt_sortdelete->setText(eString().sprintf("Delete Sort %d", sortcounter-1));

}

void eGetSettings::Opt_Changed( int i, int nr )
{
	unsigned int old = 0; eString conf;
	conf.sprintf("/getset/%s", opt[nr].c_str());
	GET_KEY(conf.c_str(), old );
	unsigned int v_key = (unsigned int) i;
	SET_KEY(conf.c_str(), v_key );
	if ( nr == 3 )
	{
		if ( v_key== 1 )
		{
		eMessageBox msg("Though free space on your disk is testet yet, downloading all available Satellites may take a long time, depending to your internet connection, reloading the settings will need some Minutes", "All Sats selected", eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
		}
		else
			setFocus(Opt_Switching[++nr]);
	}
	else
	{
		if ( nr < 5 )
			setFocus(Opt_Switching[++nr]);
		else
			setFocus(bouquet_List);
	}
}

void eGetSettings::GetBouquets()
{
	DIR *d=opendir("/var/tuxbox/config/enigma");
	if (!d)
	{
		eMessageBox msg("error reading services directory", "error");
		msg.show(); msg.exec(); msg.hide();
	} 
	else
	{
		int i=0;
		while(struct dirent *e=readdir(d))
		{	
			eString fileName=e->d_name;	
			if ( fileName.mid(fileName.rfind('.')+1) == "tv" || fileName.mid(fileName.rfind('.')+1) == "radio" )
			{
				FILE* readfile;
				eString real_fname="/var/tuxbox/config/enigma/"+fileName;
				readfile = fopen(real_fname.c_str(), "r"); 
				if ( readfile ) 
				{
					char line[128];
					if ( strlen(fgets(line, 128, readfile)) >6 )
					{
					bouquet[i]=fileName;
					bouquet_saved[i]="";
					bouquet_name[i]=line+6;
					new eListBoxEntryChaeck(bouquet_List, bouquet_name[i], (void*)i++);
					}	
				}
				fclose(readfile);
			}
			
		}
	}
	closedir(d);
}

void eGetSettings::BouquetSelected(eListBoxEntryChaeck *item)
{
	if (item)
	{
		int n=(int)item->getKey();
		if ( item->getCheck() )
			bouquet_saved[n]=bouquet[n];
		else
			bouquet_saved[n]="";
	} 
	else
		reject();
}

void eGetSettings::GetSet()
{
	system("mkdir /tmp/getset");
	system("rm -f /tmp/getset/*");
	eString uri="http://satinfo.kfsw.de/sat/settings.fcgi/";
	uri += getOptions();
	settings_uri[0]=uri;
	local_file[0]="catalog";
	if ( uri.length() < 100 || satcounter<2 )
	{
		eMessageBox msg("You have to select at least one Satellite", "Selection incomplete", eMessageBox::iconWarning|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
	}
	else
	{
		bt_getset->hide();
		t_received = t_total = s_total = 0;
		progress->show();
		doDownload(0);
		progress->hide();
	}
}

void eGetSettings::doDownload(int i)
{
	if (http[i])
		delete http[i];
	progress->show();
	progresstext->show();
	http[i]=eHTTPConnection::doRequest(settings_uri[i].c_str(), eApp, &error[i]);
	if (!http[i])
	{
		downloadDone(error[i], i);
	}
	else
	{
		eString txt;
		txt.sprintf("now downloading %s...", local_file[i].c_str());
		lb_selected->setText(txt);
		CONNECT_2_1(http[i]->transferDone, eGetSettings::downloadDone, i);
		CONNECT_2_1(http[i]->createDataSource, eGetSettings::createDataSink, i);
		http[i]->local_header["User-Agent"]="enigma-getset/0.5.0";
		http[i]->start();
	}
}

void eGetSettings::downloadDone(int err, int i)
{
	//progress->hide();
	//progresstext->hide();
	if (err || !http[i] || http[i]->code != 200)
	{
		setError(err, i);
	}
	else
	{
		if ( i++ > 0 )
		{
			if ( i == 32 || ( settings_uri[i] == "" ) )
				finished();
			else
				doDownload(i);
		}
		else //if ( i == 0 )
		{
			if ( readCatalog() )
			{
				if ( settings_uri[i] == "" )
					lb_selected->setText("failure catalog empty");
				else
					doDownload(i);
			}
			else
				lb_selected->setText("failure reading catalog");
		}
	}
}

void eGetSettings::finished()
{
	for (int i=0; i<32; i++)
	{
		dl_data[i]=0;
		http[i]=0;
	}
	keepBouquets();
	tarBouquets();
	if ( config_dir == "enigma" && ( tv || radio ) )
		restoreSaved();
	progress->hide();
	progresstext->hide();
	if ( checkFree() )
	{
		bt_install->show();
		setFocus(bt_install);
	}
	else
	{
		setFocus(bt_ok);
	}
}

int eGetSettings::readCatalog()
{
	int filesize=0;
	struct stat tmp;
	if ( stat( "/tmp/getset/catalog", &tmp ) != -1 )
		filesize=tmp.st_size;
	if ( filesize == 0 )
	{
		eString txt;
		txt.sprintf("Catalog-Filesize: %d", filesize);
		lb_selected->setText(txt);
		return 0;
	}
	int n=0;
	FILE *f=fopen("/tmp/getset/catalog", "rt");
	char temp[94];
	while ( fgets(temp, 94, f) )
	{
		n++;
		if (*temp)
			temp[strlen(temp)-1]=0; // remove trailing \n
		eString str(temp);
		eString txt=eString().sprintf("%s", temp);
		local_file[n]=txt.mid(txt.rfind('/')+1);
		//if  ( !( local_file[n].find('u') == 1  ||  local_file[n].rfind('s') == 8 ) )
			//break;
		lb_selected->setText(local_file[n].c_str());
		settings_uri[n].sprintf("http://satinfo.kfsw.de%s", temp);
	}
	fclose(f);
	if ( local_file[n] == "services" || local_file[n] == "services.xml" )
	{
		t_total=n;
		local_file[++n]="";
		return 1;
	}
	else
	{
		lb_selected->setText("catalog seems to contain no valid data");
		return 0;
	}
}


eString eGetSettings::getOptions()
{ 
	eString conf, _sat_[5], _sort_[5];
	
	for (int i=1;i<5;i++)
	{
		char* key;
		conf.sprintf("/getset/sat%d", i);
		if ( GET_KEY(conf.c_str(), key) )
			key="";
		_sat_[i]=eString().sprintf(key);
		conf.sprintf("/getset/sort%d", i);
		if ( GET_KEY(conf.c_str(), key) )
			key="";
		_sort_[i]=eString().sprintf(key);
		free(key);
	}

	unsigned int v_key=0; 
	eString paytv, paysign, presort, allsats, sformat;
	GET_KEY("/getset/paytv", v_key );
	paytv = v_key == 0 ? "no" : "yes";
	GET_KEY("/getset/paysign", v_key );
	paysign = v_key == 0 ? "no" : "yes";
	GET_KEY("/getset/presort", v_key );
	presort = v_key == 0 ? "no" : "yes";
	GET_KEY("/getset/allsats", v_key );
	allsats = v_key == 0 ? "no" : "yes";
	GET_KEY("/getset/tar", v_key );
	tar = v_key == 0 ? "no" : "yes";
	GET_KEY("/getset/neutrino", v_key);
	if ( v_key==1 )
	{
		sformat="D_Box_2_Linux_Neutrino_1_8";
		config_dir="zapit";
		system("mkdir /var/tuxbox/config/zapit");
	}
	else
	{
		sformat="D_Box_2_Linux_Enigma"; //BTW: sformat="Dreambox_Linux_Enigma";
		config_dir="enigma";
	}
	eString request;
	request.sprintf( "sat_1__%s/sat_2__%s/sat_3__%s/sat_4__%s/sort_1__%s/sort_2__%s/sort_3__%s/sort_4__%s/sort_5__/sort_6__/paytv__%s/paysign__%s/presort__%s/allsats__%s/frm__%s/design__box/iKg__build/catalog", _sat_[1].c_str(), _sat_[2].c_str(), _sat_[3].c_str(), _sat_[4].c_str(), _sort_[1].c_str(), _sort_[2].c_str(), _sort_[3].c_str(), _sort_[4].c_str(), paytv.c_str(), paysign.c_str(), presort.c_str(), allsats.c_str(), sformat.c_str() ); //box__settings.html
	return request;	
}

void eGetSettings::downloadProgress(int received, int total, int i)
{
	if ((time(0) == lasttime[i]) && (received != total))
		return;
	lasttime[i]=time(0);
	if ( total > 0 )
	{
		eString pt;
		int perc=received*100/total;
		pt.sprintf("%d/%dk %d%%",1 + received/1024, 1 + total/1024, perc);
		if ( t_total>0 )
			perc=(i)*100/t_total;
		progress->setPerc(perc);
		progresstext->setText(pt);
	} 
	else
	{
		eString pt;
		pt.sprintf("%d k received", 1 + received/1024);
		progress->setPerc(0);
		progresstext->setText(pt);
	}
	
}

eHTTPDataSource *eGetSettings::createDataSink(eHTTPConnection *connct, int i)
{
	if (dl_data[i])
		delete dl_data[i];;
	lasttime[i]=0;
	current_file="tmp/getset/" + local_file[i];
	dl_data[i]=new eHTTPDownload(connct, current_file.c_str());
	CONNECT_2_1(dl_data[i]->progress, eGetSettings::downloadProgress, i);
	return dl_data[i];
}
	
void eGetSettings::setError(int err, int i)
{
	eString errmsg;
	switch (err)
	{
	case 0:
		if (http[i] && http[i]->code != 200)
			errmsg="error: server replied " + eString().setNum(http[i]->code) + " " + http[i]->code_descr;
		break;
	case -2:
		errmsg="Can't resolve hostname!";
		break;
	case -3:
		errmsg="Can't connect! (check network settings)";
		break;
	default:
		errmsg.sprintf("unknown error %d", err);
	}
	if (errmsg.length())
	{
		if (current_url.length())
			errmsg+="\n(URL: " + current_url + ")\n(FILE: " + current_file+ ")\n";
		lb_selected->setText(errmsg);
		eMessageBox msg(errmsg, _("Error!"), eMessageBox::btOK|eMessageBox::iconError);
		msg.show(); msg.exec(); msg.hide();
		setFocus(bt_ok);
	}
}

void eGetSettings::tarBouquets()
{
	eString cmd;
	if ( config_dir=="enigma" )
		cmd="tar -cvf /tmp/saved_settings.tar /var/tuxbox/config/enigma/*uqu* /var/tuxbox/config/enigma/services";
	else
		cmd="tar -cvf /tmp/saved_settings.tar /var/tuxbox/config/zapit/*.xml";
	system(cmd.c_str());
}
void eGetSettings::keepBouquets()
{
	tv="", radio=""; 
	if ( config_dir == "enigma" )
	{
		eString nr;
		for (int i=0; i<32; i++)
		{
			if ( !bouquet_saved[i] || bouquet_saved[i] == "" )
			{
				continue;
			}
			else
			{
			int x=i+900864; //dbf..
			nr.sprintf("%x", x);
			eString service_type=bouquet_saved[i].mid(bouquet_saved[i].rfind('.'));
			eString new_name="userbouquet."+nr+service_type;
			if ( service_type == ".tv" )
				tv +="#SERVICE: 4097:7:0:"+nr+":0:0:0:0:0:0:/var/tuxbox/config/enigma/"+new_name+"\n#TYPE 16385\n/var/tuxbox/config/enigma/"+new_name+"\n";
			else
				radio +="#SERVICE: 4097:7:0:"+nr+":0:0:0:0:0:0:/var/tuxbox/config/enigma/"+new_name+"\n#TYPE 16385\n/var/tuxbox/config/enigma/"+new_name+"\n";
			
			eString cmd;
			cmd.sprintf("cp /var/tuxbox/config/enigma/%s /tmp/getset/%s", bouquet_saved[i].c_str(), new_name.c_str());
			system(cmd.c_str());
			}
		}
	}
}

void eGetSettings::restoreSaved()
{
	lb_selected->setText("restore Settings");
	eString cmd;
	if ( tv )
	{
		tv =eString().sprintf("#SERVICE: 1:64:1:0:0:0:0:0:0:0:\n#DESCRIPTION: Bouquets saved by getset 0.%d:\n", version) +tv;
		cmd.sprintf("echo '%s' >>/tmp/getset/userbouquets.tv.epl", tv.c_str());
		system(cmd.c_str());
	}
	if ( radio )
	{
		radio=eString().sprintf("#SERVICE: 1:64:1:0:0:0:0:0:0:0:\n#DESCRIPTION: Bouquets saved by getset 0.%d:\n", version) +radio;
		cmd.sprintf("echo '%s' >>/tmp/getset/userbouquets.radio.epl", radio.c_str());
		system(cmd.c_str());
	}
}

int eGetSettings::checkFree()
{	
	int freed=0, needed=0;
	for (int i=0; i<32; i++)
	{
		if ( local_file[i] == "" )
			break;
		int filesize=0;
		struct stat tmp;
		eString f_name="/var/tuxbox/config/"+config_dir+"/"+local_file[i];
		if ( stat( f_name.c_str(), &tmp ) != -1 )
			filesize=tmp.st_size;
		freed += filesize;
		
		f_name="/tmp/getset/"+local_file[i];
		if ( stat( f_name.c_str(), &tmp ) != -1 )
			filesize=tmp.st_size;
		needed += filesize;
	}
	sat_toload=(freed+needed)/1024;
	needed -=freed;
	
	struct statfs s;
	int free;
	if (statfs("/var", &s)<0)
		free=-1;
	else
		free=s.f_bfree*s.f_bsize;
		
	if ( free < needed )
	{
		eString txt;
		txt.sprintf("disk space needed: %d kb\ndisk space free: %d kb\nFree additional %d kb,\n then install settings manually!", needed/1024, free/1024, 1+(needed-free)/1024 ); //;)
		eMessageBox msg(txt, "free disk space", eMessageBox::iconInfo|eMessageBox::btOK);
		msg.show(); msg.exec(); msg.hide();
		return 0;
	}
	else
		return 1;
}

void eGetSettings::installSettings()
{		
	bt_install->hide();

	eString cmd="cp /tmp/getset/* /var/tuxbox/config/"+config_dir;
	system(cmd.c_str());
	
	if ( config_dir == "enigma" )
	{
		eString txt;
		txt.sprintf("be patient, reloading settings (~ %d sek)", 30 + sat_toload/2 );
		lb_selected->setText(txt);
		eDVB::getInstance()->settings->loadServices();
		lb_selected->setText("reloaded Services");
		eDVB::getInstance()->settings->loadBouquets();
		lb_selected->setText("reloaded Bouquets");
		eZapMain::getInstance()->loadUserBouquets();
		lb_selected->setText("reloaded UserBouquets");
	}
	system("rm -f /tmp/getset/*");
	::rmdir("/tmp/getset");
	
	for (int i=0; i<32; i++)
	{
		settings_uri[i]="";
		local_file[i]="";
	}
	bt_getset->show();
	setFocus(bt_ok);
}

eGetSettings::~eGetSettings()
{
	/*if (http)
		delete http;
	hat immer zu segfaults geführt (als http noch kein array war)????	*/
}
