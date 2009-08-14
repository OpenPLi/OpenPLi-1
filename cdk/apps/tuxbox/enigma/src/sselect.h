#ifndef __sselect_h
#define __sselect_h

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/gui/statusbar.h>
#include <lib/dvb/epgcache.h>
#include <src/channelinfo.h>

#include <lib/dvb/service.h>

class eService;
class eLabel;

class eEPGStyleSelector: public eListBoxWindow<eListBoxEntryText>
{
	int ssel;
	void init_eEPGStyleSelector();
public:
	eEPGStyleSelector(int ssel=0);
	int eventHandler( const eWidgetEvent &event );
	void entrySelected( eListBoxEntryText* e );
};

class eListBoxEntryService: public eListBoxEntry
{
	friend class eServiceSelector;
	friend class eListBoxExt<eListBoxEntryService>;
	friend struct moveFirstChar;
	friend struct moveServiceNum;
	friend struct moveServicePath;
	friend struct _selectService;
	friend struct updateEPGChangedService;
	friend struct renumber;
	eString sort;
	static gFont serviceFont, descrFont, numberFont;
	static int maxNumSize;
	static gPixmap *folder, *marker, *locked, *newfound;
	eTextPara *numPara, *namePara, *descrPara;
	int nameXOffs, descrXOffs, numYOffs, nameYOffs, descrYOffs;
	int flags;
	int num;
	int curEventId;
	void init_eListBoxEntryService();
public:
	const eString &getText() const { return sort; }
	static eListBoxEntryService *selectedToMove;
	static std::set<eServiceReference> hilitedEntrys;
	static int nownextEPG;
	int getNum() const { return num; }
	void invalidate();
	void invalidateDescr();
	static int getEntryHeight();
	eServiceReference service;
	enum { flagShowNumber=1, flagOwnNumber=2, flagIsReturn=4, flagSameTransponder=8 };
	eListBoxEntryService(eListBoxExt<eListBoxEntryService> *lb, const eServiceReference &service, int flags, int num=-1);
	~eListBoxEntryService();

	bool operator<(const eListBoxEntry &r) const
	{
		if (flags & flagIsReturn)
			return 1;
		else if (((eListBoxEntryService&)r).flags & flagIsReturn)
			return 0;
		else if (service.getSortKey() == ((eListBoxEntryService&)r).service.getSortKey())
			return sort < ((eListBoxEntryService&)r).sort;
		else // sort andersrum
			return service.getSortKey() > ((eListBoxEntryService&)r).service.getSortKey();
	}
protected:
	const eString &redraw(gPainter *rc, const eRect &rect, gColor, gColor, gColor, gColor, int hilited);
};

class eServiceSelector: public eWindow
{
#ifndef DISABLE_FILE
	friend class eFileSelector;
#endif
	eServiceReference selected;
	eServiceReference *result;
	eListBoxExt<eListBoxEntryService> *services, *bouquets;

	eLabel *key[4];
	const eWidget *rfocus;

	eChannelInfo* ci;

	eServicePath path;

	void addService(const eServiceReference &service);
	void addBouquet(const eServiceReference &service);
	int style, lastSelectedStyle;
	int serviceentryflags;

	char BrowseChar;
	eTimer BrowseTimer;
	eTimer ciDelay;

	eListBoxEntryService *goUpEntry;
private:
	void pathUp();
	void fillServiceList(const eServiceReference &ref);
	void fillBouquetList(const eServiceReference &ref);
	void serviceSelected(eListBoxEntryService *entry);
	void bouquetSelected(eListBoxEntryService *entry);
	void serviceSelChanged(eListBoxEntryService *entry);
	void bouquetSelChanged( eListBoxEntryService *entry);
	void ResetBrowseChar();
	void gotoChar(char c);
	void updateCi();
	virtual void init_eServiceSelector();
	void showRecordingInfo(eString path);
	void EPGSearchEvent(eServiceReference ref); // EPG search
	void SwitchNowNext();
public:
	void EPGUpdated();
	int eventHandler(const eWidgetEvent &event);
	virtual void setKeyDescriptions(bool editMode=false);
	void forEachServiceRef( Signal1<void,const eServiceReference&>, bool );
	int movemode;
	int editMode;
	int plockmode;
	bool isFileSelector;
	enum { styleInvalid, styleCombiColumn, styleSingleColumn, styleMultiColumn };
	enum { dirNo, dirUp, dirDown, dirFirst, dirLast };

	eServiceSelector();
	~eServiceSelector();

	enum { listAll, listSatellites, listProvider, listBouquets };
	Signal2<eServicePath,int,int> getRoot;
	Signal2<int,eServiceReference,int> getFirstBouquetServiceNum;

	Signal1<void,const eServiceReference &> addServiceToPlaylist; // add service to the Playlist
	Signal2<void,eServiceReference*,int> addServiceToUserBouquet;  // add current service to selected User Bouquet

	Signal1<void,int> setMode;        // set TV, Radio or File

	Signal1<void,eServiceSelector*>	removeServiceFromUserBouquet, // remove service from selected User Bouquet
																	showMenu, // shows the contextmenu
																	toggleStyle, // switch service selector style
																	renameService, renameBouquet,
																	deletePressed, newMarkerPressed,
																	copyToBouquetList;

	Signal3<void,
		const eServiceReference &, 		// path
		const eServiceReference &, 		// service to move
		const eServiceReference &			// service AFTER moved service
		> moveEntry;

	Signal1<void,eServiceReferenceDVB>showEPGList;

	const eServicePath &getPath() { return path; }
	void setPath(const eServicePath &path, const eServiceReference &select=eServiceReference());

	int getStyle()	{ return style; }
	void setStyle(int newStyle=-1, bool force=false);
	void actualize();
	bool selectService(const eServiceReference &ref);
	bool selectService(int num);
	bool selectServiceRecursive( eServiceReference &ref );
	bool selServiceRec( eServiceReference &ref );
	int getServiceNum( const eServiceReference &ref);
	void enterDirectory(const eServiceReference &ref);
	const eServiceReference &getSelected() { return selected; }
	const eServiceReference *choose(int irc=-1);
	const eServiceReference *next();
	const eServiceReference *prev();
	void removeCurrent(bool=false);
	void invalidateCurrent(eServiceReference ref=eServiceReference());
	void updateNumbers();
	void showMultiEPG();
	void shuffle();

	int toggleMoveMode();  // enable / disable move entry Mode ( only in userBouquets )
	int toggleEditMode();  // enable / disable edit UserBouquet Mode
};
#ifndef DISABLE_FILE
class eFileSelector: public eServiceSelector
{
	eServicePath getDirRoot(int list, int _mode);
	void init_eFileSelector(eString startPath);
public:
	eFileSelector(eString startPath);

	virtual void setKeyDescriptions(bool editMode=false);
};
#endif

#endif
