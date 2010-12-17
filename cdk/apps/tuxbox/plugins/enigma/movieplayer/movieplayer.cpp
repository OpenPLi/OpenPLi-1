/*
 * $Id: movieplayer.cpp,v 1.17 2006/01/26 19:20:33 digi_casi Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *          based on vlc plugin by mechatron
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

//#define RC_VOLUP	10
//#define RC_VOLDN	11
#define RC_LEFT		35
#define RC_RIGHT	36

#define KEY_STATE_DOWN		0
#define KEY_STATE_REPEAT	eRCKey::flagRepeat
#define KEY_STATE_UP		eRCKey::flagBreak

#include <plugin.h>
#include <xmltree.h>
#include "movieplayer.h"
#include <lib/dvb/decoder.h>
#include <lib/base/buffer.h>
#include <lib/driver/eavswitch.h>
#include <lib/system/info.h>
#include <enigma.h>

#define REL "Movieplayer Plugin, v0.9.97"

extern "C" int plugin_exec(PluginParam *par);
extern eString getWebifVersion();
extern eIOBuffer tsBuffer;

bool sortByName(const PLAYLIST& a, const PLAYLIST& b)
{
	return a.Filename < b.Filename;
}

bool sortByType(const PLAYLIST& a, const PLAYLIST& b)
{
	if (a.Filetype == b.Filetype)
		return sortByName(a, b);
	else
		return a.Filetype < b.Filetype;
}

eSCGui::eSCGui(): menu(true)
{
	int fd = eSkin::getActive()->queryValue("fontsize", 20);

	pauseBox = new eMessageBox(_("Press yellow or green button for continue..."),_("Pause"), eMessageBox::iconInfo);

  	cmove(ePoint(90, 110));
	cresize(eSize(540, 380));

	addActionMap(&i_shortcutActions->map);
	addActionMap(&i_cursorActions->map);

	silver_large_rc = isLargeRC();

	list = new eListBox<eListBoxEntryText>(this);
	list->move(ePoint(10, 10));
	list->resize(eSize(clientrect.width() - 20, 260));
	CONNECT(list->selected, eSCGui::listSelected);
	CONNECT(list->selchanged, eSCGui::listSelChanged);

	eLabel *l_root = new eLabel(this);
	l_root->move(ePoint(10, clientrect.height() - 110));
	l_root->resize(eSize(130, fd + 5));
	l_root->setProperty("align", "center");
	l_root->setProperty("vcenter", "");
	l_root->setProperty("backgroundColor", "std_dred");
	l_root->setText(_("Files"));

	eLabel *l_vcd = new eLabel(this);
	l_vcd->move(ePoint(140, clientrect.height() - 110));
	l_vcd->resize(eSize(130, fd + 5));
	l_vcd->setProperty("align", "center");
	l_vcd->setProperty("vcenter", "");
	l_vcd->setProperty("backgroundColor", "std_dgreen");
	l_vcd->setText("SVCD&VCD");

	eLabel *l_dvd = new eLabel(this);
	l_dvd->move(ePoint(270, clientrect.height() - 110));
	l_dvd->resize(eSize(130, fd + 5));
	l_dvd->setProperty("align", "center");
	l_dvd->setProperty("vcenter", "");
	l_dvd->setProperty("backgroundColor", "std_dyellow");
	l_dvd->setText("DVD");

	eLabel *l_cfg = new eLabel(this);
	l_cfg->move(ePoint(400, clientrect.height() - 110));
	l_cfg->resize(eSize(130, fd + 5));
	l_cfg->setProperty("align", "center");
	l_cfg->setProperty("vcenter", "");
	l_cfg->setProperty("backgroundColor", "std_dblue");
	l_cfg->setText(_("Options"));
	
	status = new eStatusBar(this);
	status->move(ePoint(10, clientrect.height() - 70));
	status->resize(eSize(clientrect.width() - 20, 60));
	status->loadDeco();

	timer = new eTimer(eApp);
	CONNECT(timer->timeout, eSCGui::timerHandler);

	if(!silver_large_rc)  // used for non large silver RC only 
		init_volume_bar();
	
	int mode = DATA;
	eConfig::getInstance()->getKey((pathcfg+"lastmode").c_str(), mode);

	getSavedPath();
	loadList(mode, startdir);
}

eSCGui::~eSCGui()
{
	playList.clear();
	delete timer;
	eDebug("[MOVIEPLAYER] ending...");
}

bool eSCGui::supportedFileType(eString filename)
{
	unsigned int pos = filename.find_last_of('.');
	eString extension = filename.right(filename.size() - pos - 1);
	struct videoTypeParms videoParms = eMoviePlayer::getInstance()->mpconfig.getVideoParms((eString)"File", extension);
	return videoParms.name == "File";
}

void eSCGui::loadList(int mode, eString pathfull)
{

	playList.clear();
	PLAYLIST a;
	
	VLC8=0;
    eConfig::getInstance()->getKey((pathcfg+"vlc8").c_str(), VLC8 );

	infoBox = 0; bufferingBox = 0; jumpBox = 0; skip_time = 0;
	list->setHelpText(_("Please wait ... communicating with VLC"));

	eString errPath = " " + (eString) _("Error") + " - " + pathfull + _(" is not accessible...");
	eString upPath = /*"\t" +*/ (eString)_("[GO UP]");

	nFiles=0;
	int nGoUp=0;
	int isItem=0;

	eString tmp2 = " ";
	eString tmp3;

	switch (mode)
	{
		case DATA:
			tmp2 += _("Files");
			tmp3 = "";
			break;
		case VCD:
			tmp2 += "SVCD&VCD";
			setText(tmp2 + " - Drive: " + cddrive);
			if(VLC8)
			    tmp3 = "vcd:";
			else
			    tmp3 = "vcd://";
			break;
		case DVD:
			tmp2 += "DVD";
			setText(tmp2 + " - Drive: " + cddrive);
			if(VLC8)
			    tmp3 = "dvdsimple:";
			else
				tmp3 = "dvdsimple://";
			break;
	}

	eString response;
	CURLcode httpres= sendGetRequest("/requests/browse.xml?dir=" + url_code(pathfull).strReplace(" ", "%20"), response);

	//std::replace(response.begin(), response.end(), '\\', '/');  for auto-subtitles under Windows must be commented !!!
	response.strReplace("://", ":/"); // what the heck...
	if (httpres == 0 && response.length() > 0)
	{
		if (mode == DATA)
		{
			//eDebug("[IMS] file: %s",response.c_str());
			XMLTreeParser parser("UTF-8");

			unsigned int start = 0;
			for (unsigned int pos = response.find('\n', 0); pos != std::string::npos; pos = response.find('\n', start))
			{
				eString entry = ""; entry = response.substr(start, pos - start); // one line
				if(entry.length() > 5) // all less than <root> is skipped
				{
					if (!parser.Parse(entry.c_str(), entry.length(), 0))
					{
						//eDebug("[MOVIEPLAYER] parsing settings file: %s at line %d>",parser.ErrorString(parser.GetErrorCode()), parser.GetCurrentLineNumber());
						eMessageBox msg(eString("parser.RootNode() response"), _("Error"), eMessageBox::btOK);
						msg.show();msg.exec();msg.hide();
						return;
					}
					isItem++;
				}
				start = pos + 1;
			}
			if( isItem < 4 ) // if no items
			{
				a.Filename = upPath;
				a.Fullname = "/..";
				setText(errPath);
				a.Filetype = GOUP;
				playList.push_back(a);
			}

			XMLTreeNode *root_mp = parser.RootNode();
			if (!root_mp)
			{
//				eDebug("[MOVIEPLAYER] parsing error");
				eMessageBox msg(eString("parser.RootNode()"), _("Error"), eMessageBox::btOK);
				msg.show();msg.exec();msg.hide();
				return;
			}

			for (XMLTreeNode *node = root_mp->GetChild(); node; node = node->GetNext())
			{
//				eDebug("[MOVIEPLAYER] file: %s",entry.c_str());

				eString tmptype = node->GetAttributeValue("type");
				eString tmpsize = node->GetAttributeValue("size");
				eString tmpdate = node->GetAttributeValue("date"); 
				eString tmppath = node->GetAttributeValue("path");
				eString tmpname = node->GetAttributeValue("name");

				if (tmptype =="directory")
				{
					if (tmpname == ".." || tmpname[1] == ':' && tmpname.length() == 2) // is GOUP or disk name
					{
						if( tmpname[1] == ':' )  // Win's disk name
						{
							a.Filename = tmpname;
							a.Fullname = tmppath;
							setText(tmp2 + " - " + _("Select drive:"));
							a.Filetype = GOUP;
							//eDebug("[MOVIEPLAYER] goup2: %s",a.Fullname.c_str());
							playList.push_back(a);
						}
						else if(!nGoUp) // VLC return for some net 2x root => used only one
						{
							a.Filename = upPath;
							a.Fullname = tmppath;
							eString display = a.Fullname.left(a.Fullname.length() - 3);
							path = tmp2 + " - " + _("Path:") + " " + display;
							setText(path);
							a.Filetype = GOUP;
							//eDebug("[MOVIEPLAYER] goup1: %s",a.Fullname.c_str());
							playList.push_back(a);
							nGoUp++;
							setSavedPath(display);
						}
					}
					else
					{
						a.Filename = "[DIR]  " + tmpname;
						a.Fullname = tmppath;
						a.Filetype = DIRS;
						//eDebug("[MOVIEPLAYER] dir: %s %s",a.Fullname.c_str(),a.Filename.c_str());
						playList.push_back(a);
					}
				}
				else if (tmptype == "file" )
				{
					if (supportedFileType(tmpname))
					{
						a.Filename = tmpname;
						a.Fullname = tmppath;
						a.Filesize = eString().sprintf("%s: %d%s",_("Size"), atoi(tmpsize.c_str())/1024/1024,"MB");
						a.Filetype = FILES;
						playList.push_back(a);
						nFiles++;
//						eDebug("[MOVIEPLAYER] file name: %d %s %s", nFiles, a.Fullname.c_str(),a.Filesize.c_str());
					}
				}
			}
		}
		else //VCD,SVCD,DVD
		{
			if (cddrive[cddrive.length() - 1] == ':' && cddrive.length() == 2)
			{
				pathfull = cddrive + "\\";
			}

			int title = 0; //DVD Title
			eConfig::getInstance()->getKey((pathcfg+"title").c_str(), title);

			for (int i = 0; i <= 30; i++)
			{
				if( i == 0 && !VLC8 )
					a.Filename = (eString)_(">> All <<");  // Whole DVD
				else
					a.Filename = (eString)_("Chapter") + eString().sprintf(" %02d", i);

				a.Fullname = tmp3 + cddrive;
				if(VLC8)
				    a.Fullname += "@1:" + eString().sprintf("%d-", i);
				else
				{
					if(i==0)
					{
						if(title!=0)
							a.Fullname += eString().sprintf("@%d",title);
					}
					else
				    	a.Fullname += eString().sprintf("@%d:%d",title,i);
				}
				a.Filetype = FILES;
				playList.push_back(a);
				nFiles++;
			}
		}
	}
	else
	{
		if( mode == DATA && httpres == 52 ) // path is not accesible - Win (long time, then is returned httpres=52)
		{
			a.Filename = upPath;
			a.Fullname = pathfull + "/..";
			setText(errPath);
			a.Filetype = GOUP;
			playList.push_back(a);
		}
		else
			setText(" " + (eString)_("Could not communicate with VLC server") + " " + VLC_IP);
		list->setHelpText(_("Make sure that VLC is started, IP address is configured correctly or IP of Dreambox is enabled there in .host file."));
	}

	viewList();
	eConfig::getInstance()->setKey((pathcfg+"lastmode").c_str(), mode);
}

