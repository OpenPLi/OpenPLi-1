#ifndef __enigma_main_h
#define __enigma_main_h

#include <src/sselect.h>
#include <src/enigma_lcd.h>
#include <src/engrab.h>
#include <src/rds_text.h>
#include <lib/base/message.h>
#include <lib/base/console.h>
#include <lib/dvb/si.h>
#include <lib/dvb/dvb.h>
#include <lib/dvb/edvb.h>
#include <lib/dvb/dvbservice.h>
#include <lib/gui/combobox.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/multipage.h>
#include <lib/gui/textinput.h>
#include <lib/gui/emessage.h>
#include <lib/gui/numberactions.h>
#include <lib/dvb/service.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/egauge.h>
#include <src/audio_dynamic.h>
#include <lib/dvb/subtitling.h>
#include <math.h>
#include <src/epgsearch.h>  // EPG search
extern eString getTimeStr(tm *timem, int flags);
#ifndef DISABLE_FILE
extern int freeRecordSpace(bool ofParent = false);
#endif

#define gTS_SECS	0x01
#define gTS_SHORT	0x02
#define gTS_NOAPM	0x04

class eProgress;

struct eMMIMessage
{
#ifndef DISABLE_CI
	eDVBCI *from;
#else
	int *from;
#endif
	char *data;
	int len;
	eMMIMessage()
	{
	}
#ifndef DISABLE_CI
	eMMIMessage(eDVBCI *from, char* data, int len)
#else
	eMMIMessage(int *from, char* data, int len)
#endif
		: from(from), data(data), len(len)
	{
	}
};

class eLabel;
class EIT;
class SDT;
class SDTEntry;
class PMT;
class PMTEntry;
class eNumber;
class eCheckbox;
class eButton;
class gPainter;
class NVODReferenceEntry;
class LinkageDescriptor;
class eServiceSelector;
class eRecordingStatus;
class ePlaylistEntry;
class eHelpWindow;
class eEPGWindow;

class eZapMessage
{
	int type;
	eString caption, body;
	int timeout;
	int icon;
public:
	eZapMessage(int type, const eString &caption, const eString &body, int timeout=0, int icon=-1)
		: type(type), caption(caption), body(body), timeout(timeout), icon(icon)
	{
	}

	eZapMessage::eZapMessage()
	{
	}

	eZapMessage::eZapMessage(int type)
		: type(type)
	{
	}

	const eString & getCaption() const { return caption; }
	const eString & getBody() const { return body; }
	const int getIcon() const { return icon; }
	void setTimeout(int _timeout)
	{
		timeout=_timeout;
	}
	int getTimeout() const
	{
		return timeout;
	}
	bool isSameType(const eZapMessage &msg) const
	{
		return type == msg.type;
	}
	bool isEmpty() const
	{
		return ! (caption.length() || body.length());
	}
};

#ifndef DISABLE_FILE

class eZapSeekIndices
{
private:
	struct Index
	{
		int time;
		int type;
	};
	eString filename;
	std::map<int, Index> index;
	int changed;
	int length;
public:
	eZapSeekIndices();
	void load(const eString &filename);
	void save();
	void add(int real, int time, int type = 0);
	void remove(int real);
	void removeType(int type);
	void clear();
	/**
	 * \brief retrieves next index.
	 * \param dir 0 for nearest, <0 for prev or >0 for next. returns -1 if nothing found.
	 */
	int getNext(int, int dir); 
	int getTime(int real);
	
	void setTotalLength(int length);
	int  getTotalLength();
};

class eProgressWithIndices: public eProgress
{
	eZapSeekIndices *indices;
	gColor indexmarkcolor;
public:
	eProgressWithIndices(eWidget *parent);
	void redrawWidget(gPainter *target, const eRect &area);
	void setIndices(eZapSeekIndices *indices);
};

class eMultiProgressWithIndices : public eMultiProgress
{
public:
	void setIndices(eZapSeekIndices *indices)
	{
		unsigned int i;
		for (i = 0; i < size(); i++)
		{
			((eProgressWithIndices*)(*this)[i])->setIndices(indices);
		}
	}
};

