//
// C++ Interface: management
//
// Description: 
//
//
// Author: Ruud Nabben <r.nabben@gmail.com>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef EMANAGEMENTDIALOG_H
#define EMANAGEMENTDIALOG_H

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/combobox.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <lib/gui/enumber.h>
#include <lib/gui/echeckbox.h>


// our plugin entry point, declared to use C calling convention
extern "C" int plugin_exec( PluginParam *par );

enum tDays
{
	None = 0,
	Monday = 1,
	Tuesday = 2,
	Wednesday = 4,
	Thursday = 8,
	Friday = 16,
	Saturday = 32,
	Sunday = 64,

	All = 127
};

struct Config
{
	Config(): numOfDays(3), timeOffset(0), startHour(6), startMinute(0), dirty(false), cronFound(false), cronChanged(false), cronEnabled(false) {}
	/* Should make some accessor methods, so I won't forget to set the dirty flag... */
	const eString& getCfgFile() const  { return cfgFile; }
	const eString& getName() const { return name; }
	const eString& getCommand() const { return command; }
	const eString& getMapFile() const { return mapfile; }
	const eString& getOptions() const { return options; }
	const eString& getType() const { return type; }
	const eString& getEpgLocation() const { return epgLocation; }
	int getNumOfDays() const { return numOfDays; };
	int getTimeOffset() const { return timeOffset; }
	int getStartHour() const { return startHour; }
	int getStartMinute() const { return startMinute; }
	tDays getDays() const { return days; }
	bool getCronEnabled() const { return cronEnabled; }

	bool isCronChanged() const { return cronChanged | (cronFound != cronEnabled); }
	bool isDirty() const { return dirty; }
	bool isCronFound() const { return cronFound; }

	void clearDirtyFlag() { dirty = false; }
	void clearCronChanged() { cronChanged = false; }
	void setCfgFile(const eString& f) { cfgFile= f; }
	void setName(const eString& n) { name = n; }
	void setCommand(const eString& c) { command = c; }
	void setMapFile(const eString& m) { mapfile = m; }
	void setOptions(const eString& o) { options = o; }
	void setType(const eString& t) { type = t; }
	void setEpgLocation(const eString& location) { dirty = (dirty | (epgLocation != location)); epgLocation = location; }
	void setNumOfDays(int d) { dirty = (dirty | (numOfDays != d)); numOfDays = d; }
	void setTimeOffset(int t) { dirty = (dirty | (timeOffset != t)); timeOffset = t; }
	void setStartHour(int t) { cronChanged = (cronChanged  | (startHour != t)); startHour = t; }
	void setStartMinute(int t) { cronChanged = (cronChanged | (startMinute != t)); startMinute = t; }
	void setDays(tDays d) {cronChanged = (cronChanged | (days != d)); days = d; }
	void setCronFound() { cronFound = true; }
	void setCronEnabled(bool b) { cronChanged = (cronChanged | (cronEnabled != b)); dirty = (dirty | (cronEnabled != b)); cronEnabled = b; }

	bool isValid()
	{
		return (//(cfgFile.length() > 0) &&
			(name.length() > 0) &&
			(command.length() > 0) &&
			(mapfile.length() > 0) &&
			(type.length() > 0));
	}

	private:
		eString cfgFile;
		eString name;
		eString command;
		eString mapfile;
		eString options;
		eString type;
		eString epgLocation;
		int numOfDays;
		int timeOffset;
		/* Cron options */
		int startHour;
		int startMinute;
		tDays days;
		bool dirty;
		bool cronFound;
		bool cronChanged;
		bool cronEnabled;
};

class ProgressWindow; //forward declaration, declaration and definition in rtepg.cpp
class EpgConfigDialog:public ePLiWindow
{
	private:
		eString configPath;

		std::map<eString, Config> configItems;
		Config *currentCfg;

		eNumber *cNumOfDays, *cStartTime, *cInterval;
		eComboBox *cTimeOffset;
		eCheckbox *cUseCron;

		void OnChannelClicked();
		void OnProviderClicked();
		void OnEditPreferredClicked();

		void OnChannelManagerClicked();
		void OnRetrieveClicked();
		void OnOkClicked();

		void OnDataAvail(eString data);
		void OnAppClosed(int retVal);
		void OnProgressClosed(int retVal);

		void storeConfig();
		void storeConfig(const Config& cfg);
		void getConfig();
		bool getConfig(const eString& file, Config& cfg);

		void getCronInfo(Config& cfg);
		void createCronJob(const Config& cfg);
		void OnItemSelected(eListBoxEntryText *item);
		void updateView();
		void getCurrentVals();

	public:
		EpgConfigDialog();
		~EpgConfigDialog();
};
#endif
