/*
 *  PLi extension to Enigma: recording and playing settings
 *
 *  Copyright (C) 2007 dAF2000 <David@daf2000.nl>
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

#include <setup_recordingplaying.h>
#include <enigma_mount.h>
#include <media_mapping.h>
#include <enigma_main.h>
#include <lib/system/info.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/servicedvb.h>


#define MENUNAME N_("Recording and playing")
#define MENUNAME_TIMESHIFT N_("Timeshift settings")
#define MENUNAME_TIMER N_("Timer settings")

class RecordingPlayingSetupFactory : public eCallableMenuFactory
{
public:
	RecordingPlayingSetupFactory() : eCallableMenuFactory("RecordingPlayingSetup", MENUNAME) {}
	eCallableMenu *createMenu()
	{
		return new RecordingPlayingSetup;
	}
};

RecordingPlayingSetupFactory RecordingPlayingSetup_factory;

RecordingPlayingSetup::RecordingPlayingSetup()
	: ePLiWindow(_(MENUNAME), 550), econfig(eConfig::getInstance()),
	comRecordSize(0), chkAutoPlay(0), chkEnableAC3Recording(0), 
	chkEnableTeletextRecording(0), chkEnableTimestampDetection(0), comMyRecordings(0)
{
	// Recordings on...

	bool changeSettingsAllowed = eSystemInfo::getInstance()->canRecordTS() && !eDVB::getInstance()->recorder;
	if(changeSettingsAllowed)
	{
		eLabel* lblMyRecordings = new eLabel(this);
		lblMyRecordings->setText(_("Recordings on"));
		lblMyRecordings->move(ePoint(10, yPos()));
		lblMyRecordings->resize(eSize(150, widgetHeight()));

		comMyRecordings = new eMountSelectionComboBox(
			this, 4, lblMyRecordings,
			eMountSelectionComboBox::ShowDevices |
			eMountSelectionComboBox::ShowNetwork |
			eMountSelectionComboBox::ShowCustomLocation);

		comMyRecordings->move(ePoint(180, yPos()));
		comMyRecordings->resize(eSize(360, widgetHeight()));
		CONNECT(comMyRecordings->selchanged, RecordingPlayingSetup::recordingLocationChanged);

		eString mountPoint = "";
		eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaMovies, mountPoint);
	
		// Strip "/movie" to get the mountpoint
		int postfix = mountPoint.rfind("/movie");
		mountPoint = mountPoint.left(postfix);
		eDebug("RecordingPlayingSetup mountPoint '%s'", mountPoint.c_str());

		comMyRecordings->setCurrentLocation(mountPoint);

		eNetworkMountMgr* mountMgr = eNetworkMountMgr::getInstance();
		int mountIndex = mountMgr->searchMountPoint(mountPoint);
		if(mountIndex != -1)
		{
			eMountPoint mountData = mountMgr->getMountPointData(mountIndex);
			if(!mountData.isMounted())
			{
				// Current recording location is unmounted
				// Add and mark it as unmounted
				new eListBoxEntryText(
					*comMyRecordings,
					(eString)_("Not mounted :") + " " + mountData.getLongDescription(),
					(void*)0);
					
				comMyRecordings->setCurrent((void*)0);
			}
		}

		comMyRecordings->setHelpText(_("Select location for the recording medium (will record in directory 'movie')"));
		comMyRecordings->loadDeco();

		// Record split size

		nextYPos(35);

		eLabel* lblRecordSize = new eLabel(this);
		lblRecordSize->setText(_("Record split size"));
		lblRecordSize->move(ePoint(10, yPos()));
		lblRecordSize->resize(eSize(250, widgetHeight()));
	
		comRecordSize = new eComboBox(this, 3, lblRecordSize);
		comRecordSize->move(ePoint(180, yPos()));
		comRecordSize->resize(eSize(90, widgetHeight()));
		comRecordSize->setHelpText(_("Set record split size"));
		comRecordSize->loadDeco();
		comRecordSize->clear();
	
		new eListBoxEntryText(*comRecordSize, "650 MB", (void *)(650*1024));
		new eListBoxEntryText(*comRecordSize, "700 MB", (void *)(700*1024));
		new eListBoxEntryText(*comRecordSize, "800 MB", (void *)(800*1024));
		new eListBoxEntryText(*comRecordSize, "1 GB", (void *)(1024*1024));
		new eListBoxEntryText(*comRecordSize, "1,5 GB", (void *)(1536*1024));
		new eListBoxEntryText(*comRecordSize, "2 GB", (void *)(2*1024*1024));
		new eListBoxEntryText(*comRecordSize, "4 GB", (void *)(4*1024*1024));
		new eListBoxEntryText(*comRecordSize, "8 GB", (void *)(8*1024*1024));
		new eListBoxEntryText(*comRecordSize, "16 GB", (void *)(16*1024*1024));

		int splitsize=0;
		if(econfig->getKey("/extras/record_splitsize", splitsize))
		{
			splitsize=1024*1024; // 1GB
		}

		comRecordSize->setCurrent((void *)splitsize);

		nextYPos(35);

		// Enable autoplay, timestamp detection, AC3 recording, teletext recording

		int autoplay = 1;
		int enableac3recording = 0;
		int enableteletextrecording = 0;
		int enabletimestampdetection = 0;

		econfig->getKey("/ezap/extra/autoplay", autoplay);
		econfig->getKey("/enigma/noac3recording", enableac3recording);
		econfig->getKey("/enigma/nottxrecording", enableteletextrecording);
		econfig->getKey("/enigma/notimestampdetect", enabletimestampdetection);
	
		// These Enigma keys are stored as "not"
		enableac3recording = !enableac3recording;
		enableteletextrecording = !enableteletextrecording;
		enabletimestampdetection = !enabletimestampdetection;

		chkAutoPlay	= new eCheckbox(this, autoplay, 1);
		chkAutoPlay->setText(_("Enable autoplay"));
		chkAutoPlay->move(ePoint(10, yPos()));
		chkAutoPlay->resize(eSize(260, widgetHeight()));
		chkAutoPlay->setHelpText(_("Continue playing last selected movie when entering file mode"));

		chkEnableTimestampDetection= new eCheckbox(this, enabletimestampdetection, 1);
		chkEnableTimestampDetection->setText(_("Enable timestamp detection"));
		chkEnableTimestampDetection->move(ePoint(280, yPos()));
		chkEnableTimestampDetection->resize(eSize(260, widgetHeight()));
		chkEnableTimestampDetection->setHelpText(_("Detect duration from DVB timestamps when replaying recordings"));

		nextYPos(35);
		chkEnableAC3Recording= new eCheckbox(this, enableac3recording, 1);
		chkEnableAC3Recording->setText(_("Enable AC3 recording"));
		chkEnableAC3Recording->move(ePoint(10, yPos()));
		chkEnableAC3Recording->resize(eSize(260, widgetHeight()));
		chkEnableAC3Recording->setHelpText(_("Include the AC3 audio track in the recording"));

		chkEnableTeletextRecording	= new eCheckbox(this, enableteletextrecording, 1);
		chkEnableTeletextRecording->setText(_("Enable teletext recording"));
		chkEnableTeletextRecording->move(ePoint(280, yPos()));
		chkEnableTeletextRecording->resize(eSize(260, widgetHeight()));
		chkEnableTeletextRecording->setHelpText(_("Include teletext in the recording"));
	}
	else
	{
		eLabel* lblCannotChange = new eLabel(this);
		lblCannotChange->setText(_("You are recording now and cannot change all settings"));
		lblCannotChange->move(ePoint(10, yPos()));
		lblCannotChange->resize(eSize(530, widgetHeight()));
	}
	
	buildOKButton();

	if (eSystemInfo::getInstance()->canTimeshift())
	{
		eButton* timeshiftSettings = new eButton(this);
		timeshiftSettings->setText(_("Timeshift"));
		timeshiftSettings->setShortcut("yellow");
		timeshiftSettings->setShortcutPixmap("yellow");
		timeshiftSettings->move(ePoint(140, yPos()));
		timeshiftSettings->resize(eSize(130, 40) );
		timeshiftSettings->setHelpText(_("Edit the timeshift options"));
		timeshiftSettings->loadDeco();
		CONNECT(timeshiftSettings->selected, RecordingPlayingSetup::timeshiftSettingsPressed);
	}

	eButton* timerSettings = new eButton(this);
	timerSettings->setText(_("Timer"));
	timerSettings->setShortcut("blue");
	timerSettings->setShortcutPixmap("blue");
	timerSettings->move(ePoint(280, yPos()));
	timerSettings->resize(eSize(130, 40) );
	timerSettings->setHelpText(_("Edit the recording timer options"));
	timerSettings->loadDeco();
	CONNECT(timerSettings->selected, RecordingPlayingSetup::timerSettingsPressed);
	
	buildWindow();
	CONNECT (bOK->selected, RecordingPlayingSetup::okPressed);
}

void RecordingPlayingSetup::recordingLocationChanged(eListBoxEntryText *sel)
{
	comMyRecordings->locationChanged(sel);
}

void RecordingPlayingSetup::okPressed()
{
	if(comRecordSize) econfig->setKey("/extras/record_splitsize", (int)comRecordSize->getCurrent()->getKey());
	if(chkAutoPlay) econfig->setKey("/ezap/extra/autoplay", chkAutoPlay->isChecked() ? 1 : 0);
	if(chkEnableAC3Recording) econfig->setKey("/enigma/noac3recording", chkEnableAC3Recording->isChecked() ? 0 : 1);
	if(chkEnableTeletextRecording) econfig->setKey("/enigma/nottxrecording", chkEnableTeletextRecording->isChecked() ? 0 : 1);
	if(chkEnableTimestampDetection) econfig->setKey("/enigma/notimestampdetect", chkEnableTimestampDetection->isChecked() ? 0 : 1);

	if(comMyRecordings && (int)comMyRecordings->getCurrent()->getKey() != 0)
	{
		eString mountPoint = comMyRecordings->getCurrentLocation() + "/movie";
		eMediaMapping::getInstance()->setStorageLocation(eMediaMapping::mediaMovies, mountPoint);
		eDebug("RecordingPlayingSetup::okPressed new location = %s", mountPoint.c_str());	
	}

	close(0);
}

void RecordingPlayingSetup::timeshiftSettingsPressed()
{
	RecordingPlayingTimeshiftSetup timeshiftSetup;
	timeshiftSetup.show();
	timeshiftSetup.exec();
	timeshiftSetup.hide();
}

void RecordingPlayingSetup::timerSettingsPressed()
{
	RecordingPlayingTimerSetup timerSetup;
	timerSetup.show();
	timerSetup.exec();
	timerSetup.hide();
}

void RecordingPlayingSetup::doMenu(eWidget* lcdTitle, eWidget* lcdElement)
{
	setLCD(lcdTitle, lcdElement);
	show();
	exec();
	hide();
}

RecordingPlayingTimeshiftSetup::RecordingPlayingTimeshiftSetup()
	: ePLiWindow(_(MENUNAME_TIMESHIFT), 550), econfig(eConfig::getInstance())
{
	int timeshift = 0;
	econfig->getKey("/enigma/timeshift/permanent", timeshift);

	chkTimeshift= new eCheckbox(this, timeshift, 1);
	chkTimeshift->setText(_("Enable timeshift"));
	chkTimeshift->move(ePoint(10, yPos()));
	chkTimeshift->resize(eSize(530, widgetHeight()));
	chkTimeshift->setHelpText(_("Enable permanent recording. You will always record and can zap to other channels meanwhile"));
	CONNECT(chkTimeshift->selected, RecordingPlayingTimeshiftSetup::timeshiftSelected);

	nextYPos(35);
	lblTimeshift = new eLabel(this);
	lblTimeshift->setText(_("Timeshifts on"));
	lblTimeshift->move(ePoint(10, yPos()));
	lblTimeshift->resize(eSize(150, widgetHeight()));

	comTimeshift = new eMountSelectionComboBox(
		this, 4, lblTimeshift,
		eMountSelectionComboBox::ShowDevices |
		eMountSelectionComboBox::ShowNetwork |
		eMountSelectionComboBox::ShowCustomLocation);

	comTimeshift->move(ePoint(220, yPos()));
	comTimeshift->resize(eSize(320, widgetHeight()));
	CONNECT(comTimeshift->selchanged, RecordingPlayingTimeshiftSetup::timeshiftLocationChanged);

	eString mountPoint = "";
	eMediaMapping::getInstance()->getStorageLocation(eMediaMapping::mediaTimeshifts, mountPoint);
	
	// Strip "/timeshift" to get the mountpoint
	int postfix = mountPoint.rfind("/timeshift");
	mountPoint = mountPoint.left(postfix);
	eDebug("RecordingPlayingTimeshiftSetup mountPoint '%s'", mountPoint.c_str());

	comTimeshift->setCurrentLocation(mountPoint);

	eNetworkMountMgr* mountMgr = eNetworkMountMgr::getInstance();
	int mountIndex = mountMgr->searchMountPoint(mountPoint);
	if(mountIndex != -1)
	{
		eMountPoint mountData = mountMgr->getMountPointData(mountIndex);
		if(!mountData.isMounted())
		{
			// Current recording location is unmounted
			// Add and mark it as unmounted
			new eListBoxEntryText(
				*comTimeshift,
				(eString)_("Not mounted :") + " " + mountData.getLongDescription(),
				(void*)0);
					
			comTimeshift->setCurrent((void*)0);
		}
	}

	comTimeshift->setHelpText(_("Select location for the recording medium (will record in directory 'timeshift')"));
	comTimeshift->loadDeco();

	nextYPos(35);
	iTimeshiftDelay = 30;
	iTimeshiftTime = 30;

	econfig->getKey("/enigma/timeshift/permanentdelay", iTimeshiftDelay);
	econfig->getKey("/enigma/timeshift/permanentminutes", iTimeshiftTime); 

	lblTimeshiftDelay = new eLabel(this);
	lblTimeshiftDelay->setText(_("Timeshift delay"));
	lblTimeshiftDelay->move(ePoint(10, yPos()));
	lblTimeshiftDelay->resize(eSize(250, widgetHeight()));

	timeshiftDelay = new eNumber(this, 1, 1, 999, 3, &iTimeshiftDelay, 0, lblTimeshiftDelay);
	timeshiftDelay->move(ePoint(220, yPos()));
	timeshiftDelay->resize(eSize(50, widgetHeight()));
	timeshiftDelay->setHelpText(_("Number of seconds the recording is delayed after switching channel"));
	timeshiftDelay->loadDeco();

	nextYPos(35);
	lblTimeshiftTime = new eLabel(this);
	lblTimeshiftTime->setText(_("Timeshift recording time"));
	lblTimeshiftTime->move(ePoint(10, yPos()));
	lblTimeshiftTime->resize(eSize(250, widgetHeight()));

	timeshiftTime = new eNumber(this, 1, 1, MAX_PERMANENT_TIMESHIFT_MINUTES, 3, &iTimeshiftTime, 0, lblTimeshiftTime);
	timeshiftTime->move(ePoint(220, yPos()));
	timeshiftTime->resize(eSize(50, widgetHeight()));
	timeshiftTime->setHelpText(_("Maximum number of recorded minutes"));
	timeshiftTime->loadDeco();

	timeshiftSelected();
	buildWindow();
	CONNECT(bOK->selected, RecordingPlayingTimeshiftSetup::okPressed);
}

void RecordingPlayingTimeshiftSetup::timeshiftSelected()
{
	if(chkTimeshift->isChecked())
	{
		lblTimeshift->show();
		comTimeshift->show();
		lblTimeshiftDelay->show();
		timeshiftDelay->show();
		lblTimeshiftTime->show();
		timeshiftTime->show();
	}
	else
	{
		lblTimeshift->hide();
		comTimeshift->hide();
		lblTimeshiftDelay->hide();
		timeshiftDelay->hide();
		lblTimeshiftTime->hide();
		timeshiftTime->hide();
	}
}

void RecordingPlayingTimeshiftSetup::timeshiftLocationChanged(eListBoxEntryText *sel)
{
	comTimeshift->locationChanged(sel);
}


void RecordingPlayingTimeshiftSetup::okPressed()
{
	int tmp = 0;
	econfig->getKey("/enigma/timeshift/permanent", tmp);
	unsigned char permactive = (unsigned char)tmp;

	econfig->setKey("/enigma/timeshift/permanent", chkTimeshift->isChecked() ? 255 : 0);
	econfig->setKey("/enigma/timeshift/permanentdelay", timeshiftDelay->getNumber());
	econfig->setKey("/enigma/timeshift/permanentminutes", timeshiftTime->getNumber());

	if(permactive && !chkTimeshift->isChecked())
	{
		eZapMain::getInstance()->stopPermanentTimeshift();
		eDebug("RecordingPlayingTimeshiftSetup::okPressed timeshifting disabled");
	}
	else if(!permactive && chkTimeshift->isChecked())
	{
		eZapMain::getInstance()->beginPermanentTimeshift();
		eDebug("RecordingPlayingTimeshiftSetup::okPressed timeshifting enabled");
	}
	
	if((int)comTimeshift->getCurrent()->getKey() != 0)
	{
		eString mountPoint = comTimeshift->getCurrentLocation() + "/timeshift";
		eMediaMapping::getInstance()->setStorageLocation(eMediaMapping::mediaTimeshifts, mountPoint);
		eDebug("RecordingPlayingTimeshiftSetup::okPressed new location = %s", mountPoint.c_str());	
	}

	close(0);
}

RecordingPlayingTimerSetup::RecordingPlayingTimerSetup()
	: ePLiWindow(_(MENUNAME_TIMER), 430), econfig(eConfig::getInstance())
{
	iTimerOffsetStart = 0;
	iTimerOffsetStop = 0;

	econfig->getKey("/enigma/timeroffsetstart", iTimerOffsetStart);
	econfig->getKey("/enigma/timeroffsetstop", iTimerOffsetStop); 

	eLabel* timerOffsetStart_label = new eLabel(this);
	timerOffsetStart_label->setText(_("Timer start offset"));
	timerOffsetStart_label->move(ePoint(10, yPos()));
	timerOffsetStart_label->resize(eSize(250, widgetHeight()));

	timerOffsetStart = new eNumber(this, 1, 0, 60, 2, &iTimerOffsetStart, 0, timerOffsetStart_label);
	timerOffsetStart->move(ePoint(370, yPos()));
	timerOffsetStart->resize(eSize(50, widgetHeight()));
	timerOffsetStart->setHelpText(_("Time in minutes a timer event starts earlier (0-60)"));
	timerOffsetStart->loadDeco();

	nextYPos(35);

	eLabel* timerOffsetStop_label = new eLabel(this);
	timerOffsetStop_label->setText(_("Timer stop offset"));
	timerOffsetStop_label->move(ePoint(10, yPos()));
	timerOffsetStop_label->resize(eSize(250, widgetHeight()));

	timerOffsetStop = new eNumber(this, 1, 0, 60, 2, &iTimerOffsetStop, 0, timerOffsetStop_label);
	timerOffsetStop->move(ePoint(370, yPos()));
	timerOffsetStop->resize(eSize(50, widgetHeight()));
	timerOffsetStop->setHelpText(_("Time in minutes a timer event finishes later (0-60)"));
	timerOffsetStop->loadDeco();

	nextYPos(35);

	eLabel* lblDefaultAction = new eLabel(this);
	lblDefaultAction->setText(_("Default action on timer end"));
	lblDefaultAction->move(ePoint(10, yPos()));
	lblDefaultAction->resize(eSize(260, widgetHeight()));

	timerenddefaultaction = new eComboBox(this, 3, lblDefaultAction);
	timerenddefaultaction->move(ePoint(270, yPos()));
	timerenddefaultaction->resize(eSize(150, widgetHeight()));
	timerenddefaultaction->setHelpText(_("Set default action on timer end"));
	timerenddefaultaction->loadDeco();
	timerenddefaultaction->clear();

	new eListBoxEntryText(*timerenddefaultaction, _("Nothing"), (void *)(0));
	new eListBoxEntryText(*timerenddefaultaction,  _("Standby"), (void *)(ePlaylistEntry::doGoSleep));

	if(eSystemInfo::getInstance()->canShutdown())
	{
		new eListBoxEntryText(*timerenddefaultaction, _("Shutdown"), (void *)(ePlaylistEntry::doShutdown));
	}

	int defaultaction = 0;
	if(econfig->getKey("/enigma/timerenddefaultaction", defaultaction))
	{
		defaultaction = 0;
	}
	timerenddefaultaction->setCurrent((void*)defaultaction);

	buildWindow();
	CONNECT(bOK->selected, RecordingPlayingTimerSetup::okPressed);
}

void RecordingPlayingTimerSetup::okPressed()
{
	econfig->setKey("/enigma/timerenddefaultaction", (int)timerenddefaultaction->getCurrent()->getKey());
	econfig->setKey("/enigma/timeroffsetstart", timerOffsetStart->getNumber());
	econfig->setKey("/enigma/timeroffsetstop", timerOffsetStop->getNumber());

	close(0);
}