#endif

class NVODStream: public eListBoxEntryTextStream
{
	friend class eListBox<NVODStream>;
	friend class eNVODSelector;
	int begTime;
	const eString &redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
	void EITready(int error);
	int validate();
	void selfDestroy();
	Signal0<void> ready;
	bool operator<( const eListBoxEntry& e ) const
	{
		return begTime < ((NVODStream&)e).begTime;
	}
public:
	int getBegTime() const { return begTime; }
	NVODStream(eListBox<NVODStream> *listbox, eDVBNamespace dvb_namespace, const NVODReferenceEntry *ref, int type);
	eServiceReferenceDVB service;
	EIT eit;
};

class eNVODSelector: public eListBoxWindow<NVODStream>
{
	int count;
	void selected(NVODStream *);
	Signal0<void> clearEntrys;
	void readyCallBack();
public:
	eNVODSelector();
	void clear();
	void add(eDVBNamespace dvb_namespace, NVODReferenceEntry *ref);
};

class AudioStream: public eListBoxEntryText
{
	friend class eListBox<AudioStream>;
	friend class eAudioSelector;
public:
	eDVBServiceController::audioStream stream;
	void update();
	AudioStream(eListBox<AudioStream> *listbox, eDVBServiceController::audioStream &stream);
	bool operator < ( const AudioStream& e) const	{	return 0;	}
};

class eAudioSelector: public eListBoxWindow<AudioStream>
{
	void selected(AudioStream *);
	eAudioDynamicConfig *m_dyncfg;
	int eventHandler(const eWidgetEvent &);
	eListBox<eListBoxEntryText> *m_subtitles, *m_stereo_mono;
	void subtitleSelected(eListBoxEntryText *entry);
	void init_eAudioSelector();
public:
	eAudioSelector();
	void clear();
	void update(std::list<eDVBServiceController::audioStream>&);
	void add(eDVBServiceController::audioStream &pmt);
	void addSubtitle(const PMTEntry *entry);
	void addTTXSubtitles();
};

class ePSAudioSelector: public eListBoxWindow<eListBoxEntryText>
{
	void selected(eListBoxEntryText *);
	int eventHandler(const eWidgetEvent &);	
	eAudioDynamicConfig *m_dyncfg;
	eListBox<eListBoxEntryText> *m_stereo_mono;
	void init_ePSAudioSelector();
public:
	int getCount() { return list.getCount(); }
	ePSAudioSelector();
	void add(unsigned int id);
	void clear();
};

class eVideoSelector: public eListBoxWindow<eListBoxEntryText>
{
	void selected(eListBoxEntryText *);
	int eventHandler(const eWidgetEvent &);
public:
	eVideoSelector();
	void clear();
	void add(PMTEntry *);
};

class SubService: public eListBoxEntryText
{
	friend class eListBox<SubService>;
	friend class eSubServiceSelector;
public:
	SubService(eListBox<SubService> *listbox, eDVBNamespace dvb_namespace, const LinkageDescriptor *descr);
	eServiceReferenceDVB service;
};

class eSubServiceSelector
: public eListBoxWindow<SubService>
{
	int quickzap;
	void selected(SubService *);
	eButton *bAddToUserBouquet, *bToggleQuickZap;
	virtual void willShow();
	void addPressed();
	void quickZapPressed();
	void init_eSubServiceSelector(bool showbuttons);
public:
	Signal2<void, eServiceReference*, int> addToUserBouquet;
	bool quickzapmode();
	void prev();
	void next();
	void play();
	eSubServiceSelector(bool showbuttons=true);
	eServiceReferenceDVB *getSelected() { return list.getCount()?&list.getCurrent()->service:0; }
	void clear();
	void selectCurrent();
	void disableQuickZap();
	void add(eDVBNamespace dvb_namespace, const LinkageDescriptor *ref);
};