void eSCGui::viewList()
{
	sort(playList.begin(), playList.end(), sortByType);

	list->beginAtomic();
	list->clearList();
	int current = 0;

	for (PlayList::iterator p = playList.begin(); p != playList.end(); p++)
		(*p).listEntry = new eListBoxEntryText(list, (*p).Filename, (void *)current++);

	list->endAtomic();
	setStatus(0);
}

void eSCGui::setStatus(int val)
{
	if (playList.size() >  0)
	{
		switch (playList[val].Filetype)
		{
			case GOUP:
				list->setHelpText(_("Go up one directory level"));
				break;
			case DIRS:
				list->setHelpText(_("Enter directory"));
				break;
			case FILES:
				list->setHelpText(playList[val].Filename + "\n" + playList[val].Filesize);
				break;
		}
	}
}

void eSCGui::listSelChanged(eListBoxEntryText *item)
{
	if (item)
		setStatus((int)item->getKey());
}

void eSCGui::listSelected(eListBoxEntryText *item)
{
	if (item)
	{
		if(menu)
			val = (int)item->getKey();
		if (playList[val].Filetype == FILES)
			playerStart(val);
		else
			loadList(DATA, playList[val].Fullname);
	}
}

void eSCGui::showMenu()
{
	if (!menu)
	{
		timer->stop();
		if(bufferingBox)
		{
			bufferingBox->hide();
			delete bufferingBox;
			bufferingBox=0;
		}
		pauseBox->hide();
		hide();
		cmove(ePoint(90, 110));
		status->loadDeco();
		show();

		menu = true;
	}
}

void eSCGui::timerHandler()
{
// 	eDebug("[MOVIEPLAYER PLUGIN] timerHandler: status = %d", eMoviePlayer::getInstance()->status.STAT);

	switch (eMoviePlayer::getInstance()->status.STAT)
	{
		case eMoviePlayer::STREAMERROR:
		{
//		        eDebug("#   STREAMERROR:");
			setText((eString)_(" A streaming error occurred..."));
			list->setHelpText(_("Please make sure that VLC is started on the PC and that it can play the file you selected."));
			int stop_err=0;
			eConfig::getInstance()->getKey((pathcfg+"err").c_str(), stop_err );
			if(stop_err)
			{
				showMenu();
				break;
			}
			else
			{
				eDebug("A streaming error occurred, trying next file...");
				if(bufferingBox)
				{
					bufferingBox->hide();
					delete bufferingBox;
					bufferingBox=0;
				}
			}
		}
		case eMoviePlayer::STOPPED:
		{
			int play_next=1;
			eConfig::getInstance()->getKey((pathcfg+"playnext").c_str(), play_next );

			if (playList.size() > 1 && play_next)
			{
				do
				{
					if (++val >= playList.size())
			 			val = 0;
		 		} while (playList[val].Filetype != FILES);

				playerStart(val);
				list->setCurrent(playList[val].listEntry);
			}
			else
				showMenu();
			break;
		}
		default:
		{       
			if(infoBox)
			{
				eString text = "";
				infoBox->setText(filePos(0,"","",text));
			}

			if (!eMoviePlayer::getInstance()->status.BUFFERFILLED)
			{
				if(infoBox)
				{
					infoBox->hide();
					delete infoBox;
					infoBox=0;
				}
				if(!bufferingBox)
				{
					bufferingBox = new eMessageBox(_("Buffering video stream... Please wait."),"Movieplayer", eMessageBox::iconInfo);
					bufferingBox->show();
				}
				else
				{
					int a = tsBuffer.size()/(eMoviePlayer::getInstance()->initialBuffer/100);
					bufferingBox->setText(eString().sprintf("Movieplayer - %s:  %d%c",_("finished"),a,'%'));
				}
			}
			else
			{
				if(bufferingBox)
				{
					bufferingBox->hide();
					delete bufferingBox;
					bufferingBox=0;
				}
			}
			if(jumpBox)
			{
				int msgTime=3;
				eConfig::getInstance()->getKey((pathcfg+"msgtime").c_str(), msgTime );
				if(++skip_time >= (unsigned int)msgTime)
				{
					jumpBox->hide();
					delete jumpBox;
					jumpBox=0;
					skip_time = 0;
				}
			}
			timer->start(1000, true);
			break;
		}
	}
}

