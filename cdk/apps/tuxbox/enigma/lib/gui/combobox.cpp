#include <lib/gui/combobox.h>
#include <lib/gdi/font.h>

eComboBox::eComboBox( eWidget* parent, int OpenEntries, eLabel* desc, int takefocus, const char *deco )
:eButton(parent, desc, takefocus, deco),
listbox(0, 0, takefocus),
button( this, 0, 0, eSkin::getActive()->queryValue("eComboBox.smallButton.decoWidth",0)?"eButton":""),
pm(0), entries(OpenEntries), current(0)
{
	init_eComboBox();
}
void eComboBox::init_eComboBox()
{
	align=eTextPara::dirLeft;
	if ( eSkin::getActive()->queryValue("eComboBox.smallButton.decoWidth",0) )
		button.loadDeco();
	button.setBlitFlags(BF_ALPHATEST);

	pm=eSkin::getActive()->queryImage("eComboBox.arrow");
	button.setPixmap(pm);
	listbox.hide();
	listbox.setDeco("eComboBox.listbox");
	listbox.loadDeco();

	gColor background = eSkin::getActive()->queryScheme("eComboBox.listbox.normal.background");
	gColor foreground = eSkin::getActive()->queryScheme("eComboBox.listbox.normal.foreground");
	if (background) 
		listbox.setBackgroundColor(background);
	if (foreground) 
		listbox.setForegroundColor(foreground);
	background = eSkin::getActive()->queryScheme("eComboBox.listbox.selected.background");
	foreground = eSkin::getActive()->queryScheme("eComboBox.listbox.selected.foreground");
	listbox.setActiveColor(background, foreground);

	CONNECT( selected, eComboBox::onOkPressed );
	CONNECT( listbox.selected, eComboBox::onEntrySelected );
	CONNECT( listbox.selchanged, eComboBox::onSelChanged );
	CONNECT( getTLW()->focusChanged, eComboBox::lbLostFocus );
	listbox.zOrderRaise();
	addActionMap(&i_cursorActions->map);
}

eComboBox::~eComboBox()
{
	if ( listbox.isVisible() )
		setFocus(this);
}

void eComboBox::onOkPressed()
{
	if ( flags & flagShowEntryHelp)
	{
		oldHelpText=helptext;
		setHelpText( listbox.getCurrent()->getHelpText() );
	}
	if ( flags & flagSorted )
		listbox.sort();
	parent->setFocus( &listbox );
	ePoint pt = getAbsolutePosition();
	if ( pt.y() + getSize().height() + listbox.getSize().height() > 520)
		listbox.move( ePoint( pt.x(), pt.y()-listbox.getSize().height() ) );
	else
		listbox.move( ePoint( pt.x(), pt.y()+getSize().height() ) );

	eWindow::globalCancel( eWindow::OFF );
	listbox.show();
}

int eComboBox::setProperty( const eString& prop, const eString& val )
{
	if ( prop == "sorted" )
		flags |= flagSorted;
	else if (prop == "openEntries" )
		entries = atoi( val.c_str() );
	else if (prop == "showEntryHelp" )
	{
		flags |= flagShowEntryHelp;
		listbox.setFlags(eListBoxBase::flagShowEntryHelp);
	}
	else if (prop == "openWidth" )
	{
		int width=listbox.getSize().width();
		width = atoi(val.c_str());
		setOpenWidth( width );
	}
	else
		return eButton::setProperty( prop, val);
	return 0;
}

int eComboBox::eventHandler( const eWidgetEvent& event )
{
	switch (event.type)
	{
		case eWidgetEvent::evtShortcut:
			parent->setFocus(this);
			onOkPressed();
			break;
		case eWidgetEvent::changedPosition:
		case eWidgetEvent::changedSize:
		{
			eListBoxEntryText* cur = listbox.getCount()?listbox.getCurrent():0;
			listbox.resize( eSize( getSize().width(), eListBoxEntryText::getEntryHeight()*entries+listbox.getDeco().borderBottom+listbox.getDeco().borderTop ) );
			int smButtonDeco = eSkin::getActive()->queryValue("eComboBox.smallButton.decoWidth", pm?pm->x:0 );
			if (deco)
			{
				button.resize( eSize(smButtonDeco, crect.height()) );
				button.move( ePoint( crect.right()-smButtonDeco, crect.top() ) );
			}
			else
			{
				button.resize( eSize(smButtonDeco, height()) );
				button.move( ePoint( clientrect.right()-smButtonDeco, clientrect.top() ) );
			}
			if (pm)
				button.pixmap_position = ePoint( (button.getSize().width() - pm->x) / 2, (button.getSize().height() - pm->y) / 2 );
			if (cur)
				listbox.setCurrent(cur);
		}
		default:
			return eButton::eventHandler( event );	
	}
	return 1;
}