class eServiceNumberWidget: public eWindow
{
	eNumber *number;
	int chnum;
	eTimer *timer;
	void init_eServiceNumberWidget(int initial);
private:
	void selected(int*);
	void timeout();
	void numberChanged();
public:
	eServiceNumberWidget(int initial);
	~eServiceNumberWidget();
};

#define ENIGMA_NVOD		1	
#define ENIGMA_AUDIO	2
#define ENIGMA_SUBSERVICES 4
#define ENIGMA_VIDEO	8
#define ENIGMA_AUDIO_PS 16

class eEventDisplay;
class eServiceEvent;

class ePlaylist;

class TextEditWindow: public eWindow
{
	eTextInputField *input;
	eTextInputFieldHelpWidget *image;
	eLabel *descr;
	int eventHandler( const eWidgetEvent &e );
	void init_TextEditWindow( const char *InputFieldDescr, const char* useableChars );
public:
	TextEditWindow( const char *InputFieldDescr, const char* useableChar=0 );
	const eString& getEditText() { return input->getText(); }
	void setEditText( const eString& str ) { input->setText( str ); }
	void setMaxChars( int maxChars ) { input->setMaxChars( maxChars ); }
};

#ifndef DISABLE_FILE
class SkipEditWindow: public eWidget
{
	eTextInputField *input;
	eLabel *description;
	int eventHandler( const eWidgetEvent &e );
public:
	SkipEditWindow( const char *InputFieldDescr);
	const eString& getEditText() { return input->getText(); }
	void setEditText( const eString& str ) { input->setText( str ); }
	void setMaxChars( int maxChars ) { input->setMaxChars( maxChars ); }
};
#endif

class UserBouquetSelector: public eListBoxWindow<eListBoxEntryText>
{
	std::list<ePlaylistEntry> &SourceList;
	void selected( eListBoxEntryText * );
public:
	UserBouquetSelector( std::list<ePlaylistEntry> &list );
	eServiceReference curSel;
};

class eZapMain: public eWidget
{
	friend class eEPGSelector;
	friend class eWizardScanInit;
	friend class eSubServiceSelector;
public:
	enum { modeTV, modeRadio, modeFile, modeEnd };
	enum { stateSleeping=2, stateInTimerMode=4, stateRecording=8, recDVR=16, recVCR=32, recNgrab=64, statePaused=128, recPermanentTimeshift=256 };
	enum { messageGoSleep=2, messageShutdown, messageNoRecordSpaceLeft, messageWakeUp, messageCheckVCR };
	enum { pathBouquets=1, pathProvider=2, pathRecordings=4, pathPlaylist=8, pathAll=16, pathRoot=32, pathSatellites=64 };
	enum { listAll, listSatellites, listProvider, listBouquets };
private:
	eMultiLabel *lfreq_val, *lsymrate_val, *lpolar_val, *lfec_val,
		*SoftCam, *SoftcamInfo, *VidFormat, *fileinfos;
	eLabel 	*lsnr_num, *lsync_num, *lber_num, *lsnr_text, *lagc_text, *lber_text,
		*Description,
		*ButtonRedEn, *ButtonRedDis,
		*ButtonGreenEn, *ButtonGreenDis,
		*ButtonYellowEn, *ButtonYellowDis,
		*ButtonBlueEn, *ButtonBlueDis,
		*DolbyOn, *DolbyOff, *CryptOn, *CryptOff, *WideOn, *WideOff, *VtxtOn, *VtxtOff, *AudioOn, *AudioOff, *recstatus, *recchannel,
		mute, volume,
		*IrdetoEcm, *SecaEcm, *ViaEcm, *CWEcm, *NagraEcm, *NDSEcm, *ConaxEcm, *BetaEcm, *PowerVuEcm, *DreamCrEcm, *RusCrEcm, *IceCrEcm, *CodiCrEcm,
		*IrdetoNo, *SecaNo, *ViaNo, *CWNo, *NagraNo, *NDSNo, *ConaxNo, *BetaNo, *PowerVuNo, *DreamCrNo, *RusCrNo, *IceCrNo, *CodiCrNo,
		*IrdetoUca, *SecaUca, *ViaUca, *CWUca, *NagraUca, *NDSUca, *ConaxUca, *BetaUca, *PowerVuUca, *DreamCrUca, *RusCrUca, *IceCrUca, *CodiCrUca,
		*DVRSpaceLeft, *YellowButtonDesc, *BlueButtonDesc, *GreenButtonDesc;

