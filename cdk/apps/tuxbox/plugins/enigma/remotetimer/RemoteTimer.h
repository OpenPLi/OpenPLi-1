#include <lib/gui/combobox.h>
#include <lib/gui/enumber.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/emessage.h>
#include <lib/gui/listbox.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/elabel.h>
#include <lib/gui/eskin.h>
#include <enigma_dyn_utils.h>
#include <epgwindow.h>
#include <lib/dvb/si.h>
#include <lib/gdi/font.h>
using namespace std;
#include <fstream>
#include "lib/base/console.h"
#include <lib/base/estring.h>
#include <lib/gui/ePLiWindow.h>

#include <enigma_main.h>

#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#define _E2_

class SendPartnerBoxRequest
{
 	CURLcode sendGetRequest (const eString& url, eString& response, eString User, eString Password);
 	eString sIP;
 	eString sLogin;
 	eString sPassword;
 	eString sPort;
public:
 	SendPartnerBoxRequest(eString url);
 	eString response;
 	eString error;
 	int result;
 	void dialog(eString message, int ok); 
 	eString getBoxParInfo(eString buf, const char* param, int removeSpace);
 	
// 	void PartnerBoxGetRequest(eString url);
};

struct EPGData 
{
    eString DisplayName;
    eString Description;
	eString Details;
	eString EventStart;
    eString EventDuration;
	eString reference;
	eString name;
	eString provider;
	eString orbital_position;
	eService *service;
	eString Eventdate;
//	eString Genre;
	EITEvent *event;
	eString start;
	eString duration;
};

struct TimerData
{
	eString type;
	eString action;
	eString status;
	eString reference;
	eString name;
	eString date;
	eString time;
	eString duration;
	eString description;
	eString start;
	eString typedata;
	// E2 properties
#ifdef _E2_
	eString state;
	eString disabled;
	eString timeend;
	eString repeated;
#endif	
};

#define ConfFile  "/var/tuxbox/config/RemoteTimer.conf"

// Global Variables for Screen position and Screen size
int x_,y_,w_,h_;
int modeZoom;
int local = 0;
#ifdef _E2_
int E2=0;
#endif
// Global Variables --> Boxs data
eString IP, Login, Password, Bouquet, Action, Box;

class eListBoxEntryData: public eListBoxEntryText
{
public:
        eString name;
        EPGData mEPGData;
	    TimerData mTimerData;
        eListBoxEntryData(eListBox<eListBoxEntryData> *listbox,eString name,EPGData mEPGData)
        : eListBoxEntryText((eListBox<eListBoxEntryText> *)listbox, name), name(name), mEPGData(mEPGData)
        {
        }
	    eListBoxEntryData(eListBox<eListBoxEntryData> *listbox,eString name,TimerData mTimerData)
        : eListBoxEntryText((eListBox<eListBoxEntryText> *)listbox, name), name(name), mTimerData(mTimerData)
        {
        }
};

class eServiceList: public ePLiWindow
{
    eButton *bt_abort, *bt_event, *bt_rec, *bt_epg, *bt_timerlist;
    eLabel *lb_selected, *lb_info;
    int counter;
	eStatusBar *status;
	EPGData mEPGData;
	eListBox<eListBoxEntryData> *p_list;
	void p_listselected(eListBoxEntryData *item);
	void p_listselectionChanged(eListBoxEntryData *item);
	void read_xml(eString file);
	void GetEPGData(eString sReference, eString &ListAnzeige, EPGData &mEPGData);
	void EPG();
	void REC();
	void ShowDetails();
	void Config();
	void wget_DataAvail(eString str);
	void wget_Closed(int d);
	eString Recording;
	void ShowTimerList();
	void CheckForRecording();
	void REC_EVENT();
	void StopRec();
	int eventHandler(const eWidgetEvent &event);
	static eServiceList *instance;
public:
    eServiceList(eString XMLString);
    ~eServiceList();
    static eServiceList *getInstance() { return instance; }
    eString getActualTime(int type);
    eTimer *timer;
    void timerHandler();
   	int cancel;
   	void Record(EPGData nEPGData);
   	void SetTimer(EPGData nEPGData);
   	int messages(int msg, int time);
};

class eEPGList: public ePLiWindow
{
	EPGData mEPGData;
	eButton *bt_abort, *bt_add, *bt_info, *bt_timer;
    eListBox<eListBoxEntryData> *p_list;
	eStatusBar *status;
	void GetEPGData(eString sReference);
	void ShowInfo();
	void p_listselected(eListBoxEntryData *item);
	void SetWebTimer();
	void ShowTimerList();
	int eventHandler(const eWidgetEvent &event);
public:
	eEPGList(EPGData nEPGData);
};

class eSetTimer: public ePLiWindow
{
	eLabel *lb_selected;
public:
	eSetTimer(EPGData nEPGData);
};

class eTimerList: public ePLiWindow
{
    eButton *bt_abort, *bt_clean, *bt_erase, *bt_delete;
    eLabel *lb_selected, *lb_info;
	eStatusBar *status;
	TimerData mTimerData;
	eListBox<eListBoxEntryData> *p_list;
	void p_listselectionChanged(eListBoxEntryData *item);
	void read_xml(eString filename);
	void ShowDetails();
	void CleanUp();
	void ClearList();
	void Delete();
	static eTimerList *instance;
public:
    eTimerList(eString XMLString);
    void URLDecode(eString &encodedString);
	int Reload;
	eString Time_from_to( eString &start, eString &duration );
	static eTimerList *getInstance() { return instance; }

};

class eShowEPG: public eWindow
{
	eStatusBar *status;
	eLabel *label, *lb_date, *lb_channel;
	eWidget *visible;
	eProgress *scrollbar;
	int pageHeight;
	int total;
	int eventHandler(const eWidgetEvent &event);
	void updateScrollbar();
public:
	eShowEPG(eString Description, eString ShowEPGData, eString Eventdate, eString ServiceName, eString Temp);
};

class eSelection: public ePLiWindow
{
	eComboBox *BoxSelection;
	eButton *OK;
	eLabel* lblKey;
	void Selected();

public:
	eSelection();
	int intFound;
	int ok;
	int cfg;
};

class eTimerConfig : public ePLiWindow
{
	public:
		eTimerConfig();
	private:
		int iTimerOffsetStart;
		int iTimerOffsetStop;	
		eNumber* timerOffsetStart;
		eNumber* timerOffsetStop;
		void okPressed();
};




