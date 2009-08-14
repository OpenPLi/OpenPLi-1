#include <plugin.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

#include <lib/system/econfig.h>

#include "dbepg.h"
#include "channelmanager.h"
#include "retrievalwnd.h"
#include "util.h"

namespace std
{
	std::ostream& std::operator<< (std::ostream& f , const Config& cfg)
	{
		f << "name=" << cfg.getName() << std::endl;
		f << "command=" << cfg.getCommand() << std::endl;
		f << "mapfile=" << cfg.getMapFile() << std::endl;
		f << "options=" << cfg.getOptions() << std::endl;
		f << "type=" << cfg.getType() << std::endl;
		f << "numOfDays=" << cfg.getNumOfDays() << std::endl;
		f << "timeOffset=" << (cfg.getTimeOffset()*3600) << std::endl;
		f << "use_cron=" << (cfg.getCronEnabled() ? "true" : "false") << std::endl;    //not really needed to store
		f << "epgLocation=" << cfg.getEpgLocation() << std::endl;
		return f;
	}
}

// our entry point.
int plugin_exec( PluginParam *par )
{
	EpgConfigDialog setup;
	setup.show ();
	setup.exec ();
	setup.hide ();
	return 0;
}

EpgConfigDialog::EpgConfigDialog():
	ePLiWindow("EPG Configuration", 400),
	configPath(CONFIGDIR "/dbepg"),
	currentCfg(0)
{
	getConfig();
	/* abort if no valid configurations were found */
	if (configItems.size() == 0)
	{
		eMessageBox("No valid packages found. Please check your configuration.", "Error");
	}

	/* Selector for configuration */
	makeNewLabel(this, "Package", 10, yPos(), 130, widgetHeight());
	eComboBox *cb = new eComboBox(this);
	setWidgetGeom( cb, 190, yPos(), 200, widgetHeight());
	cb->setHelpText("Select the EPG provider");
	cb->loadDeco();
	cb->show();

	eListBoxEntryText *first = 0;
	for (std::map<eString, Config>::const_iterator it=configItems.begin(); it!=configItems.end(); ++it)
	{
		eListBoxEntryText *le= new eListBoxEntryText(*cb, it->second.getName());       //weird syntax....
		if (!first)
		{
			first = le;
		}
	}
	CONNECT(cb->selchanged, EpgConfigDialog::OnItemSelected);

	nextYPos(35);
	makeNewLabel(this, "Time offset", 10, yPos(), 130, widgetHeight());

	cTimeOffset = new eComboBox(this);
	setWidgetGeom( cTimeOffset, 190, yPos(), 200, widgetHeight());
	cTimeOffset->setHelpText("Select the time offset between EPG provider and your time zone");
	cTimeOffset->loadDeco();
	cTimeOffset->show();

	for (int i=-12; i<=12; i++)
	{
		char buf[30];
		sprintf(buf, "%2d hours", i);
		new eListBoxEntryText(*cTimeOffset, buf, (void*) i);       //weird syntax....
	}

	cTimeOffset->setCurrent((void*) 0); //will this work...

//     cTimeOffset = new eNumber(this, 1, -12, 12, 3, 0);
//     setWidgetGeom(cTimeOffset, 130+10+10, yPos(), 100, widgetHeight());
//     cTimeOffset->loadDeco();
//     cTimeOffset->setFlags(eNumber::flagPosNeg);
//     cTimeOffset->show();

	nextYPos(35);
	makeNewLabel(this, "Number of days", 10, yPos(), 130, widgetHeight());
	cNumOfDays = new eNumber( this, 1, 0, 365, 1, 0);
	setWidgetGeom( cNumOfDays, 190, yPos(), 200, widgetHeight() );
	cNumOfDays->setHelpText("Select number of EPG days to retrieve");
	cNumOfDays->loadDeco();
	cNumOfDays->show();

	nextYPos(35);
	makeNewLabel(this, "Cron options:", 10, yPos(), 130, widgetHeight());

//     makeNewLabel(this, "Interval", 10, yPos(), 130, widgetHeight());
//     cInterval =  new eNumber( this, 1, 1, 6, 1, 0);
//     //makeNewNumber(this, 0, 130+10+10, yPos(), 100, widgetHeight(), 6, 1, 7);
//     cInterval->setNumber(interval);
//     setWidgetGeom( cInterval, 150, yPos(), 100, widgetHeight() );
//     cInterval->loadDeco();
//     cInterval->show();

	nextYPos();
	cUseCron = makeNewCheckbox(this, "Use cron", 10, yPos(), 170, widgetHeight());
	cUseCron->setHelpText("Enable cron to retrieve EPG each day automatically");

	makeNewLabel(this, "Update at", 190, yPos(), 130, widgetHeight());
	cStartTime = new eNumber( this, 2, 0, 59, 2, 0, 0);
	setWidgetGeom( cStartTime , 310, yPos(), 80, widgetHeight() );
	cStartTime->setHelpText("Time EPG will be retreived");
	cStartTime->loadDeco();
	cStartTime->setFlags( eNumber::flagDrawPoints|eNumber::flagFillWithZeros|eNumber::flagTime );
	cStartTime->show();

	nextYPos(40);
	eButton *bt_channels = makeNewButton(this, "Channels",
		10, yPos(), 120, 40, "blue");
	bt_channels->setHelpText("Map channels");
	CONNECT(bt_channels->selected, EpgConfigDialog::OnChannelManagerClicked);

	eButton *bt_retrieve = makeNewButton(this, "Retrieve",
		270, yPos(), 120, 40, "yellow");
	bt_retrieve->setHelpText("Retrieve EPG data");
	CONNECT(bt_retrieve->selected, EpgConfigDialog::OnRetrieveClicked);

	/* Don't send the signal until gui is fully build....*/
	if (first)
	{
		cb->setCurrent(first, true);    //do send a selected message, will update view..
	}

	buildWindow();
	CONNECT(bOK->selected, EpgConfigDialog::OnOkClicked);
}