	eMultiWidget *dvbInfoBar, *fileInfoBar, *dvrInfoBar;

	eMultiLabel *Clock, *Date, *ChannelNumber, *ChannelName, *ProviderName, *EINow, *EINext, *EINowDuration, *EINextDuration, *EINowTime, *EINextTime, *EINextETime;
	eMultiWidget *OSDExtra, *OSDVerbose, *miniZap, *maxiZap;

	eSubtitleWidget *subtitle;
	eGauge *aHour, *aMins, *aSecs;

	RDSTextDecoder rdstext_decoder;
	eString previousPlugin;

	int dvrfunctions;
	int stateOSD; // 0 = hidden, 1 = mainzap is (becoming) visible, 2 = minizap is (becoming) visible
	int timeoutInfobar;
	int showInfobarOnZap;
	char softcamName[128];
	int currentcaid;
	char softcamInfo[128];
	bool validEITReceived;

	// eRecordingStatus *recstatus;

#ifndef DISABLE_FILE
	eMultiProgressWithIndices *Progress;
#else
	eMultiProgress *Progress;
#endif
	eProgress *p_snr, *p_agc, *p_ber;
	eProgress *VolumeBar; /* this is the standard volumebar, in it's own widget */
	eMultiProgress *IBVolumeBar; /* these are all custom volume bars/gauges, integrated in the OSD widgets */
	eMessageBox *pMsg, *pRotorMsg;

	eLock messagelock;
	std::list<eZapMessage> messages;
	eFixedMessagePump<int> message_notifier;
//#ifndef DISABLE_CI
	eFixedMessagePump<eMMIMessage> mmi_messages;
//#endif
	eFixedMessagePump<eEPGCache::Message> epg_messages;

	eTimer timeout, clocktimer, messagetimeout,
					progresstimer, volumeTimer, recStatusBlink,
					doubleklickTimer, unusedTimer, permanentTimeshiftTimer, epgNowNextTimer;
/* SNR,AGC,BER DISPLAY */
	eTimer *snrTimer;
/* SNR,AGC,BER DISPLAY */

	Connection doubleklickTimerConnection;

	int cur_start, cur_duration, cur_event_id;
	eString cur_event_text;

	eNVODSelector nvodsel;
	eSubServiceSelector subservicesel;
	ePSAudioSelector audioselps;
	eAudioSelector audiosel;
	eVideoSelector videosel;
	eEventDisplay *actual_eventDisplay;
	eServiceReferenceDVB refservice;

	ePlaylist *playlist; // history / current playlist entries
	eServiceReference playlistref;

#ifndef DISABLE_FILE
	ePlaylist *recordings;
	eServiceReference recordingsref;
#endif

	ePlaylist *userTVBouquets;
	eServiceReference userTVBouquetsRef;

	ePlaylist *userRadioBouquets;
	eServiceReference userRadioBouquetsRef;

	ePlaylist *userFileBouquets;
	eServiceReference userFileBouquetsRef;

	ePlaylist *currentSelectedUserBouquet; // in addToFavourite Mode...
	eServiceReference currentSelectedUserBouquetRef;

	eString eplPath; // where we store Favlists? user changeable...

	ePlaylist *addUserBouquet( eServiceReference&, ePlaylist*, const eString&, const eString&, eServiceReference &, bool );

	int timeshift;
	int playlistmode; // curlist is a list controlled by the user (not just a history).
	int entered_playlistmode;

