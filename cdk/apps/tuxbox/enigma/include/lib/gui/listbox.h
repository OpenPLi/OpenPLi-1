#ifndef __listbox_h
#define __listbox_h

#include <sstream>

#include <lib/driver/rc.h>
#include <lib/gdi/grc.h>
#include <lib/gdi/fb.h>
#include <lib/gui/ewidget.h>
#include <lib/gui/eskin.h>
#include <lib/gui/ewindow.h>
#include <lib/gui/guiactions.h>
#include <lib/gui/statusbar.h>

int calcFontHeight( const gFont& font );

class eListBoxEntry;
class eProgress;

class eListBoxBase: public eDecoWidget
{
	int removed_height_pixel;
	eProgress *scrollbar;
	const eWidget* descr;
//#ifndef DISABLE_LCD
	eLabel* tmpDescr; // used for description Label in LCD
//#endif
	gColor colorActiveB, colorActiveF;
	enum  { arNothing, arCurrentOld, arAll};
	int movemode, MaxEntries, flags, item_height, columns, in_atomic, atomic_redraw, atomic_old, atomic_new;
	bool atomic_selchanged;
	bool atomic_selected;
	unsigned int entries;
	int currentPos;
	void init_eListBoxBase();
protected:
	ePtrList<eListBoxEntry>::iterator top, bottom, current;
	eListBoxBase(eWidget* parent, const eWidget* descr=0, int takefocus=1, int item_height=0, const char *deco="eListBox" );
	ePtrList<eListBoxEntry> childs;
	eListBoxEntry* getCurrent()	{ return current != childs.end() ? *current : 0; }
	eListBoxEntry* getNext() { ePtrList<eListBoxEntry>::iterator c=current; ++c; return c != childs.end() ? *c : 0; }
	eListBoxEntry* getFirst() { return getCount() != 0 ? *(childs.begin()) : 0; }
	eListBoxEntry* goNext();
	eListBoxEntry* goPrev();
	int setProperty(const eString &prop, const eString &value);
	int eventHandler(const eWidgetEvent &event);
	void lostFocus();
	void gotFocus();
private:
	eRect getEntryRect(int n);
	void eraseBackground() {};
	void recalcMaxEntries();
	void recalcClientRect();
	void recalcScrollBar();
	int newFocus();
	void redrawWidget(gPainter *target, const eRect &area);
	void init();
	virtual void SendSelected( eListBoxEntry* entry )=0;
	virtual void SendSelChanged( eListBoxEntry* entry )=0;
public:
	~eListBoxBase();
	enum	{		dirPageDown, dirPageUp, dirDown, dirUp, dirFirst, dirLast	};
	enum	{		flagNoUpDownMovement=1,		flagNoPageMovement=2,		flagShowEntryHelp=4, flagShowPartial=8, flagLostFocusOnFirst=16, flagLostFocusOnLast=32, flagHasShortcuts=64, flagColorShortcutsFirst= 128 };
	enum	{		OK = 0,		ERROR=1,		E_ALLREADY_SELECTED = 2,		E_COULDNT_FIND = 4,		E_INVALID_ENTRY = 8,	 E_NOT_VISIBLE = 16		};
	void setFlags(int);
	int getFlags() const { return flags; }
	void removeFlags(int);
	void invalidateEntry(int n){	invalidate(getEntryRect(n));}
	void invalidateContent();
	void setColumns(int col);
	int getColumns() { return columns; }
	void setMoveMode(int move) { movemode=move; }
	void append(eListBoxEntry* e, bool holdCurrent=false, bool front=false);
	void take(eListBoxEntry *e, bool holdCurrent=false);
	inline void remove(eListBoxEntry* entry, bool holdCurrent=false);
	void clearList();
	int getCount() { return entries; }
	int setCurrent(const eListBoxEntry *c, bool sendSelected=false);
	void sort();
	void shuffle();
	int moveSelection(int dir, bool sendSelected=false);
	void setActiveColor(gColor back, gColor front);
	gColor getActiveBackColor() { return colorActiveB; }
	void beginAtomic();
	void endAtomic();
	void FakeFocus( int i ) { have_focus=i; }
	void invalidateCurrent();
	int getShortcut(eListBoxEntry* e);
	int eventHandlerShortcuts(const eWidgetEvent &event);
};