void eSCGui::playerStart(int val)
{
	if (menu)
	{
		hide();
		cmove(ePoint(90, 800));
		show();
		menu = false;
	}
	if(jumpBox)
	{
		jumpBox->hide();
		delete jumpBox;
		jumpBox=0;
	}

	eDebug("\n[VLC] trying play %d \"%s\"", val, playList[val].Fullname.c_str());

	if (eMoviePlayer::getInstance()->status.STAT != eMoviePlayer::STOPPED &&
		eMoviePlayer::getInstance()->status.STAT != eMoviePlayer::STREAMERROR)
	{
		if(eMoviePlayer::getInstance()->status.BUFFERFILLED) // info about played file - OK button:
		{
			eDebug("\n[VLC] BUFFERFILLED");
			if(infoBox)
			{
				infoBox->hide();
				delete infoBox;
				infoBox=0;
			}
			else
			{
				eString text="";
				eString caption = filePos(1,playList[val].Filename,playList[val].Filesize, text);
				infoBox = new eMessageBox( text, caption, eMessageBox::iconInfo );
				infoBox->show();
			}
		}
	}
	else
	{	
		eDebug("\n[VLC] will be play ...");
		setText(path); // refresh title
		Start2(val);
	}
}

void eSCGui::Start2(int value)
{
	int mode = DATA;
	eConfig::getInstance()->getKey((pathcfg+"lastmode").c_str(), mode);
	eMoviePlayer::getInstance()->control("start2", mrl_par(val,mode).c_str());
	list->setCurrent(playList[value].listEntry);
	timer->start(2000, true);
}

eString eSCGui::mrl_par(int value, int mode)
{
	changeSout();

	int input_txt = 0; // input as number or text
	eConfig::getInstance()->getKey((pathcfg+"input").c_str(), input_txt);

	int sub_color = 0; // input as number or text
	eConfig::getInstance()->getKey((pathcfg+"scolor").c_str(), sub_color);

	eString mrlpars = "";
	if(input_txt)
	{
		eString a_lang = "   "; 
		eConfig::getInstance()->getKey((pathcfg+"alang").c_str(), a_lang);
		eString s_lang = "   "; 
		eConfig::getInstance()->getKey((pathcfg+"slang").c_str(), s_lang);
		if(a_lang != "   ")
			mrlpars += eString().sprintf(" :audio-language=%s",a_lang.c_str());
		if(s_lang != "   ")
			mrlpars += eString().sprintf(" :sub-language=%s",s_lang.c_str());
	}
	else
	{
		int a_track = -1; 
		eConfig::getInstance()->getKey((pathcfg+"atrack").c_str(), a_track);
		int s_track = -1; 
		eConfig::getInstance()->getKey((pathcfg+"strack").c_str(), s_track);
		if(a_track != -1)
			mrlpars += eString().sprintf(" :audio-track=%d",a_track);
		if(s_track != -1)
			mrlpars += eString().sprintf(" :sub-track=%d",s_track);
	}
/* prepared for all in one:
	eString mrlpars =  "";
	eString a_lang = "eng"; eConfig::getInstance()->getKey((pathcfg+"alang").c_str(), a_lang);
	eString s_lang = "eng"; eConfig::getInstance()->getKey((pathcfg+"slang").c_str(), s_lang);

	if(a_lang == "   ")
		mrlpars += "";
	if(a_lang.length() < 3)
		mrlpars += eString().sprintf(" :audio-track=%s",a_lang.c_str());
	else
		mrlpars += eString().sprintf(" :audio-language=%s",a_lang.c_str());
	
	if(s_lang == "   ")
		mrlpars += "";
	else
	if(s_lang.length() < 3)
		mrlpars += eString().sprintf(" :sub-track=%s",s_lang.c_str());
	else
		mrlpars += eString().sprintf(" :sub-language=%s",s_lang.c_str());
*/
	eString tmp = "";
	if(mode==DATA)
		tmp += "file:///";
	tmp += url_code(playList[value].Fullname) + mrlpars;
	tmp += ( mode==DATA ? " :file-caching=1000" : " :dvdread-caching=2000" );
	if(sub_color)
		tmp += " :freetype-color=0xFFFF00"; //16776960";
	return tmp;
}

eString eSCGui::url_code( eString origname )
{
//*** for national chars in names/paths ... only all > 'z' , never convert '.' => don't use httpEscape !!!
	eString pconverted;
	for (unsigned int i = 0; i < origname.length(); ++i)
	{
		int c = origname.c_str()[i];
		if(c > 'z')
			pconverted += eString().sprintf("%c%x",'%',c);
		else
			pconverted += c;
	}
	return pconverted;
}

eString eSCGui::filePos(int both, eString name, eString size, eString& text)
{
	eString restmp="";
	eString caption="Movieplayer";
	sendGetRequest(CMD, restmp);
	int l = atoi(getPar(restmp,"<length>").c_str());
	int t = atoi(getPar(restmp,"<time>").c_str());
	if(l)
	{
		if(both)
			text = eString().sprintf("%s\n\n%s: %d:%02d\t%s",name.c_str(),_("Duration"), l/60, l%60, size.c_str());
		caption += eString().sprintf(" - %s:  %d:%02d", _("position in played file"), t/60, t%60);
	}
	else
	{
		if(both)
			text = eString().sprintf("%s\n\n\t%s",name.c_str(),size.c_str());
	}
	return caption;
}

void eSCGui::changeSout() 
{
	int async = 1;
	eConfig::getInstance()->getKey((pathcfg+"async").c_str(), async );
	if(!async)
		eMoviePlayer::getInstance()->control("async", ""); // play without audio-sync

	int subtitles = 1;
	eConfig::getInstance()->getKey((pathcfg+"sub").c_str(), subtitles );
	if(!subtitles)
		eMoviePlayer::getInstance()->control("subtitles", ""); // play without subtitles

	int nsf = 0;
	eConfig::getInstance()->getKey((pathcfg+"nsf").c_str(), nsf);
	if(nsf)
		eMoviePlayer::getInstance()->control("nsf", ""); // can play file without A or V
	
	int origres = 1;
	eConfig::getInstance()->getKey((pathcfg+"origres").c_str(), origres);
	if(origres)
		eMoviePlayer::getInstance()->control("origres", ""); // streaming in original video resolution 
}

// Help message, positions of jump 0-9 are used for jump message in eventHandler !
static char *NAME[] =
{
    _("RED:\tRewind 5 percent"),
	_("GREEN:\tPlay/Resync"),
	_("YELLOW:\tPause -max cca 8s"),
	_("BLUE:\tFast forward 5 percent/Options"),
	_("OK:\tPlay/Info about played file"),
	_("1:\tRewind 15 seconds"),  
	_("2:\tGo to 20 percent"),
	_("3:\tFast forward 15 seconds"),
	_("4:\tRewind 1 minute"),
	_("5:\tGo to 50 percent"),
	_("6:\tFast forward 1 minute"),
	_("7:\tRewind 5 minutes"),
	_("8:\tGo to 80 percent"),
	_("9:\tFast forward 5 minutes"),
	_("0:\tRewind to begin"),
	_("UP/DOWN:\tNext/previous item"),
	_("EXIT:\tCancel playback"),
	_("MENU:\tOptions when is playback"),
	" "
/*	_("RED\tRewind 5 percent"),
	_("GREEN:\tPlay/Resync"),
	_("YELLOW:\tPause - max cca 8s"),
	_("BLUE:\tFast forward 5 percent/Options"),
	_("OK:\tPlay/Info about played file"),
	_("1/3:\tRewind/Forward 15 seconds"),
	_("2:\tGo to 20 percent"),
	_("4/6:\tRewind/Forward 1 minute"),
	_("5:\tGo to 50 percent"),
	_("7/9:\tRewind/Forward 5 minutes"),
	_("8:\tGo to 80 percent"),
	_("0:\tRewind to begin"),
	_("UP/DOWN:\tNext/Previous item"),
	_("EXIT:\tCancel playback"),
	_("MENU:\tOptions when is playback")
*/
};

void eSCGui::pause()
{
	if (eMoviePlayer::getInstance()->status.STAT == eMoviePlayer::PLAY)
	{
		Decoder::Pause(2); // freeze
	}
	else
	{
		Decoder::Resume(true);
	}
//	Decoder::setAutoFlushScreen(0);
//	Decoder::setAutoFlushScreen(1);
}