void EpgConfigDialog::OnItemSelected(eListBoxEntryText *item)
{
	if (item)
	{
		getCurrentVals();
		eString txt = item->getText();
		currentCfg = &configItems[txt];
		updateView();
	}
}

void EpgConfigDialog::updateView()
{
	if (currentCfg)
	{
		int index = currentCfg->getTimeOffset();
		if (!(index >= -12 && index <=12))
		{
			index = 0;
		}

		cTimeOffset->setCurrent((void*) index);
		//cTimeOffset->setNumber(currentCfg->getTimeOffset());
		cNumOfDays->setNumber(currentCfg->getNumOfDays() );
		cStartTime->setNumber(0, currentCfg->getStartHour());
		cStartTime->setNumber(1, currentCfg->getStartMinute());
		cUseCron->setCheck(currentCfg->getCronEnabled());
	}
}

void EpgConfigDialog::OnChannelManagerClicked()
{
	if (currentCfg)
	{
		if (currentCfg->isValid())
		{
			FileMap fm(configPath, currentCfg->getMapFile(), currentCfg->getType());
			ChannelManager cm(fm);
			hide();
			cm.show();
			cm.exec();
			cm.hide();
			show();
		}
	}
}

/*
void EpgConfigDialog::OnChangeCronClicked()
{
    if (currentCfg)
    {
        if (currentCfg->isValid())
        {
            ChangeCronDlg cronDlg;
            hide();
            cronDlg.setData(currentCfg->getStartHour(),
                            currentCfg0->getStartMinute());
            cronDlg.show();
            int result = cronDlg.exec();
            if (result)
            {
                currentCfg->setStartHour(cronDlg.getStartHour());
                currentCfg->setStartMinute(cronDlg.getStartMinute());
            }
            cronDlg.hide();
            show();
        }
    }
}*/

EpgConfigDialog::~EpgConfigDialog()
{
	// we have to do almost nothing here. all widgets are automatically removed
	// since they are child of us. the eWidget-destructor will to this for us.
}

