#ifndef __enigma_plugins_h
#define __enigma_plugins_h

#include <plugin.h>

#include <lib/base/thread.h>
#include <lib/base/message.h>
#include <lib/base/console.h>
#include <lib/gui/listbox.h>
#include <lib/gui/emessage.h>

class ePlugin: public eListBoxEntryText
{
	friend class eZapPlugins;
	friend class eListBox<ePlugin>;

public:
	int version, type;
	eString depend, sopath, pluginname, requires, cfgname, desc, name;
	bool needfb, needrc, needlcd, needvtxtpid, needoffsets, showpig;
	int posx, posy, sizex, sizey;
	void init_ePlugin(const char *cfgfile, eSimpleConfigFile &config, const char* descr);
public:
	ePlugin(eListBox<ePlugin> *parent, const char *cfgfile, eSimpleConfigFile &config, const char* descr = NULL);
	ePlugin(eListBox<ePlugin> *parent, const char *ppanelFileName);

	bool operator < ( const ePlugin & e ) const 
	{
		return cfgname < e.cfgname;
	}
};

class eZapPlugins: public eListBoxWindow<ePlugin>
{
public:
	enum Types 
	{
		AnyPlugin = -1, 
		GamePlugin = 1, 
		StandardPlugin = 2, 
		ScriptPlugin = 3, 
		AutostartPlugin = 4, 
		FileExtensionScriptPlugin = 5,
		PPanelPlugin = 100
	};

	struct FileExtensionScriptInfo
	{
		int needfb;
		int needrc;
		int needlcd;
		eString file_pattern;
		eString directory_pattern;
		eString command;
	};

private:
	eString previousPlugin;
	static const char *PluginPath[3];
	void selected(ePlugin *);
	int type;
	void init_eZapPlugins(eWidget* lcdTitle, eWidget* lcdElement);
protected:
//	int eventHandler(const eWidgetEvent &event);
public:
	eZapPlugins(Types type, eWidget* lcdTitle=0, eWidget* lcdElement=0);
	eString execPluginByName(const char* name, bool onlySearch=false);
	int execSelectPrevious(eString &previous);
	void execPlugin(ePlugin* plugin);
	int exec();
	int find(bool ignore_requires=false);
	static int listPlugins(
		Types type,
		std::vector<eString>* filenames,
		std::vector<eString>* descriptions);
	static int getAutostartPlugins(std::vector<eString> &list);
	static int getFileExtensionPlugins(std::vector<FileExtensionScriptInfo> &list);
};

class ePluginThread: public eThread, public Object
{
public:
	static PluginParam *first, *tmp;

private:
	eFixedMessagePump<int> message;
	void *libhandle[20];
	int argc, tpid;
	static ePluginThread *instance;
	eString depend, sopath, pluginname;
	bool needfb, needrc, needlcd, needvtxtpid, needoffsets, showpig;
	int posx, posy, sizex, sizey, wasVisible;
	eZapPlugins *wnd;
	const char *PluginPath[3];
	void thread();
	void thread_finished();
	void finalize_plugin();
	void recv_msg(const int &);

public:
	ePluginThread( ePlugin *p, const char *PluginPath[3], eZapPlugins *wnd )
		:message(eApp,1), depend(p->depend), sopath(p->sopath), pluginname(p->pluginname),
		needfb(p->needfb), needrc(p->needrc), needlcd(p->needlcd),
		needvtxtpid(p->needvtxtpid), needoffsets(p->needoffsets),
		showpig(p->showpig), posx(p->posx), posy(p->posy), sizex(p->sizex),
		sizey(p->sizey), wasVisible(0), wnd(wnd)
	{
		CONNECT(message.recv_msg, ePluginThread::recv_msg);
		instance = this;
		this->PluginPath[0] = PluginPath[0];
		this->PluginPath[1] = PluginPath[1];
		this->PluginPath[2] = PluginPath[2];
	}
	~ePluginThread()
	{
		instance = NULL;
	}
	void start();
	static ePluginThread *getInstance() { return instance; }
	static void MakeParam(const char * const id, int val);
};

class eScriptOutputWindow: public eWindow
{
private:
	eLabel *label;
	eWidget *visible;
	eProgress *scrollbar;
	int pageHeight;
	int total;
	int lines;
	int eventHandler(const eWidgetEvent &event);
	void updateScrollbar();
	eConsoleAppContainer *script;
	void getData( eString );
	void scriptClosed(int);
	eString scriptOutput;
public:
	eScriptOutputWindow(ePlugin *plugin);
	~eScriptOutputWindow();
};

#endif /* __enigma_plugins_h */