int eSCGui::eventHandler(const eWidgetEvent &e)
{
	int jump = 0;
	eString command  = CMD;
	eString restmp = "";

	switch (e.type)
	{		
	  	case eWidgetEvent::evtAction:
			if (e.action == &i_cursorActions->cancel)
			{
				if(infoBox)
				{
					infoBox->hide();
					delete infoBox;
					infoBox=0;
				}
				else
				{
					if (eMoviePlayer::getInstance()->status.STAT != eMoviePlayer::STOPPED)
						eMoviePlayer::getInstance()->control("stop", "");

					if (!menu)
						showMenu();
					else
					{
						// set back DVB
						if(eMoviePlayer::getInstance()->status.DVB)
					    	eMoviePlayer::getInstance()->startDVB();
						eMoviePlayer::getInstance()->control("endplg", "");
						eMoviePlayer::getInstance()->supportPlugin();
						close(0);
					}
				}
			}
			else
			if (e.action == &i_cursorActions->help)
			{
				if(!bufferingBox)
				{
					if (menu)
						hide();
    				eSCGuiHelp help;
    				help.show();
    				help.exec();
    				help.hide();
    				if (menu)
						show();
    			}
    		}
			else
			if (e.action == &i_shortcutActions->red)
			{
				if (menu)
				{
					getSavedPath();
					loadList(DATA, startdir);
				}
				else
				{
					if(!bufferingBox && eMoviePlayer::getInstance()->status.BUFFERFILLED)
					{
						command+="seek&val=%2d5%";sendGetRequest(command, restmp);
						if(!jumpBox)
						{
							jumpBox = new eMessageBox(_("Rewind 5 percent"),_("Skipping"), eMessageBox::iconInfo);
							jumpBox->show();
						}
					}
				}
			}
			else
			if (e.action == &i_shortcutActions->green)
			{
				if (menu)
				{	
					getSavedPath();
					loadList(VCD, "");
				}
				else
				{
					if(!bufferingBox)
					{
    					if (eMoviePlayer::getInstance()->status.STAT == eMoviePlayer::PLAY)
    					{
    						command+="pl_pause";
    						for( int i = 0; i < 2; i++)  // pseudo resync
    						{
    							sendGetRequest(command, restmp); //pause
    							usleep(10000);
    							sendGetRequest(command, restmp); //play
    							usleep(10000);
    						}
    						eMoviePlayer::getInstance()->control("resync", "");
    					}
    					else
    					{
    						command+="pl_pause";sendGetRequest(command, restmp);
    						pause();
    						eMoviePlayer::getInstance()->control("play", "");
    						pauseBox->hide();
    					}
    				}
    			}
			}
			else
			if (e.action == &i_shortcutActions->yellow)
			{
				if (menu)
				{	
					getSavedPath();
					loadList(DVD, "");
				}
				else
				{
    				if(!bufferingBox)
					{
						command+="pl_pause";sendGetRequest(command, restmp);
						pause();
    					if (eMoviePlayer::getInstance()->status.STAT == eMoviePlayer::PAUSE)
    					{
    						eMoviePlayer::getInstance()->control("play", "");
    						pauseBox->hide();
    					}
    					else
    					{
    						eMoviePlayer::getInstance()->control("pause", "");
    						pauseBox->show();
    					}
    				}
    			}
			}
			else
			if (e.action == &i_shortcutActions->blue)
			{
				if (menu)
				{
					hide();
					callConfig();
					show();
				}
				else
				{
					if(!bufferingBox && eMoviePlayer::getInstance()->status.BUFFERFILLED)
					{
						command+="seek&val=%2b5%";sendGetRequest(command, restmp);
						if(!jumpBox)
						{
							jumpBox = new eMessageBox(_("Fast forward 5 percent"),_("Skipping"), eMessageBox::iconInfo);
							jumpBox->show();
						}
					}
				}
			}
			else
			if (e.action == &i_shortcutActions->menu)
			{
				if(!bufferingBox)
				{
					if (menu)
						hide();
					callConfig();
					if (menu)
						show();
				}
			}
			else
			if (e.action == &i_cursorActions->up)
			{
				if (!menu && !bufferingBox)
				{
					if(infoBox)
					{
						infoBox->hide();
						delete infoBox;
						infoBox=0;
					}
					if(val<nFiles)
					{
						if (eMoviePlayer::getInstance()->status.STAT != eMoviePlayer::STOPPED)
							eMoviePlayer::getInstance()->control("stop", "");
						Start2(++val);
					}
				}
				else
					return eWindow::eventHandler(e);
			}
			else
			if (e.action == &i_cursorActions->down)
			{
				if (!menu && !bufferingBox)
				{
					if(infoBox)
					{       
						infoBox->hide();
						delete infoBox;
						infoBox=0;
					}
					if(val > 0 )
					{
						if (eMoviePlayer::getInstance()->status.STAT != eMoviePlayer::STOPPED)
							eMoviePlayer::getInstance()->control("stop", "");
						Start2(--val);
					}
				}
				else
					return eWindow::eventHandler(e);
			}
			else
			if (e.action == &i_shortcutActions->number1)
				jump = 1;
			else
			if (e.action == &i_shortcutActions->number2)
				jump = 2;
			else
			if (e.action == &i_shortcutActions->number3)
				jump = 3;
			else
			if (e.action == &i_shortcutActions->number4)
				jump = 4;
			else
			if (e.action == &i_shortcutActions->number5)
				jump = 5;
			else
			if (e.action == &i_shortcutActions->number6)
				jump = 6;
			else
			if (e.action == &i_shortcutActions->number7)
				jump = 7;
			else
			if (e.action == &i_shortcutActions->number8)
				jump = 8;
			else
			if (e.action == &i_shortcutActions->number9)
				jump = 9;
			else
			if (e.action == &i_shortcutActions->number0)
				jump = 10;
			else
				break;
	
			if (jump > 0 && !menu)
			{
				switch(jump)
				{
					// rewind relative
					case 1:	command+="seek&val=%2d15"; break;
					case 4: command+="seek&val=%2d60"; break;
					case 7: command+="seek&val=%2d5m"; break;
					// go to position in %
					case 2: command+="seek&val=20%"; break;
					case 5: command+="seek&val=50%"; break;
					case 8: command+="seek&val=80%"; break;
					//  forward relative
					case 3: command+="seek&val=%2b15"; break;
					case 6: command+="seek&val=%2b60"; break;
					case 9: command+="seek&val=%2b5m"; break;
					// rewind to begin
					case 10:command+="seek"; break;
				}
				if(!bufferingBox && eMoviePlayer::getInstance()->status.BUFFERFILLED)
				{
    				sendGetRequest(command, restmp);
    				eString tmp = eString().sprintf(NAME[jump+4],jump) ;
    				eMoviePlayer::getInstance()->control("jump", eString().sprintf("%d", jump).c_str());
    				if(!jumpBox)
    				{
						jumpBox = new eMessageBox(tmp.strReplace(":\t", " - "),_("Skipping"), eMessageBox::iconInfo);
						jumpBox->show();
    				}
    			}
			}
			return 1;
		case eWidgetEvent::evtKey:
	  	{
			if (((e.key)->flags == KEY_STATE_DOWN || (e.key)->flags == KEY_STATE_REPEAT) &&  !silver_large_rc && !menu) {
				switch ( (e.key)->code  ) {
					case RC_LEFT:
						eSCGui::volumeDown();
					break;
					case RC_RIGHT:
						eSCGui::volumeUp();
					break;
				}
			}	
		}
	   	break;
		default:
			break;
	}
	return eWindow::eventHandler(e);
}

size_t CurlDummyWrite (void *ptr, size_t size, size_t nmemb, void *data)
{
	std::string *pStr = (std::string *)data;
	*pStr += (char *)ptr;
	return size * nmemb;
}

CURLcode eSCGui::sendGetRequest (const eString& url, eString& response)  // send http commands to VLC, in response return info
{
	CURL *curl;
	CURLcode httpres;

	eString tmpurl= "http://" + VLC_IP + ":" + VLC_IF_PORT + url;
	response = "";

	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, tmpurl.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlDummyWrite);
	curl_easy_setopt(curl, CURLOPT_FILE, (void *)&response);

	if (VLC_AUTH)
		curl_easy_setopt (curl, CURLOPT_USERPWD, VLC_AUTH.c_str());
	curl_easy_setopt (curl, CURLOPT_FAILONERROR, true);

	int timeout = 5;
	eConfig::getInstance()->getKey((pathcfg+"timeout").c_str(), timeout);
//	eDebug("[VLC] Timeout: %d",timeout);
	curl_easy_setopt (curl, CURLOPT_TIMEOUT, timeout);
	httpres = curl_easy_perform (curl);
