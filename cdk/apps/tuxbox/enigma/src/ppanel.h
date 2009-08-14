/*
 *  PPanel
 *
 *  Original source: The_Hydra
 *  Modified for PLi: dAF2000
 *
 *  History and licence see ppanel.cpp
 */

#ifndef __src_ppanel_h
#define __src_ppanel_h

#include <enigma_plugins.h>
#include <setup_window.h>

#include <lib/gui/ewindow.h>
#include <lib/gui/listbox.h>
#include <lib/picviewer/pictureviewer.h>

class eLabel;
class eButton;
class eComboBox;
class XMLTreeParser;
class PPanel;

class MiniPressOKWindow : public eWindow
{
	private:
	eButton *btRestore;
	
	public:
	MiniPressOKWindow();
};

class BaseWindow:public eListBoxWindow<eListBoxEntryMenu>
{
   public:
   BaseWindow(eString title);
   void setError(const eString& errmsg);
};

class ePPanelEntry:public eListBoxEntryCheck
{
   protected:
	PPanel &parentdlg;
	XMLTreeNode *node;
   eString name;
   
   eString runBefore, runBeforeOut, runAfter, runAfterOut, confirmation, quit;

   public:
	ePPanelEntry(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext, 
      XMLTreeNode *node, 
      bool checked = false);
      
   int preActions(void);
   int postActions(void);
};

class eListBoxEntryPPanel:public ePPanelEntry
{
   protected:
   eString target;

   public:
   eListBoxEntryPPanel(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext, 
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryFile:public ePPanelEntry
{
   protected:
   eString url, target, dontCheck;

   public:
   eListBoxEntryFile(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext, 
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryTarball:public ePPanelEntry
{
   protected:
   eString url, packageType, action, package, version;

   public:
   eListBoxEntryTarball(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString extendedName,
      eString helptext,
      XMLTreeNode *node, 
      bool installed = false);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryTarballManual:public BaseWindow
{
   protected:
   eString installDirectory;
   void doInstall(eListBoxEntryMenu *item);

   public:
   eString selectedPackage;
   eListBoxEntryTarballManual(eString directory);
};

class eListBoxEntryPlugins:public ePPanelEntry
{
   public:
   eListBoxEntryPlugins(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext, 
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryPlugin: public ePPanelEntry
{
   protected:
   eString target;

   public:
   eListBoxEntryPlugin(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext, 
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryExecute:public ePPanelEntry
{
   protected:
   eString target;

   public:
   eListBoxEntryExecute(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext, 
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryComment:public eListBoxEntryMenu
{
   public:
   eListBoxEntryComment(
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name);
};

class eListBoxEntryRemove:public ePPanelEntry
{
   public:
   eListBoxEntryRemove(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext, 
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryRemoveScreen:public BaseWindow
{
   protected:
   void doRemove(eListBoxEntryMenu *item);
   eString confirmation;

   public:
   eListBoxEntryRemoveScreen(const eString &confirm);
};

class eListBoxEntryMedia:public ePPanelEntry
{
   protected:
   eString target, stream;

   public:
   eListBoxEntryMedia(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext,
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryConfigWrite:public ePPanelEntry
{
   protected:
   eString target, value;

   public:
   eListBoxEntryConfigWrite(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext,
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class eListBoxEntryEnigmaMenu:public ePPanelEntry
{
   protected:
   eString target;

   public:
   eListBoxEntryEnigmaMenu(
      PPanel &parentdlg, 
      eListBox<eListBoxEntryMenu> *listbox, 
      eString name, 
      eString helptext,
      XMLTreeNode *node);

   void LBSelected(eListBoxEntry* t);
};

class PPanel:public BaseWindow
{
   protected:
   XMLTreeParser *directory;
   eConsoleAppContainer *updateRunner;

   void recover(const eString &xmlFile);
   XMLTreeNode* loadFile(FILE *fh);
   void loadItems(XMLTreeNode *category);
   void itemSelected(eListBoxEntryMenu *item);
   void updatePPanel(eString &url, eString &target);
   void runnerClosed(int state);
   PPanel(const eString &xmlFile);
   PPanel(XMLTreeNode *node);

   public:
	static eString getPPanelName(const eString &xmlFile);
	static void runPPanel(const eString& xmlFile, eWidget* lcdTitle = 0, eWidget* lcdElement = 0);
	static void runPPanel(XMLTreeNode *node, eWidget* lcdTitle = 0, eWidget* lcdElement = 0);
      
   ~PPanel();
};

#ifdef EHTTPDOWNLOAD

class DownloadIndicator:public eWindow
{
   protected:
   eString filename;
   eString url;
   eHTTPConnection *http;
	eHTTPDownload *download;
   int error;
	int lasttime;
	eProgress *progress;
	eLabel *progresstext;
	eLabel *downloadtext;

   public:
   DownloadIndicator(
      eString caption, 
      eString message, 
      eString filename, 
      eString url);
      
   ~DownloadIndicator();

   void startDownload(const char *url, const char *filename);
	void downloadDone(int err);
	eHTTPDataSource *createDownloadDataSink(eHTTPConnection *conn);
   void downloadProgress(int received, int total);
};

#else

class DownloadIndicator:public eWindow
{
   protected:
   eLabel *etext;
   eTimer *updateTimer;
   eConsoleAppContainer *downloader;
   eString originalText;
   eString filename;
   eString url;
   bool downloadDone;
   void update(void);
   void downloadClosed(int state);
   
   int DownloadIndicator::eventHandler(const eWidgetEvent &e);
   
   public:
   DownloadIndicator(
      eString caption, 
      eString message, 
      eString filename, 
      eString url);
      
   ~DownloadIndicator();
   
   int exec();
};

#endif // EHTTPDOWNLOAD

class InstalledPackages
{
   public:
   struct PackageInfo
   {
      eString name;
				
		PackageInfo::PackageInfo() : name("")
		{
		};
		
		PackageInfo::PackageInfo(const eString& name) : name(name)
		{
		};
		
   };
      
   std::vector<PackageInfo> packages;

   InstalledPackages();
	void reload();
   void addPackage(PackageInfo packageInfo);
   void removePackage(const eString& packageName);
   PackageInfo findPackage(const eString& packageName);
};

#endif
