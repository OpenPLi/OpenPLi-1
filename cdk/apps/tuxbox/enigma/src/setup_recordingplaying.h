#ifndef __setup_recordingplaying_h
#define __setup_recordingplaying_h

#include <lib/gui/ePLiWindow.h>
#include <lib/gui/enumber.h>
#include <lib/gui/elabel.h>
#include <lib/gui/combobox.h>
#include <lib/gui/echeckbox.h>
#include <enigma_mount.h>
#include <callablemenu.h>

class RecordingPlayingSetup : public ePLiWindow, public eCallableMenu
{
	public:
		RecordingPlayingSetup();

		/* eCallableMenu functions */
		void doMenu(eWidget* lcdTitle, eWidget* lcdElement);

	private:
		eConfig* econfig;
		eComboBox* comRecordSize;
		eCheckbox* chkAutoPlay;
		eCheckbox* chkEnableAC3Recording;
		eCheckbox* chkEnableTeletextRecording;
		eCheckbox* chkEnableTimestampDetection;
		eMountSelectionComboBox* comMyRecordings;
		
		void recordingLocationChanged(eListBoxEntryText *sel);
		void okPressed();
		void timeshiftSettingsPressed();
		void timerSettingsPressed();
};

class RecordingPlayingTimeshiftSetup : public ePLiWindow
{
	public:
		RecordingPlayingTimeshiftSetup();
		
	private:
		eConfig* econfig;
		int iTimeshiftDelay;
		int iTimeshiftTime;
		eCheckbox* chkTimeshift;
		eLabel* lblTimeshift;
		eMountSelectionComboBox* comTimeshift;
		eLabel* lblTimeshiftDelay;
		eNumber* timeshiftDelay;
		eLabel* lblTimeshiftTime;
		eNumber* timeshiftTime;	

		void timeshiftLocationChanged(eListBoxEntryText *sel);
		void timeshiftSelected();
		void okPressed();
};

class RecordingPlayingTimerSetup : public ePLiWindow
{
	public:
		RecordingPlayingTimerSetup();
		
	private:
		eConfig* econfig;
		int iTimerOffsetStart;
		int iTimerOffsetStop;	
		eNumber* timerOffsetStart;
		eNumber* timerOffsetStop;
		eComboBox* timerenddefaultaction;

		void okPressed();
};

#endif /* __setup_recordingplaying_h */