//	eDebug("[VLC] HTTP Result: %d", httpres);
	curl_easy_cleanup(curl);
	return httpres;
}

void eSCGui::getSavedPath()
{
	int vlcsrv = 0;
 	eConfig::getInstance()->getKey((pathcfg+"vlcsrv").c_str(), vlcsrv);

	eMoviePlayer::getInstance()->mpconfig.load();
	server = eMoviePlayer::getInstance()->mpconfig.getVlcCfg(vlcsrv);

	VLC_IP = server.serverIP;
	VLC_IF_PORT = server.webifPort;

	startdir = server.startDir;
	cddrive = server.CDDrive;

	if (server.vlcUser && server.vlcPass)
		VLC_AUTH = server.vlcUser + ":" + server.vlcPass;
	
	int savepath=0;
	eConfig::getInstance()->getKey((pathcfg+"savepath").c_str(), savepath);
	if(savepath)
		eConfig::getInstance()->getKey((pathcfg+"path"+eString().sprintf("%d",vlcsrv)).c_str(), startdir);
}

void eSCGui::setSavedPath(eString display)
{
	int savepath = 0;
	eConfig::getInstance()->getKey((pathcfg+"savepath").c_str(),savepath);
	if(savepath)
	{
		int vlcsrv = 0;
		eConfig::getInstance()->getKey((pathcfg+"vlcsrv").c_str(), vlcsrv);
		eConfig::getInstance()->setKey((pathcfg+"path"+eString().sprintf("%d",vlcsrv)).c_str(),display);
	}
}

void eSCGui::callConfig()
{
	int vlcsrv = 0;
 	eConfig::getInstance()->getKey((pathcfg+"vlcsrv").c_str(), vlcsrv);
	eSCGuiConfig cfg;
	cfg.show();
	cfg.exec();
	cfg.hide();
	int mode = DATA;
	eConfig::getInstance()->getKey((pathcfg+"lastmode").c_str(), mode);
	int vlcsrv1 = 0;
 	eConfig::getInstance()->getKey((pathcfg+"vlcsrv").c_str(), vlcsrv1);
	if(vlcsrv != vlcsrv1)
	{
		getSavedPath();
		if(mode == DATA)
			loadList(mode, startdir);
		else	
	    	loadList(mode, "");
	}
}

eString eSCGui::getPar(eString buf, const char* parameter)
{
/* parameter can be:
--------------------
<time>
<length>
<volume>
<state>
<position>
<state>
<random>
<loop>
<repeat> */

    eString par = "";
//	eDebug("[getPar] entry: %s",buf.c_str());
	unsigned int start = 0;
	for (unsigned int pos = buf.find('\n', 0); pos != std::string::npos; pos = buf.find('\n', start))
	{
		eString entry = ""; entry = buf.substr(start, pos - start); // one line
		if(entry.find(parameter) != eString::npos)
		{
			par = entry.strReplace(parameter,"");
//			eDebug("[getPar] parameter: %s",par.c_str());
			par = par.left(par.find_last_of('<')).strReplace(" ","");
			break;
		}
		start = pos + 1;
	}
	eDebug("[getPar] parameter: %s par: %s",parameter,par.c_str());
	return par;
}

int eSCGui::isLargeRC()
{
	int large_silver=0;	
	eString style="default";
	eConfig::getInstance()->getKey("/ezap/rc/style", style);
	if((eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7020 
	|| eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM7000
	|| eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM600PVR) 
	|| style=="7000")
		large_silver = 1;

	return large_silver;
}

void eSCGui::volumeUp()
{
	eAVSwitch::getInstance()->changeVolume(0, -2);
	if ( eMoviePlayer::getInstance()->status.STAT == eMoviePlayer::PLAY && !IBVolumeBar->isVisible() )
	{	
		volume->show();
		volumeTimer->start(2000, true);
	}
}

void eSCGui::volumeDown()
{
	eAVSwitch::getInstance()->changeVolume(0, +2);
	if ( eMoviePlayer::getInstance()->status.STAT == eMoviePlayer::PLAY && !IBVolumeBar->isVisible() )
	{
		volume->show();
		volumeTimer->start(2000, true);
	}
}

void eSCGui::hideVolumeSlider()
{
	volume->hide();
}

void eSCGui::updateVolume(int mute_state, int vol)
{
	if (mute_state)
	{
		volume->hide();
	}
	else
	{
		VolumeBar->setPerc((63-vol)*100/63);
		IBVolumeBar->setPerc((63-vol)*100/63);
	}
}

void eSCGui::init_volume_bar()
{
	eString x, y;
	volume = new eLabel( eZap::getInstance()->getDesktop( eZap::desktopFB ) );

	gPixmap *pm = NULL;
	if ((pm = eSkin::getActive()->queryImage("volume.background.pixmap")))
	{
		x = eSkin::getActive()->queryValue("volume.background.pos.x", "0");
		y = eSkin::getActive()->queryValue("volume.background.pos.y", "0");
	}
	else if ((pm = eSkin::getActive()->queryImage("volume_grafik")))
	{
		x = eSkin::getActive()->queryValue("volume.grafik.pos.x", "0");
		y = eSkin::getActive()->queryValue("volume.grafik.pos.y", "0");
	}

	if (pm)
	{
		volume->setPixmap(pm);
		volume->setProperty("position", x + ":" + y);
		volume->resize( eSize( pm->x, pm->y ) );
		volume->setPixmapPosition(ePoint(0,0));
		volume->hide();
		volume->setBlitFlags( BF_ALPHATEST );
	}

	if (eSkin::getActive()->queryValue("volume.slider.gauge", 0))
		VolumeBar = new eGauge(volume);
	else
		VolumeBar = new eProgress(volume);
	VolumeBar->setSliderPixmap(eSkin::getActive()->queryImage("volume.slider.pixmap"));

	if (eSkin::getActive()->queryValue("volume.slider.alphatest", 0))
		VolumeBar->setProperty("alphatest", "on");
	
	x = eSkin::getActive()->queryValue("volume.slider.pos.x", "0"),
	y = eSkin::getActive()->queryValue("volume.slider.pos.y", "0");
	VolumeBar->setProperty("position", x + ":" + y);

	x = eSkin::getActive()->queryValue("volume.slider.width", "0"),
	y = eSkin::getActive()->queryValue("volume.slider.height", "0");
	VolumeBar->setProperty("size", x + ":" + y);

	VolumeBar->setLeftColor( eSkin::getActive()->queryColor("volume_left") );
	VolumeBar->setRightColor( eSkin::getActive()->queryColor("volume_right") );
	VolumeBar->setBorder(0);

	VolumeBar->setDirection(eSkin::getActive()->queryValue("volume.slider.direction", 0));
	VolumeBar->setStart(eSkin::getActive()->queryValue("volume.slider.start", 0));

	ASSIGN_MULTIPLE(IBVolumeBar, eMultiProgress, "volume_bar");

	volumeTimer = new eTimer(eApp);
	CONNECT(volumeTimer->timeout, eSCGui::hideVolumeSlider );

	CONNECT(eAVSwitch::getInstance()->volumeChanged, eSCGui::updateVolume);	
}

eSCGuiHelp::eSCGuiHelp()  // Help window
{
	cmove(ePoint(90, 80));
	cresize(eSize(540, 440));

	eString rel = REL;
	setText((eString)_("Help")+" - "+rel);

	list = new eListBox<eListBoxEntryText>(this);
	list->move(ePoint(10, 10));
	list->resize(eSize(clientrect.width() - 20, clientrect.height() - 20));

//	new eListBoxEntryText(list, REL);
	int i = 0;
	while (eString(NAME[i]) != " ")
		new eListBoxEntryText(list, NAME[i++]);
}