void EpgConfigDialog::OnRetrieveClicked(void)
{
	if (currentCfg)
	{
		getCurrentVals();
		/* now ensure the configfile is up to date, before we start retrieval. Retrieve scripts might use some of our configvalues. */
		storeConfig(*currentCfg);
		if (currentCfg->isValid())
		{
			FileMap fm(configPath, currentCfg->getMapFile(), currentCfg->getType());
			RetrievalWnd w("Retrieving data, please wait...", currentCfg->getCommand(), fm.getMappedCount()); //add command to execute....
			w.show();
			w.exec();
			w.hide();
		}
	}
}

void EpgConfigDialog::getCronInfo(Config& cfg)
{
	//parse crontab entry
	//Should use crontab -l?
	std::ifstream f;
	system("crontab -l > /tmp/crontmp");
	f.open("/tmp/crontmp");
	if (f.is_open())
	{
		eString tmp;
		while (!f.eof() && getline(f, tmp))
		{
			int pos = tmp.find(cfg.getCommand().c_str());
			if (pos > 0)
			{
				cfg.setCronFound();
				int p1 = tmp.find(' ');
				int p2 = tmp.find(' ', p1);

				cfg.setStartMinute(atoi(tmp.substr(0, p1).c_str()));
				cfg.setStartHour(atoi(tmp.substr(p1+1, p2).c_str()));

				std::cout << cfg.getStartMinute() << ", " << cfg.getStartHour() << std::endl;
				break;
			}
		}

		f.close();
	}
}

void EpgConfigDialog::createCronJob(const Config& cfg)
{
	//if a cron job was found, but cronEnabled is false, remove it...
	if (!cfg.getCronEnabled())
	{
		if (cfg.isCronFound())
		{
			std::stringstream cmd;
			//Remove in existing file
			cmd << "crontab -l | grep -v '"<< cfg.getCommand() << "' | crontab -";
			system(cmd.str().c_str());
		}

		return;
	}

	//Crond warns against replacing the crontab itself. It wants us to use a crontab editor...
	if (cfg.isCronFound())
	{
		std::stringstream cmd;
		eString tmp = cfg.getCommand();
		tmp = tmp.strReplace("/", "\\/");
		//Replace in existing file
		cmd << "crontab -l | sed 's/\\([^ ]* [^ ]*\\)\\(.*" << tmp << "*\\)/" << cfg.getStartMinute() << " " << cfg.getStartHour() << "\\2/' | crontab -";
		system(cmd.str().c_str());
	}
	else
	{
		std::stringstream ss;
		//Build our cron command
		char command[128] = "";
		strncpy(command, cfg.getCommand().c_str(), 127);
		ss << cfg.getStartMinute() << " " << cfg.getStartHour() << " * * * " << cfg.getCommand() << " > /tmp/" << basename(command) << ".log 2>&1";

		std::string cron = ss.str();
		std::cout << cron << std::endl;

		//copy existing, append, and replace entire crontab file
		system("crontab -l > /tmp/crontmp ");
		std::ofstream o("/tmp/crontmp", std::ofstream::app | std::ofstream::out);
		if (o.is_open())
		{
			o << cron << std::endl;
			o.close();
			system("crontab /tmp/crontmp; rm /tmp/crontmp");
		}
	}
}

//use events from widgets to determine if values have been changed
void EpgConfigDialog::getCurrentVals()
{
	if (currentCfg)
	{
		eListBoxEntryText *l = cTimeOffset->getCurrent();
		if (l)
		{
			int hours = (int) l->getKey();
			currentCfg->setTimeOffset(hours);
		}

		currentCfg->setNumOfDays(cNumOfDays->getNumber());
		currentCfg->setStartHour(cStartTime->getNumber(0));
		currentCfg->setStartMinute(cStartTime->getNumber(1));
		currentCfg->setCronEnabled(cUseCron->isChecked() != 0);
		//     interval = 0; //cInterval->getNumber();
	}
}

void EpgConfigDialog::OnOkClicked()
{
	getCurrentVals();

	storeConfig();
	close(0);
}

void EpgConfigDialog::storeConfig()
{
	for (std::map<eString, Config>::const_iterator it=configItems.begin(); it != configItems.end(); ++it)
	{
		storeConfig(it->second);
	}
}