template <class T>
class eListBox: public eListBoxBase
{
	void SendSelected( eListBoxEntry* entry )
	{
		/*emit*/ selected((T*)entry);
	}
	void SendSelChanged( eListBoxEntry* entry )
	{
		/*emit*/ selchanged((T*)entry);
	}
public:
	Signal1<void, T*> selected;
	Signal1<void, T*> selchanged;
	eListBox(eWidget *parent, const eWidget* descr=0, int takefocus=1 )
		:eListBoxBase( parent, descr, takefocus, T::getEntryHeight() )
	{
	}
	T* getCurrent()	{ return (T*)eListBoxBase::getCurrent(); }
	T* getNext() { return (T*)eListBoxBase::getNext(); }
	T* getFirst() { return (T*)eListBoxBase::getFirst(); }
	T* goNext() { return (T*)eListBoxBase::goNext(); }
	T* goPrev() { return (T*)eListBoxBase::goPrev(); }

	template <class Z>
	int forEachEntry(Z ob)
	{
		for (ePtrList<eListBoxEntry>::iterator i(childs.begin()); i!=childs.end(); ++i)
			if ( ob((T&)(**i)) )
				return OK;

		return ERROR;
	}

	template <class Z>
	int forEachVisibleEntry(Z ob)
	{
		if (!isVisible())
			return E_NOT_VISIBLE;

		for (ePtrList<eListBoxEntry>::iterator i(top); i!=bottom; ++i)
			if ( ob((T&)(**i)) )
				return OK;

		return ERROR;
	}
};

class eListBoxBaseExt: public eListBoxBase
{
	// for textbrowsing via keyboard
	eString browseText;
	std::list<eListBoxEntry*> browseHistory;
	eTimer browseTimer;
	void browseTimeout() { browseText=""; }
	int eventHandler( const eWidgetEvent &e );
protected:
	void gotFocus();
	eListBoxBaseExt(eWidget* parent, const eWidget* descr=0, int takefocus=1, int item_height=0, const char *deco="eListBox" );
public:
// for text browsing via keyboard	
	int keyDown(int key);
	void lostFocus();
	void clearList();
};

template <class T>
class eListBoxExt: public eListBoxBaseExt
{
	void SendSelected( eListBoxEntry* entry )
	{
		/*emit*/ selected((T*)entry);
	}
	void SendSelChanged( eListBoxEntry* entry )
	{
		/*emit*/ selchanged((T*)entry);
	}
public:
	Signal1<void, T*> selected;
	Signal1<void, T*> selchanged;
	eListBoxExt(eWidget *parent, const eWidget* descr=0, int takefocus=1 )
		:eListBoxBaseExt( parent, descr, takefocus, T::getEntryHeight() )
	{
	}
	T* getCurrent()	{ return (T*)eListBoxBase::getCurrent(); }
	T* getNext() { return (T*)eListBoxBase::getNext(); }
	T* goNext() { return (T*)eListBoxBase::goNext(); }
	T* goPrev() { return (T*)eListBoxBase::goPrev(); }

	template <class Z>
	int forEachEntry(Z ob)
	{
		for (ePtrList<eListBoxEntry>::iterator i(childs.begin()); i!=childs.end(); ++i)
			if ( ob((T&)(**i)) )
				return OK;

		return ERROR;
	}

	template <class Z>
	int forEachVisibleEntry(Z ob)
	{
		if (!isVisible())
			return E_NOT_VISIBLE;

		for (ePtrList<eListBoxEntry>::iterator i(top); i!=bottom; ++i)
			if ( ob((T&)(**i)) )
				return OK;

		return ERROR;
	}
};

class eListBoxEntry: public Object
{
	friend class eListBox<eListBoxEntry>;
protected:
	eListBox<eListBoxEntry>* listbox;
	eString helptext;
	int selectable;
public:
	void clearLB() { listbox=0; }
	int isSelectable() const { return selectable; }
	eListBoxEntry(eListBox<eListBoxEntry>* parent, eString hlptxt=0, int selectable=3 )
		:listbox(parent), helptext(hlptxt?hlptxt:eString(" ")),
		selectable(selectable)
	{
		if (listbox)
			listbox->append(this);
	}
	virtual ~eListBoxEntry()
	{
		if (listbox)
			listbox->take(this);
	}
	virtual bool operator < ( const eListBoxEntry& e)const
	{
		return false;
	}
	virtual int eventHandler( const eWidgetEvent &e )
	{
		return 0;
	}
	virtual const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state )=0;
	void drawEntryRect(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state);
	void drawEntryBorder(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF);
	const eString &getHelpText() const { return helptext; }
	virtual const eString& getText() const 
	{ 
		static eString str;
		return str; 
	}
};