	int flags;
	int isVT;
	int isEPG;
	int is16_9;
	int isAC3;
	int isCrypted;
	int showOSDOnEITUpdate;
	int serviceFlags;
	int isSeekable() const { return serviceFlags & eServiceHandler::flagIsSeekable; }
//#ifndef DISABLE_LCD
	eZapLCD lcdmain;
//#endif
	void eraseBackground(gPainter *, const eRect &where);
	void setEIT(EIT *);
	void epgNowNextRefresh();
	int setEPGNowNext();
	void setNow(EITEvent *event);
	void setNext(EITEvent *event);
	int setEITcache();
	void handleNVODService(SDTEntry *sdtentry);

	// actions
	void init_main();
	void exit_main();
public:
	void showServiceSelector(int dir, int newTarget=0 );
#ifndef DISABLE_FILE
	void play();
	void stop();
	void pause();
	void record();
	void startSkip(int dir);
	void repeatSkip(int dir);
	void stopSkip(int dir);
	void endSkip(void);
	void skipLoop();
	enum { skipForward, skipReverse };
	int isRecording() {return state & stateRecording;}
	int isRecordingPermanentTimeshift() {return state & recPermanentTimeshift;}
	eString RecordingChannel() {return state & stateRecording ? recchannel->getText() : "";}
	int dvrActive(void) {return dvrfunctions;}
#endif
	int isSleeping() {return state & stateSleeping;}
	void hideInfobar();
private:
	void nextService(int add=0);
	void prevService();
	void playlistNextService();
	void volumeUp();
	void volumeDown();
	void hideVolumeSlider();
	void toggleMute();
	void showMainMenu();
	eLabel *EPGUpdateInfo;  				// EPG search
	eWidget *EPGRefresherWindow; 				// EPG search
	void EPGSearchEvent(eServiceReferenceDVB service); 	// EPG search
/* SNR,AGC,BER DISPLAY */
	void showSNR();
/* SNR,AGC,BER DISPLAY */
	void showFreq();
/* CA ECM DISPLAY */
	void showECM(int usedcaid);
/* CA ECM DISPLAY */
	timeval standbyTime;
	int standby_nomenu;

	void standbyPress(int nomenu);
	void standbyRepeat();
	void standbyRelease();
	void showSubserviceMenu();
	void showAudioMenu();
	void runVTXT();
	void showPluginScreen();
	void showSelectorStyleEPG();
	void showCurrentStyleEPG();
	void showMultiEPG();
	void runPluginEPG();
	void showEPG();
	void showEPG_Streaminfo();
	void StartInfoBarTimer();
	void myshowInfobar();
	void showInfobar(bool startTimeout = false);
	void showHelp( ePtrList<eAction>*, eString & );
	void SoftcamNameChanged(const char *newname);
	void usedCaidChanged(int newcaid);
	void SoftcamInfoChanged(const char *newSoftcamInfo);

#ifndef DISABLE_FILE
	int skipcounter;
	int skipping;
	int skipspeed;
	int seekstart;
	int seekpos;
	eTimer *skipTimer;
	eWidget *skipWidget;
	eWidget *seekWidget;
	eLabel *skipLabel1;
	eLabel *skipLabel2;
#endif

	void shufflePlaylist(eServiceSelector*);	
	void showServiceMenu(eServiceSelector*);

	void addService(const eServiceReference &service);

	void doPlaylistAdd(const eServiceReference &service);
	void addServiceToUserBouquet(eServiceReference *s, int dontask=0);
public:
	void addServiceToLastScannedUserBouquet (const eServiceReference &service, int service_type, int services_scanned, bool newService);
	void fillFastscanBouquet(eString bouquetname, std::map<int, eServiceReferenceDVB> &numbered_channels, int originalNumbering, bool radio=false);
	bool existsBouquet(eString bouquetname, bool radio=false);

private:
	void addServiceToCurUserBouquet(const eServiceReference &ref);
	void removeServiceFromUserBouquet( eServiceSelector *s );