void EpgConfigDialog::storeConfig(const Config& cfg)
{
	if (cfg.isCronChanged())
	{
		createCronJob(cfg);
	}

	if (!cfg.isDirty())
	{
		return;
	}

	std::ofstream f;
	eString fileName = configPath + "/" + cfg.getCfgFile();
	f.open(fileName.c_str());
	if (f.is_open())
	{
        /* Should use stream operators here, and let cfg determine it's own methods...*/
//         f << "name=\"" << cfg.getName() << "\"" << std::endl;
//         f << "command=\"" << cfg.getCommand() << "\"" << std::endl;
//         f << "mapfile=\"" << cfg.getMapFile() << "\"" << std::endl;
//         f << "options=\"" << cfg.getOptions() << "\"" << std::endl;
//         f << "type=\"" << cfg.getType() << "\"" << std::endl;
//         f << "numOfDays=\"" << cfg.getNumOfDays() << "\"" << std::endl;
//         f << "timeOffset=\"" << cfg.getTimeOffset() << "\"" << std::endl;
//         f << "use_cron=\"" << (cfg.getCronEnabled() ? "true" : "false") << "\"" << std::endl;    //not really needed to store this one..
		f << cfg;
		f.close();
	}
}

int filter(const struct dirent *item)
{
	return strstr(item->d_name, ".cfg") == NULL ? 0 : 1;
}
void EpgConfigDialog::getConfig()
{
	//find all *.cfg files, and read
	struct dirent **namelist;
	int n;
	configItems.clear();
	n = scandir(configPath.c_str(), &namelist, filter, alphasort);
	if (n > 0)
	{
		while(n--)
		{
			Config cfg;
			if (getConfig(namelist[n]->d_name, cfg))
			{
				getCronInfo(cfg);
				configItems[cfg.getName()] = cfg; //places a copy --> must do getCronInfo first..
			}

			free(namelist[n]);
		}

		free(namelist);
	}
}

bool EpgConfigDialog::getConfig(const eString& file, Config& cfg)
{
	bool result = false;
	std::ifstream f;
	eString fileName = configPath + "/" + file;
	f.open(fileName.c_str());
	if (f.is_open())
	{
		//In future, make this one aware of comments, and any other text which we do not understand
		//store every line we read, modify only those we understand, and write out the lines in
		//the same order...
		cfg.setCfgFile(file);
		eString tmp;
		while (!f.eof() && getline(f, tmp))
		{
			int pos = tmp.find('=');
			if (pos > 0)
			{
				eString arg = tmp.substr(0, pos);
				eString val = tmp.substr(pos+1);
				//remove quotes
				//TODO: trim whitespace
				if (val[0] == '"')
				{
					int pos = val.find('"', 1);
					if (pos > 0)
					{
						val = val.substr(1, pos-1);
					}
				}
				if (arg == "timeOffset")
				{
					cfg.setTimeOffset(atoi(val.c_str())/3600);
				}
				else if (arg == "numOfDays")
				{
					cfg.setNumOfDays(atoi(val.c_str()));
				}
				else if (arg == "command")
				{
					cfg.setCommand(val);
				}
				else if (arg == "type")
				{
					cfg.setType(val);
				}
				else if (arg == "name")
				{
					cfg.setName(val);
				}
				else if (arg == "mapfile")
				{
					cfg.setMapFile(val);
				}
				else if (arg == "options")
				{
					cfg.setOptions(val);
				}
				else if (arg == "use_cron")
				{
					cfg.setCronEnabled(val == "true" || val == "1");
				}
				else if (arg == "epgLocation")
				{
					cfg.setEpgLocation(val);
				}
			}
		}

		cfg.clearDirtyFlag();
		cfg.clearCronChanged();
		result = cfg.isValid();
		f.close();
	}

	/* now replace the epgLocation from our configfile with the current enigma value */
	eString epgLocation = "/media/hdd";
	eConfig::getInstance()->getKey("/enigma/epgSQLiteDir", epgLocation);
	epgLocation += "/epg.db";
	cfg.setEpgLocation(epgLocation);
	return result;
}