int eComboBox::moveSelection ( int dir, bool sendSelChanged )
{
	int ret = listbox.moveSelection( dir, sendSelChanged );
	eListBoxEntryText *cur = listbox.getCurrent();
	if ( cur )
	{
		setText( cur->getText() );
		current = cur;
	}
	return ret;
}

void eComboBox::onEntrySelected( eListBoxEntryText* e)
{
	if ( parent->getFocus() == &listbox && (flags & flagShowEntryHelp) )
		setHelpText( oldHelpText );

	if (e)
	{
		setText(e->getText());
		setFocus( this );
#ifndef DISABLE_LCD
		if ( parent->LCDElement )
			parent->LCDElement->setText("");
#endif
		current = e;
		/* emit */ selchanged_id(this, current);
		/* emit */ selchanged(current);
	}
	else
		setFocus( this );
}

void eComboBox::onSelChanged(eListBoxEntryText* le)
{
#ifndef DISABLE_LCD
	if ( parent->getFocus() == &listbox )
	{
		if (flags & flagShowEntryHelp)
			setHelpText( listbox.getHelpText() );
		if ( LCDTmp )
			LCDTmp->setText( le->getText() );
		else if ( parent->LCDElement )
			parent->LCDElement->setText( le->getText() );
	}
#endif
}

void eComboBox::takeEntry( eListBoxEntryText* le )
{
	if (le)
		listbox.take(le);
}

eListBoxEntryText *eComboBox::takeEntry( int num )
{
	if ( listbox.getCount() <= num)
	{
		setCurrent( num );
		eListBoxEntryText *cur = listbox.getCurrent();
		listbox.take(cur);
		return cur;
	}
	return 0;
}

eListBoxEntryText *eComboBox::takeEntry( void *key )
{
	setCurrent(key);
	if (listbox.getCurrent() && key == listbox.getCurrent()->getKey() )
	{
		eListBoxEntryText *cur = listbox.getCurrent();
		listbox.take( cur );
		return cur;
	}
	return 0;
}

int eComboBox::setCurrent( const eListBoxEntryText* le, bool sendSelChanged )
{
	if (!le)
		return E_INVALID_ENTRY;

	int err = listbox.setCurrent( le, sendSelChanged );
	if( err && err != E_ALLREADY_SELECTED )
		return err;

	setText( listbox.getCurrent()->getText() );
	current = listbox.getCurrent();

	return OK;
}

struct selectEntryByNum
{
	int num;
	eListBox<eListBoxEntryText>* lb;
	bool sendSelChanged;

	selectEntryByNum(int num, eListBox<eListBoxEntryText> *lb, bool sendSelChanged=false): num(num), lb(lb), sendSelChanged(sendSelChanged)
	{
	}

	bool operator()(const eListBoxEntryText& le)
	{
		if (!num--)
		{
			lb->setCurrent(&le, sendSelChanged);
	 		return 1;
		}
		return 0;
	}
};

int eComboBox::setCurrent( int num, bool sendSelChanged )
{
	if ( num > listbox.getCount() )
		return E_INVALID_ENTRY;

	int err = listbox.forEachEntry( selectEntryByNum(num, &listbox, sendSelChanged ) );
	if ( err )
		return E_COULDNT_FIND;

	setText( listbox.getCurrent()->getText() );
	current = listbox.getCurrent();

	return OK;
}

struct selectEntryByKey
{
	void* key;
	eListBox<eListBoxEntryText>* lb;
	bool sendSelChanged;

	selectEntryByKey(void *key, eListBox<eListBoxEntryText> *lb, bool sendSelChanged=false):key(key), lb(lb), sendSelChanged(sendSelChanged)
	{
	}

	bool operator()(const eListBoxEntryText& le)
	{
		if ( le.getKey() == key )
		{
			lb->setCurrent(&le, sendSelChanged );
			return 1;
		}
		return 0;
	}
};

void eComboBox::lbLostFocus( const eWidget *w )
{
	if ( w != &listbox && listbox.isVisible() )
	{
		listbox.hide();
		eWindow::globalCancel( eWindow::ON );
	}
}

int eComboBox::setCurrent( void* key, bool sendSelChanged )
{
	if (!listbox.getCount())
		return E_COULDNT_FIND;

	int err;
	if ( (err=listbox.forEachEntry( selectEntryByKey(key, &listbox, sendSelChanged ) ) ) )
		return E_COULDNT_FIND;

	setText( listbox.getCurrent()->getText() );
	current = listbox.getCurrent();

	return OK; 
}

eListBoxEntryText* eComboBox::getCurrent()
{
	return current;
}