	void showServiceInfobar(int show);

	static eZapMain *instance;

	eServicePath modeLast[modeEnd];

	int mode,             // current mode .. TV, Radio, File
			state,
			wasSleeping;

	void onRotorStart( int newPos );
	void onRotorStop();
	void onRotorTimeout();
protected:
	int eventHandler(const eWidgetEvent &event);
private:
	void executeProgrammableButton(const eString& buttonKey, const eString& defaultAction);
	void clearWasSleeping() { wasSleeping=0; }
	void ShowTimeCorrectionWindow( tsref ref );
	bool CheckService(const eServiceReference &ref );
	void handleServiceEvent(const eServiceEvent &event);
	void startService(const eServiceReference &, int);
	void gotEIT();
	void gotSDT();
	void gotPMT();
	void timeOut();
	void leaveService();
	void clockUpdate();
	void updateVolume(int mute_state, int vol);
	void prepareDVRHelp();
	void prepareNonDVRHelp();
	void set16_9Logo(int aspect);
	void setSmartcardLogo(bool b);
	void setAC3Logo(bool b);
	void setVTButton(bool b);
	void setEPGButton(bool b);
	void updateProgress();
	void updateServiceNum( const eServiceReference& );
	void getPlaylistPosition();
	bool handleState(int justask=0);
#ifndef DISABLE_FILE
	void blinkRecord();
	void toggleIndexmark();
	bool indexSeek(int dir);
	eZapSeekIndices indices;
	int  indices_enabled;
	void redrawIndexmarks();
	void deleteFile(eServiceSelector *);
	void renameFile(eServiceSelector *);
	void showHDDSpaceLeft();
	void startPermanentTimeshift();
	int led_timer;
	eTimer ledStatusBack;
	void ledBack();
	bool AnalogNoSec;

#endif // DISABLE_FILE

// Called from other context.. 	
	void EPGUpdated();
	void EPGAvail(bool);
	void EPGOrganiseRequest();

public:
	void deleteFile(eServiceReference);
	int renameFile(const eString&, const eString&, const eString&);
	bool checkRecordState() { return handleState(); }
    	eServicePath getRoot(int list, int mode=-1);
	int getFirstBouquetServiceNum( eServiceReference ref, int mode=-1);
	void deleteService(eServiceSelector *);
	void renameBouquet(eServiceSelector *);
	void renameService(eServiceSelector *);
	void createMarker(eServiceSelector *);
	void createEmptyBouquet(int mode);
	void copyProviderToBouquets(eServiceSelector *);
	void toggleScart( int state );
	void postMessage(const eZapMessage &message, int clear=0);
	void gotMessage(const int &);
	void gotEPGMessage(const eEPGCache::Message&);
	void startMessages();
	void stopMessages();
	void pauseMessages();
	void nextMessage();
	void gotoStandby();
	void wakeUp();
	void showEPGList(eServiceReferenceDVB ref);
	void AnalogSkinClock(tm *timem, bool secOn);
	static eZapMain *getInstance() { return instance; }

	enum {
		psAdd=1,      // just add, to not change current
		psRemove=2,   // remove before add
		psActivate=4, // use existing entry if available
		psDontAdd=8,  // just play
		psSeekPos=16, //
		psSetMode=32, // change mode ( TV, radio, file )
		psNoUser=64   // no timer stateCheck and same TP Checking
	};

	void playService(const eServiceReference &service, int flags);
	void playlistPrevService();

#ifndef DISABLE_FILE
	int recordDVR(int onoff, int user, time_t evtime=0, const char* event_name=0 ); // starts recording
	const eServiceReference& getRecordingsref() { return recordingsref; }
	ePlaylist *getRecordings() { return recordings; }
	void loadRecordings( bool create = false );
	void saveRecordings( bool destory = false );
	void clearRecordings();
	int isSkipping() { return skipping; }
	void stopPermanentTimeshift();
	void beginPermanentTimeshift();
	void addTimeshiftToRecording();
#endif
	int get16_9Logo() {return is16_9;}
	int getSmartcardLogo() {return isCrypted;}
	int getAC3Logo() {return isAC3;}
	int getVTButton() {return isVT;}
	int getEPGButton() {return isEPG;}

#ifndef DISABLE_NETWORK
	void startNGrabRecord();
	void stopNGrabRecord();
#endif

