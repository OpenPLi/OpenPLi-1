#include <lib/gui/emessage.h>

#include <lib/gui/elabel.h>
#include <lib/gui/ebutton.h>
#include <lib/gui/eskin.h>
#include <lib/gdi/font.h>
#include <lib/base/i18n.h>

eMessageBox::eMessageBox(eString message, eString caption, int flags, int def, int timeout )
	:eWindow(0), timer(0), sectimer(0), icon(0), def(0), timeout(timeout)
{
	init_eMessageBox(message,caption,flags,def,timeout);
}
void eMessageBox::init_eMessageBox(eString message, eString caption, int flags, int def, int timeout )
{
	if ( timeout )
	{
		timer = new eTimer(eApp);
		switch(def)
		{
			case btYes:
				CONNECT( timer->timeout, eMessageBox::pressedYes );
				break;
			case btNo:
				CONNECT( timer->timeout, eMessageBox::pressedNo );
				break;
			case btOK:
				CONNECT( timer->timeout, eMessageBox::pressedOK );
				break;
			default:
			case btCancel:
				CONNECT( timer->timeout, eMessageBox::pressedCancel );
				break;
		}
	}
	setText(caption);
	
	int fontsize = eSkin::getActive()->queryValue("fontsize", 20);
	eString posx = eSkin::getActive()->queryValue("eMessageBox.pos.x", "");
	eString posy = eSkin::getActive()->queryValue("eMessageBox.pos.y", "");
	/* set the default size, but wait till we've calculated the actual size, before we set the position */
	resize(eSize(450, 430));

	if ( flags > 15 ) // we have to draw an icon
	{
		gPixmap *pm=0;
		switch ( flags & ~15 )
		{
			case iconInfo:
				pm = eSkin::getActive()->queryImage( "icon_info" );
			break;
			case iconQuestion:
				pm = eSkin::getActive()->queryImage( "icon_question" );
			break;
			case iconWarning:
				pm = eSkin::getActive()->queryImage( "icon_warning" );
			break;
			case iconError:
				pm = eSkin::getActive()->queryImage( "icon_error" );
			break;
		}
		if (pm)
		{
			icon = new eLabel(this);
			icon->setPixmap( pm );
			icon->pixmap_position=ePoint(0,0);
			icon->resize( eSize(pm->x, pm->y) );
			icon->setBlitFlags( BF_ALPHATEST );
		}
	}

	text=new eLabel(this);
	text->setText(message);
	text->resize( eSize( clientrect.width(), clientrect.height() ));
	text->setFlags( RS_WRAP|eLabel::flagVCenter );
	eSize txtSize=text->getExtend();
	txtSize+=eSize(15,10);  // the given Size of the Text is okay... but the renderer sucks...
	text->resize(txtSize);

	// here the two labels ( icon, text) has the correct size..  now we calc the border

	eSize ext;

	if ( icon )
	{
		if ( icon->getSize().height() > text->getSize().height() )
		{
			// eDebug("icon is higher");
			eSize s = icon->getSize();
			icon->move( ePoint( 20, 20 ) );
			text->move( ePoint( 20 + s.width() + 20, icon->getPosition().y() + s.height() / 2 - txtSize.height() / 2 ) );
			ext.setHeight( icon->getPosition().y() + icon->getSize().height() + 20 );
		}
		else
		{
			// eDebug("text is higher");
			text->move( ePoint( 20 + icon->getSize().width() + 20 , 20 ) );
			icon->move( ePoint( 20, text->getPosition().y() + text->getSize().height() / 2 - icon->getSize().height() / 2 ) );
			ext.setHeight( text->getPosition().y() + text->getSize().height() + 20 );
		}
		ext.setWidth( text->getPosition().x() + text->getSize().width() + 20 );
	}
	else
	{
		text->move( ePoint(20, 20) );
		ext.setWidth( text->getPosition().x() + text->getSize().width()+20 );
		ext.setHeight( text->getPosition().y() + text->getSize().height() + 20 );
	}
	
	if (ext.width()<150)
		ext.setWidth(150);

	int xpos=20;

	if ( flags & 15)
	{
		for (int i=btOK; i<btMax; i<<=1)
			if (flags & i)
			{
				eButton *b=new eButton(this);
				b->resize(eSize(size.width(), fontsize+4));
				const char *t="", *shortcut="";
				switch (i)
				{
					case btOK: t=_("OK"); shortcut="green"; CONNECT(b->selected, eMessageBox::pressedOK); break;
					case btCancel: t=_("Cancel"); shortcut="red"; CONNECT(b->selected, eMessageBox::pressedCancel); break;
					case btYes: t=_("Yes"); shortcut="green"; CONNECT(b->selected, eMessageBox::pressedYes); break;
					case btNo: t=_("No"); shortcut="red"; CONNECT(b->selected, eMessageBox::pressedNo); break;
				}
				b->setProperty("shortcut", shortcut);
				b->setText(t);
				eSize bSize=b->getExtend();
				bSize.setWidth( bSize.width() * 2 );
				bSize.setHeight( fontsize + 4 + 10 );
				b->resize(bSize);
				b->move( ePoint( xpos, ext.height() ) );

				b->loadDeco();

				if (def == i)
					this->def = b;

				xpos += bSize.width()+20;
			}
		
		if ( timeout )
		{
			lTimeout = new eLabel(this);
			lTimeout->setText( eString().sprintf("%d", timeout) );
			lTimeout->resize( eSize(50, fontsize+4+10) );       
			lTimeout->move( ePoint( xpos+10, ext.height() ) );
			lTimeout->setFlags(eLabel::flagVCenter);
			xpos += lTimeout->getSize().width()+10;
		}

		if ( xpos+20 > ext.width() )
			cresize( eSize( xpos+20, ext.height() + fontsize + 4 + 10 + 20 ) );
		else
			cresize( eSize( ext.width(), ext.height() + fontsize + 4 + 10 + 20 ) );
	}
	else
		cresize( ext );

	/* now we know the size, set the position (which might depend on the size) */
	if (posx.size() && posy.size())
	{
		setProperty("position", posx + ":" + posy);
	}
	else
	{
		valign();
	}
	zOrderRaise();
}

int eMessageBox::eventHandler( const eWidgetEvent &e )
{
	switch (e.type)
	{
		case eWidgetEvent::evtAction:
			if ( timer && timer->isActive() )
			{
				timer->stop();
				delete sectimer;
				sectimer=0;
				updateTimeoutLabel();
			}
			break;
		case eWidgetEvent::execBegin:
			if ( def )
				setFocus(def);
			if ( timeout )
			{
				timer->start(timeout*1000, true);
				delete sectimer;
				sectimer = new eTimer(eApp);
				CONNECT( sectimer->timeout, eMessageBox::updateTimeoutLabel );
				sectimer->start(1000);
			}
		default:
			break;
	}
	return eWindow::eventHandler( e );
}

void eMessageBox::pressedOK()
{
	if ( in_loop )
	  close(btOK);
	else
		hide();
}

void eMessageBox::pressedCancel()
{
	if ( in_loop )
	  close(btCancel);
	else
		hide();
}

void eMessageBox::pressedYes()
{
	if ( in_loop )
	  close(btYes);
	else
		hide();
}

void eMessageBox::pressedNo()
{
	if ( in_loop )
		close(btNo);
	else
		hide();
}

void eMessageBox::updateTimeoutLabel()
{
	if ( timeout )
	{
		--timeout;
		lTimeout->setText(eString().sprintf("%d", timeout));
	}
	else
		lTimeout->hide();
}

eMessageBox::~eMessageBox()
{
	delete timer;
	delete sectimer;
}