eSCGuiConfig::eSCGuiConfig(): ePLiWindow(_("Options"), 440)  // Config window
{
    int fc = 10;
    int sc = 230;
    int l = 210;
    
	play_next = 1;
	eConfig::getInstance()->getKey((pathcfg+"playnext").c_str(), play_next );

	int Timeout = 5;
	eConfig::getInstance()->getKey((pathcfg+"timeout").c_str(), Timeout);

	int msgTime = 3;
	eConfig::getInstance()->getKey((pathcfg+"msgtime").c_str(), msgTime );

	int savepath = 0;
	eConfig::getInstance()->getKey((pathcfg+"savepath").c_str(), savepath );

	int stop_err = 0;
	eConfig::getInstance()->getKey((pathcfg+"err").c_str(), stop_err );

	int async = 1;
	eConfig::getInstance()->getKey((pathcfg+"async").c_str(), async );

	int subtitles = 1;
	eConfig::getInstance()->getKey((pathcfg+"sub").c_str(), subtitles );

	int resdvb = 0;
	eConfig::getInstance()->getKey((pathcfg+"resdvb").c_str(), resdvb );

	int nsf = 0;
	eConfig::getInstance()->getKey((pathcfg+"nsf").c_str(), nsf);
	
	int vlc8 = 0;
	eConfig::getInstance()->getKey((pathcfg+"vlc8").c_str(), vlc8 );
	
	int origres = 0;
	eConfig::getInstance()->getKey((pathcfg+"origres").c_str(), origres);
    
	int width = 0;
	eConfig::getInstance()->getKey((pathcfg+"w").c_str(), width);

	int height = 0;
 	eConfig::getInstance()->getKey((pathcfg+"h").c_str(), height);

	int input_txt = 0;
	eConfig::getInstance()->getKey((pathcfg+"input").c_str(), input_txt);
	
	eString a_lang = "   ";
    eConfig::getInstance()->getKey((pathcfg+"alang").c_str(), a_lang);

	eString s_lang = "   ";
    eConfig::getInstance()->getKey((pathcfg+"slang").c_str(), s_lang);

	int s_track = -1;
	eConfig::getInstance()->getKey((pathcfg+"strack").c_str(), s_track);

	int a_track = -1;
	eConfig::getInstance()->getKey((pathcfg+"atrack").c_str(), a_track);

	int title = 0;
	eConfig::getInstance()->getKey((pathcfg+"title").c_str(), title);

	int sub_color = 0;
	eConfig::getInstance()->getKey((pathcfg+"scolor").c_str(), sub_color);

	int scale = 0;
	eConfig::getInstance()->getKey((pathcfg+"scale").c_str(), scale);

	loadedOK=1;
	if(!load_codes())
	{
		loadedOK=0;
		input_txt=0;
	}

	lVlcSrv = new eLabel(this);
	lVlcSrv->move(ePoint(fc, yPos()));
	lVlcSrv->resize(eSize(70, widgetHeight()));
	lVlcSrv->setText(_("Server:"));
	lVlcSrv->loadDeco();

	comVlcSrv = new eComboBox(this, NR_VLC_SRV + 1, lVlcSrv);
	comVlcSrv->move(ePoint(fc + 70, yPos()));
	comVlcSrv->resize(eSize(2*l - 70, widgetHeight()));
	comVlcSrv->setHelpText(_("Select VLC server"));
	comVlcSrv->loadDeco();
	
	int vlcsrv = 0;
 	eConfig::getInstance()->getKey((pathcfg+"vlcsrv").c_str(), vlcsrv);
	int mode = DATA;
	eConfig::getInstance()->getKey((pathcfg+"lastmode").c_str(), mode);

	struct serverConfig server;
	eMoviePlayer::getInstance()->mpconfig.load();
	for(int i=0; i<= NR_VLC_SRV; i++)
	{
		server = eMoviePlayer::getInstance()->mpconfig.getVlcCfg(i);
		if (mode == DATA)
		{
			eString startDir = server.startDir;
			if(savepath)
				eConfig::getInstance()->getKey((pathcfg+"path"+eString().sprintf("%d",i)).c_str(),startDir);	
			new eListBoxEntryText(*comVlcSrv, server.serverIP + "  " + startDir, (void*)(i));
		}
		else
			new eListBoxEntryText(*comVlcSrv, server.serverIP + "  " + server.CDDrive, (void*)(i));
	}
	comVlcSrv->setCurrent((void *)vlcsrv);

	nextYPos(35);

	setResInputTxt=new eCheckbox(this, input_txt, loadedOK);
	setResInputTxt->setText(_("T/L"));
	setResInputTxt->move(ePoint(fc, yPos()));
	setResInputTxt->resize(eSize(55, widgetHeight()));
	setResInputTxt->setHelpText(_("Set audio and subtitles as track or as language"));
	CONNECT (setResInputTxt->checked, eSCGuiConfig::setCheckInputTxt);


    lAudioLang = new eLabel(this);
   	lAudioLang->move(ePoint(fc + 70, yPos()));
    lAudioLang->resize(eSize(l, widgetHeight())); 
    lAudioLang->setText((eString)_("Audio:"));
	lAudioLang->setFlags(eLabel::flagVCenter);
/*#ifdef TXT
	txtAudioLang=new eTextInputField(this,lAudioLang); 
    txtAudioLang->setMaxChars(3); 
	txtAudioLang->setText(a_lang);   	
    txtAudioLang->move(ePoint(fc + 130, yPos()));
    txtAudioLang->resize(eSize (60, widgetHeight()));
    txtAudioLang->setHelpText(_("Set audio language as ISO639"));
    txtAudioLang->loadDeco();
#else*/	
	comAudioLang = new eComboBox(this, 3, lAudioLang);
	comAudioLang->move(ePoint(fc + 130, yPos()));
    comAudioLang->resize(eSize (60, widgetHeight()));
    comAudioLang->setHelpText(_("Select audio language"));
    comAudioLang->loadDeco();
    int aitem = 0;
    for(int i = 0; i < (int)codeList.size(); i++)
    {
        new eListBoxEntryText(*comAudioLang, getCode(i).code.c_str(), (void*)i);
        if( getCode(i).code == a_lang)
            aitem = i;
    }
    comAudioLang->setCurrent((void *)aitem);
//#endif
	 
    lSubLang = new eLabel(this);
   	lSubLang->move(ePoint(sc - 20, yPos()));
    lSubLang->resize(eSize(l, widgetHeight())); 
    lSubLang->setText(_("Sub:"));
	lSubLang->setFlags(eLabel::flagVCenter);
/*#ifdef TXT
	txtSubLang=new eTextInputField(this,lSubLang); 
    txtSubLang->setMaxChars(3);    	
	txtSubLang->setText(s_lang);
    txtSubLang->move(ePoint(sc + 25 , yPos()));
    txtSubLang->resize(eSize (60, widgetHeight()));
    txtSubLang->setHelpText(_("Set subtitle language as ISO639"));
    txtSubLang->loadDeco();
#else*/	
	comSubLang = new eComboBox(this, 3, lSubLang);
	comSubLang->move(ePoint(sc + 25 , yPos()));
    comSubLang->resize(eSize (60, widgetHeight()));
    comSubLang->setHelpText(_("Select subtitle language"));
    comSubLang->loadDeco();
    int sitem = 0;
    for(int i = 0; i < (int)codeList.size(); i++)
    {
        new eListBoxEntryText(*comSubLang, getCode(i).code.c_str(), (void*)i);
        if( getCode(i).code == s_lang)
            sitem = i;
    }
    comSubLang->setCurrent((void *)sitem);
//#endif

	lAudioTrack = new eLabel(this);
	lAudioTrack->move(ePoint(fc + 70, yPos()));
	lAudioTrack->resize(eSize(l, widgetHeight()));
	lAudioTrack->setText(_("Audio:"));
	lAudioTrack->setFlags(eLabel::flagVCenter);
	lAudioTrack->loadDeco();

	comAudioTrack = new eComboBox(this, 3, lAudioTrack);
	comAudioTrack->move(ePoint(fc + 130, yPos()));
	comAudioTrack->resize(eSize (55, widgetHeight()));
	comAudioTrack->setHelpText(_("Select audio track (-1 is default)"));
	comAudioTrack->loadDeco();

	for(int time = -1; time <= 20; ++time)
	{
		new eListBoxEntryText(*comAudioTrack, eString().sprintf("%d", time), (void*)(time));
	}
	comAudioTrack->setCurrent((void *)a_track);

	lSubTrack = new eLabel(this);
	lSubTrack->move(ePoint(sc - 20, yPos()));
	lSubTrack->resize(eSize(l, widgetHeight()));
	lSubTrack->setText(_("Sub:"));
	lSubTrack->setFlags(eLabel::flagVCenter);
	lSubTrack->loadDeco();

	comSubTrack = new eComboBox(this, 3, lSubTrack);
	comSubTrack->move(ePoint(sc + 25, yPos()));
	comSubTrack->resize(eSize (55, widgetHeight()));
	comSubTrack->setHelpText(_("Select subtitle track (-1 is default)"));
	comSubTrack->loadDeco();

	for(int time = -1; time <= 20; ++time)
	{
		new eListBoxEntryText(*comSubTrack, eString().sprintf("%d", time), (void*)(time));
	}
	comSubTrack->setCurrent((void *)s_track);
	
	lTitle = new eLabel(this);
	lTitle->move(ePoint(sc + 95, yPos()));
	lTitle->resize(eSize(80, widgetHeight()));
	lTitle->setText((eString)"Title:");
	lTitle->setFlags(eLabel::flagVCenter);
	lTitle->loadDeco();

	comTitle = new eComboBox(this, 3, lTitle);
	comTitle->move(ePoint(sc + 145, yPos()));
	comTitle->resize(eSize (55, widgetHeight()));
	comTitle->setHelpText(_("Select DVD title (0 is default)"));
	comTitle->loadDeco();

	for(int time = 0; time <= 20; ++time)
	{
		new eListBoxEntryText(*comTitle, eString().sprintf("%d", time), (void*)(time));
	}
	comTitle->setCurrent((void *)title);

	nextYPos(35);

	playNext=new eCheckbox(this, play_next, 1);
	playNext->setText(_("Continuous playback"));
	playNext->move(ePoint(fc, yPos()));
	playNext->resize(eSize(l, widgetHeight()));
	playNext->setHelpText(_("Playback next file from playlist"));
	CONNECT (playNext->checked, eSCGuiConfig::setCheckStopErr);

	stopErr=new eCheckbox(this, stop_err, 1);
	stopErr->setText(_("Stop after error"));
	stopErr->move(ePoint(sc, yPos()));
	stopErr->resize(eSize(l, widgetHeight()));
	stopErr->setHelpText(_("Don't playback next file from list if previous playback failed"));

	nextYPos(35);

	savePath=new eCheckbox(this, savepath, 1);
	savePath->setText(_("Last directory path"));
	savePath->move(ePoint(fc, yPos()));
	savePath->resize(eSize(l, widgetHeight()));
	savePath->setHelpText(_("Use last path or path from .xml file"));

    resDVB=new eCheckbox(this, resdvb, 1);
	resDVB->setText(_("Restoring DVB"));
	resDVB->move(ePoint(sc, yPos()));
	resDVB->resize(eSize(l, widgetHeight()));
	resDVB->setHelpText(_("Restoring DVB after each played file"));
	CONNECT (resDVB->checked, eSCGuiConfig::setDVB);

    if (eMoviePlayer::getInstance()->status.STAT != eMoviePlayer::PLAY)
        resDVB->show();
	else
	    resDVB->hide();
	    
	nextYPos(35);

	subTitles=new eCheckbox(this, subtitles, 1);
	subTitles->setText(_("Auto Subtitles"));
	subTitles->move(ePoint(fc, yPos()));
	subTitles->resize(eSize(l - 50, widgetHeight()));
	subTitles->setHelpText(_("Use autoselection of subtitles by VLC"));

	subColor=new eCheckbox(this, sub_color, 1);
	subColor->setText(_("Yellow subtitles"));
	subColor->move(ePoint(sc, yPos()));
	subColor->resize(eSize(l, widgetHeight()));
	subColor->setHelpText(_("Colored subtitles to yellow (files)"));

    nextYPos(35);
	
	aSync=new eCheckbox(this, async, 1);
	aSync->setText(_("Audio-sync"));
	aSync->move(ePoint(fc, yPos()));
	aSync->resize(eSize(l, widgetHeight()));
	aSync->setHelpText(_("Use audio-sync parameter for playback"));

	setNsf=new eCheckbox(this, nsf, 1);
	setNsf->setText(_("Non-standard file"));
	setNsf->move(ePoint(sc, yPos()));
	setNsf->resize(eSize(l, widgetHeight()));
	setNsf->setHelpText(_("Can playback file with missing audio or video"));

	nextYPos(35);

	Scale=new eCheckbox(this, scale, 1);
	Scale->setText(_("Scale"));
	Scale->move(ePoint(fc, yPos()));
	Scale->resize(eSize(90, widgetHeight()));
	Scale->setHelpText(_("Use scale=1 (better for DM500)"));
	
	setRes=new eCheckbox(this, origres , 1);
	setRes->setText(_("Resolution"));
	setRes->move(ePoint(sc -120, yPos()));
	setRes->resize(eSize(120, widgetHeight()));
	setRes->setHelpText(_("Streaming with user setting."));
	CONNECT (setRes->checked, eSCGuiConfig::setCheckRes);

	l_width = new eLabel(this);
	l_width->setText(_("hor."));
	l_width->move(ePoint(sc+10, yPos()));
	l_width->resize(eSize(30, widgetHeight()));
	l_width->setFlags(eLabel::flagVCenter);

   	setWidth = new eNumber(this, 1, 0, 720, 3, &width, 0, l_width);
    setWidth->move(ePoint(sc + 45, yPos()));
    setWidth->resize(eSize(55, widgetHeight()));
    setWidth->setHelpText(_("Horizontal resolution for streaming (0-720). 0 for original."));
	setWidth->setFlags(eNumber::flagVCenter);
    setWidth->loadDeco();
	
    l_height = new eLabel(this);
    l_height->setText(_("ver."));
    l_height->move(ePoint(sc + 110, yPos()));
    l_height->resize(eSize(40, widgetHeight()));
	l_height->setFlags(eLabel::flagVCenter);
	
    setHeight = new eNumber(this, 1, 0, 576, 3, &height, 0, l_height);
    setHeight->move(ePoint(sc + 145, yPos()));
    setHeight->resize(eSize(55, widgetHeight()));
    setHeight->setHelpText(_("Vertical resolution for streaming (0-576). 0 for original."));
	setHeight->setFlags(eNumber::flagVCenter);
    setHeight->loadDeco();
    
    nextYPos(35);

	lNrSec = new eLabel(this);
	lNrSec->move(ePoint(fc, yPos()));
	lNrSec->resize(eSize(80, widgetHeight()));
	lNrSec->setText(_("Timeout:"));
	lNrSec->loadDeco();
	lNrSec->setFlags(eLabel::flagVCenter);

	comNrSec = new eComboBox(this, 3, lNrSec);
	comNrSec->move(ePoint(fc + 80, yPos()));
	comNrSec->resize(eSize (55, widgetHeight()));
	comNrSec->setHelpText(_("Time of testing comunication with VLC (sec)"));
	comNrSec->loadDeco();

	for(int time = 1; time <= 10; ++time)
	{
		new eListBoxEntryText(*comNrSec, eString().sprintf("%d", time), (void*)(time));
	}
	comNrSec->setCurrent((void *)Timeout);

	lmsgTime = new eLabel(this);
	lmsgTime->move(ePoint(fc + 145, yPos()));
	lmsgTime->resize(eSize(150, widgetHeight()));
	lmsgTime->setText(_("Jump message:"));
	lmsgTime->loadDeco();
	lmsgTime->setFlags(eLabel::flagVCenter);

	comMsgTime = new eComboBox(this, 3, lmsgTime);
	comMsgTime->move(ePoint(sc + 60, yPos()));
	comMsgTime->resize(eSize (55, widgetHeight()));
	comMsgTime->setHelpText(_("Time of displaying skipping message (sec)"));
	comMsgTime->loadDeco();

	for(int time = 0; time <= 10; ++time)
	{
		new eListBoxEntryText(*comMsgTime, eString().sprintf("%d", time), (void*)(time));
	}
	comMsgTime->setCurrent((void *)msgTime);

//	nextYPos(35);

    setVlc8 = new eCheckbox(this, vlc8 , 1);
	setVlc8->setText(_("v0.86"));
	setVlc8->move(ePoint(sc + 125, yPos()));
	setVlc8->resize(eSize(75, widgetHeight()));
	setVlc8->setHelpText(_("Support VLC v0.86"));

	buildWindow();
	showDVD(mode,input_txt);
    setCheckRes(origres);
    CONNECT (bOK->selected, eSCGuiConfig::saveCFG );
}