inline void eListBoxBase::remove(eListBoxEntry* entry, bool holdCurrent)
{ 
	take(entry,holdCurrent);
	delete entry;
}

// eListBoxEntrySeparator is deprecated now..
// please use eListBoxEntryTextSeparotor or MenuSeparator in new projects...
class eListBoxEntrySeparator: public eListBoxEntry
{
	gPixmap *pm;
	__u8 distance;
	bool alphatest;
public:
	eListBoxEntrySeparator( eListBox<eListBoxEntry> *lb, gPixmap *pm, __u8 distance, bool alphatest=false )
		:eListBoxEntry(lb, 0, 0), pm(pm), distance(distance), alphatest(alphatest)
	{
	}
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
};

class eListBoxEntryText: public eListBoxEntry
{
	friend class eListBox<eListBoxEntryText>;
protected:
	eString text;
	void *key;
	int align;
	eTextPara *para;
	int yOffs;
	static gFont font;
	int keytype;
public:
	enum { value, ptr };
	static int getEntryHeight();

	eListBoxEntryText(eListBox<eListBoxEntryText>* lb, const char* txt=0, void *key=0, int align=0, const eString &hlptxt="", int keytype = value )
		:eListBoxEntry( (eListBox<eListBoxEntry>*)lb, hlptxt ), text(txt),
		 key(key), align(align), para(0), keytype(keytype)
	{
	}

	eListBoxEntryText(eListBox<eListBoxEntryText>* lb, const eString& txt, void* key=0, int align=0, const eString &hlptxt="", int keytype = value )
		:eListBoxEntry( (eListBox<eListBoxEntry>*)lb, hlptxt ), text(txt),
		 key(key), align(align), para(0), keytype(keytype)
	{
	}

	~eListBoxEntryText();

	virtual bool operator < ( const eListBoxEntry& e) const
	{
		if (key == ((eListBoxEntryText&)e).key || keytype == ptr)
			return text < ((eListBoxEntryText&)e).text;
		else
			return key < ((eListBoxEntryText&)e).key;
	}

	void *& getKey() { return key; }
	const void* getKey() const { return key; }
	const eString& getText() const { return text; }
	void SetText(const eString& txt); // not setText !!!
protected:
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
};

class eListBoxSeparator
{
	gPixmap *pm;
	__u8 distance;
	bool alphatest;
public:
	eListBoxSeparator(gPixmap *pm, __u8 distance, bool alphatest=false )
		: pm(pm), distance(distance), alphatest(alphatest)
	{
	}
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
};

class eListBoxEntryTextSeparator: public eListBoxEntryText, public eListBoxSeparator
{
public:
	eListBoxEntryTextSeparator( eListBox<eListBoxEntryText> *lb, gPixmap *pm, __u8 distance, bool alphatest=false )
		:eListBoxEntryText(lb), eListBoxSeparator(pm, distance, alphatest)
	{
		selectable = 0;
	}
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state ) { return eListBoxSeparator::redraw(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state); }
};

class eListBoxEntryTextStream: public eListBoxEntry
{
	friend class eListBox<eListBoxEntryTextStream>;
protected:
	std::stringstream text;
	static gFont font;
public:
	static int getEntryHeight();

	eListBoxEntryTextStream(eListBox<eListBoxEntryTextStream>* lb)
		:eListBoxEntry((eListBox<eListBoxEntry>*)lb)
	{
	}

	bool operator < ( const eListBoxEntryTextStream& e) const
	{
		return text.str() < e.text.str();
	}

protected:
	const eString &redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
};

class eListBoxEntryTextStreamSeparator: public eListBoxEntryTextStream, public eListBoxSeparator
{
public:
	eListBoxEntryTextStreamSeparator( eListBox<eListBoxEntryTextStream> *lb, gPixmap *pm, __u8 distance, bool alphatest=false )
		:eListBoxEntryTextStream(lb), eListBoxSeparator(pm, distance, alphatest)
	{
		selectable = 0;
	}
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state ) { return eListBoxSeparator::redraw(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state); }
};