	void setMode(int mode, int user=0); // user made change?
	int getMode() { return mode; }

	void toggleTimerMode(int state);
	int toggleEditMode(eServiceSelector *, int mode=-1);
	void toggleMoveMode(eServiceSelector *);
	int handleStandby(int i=0);

	void setServiceSelectorPath(eServicePath path);
	void getServiceSelectorPath(eServicePath &path);
	void getAllBouquetServices(std::list<ePlaylistEntry> &servicelist);

	void moveService(const eServiceReference &path, const eServiceReference &ref, const eServiceReference &after);

	void loadPlaylist( bool create = false );
	void savePlaylist( bool destory = false );

	eString getEplPath() { return eplPath; }

	void loadUserBouquets( bool destroy=true );  // this recreate always all user bouquets...

	void destroyUserBouquets( bool save=false ); 

	void saveUserBouquets();  // only save

	void reloadPaths(int reset=0);

	int doHideInfobar();

#ifndef DISABLE_CI
	void receiveMMIMessageCI1( const char* data, int len );
	void receiveMMIMessageCI2( const char* data, int len );
	void handleMMIMessage( const eMMIMessage &msg );
#endif

	void gotRDSText(eString);

	int switchToNum( int num, bool onlyBouquets=false );

	void reloadSettings();

	eZapMain();
	~eZapMain();
};

class eServiceContextMenu: public eListBoxWindow<eListBoxEntryText>
{
	eServiceReference ref;
	void entrySelected(eListBoxEntryText *s);
	void init_eServiceContextMenu(const eServiceReference &ref, const eServiceReference &path, eWidget *LCDTitle=0, eWidget *LCDElement=0);
public:
	int getCount() { return list.getCount(); }
	eServiceContextMenu(const eServiceReference &ref, const eServiceReference &path, eWidget *LCDTitle=0, eWidget *LCDElement=0);
};

class eSleepTimerContextMenu: public eListBoxWindow<eListBoxEntryText>
{
	eServiceReference ref;
	void entrySelected(eListBoxEntryText *s);
public:
	eSleepTimerContextMenu(eWidget *LCDTitle=0, eWidget *LCDElement=0);
};

class eShutdownStandbySelWindow: public eWindow
{
protected:
	eCheckbox *Standby, *Shutdown;
	void StandbyChanged(int);
	void ShutdownChanged(int);
	void fieldSelected(int *){focusNext(eWidget::focusDirNext);}
	virtual void setPressed()=0;
	eButton *set;
	eNumber *num;
	eCheckbox *ampm;
public:
	int getCheckboxState();
	eShutdownStandbySelWindow( eWidget *parent, int len, int min, int max, int maxdigits, int *init, int isactive=0, eWidget* descr=0, int grabfocus=1, const char* deco="eNumber" );
};

class eSleepTimer: public eShutdownStandbySelWindow
{
	void setPressed();
public:
	eSleepTimer();
};

#ifndef DISABLE_FILE

class eRecordContextMenu: public eListBoxWindow<eListBoxEntryText>
{
	eServiceReference ref;
	void entrySelected(eListBoxEntryText *s);
	void init_eRecordContextMenu();
public:
	eRecordContextMenu(eWidget *LCDTitle=0, eWidget *LCDElement=0);
};

class eRecTimeInput: public eShutdownStandbySelWindow
{
	void setPressed();
	void init_eRecTimeInput();
public:
	eRecTimeInput();
};

class eTimerInput: public eShutdownStandbySelWindow
{
	void setPressed();
public:
	eTimerInput();
};

#endif //DISABLE_FILE
#endif /* __enigma_main_h */