void eSCGuiConfig::saveCFG()
{
	eConfig::getInstance()->setKey((pathcfg+"playnext").c_str(),(int)playNext->isChecked() ? 1 : 0 );
	eConfig::getInstance()->setKey((pathcfg+"savepath").c_str(),(int)savePath->isChecked() ? 1 : 0);
 	eConfig::getInstance()->setKey((pathcfg+"err").c_str(),(int)stopErr->isChecked() ? 1 : 0);
 	eConfig::getInstance()->setKey((pathcfg+"timeout").c_str(),(int)comNrSec->getCurrent()->getKey());
 	eConfig::getInstance()->setKey((pathcfg+"msgtime").c_str(),(int)comMsgTime->getCurrent()->getKey());
 	eConfig::getInstance()->setKey((pathcfg+"sub").c_str(),(int)subTitles->isChecked() ? 1 : 0 );
	eConfig::getInstance()->setKey((pathcfg+"async").c_str(),(int)aSync->isChecked() ? 1 : 0);
	eConfig::getInstance()->setKey((pathcfg+"resdvb").c_str(),(int)resDVB->isChecked() ? 1 : 0);
	eConfig::getInstance()->setKey((pathcfg+"nsf").c_str(),(int)setNsf->isChecked() ? 1 : 0);
	eConfig::getInstance()->setKey((pathcfg+"vlc8").c_str(),(int)setVlc8->isChecked() ? 1 : 0);
    eConfig::getInstance()->setKey((pathcfg+"origres").c_str(),(int)setRes->isChecked() ? 1 : 0);
    eConfig::getInstance()->setKey((pathcfg+"w").c_str(),(int)setWidth->getNumber());
 	eConfig::getInstance()->setKey((pathcfg+"h").c_str(),(int)setHeight->getNumber());
	eConfig::getInstance()->setKey((pathcfg+"vlcsrv").c_str(),(int)comVlcSrv->getCurrent()->getKey());
/*#ifdef TXT
	eConfig::getInstance()->setKey((pathcfg+"slang").c_str(),txtSubLang->getText().c_str());
	eConfig::getInstance()->setKey((pathcfg+"alang").c_str(),txtAudioLang->getText().c_str());
#else*/
	if(loadedOK)
	{
		eConfig::getInstance()->setKey((pathcfg+"slang").c_str(),(getCode((int)comSubLang->getCurrent()->getKey()).code).c_str());
		eConfig::getInstance()->setKey((pathcfg+"alang").c_str(),(getCode((int)comAudioLang->getCurrent()->getKey()).code).c_str());
	}
//#endif
	eConfig::getInstance()->setKey((pathcfg+"strack").c_str(),(int)comSubTrack->getCurrent()->getKey());
	eConfig::getInstance()->setKey((pathcfg+"atrack").c_str(),(int)comAudioTrack->getCurrent()->getKey());

	eConfig::getInstance()->setKey((pathcfg+"title").c_str(),(int)comTitle->getCurrent()->getKey());
	eConfig::getInstance()->setKey((pathcfg+"input").c_str(),(int)setResInputTxt->isChecked() ? 1 : 0);
	eConfig::getInstance()->setKey((pathcfg+"scolor").c_str(),(int)subColor->isChecked() ? 1 : 0);
	eConfig::getInstance()->setKey((pathcfg+"scale").c_str(),(int)Scale->isChecked() ? 1 : 0);

	codeList.clear();
	close(0);
}