class eListBoxEntryMenu: public eListBoxEntryText
{
	friend class eListBox<eListBoxEntryMenu>;
public:
	Signal0<void> selected;

	eListBoxEntryMenu(eListBox<eListBoxEntryMenu>* lb, const char* txt, const eString &hlptxt="", int align=0, void *key = NULL, int keytype = value )
		:eListBoxEntryText((eListBox<eListBoxEntryText>*)lb, txt, key, align, hlptxt, keytype)
	{
		if (listbox)
			CONNECT(listbox->selected, eListBoxEntryMenu::LBSelected);
	}
	eListBoxEntryMenu(eListBox<eListBoxEntryMenu>* lb, const eString &txt, const eString &hlptxt="", int align=0, void *key = NULL, int keytype = value )
		:eListBoxEntryText((eListBox<eListBoxEntryText>*)lb, txt, key, align, hlptxt, keytype)
	{
		if (listbox)
			CONNECT(listbox->selected, eListBoxEntryMenu::LBSelected);
	}
	
	virtual void LBSelected(eListBoxEntry* t)
	{
		if (t == this)
			/* emit */ selected();
	}
};

class eListBoxEntryMenuItem: public eListBoxEntryMenu
{
public:
	eListBoxEntryMenuItem(eListBox<eListBoxEntryMenu>* lb, const char* txt=0, void *key=0, int align=0, const eString &hlptxt="", int keytype = value)
		:eListBoxEntryMenu((eListBox<eListBoxEntryMenu>*)lb, txt, hlptxt, align, key, keytype)
	{
	}
};

class eListBoxEntryMenuSeparator: public eListBoxEntryMenu, public eListBoxSeparator
{
public:
	eListBoxEntryMenuSeparator( eListBox<eListBoxEntryMenu> *lb, gPixmap *pm, __u8 distance, bool alphatest=false )
		:eListBoxEntryMenu(lb, NULL), eListBoxSeparator(pm, distance, alphatest)
	{
		selectable = 0;
	}
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state ) { return eListBoxSeparator::redraw(rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state); }
};

class eListBoxEntryCheck: public eListBoxEntryMenu
{
	gPixmap *pm;
	eString regKey;
	int checked;
	void LBSelected(eListBoxEntry* t);
public:
	Signal1<void,bool> selected;
	eListBoxEntryCheck( eListBox<eListBoxEntryMenu> *lb, const char* text, const char* regkey, const eString& hlptxt="" );
	eListBoxEntryCheck( eListBox<eListBoxEntryMenu> *lb, const eString& text, const eString& hlptxt="", int align=0, void* key = NULL, int keytype = value );
	void setChecked(int checkIt);
	const eString& redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state );
};

class eListBoxEntryMulti: public eListBoxEntryMenu
{
	std::list< std::pair< int, eString > > entrys;
	std::list< std::pair< int, eString > >::iterator cur;
public:
	eListBoxEntryMulti( eListBox<eListBoxEntryMenu> *lb, const char *hlptext );
	void add( const char *text, int key );
	void add( const eString &text, int key );
	int eventHandler( const eWidgetEvent &e );
	void setCurrent( int );
};

template <class T>
class eListBoxWindow: public eWindow
{
public:
	int Entrys;
	int width;
	eListBox<T> list;
	eStatusBar *statusbar;
	eListBoxWindow(eString Title="", int Entrys=0, int width=400, bool sbar=0);
};

template <class T>
inline eListBoxWindow<T>::eListBoxWindow(eString Title, int Entrys, int width, bool sbar)
	: eWindow(0), Entrys(Entrys), width(width), list(this), statusbar(sbar?new eStatusBar(this):0)
{
	setText(Title);
	cresize( eSize(width, (sbar?60:10)+Entrys*T::getEntryHeight() ) );
	list.setFlags( eListBoxBase::flagShowEntryHelp );
	list.move(ePoint(10, 5));
	list.resize(eSize(getClientSize().width()-20, getClientSize().height()-(sbar?60:10) ));
	if (sbar)
	{
		statusbar->setFlags(eStatusBar::flagVCenter);
		statusbar->move( ePoint(0, getClientSize().height()-50) );
		statusbar->resize( eSize( getClientSize().width(), 50) );
		statusbar->loadDeco();
		statusbar->show();
	}
}

#endif
