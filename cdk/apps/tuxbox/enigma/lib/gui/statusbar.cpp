#include <lib/gui/statusbar.h>

#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/font.h>

eStatusBar::eStatusBar( eWidget* parent, const char *deco)
	:eLabel(parent, 0, 0, deco), current(0)
{
	init_eStatusBar();
}
void eStatusBar::init_eStatusBar()
{
	setFont( eSkin::getActive()->queryFont("eStatusBar") );
	setForegroundColor ( eSkin::getActive()->queryColor("eStatusBar.foreground") );
	setBackgroundColor ( eSkin::getActive()->queryColor("eStatusBar.background") );
	eLabel::setFlags( RS_WRAP | eLabel::flagVCenter );
	eLabel::text_position = ePoint(eSkin::getActive()->queryValue("eStatusBar.margin", 0), 0);
	initialize();
}
void eStatusBar::initialize()
{
	if ( parent )
	{
		if ( flags & flagOwnerDraw )
		{
			if ( conn.connected() )
				conn.disconnect();
		}
		else
			conn = CONNECT( parent->focusChanged, eStatusBar::update );
	}
}

void eStatusBar::update( const eWidget* p )
{
	if (p)
	{
		current = p;
		setText( current->getHelpText() );
	}
}

void eStatusBar::setFlags( int fl )	
{
	eLabel::setFlags(fl);
	initialize();
}

int eStatusBar::setProperty(const eString &prop, const eString &value)
{
	if (prop=="ownerDraw")
		flags |= flagOwnerDraw;
	else
		return eLabel::setProperty(prop, value);

	initialize();

	return 0;
}

static eWidget *create_eStatusBar(eWidget *parent)
{
	return new eStatusBar(parent);
}

class eStatusBarSkinInit
{
public:
	eStatusBarSkinInit()
	{
		eSkin::addWidgetCreator("eStatusBar", create_eStatusBar);
	}
	~eStatusBarSkinInit()
	{
		eSkin::removeWidgetCreator("eStatusBar", create_eStatusBar);
	}
};

eAutoInitP0<eStatusBarSkinInit> init_eStatusBarSkinInit(eAutoInitNumbers::guiobject, "eStatusBar");