void eSCGuiConfig::setCheckRes(int status)
{
	if (status)
	{
		l_width->show();
		l_height->show();
		setWidth->show();
		setHeight->show();
	}
	else
	{
		l_width->hide();
		l_height->hide();
		setWidth->hide();
		setHeight->hide();
	}
}

void eSCGuiConfig::setCheckInputTxt(int status)
{
	if (status)
	{
		trackHide();
		langShow();
	}
	else
	{
		langHide();
		trackShow();
	}
}

void eSCGuiConfig::setCheckStopErr(int status)
{
	if (status)
		stopErr->show();
	else
		stopErr->hide();
}

void eSCGuiConfig::showDVD(int mode, int input_txt)
{
	if(mode!=DATA)
	{
		lTitle->show();
		comTitle->show();
	}
	else
	{
		lTitle->hide();
		comTitle->hide();
	}

	if(input_txt)
	{
		trackHide();
		langShow();
	}
	else
	{
		langHide();
		trackShow();
	}
}

void eSCGuiConfig::langHide()
{
/*#ifdef	TXT
	txtSubLang->hide();
	txtAudioLang->hide();
#else*/
	comSubLang->hide();
	comAudioLang->hide();
//#endif
	lSubLang->hide();
	lAudioLang->hide();
}
void eSCGuiConfig::langShow()
{
/*#ifdef TXT
	txtSubLang->show();
	txtAudioLang->show();
#else*/
	comSubLang->show();
	comAudioLang->show();
//#endif
	lSubLang->show();
	lAudioLang->show();
}
void eSCGuiConfig::trackHide()
{
/*#ifdef	TXT
	txtSubLang->hide();
	txtAudioLang->hide();
#else*/
	comSubTrack->hide();
	comAudioTrack->hide();
	lSubTrack->hide();
	lAudioTrack->hide();
//#endif
}
void eSCGuiConfig::trackShow()
{
/*#ifdef TXT
	txtSubLang->show();
	txtAudioLang->show();
#else*/
	comSubTrack->show();
	comAudioTrack->show();
	lSubTrack->show();
	lAudioTrack->show();
//#endif
}

void eSCGuiConfig::setDVB(int status)
{
	if (status)
	{
		if( eMoviePlayer::getInstance()->status.DVB )
		{
			eMoviePlayer::getInstance()->control("dvbon", "");
			eMoviePlayer::getInstance()->startDVB();
		}
	}
	else
	{
		eMoviePlayer::getInstance()->control("dvboff", "");
		eMoviePlayer::getInstance()->stopDVB();
	}
}

bool eSCGuiConfig::load_codes()
{
	XMLTreeParser parser("ISO-8859-1");

	eString file = LANGFILE1;
	FILE *in = fopen(file.c_str(), "rt");
	if (!in) 
	{
		file = LANGFILE0;
		in = fopen(file.c_str(), "rt");
		if (!in) 
		{
			eDebug("[Movieplayer] unable to open %s", file.c_str()); 
			return false;
		}
	}

	codeList.clear();
	
	bool done = false;
	while (!done)
	{
		char buf[1024]; 
		unsigned int len = fread(buf, 1, sizeof(buf), in);
		done = len < sizeof(buf);
		if (!parser.Parse(buf, len, done))
		{
			eDebug("[Movieplayer] parsing settings file: %s at line %d>", parser.ErrorString(parser.GetErrorCode()), parser.GetCurrentLineNumber());
			fclose(in);
			return false;
		}
	}

	fclose(in);

	XMLTreeNode *root = parser.RootNode();
	if (!root)
		return false;

	for (XMLTreeNode *node = root->GetChild(); node; node = node->GetNext())
	{
		if (!strcmp(node->GetType(), "language"))
		{
			eString tmplng = node->GetAttributeValue("code");
			if (!tmplng)
			{
				eDebug("[Movieplayer] parse error in mplanguages.xml");
				return false;
			}
			else
			{
				struct languages l;
				l.code = tmplng;
				codeList.push_back(l);
			}
		}
	}
	return true;
}

struct languages eSCGuiConfig::getCode(int i)
{
	struct languages code;
	
	if(i < (int)codeList.size())
		code = codeList[i];

	return code;
}

int plugin_exec(PluginParam *par)
{
	eSCGui dlg;
	eString webifVersion = getWebifVersion();
	if (webifVersion.find("Expert") != eString::npos)
	{
		eMoviePlayer::getInstance()->control("runplg", "");
		int resdvb = 1;
		eConfig::getInstance()->getKey((pathcfg+"resdvb").c_str(), resdvb );
		if(!resdvb)
		{
			eMoviePlayer::getInstance()->stopDVB();
			eMoviePlayer::getInstance()->control("dvboff", "");
		}
		dlg.show();
		dlg.exec();
		dlg.hide();
	}
	else
	{
		eMessageBox msg(eString("This plugin requires the EXPERT version of the web interface to be installed.\nInstalled web interface version: " + webifVersion), _("Error"), eMessageBox::btOK);
		msg.show();
		msg.exec();
		msg.hide();
	}
	return 0;
}
