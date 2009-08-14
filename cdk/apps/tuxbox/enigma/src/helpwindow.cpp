#include <helpwindow.h>
#include <xmltree.h>
#include <unistd.h>

#include <lib/base/i18n.h>
#include <lib/dvb/edvb.h>
#include <lib/gdi/font.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/guiactions.h>
#include <lib/system/init_num.h>
#include <lib/system/info.h>

struct enigmaHelpWindowActions
{
	eActionMap map;
	eAction close, up, down;
	enigmaHelpWindowActions():
		map("helpwindow", _("Help window")),
		close(map, "close", _("close the help window"), eAction::prioDialog),
		up(map, "up", _("scroll up"), eAction::prioDialogHi),
		down(map, "down", _("scroll down"), eAction::prioDialogHi)
	{
	}
};

eAutoInitP0<enigmaHelpWindowActions> i_helpwindowActions(eAutoInitNumbers::actions, "enigma helpwindow actions");

eHelpWindow::eHelpWindow(ePtrList<eAction> &parseActionHelpList, eString &helptext):
	eWindow(1), curPage(0)
{
	init_eHelpWindow(parseActionHelpList,helpID);
}
void eHelpWindow::init_eHelpWindow(ePtrList<eAction> &parseActionHelpList, int helpID)
{
	int xpos=60, ypos=0, labelheight, imgheight;

	scrollbar = new eProgress(this);
	scrollbar->setName("scrollbar");
	scrollbar->setStart(0);
	scrollbar->setPerc(100);

	visible = new eWidget(this);
	visible->setName("visible");

	eSkin *skin=eSkin::getActive();
	if (skin->build(this, "eHelpWindow"))
		eFatal("skin load of \"eHelpWindow\" failed");

	scrollbox = new eWidget(visible);
	scrollbox->move(ePoint(0, 0));
	scrollbox->resize(eSize(visible->width(), visible->height()*8));

	const std::set<eString> &styles=eActionMapList::getInstance()->getCurrentStyles();

	lastEntry=0;
	entryBeg[lastEntry++]=0;
	int pageend=visible->height();

	for ( ePtrList<eAction>::iterator it( parseActionHelpList.begin() ); it != parseActionHelpList.end() ; it++ )
	{
		std::map< eString, keylist >::iterator b;
		
		for (std::set<eString>::const_iterator si(styles.begin()); si != styles.end(); ++si)
		{
			b=it->keys.find(*si);
			if (b == it->keys.end())
				continue;

			keylist &keys = b->second;
			for ( keylist::iterator i( keys.begin() ); i != keys.end() ; i++ )
			{
				imgheight=0;
				if ( strstr( i->producer->getDescription(), eSystemInfo::getInstance()->getHelpStr() ) )
				{
					if (i->picture)
					{
						gPixmap *image=eSkin::getActive()->queryImage(i->picture);

						if (image)
						{
							label = new eLabel(scrollbox);
							label->setFlags(eLabel::flagVCenter);
							label->move(ePoint(0, ypos));
							imgheight = image->y;
							label->resize(eSize(xpos, imgheight));
							label->setBlitFlags(BF_ALPHATEST);
							label->setPixmap(image);
							label->setPixmapPosition(ePoint((xpos-10)/2-image->x/2, 0));
						}
					}

					label = new eLabel(scrollbox);
					label->setFlags(eLabel::flagVCenter);
					label->setFlags(RS_WRAP);
					label->move(ePoint(xpos, ypos));
					label->resize(eSize(visible->width()-xpos-20, 200));
					// since they are inited before language is set, call gettext again.
					label->setText(gettext(it->getDescription()));
					labelheight=label->getExtend().height();
					int hi = (labelheight > imgheight) ? labelheight : imgheight;
					label->resize(eSize(visible->width() - xpos - 20, hi));

					ypos += hi + 20;
					if ( ypos - 20 > pageend )
					{
						pageend=ypos - hi - 20;
						entryBeg[lastEntry++]=pageend;
						pageend+=visible->height();
					}
					break;  // add only once :)
				}
			}
		}
	}

	if (helptext.size())
	{
		label = new eLabel(scrollbox);
		label->setFlags(eLabel::flagVCenter);
		label->setFlags(RS_WRAP);
		label->move(ePoint(0, ypos));
		label->resize(eSize(visible->width(), 200));
		label->setText(::gettext(helptext.c_str()));
		labelheight = label->getExtend().height();
		label->resize(eSize(visible->width(), labelheight));
		int tmp = ypos+labelheight;
		while ( tmp > pageend )
		{
			entryBeg[lastEntry++] = ypos - 20;
			ypos += visible->height() - 20;
			pageend = ypos;
		}
	}

	--lastEntry;
	cur = 0;
	doscroll=ypos>visible->height();

	if (!doscroll)
		scrollbar->hide();
	else
		updateScrollbar();
	  
	addActionMap(&i_helpwindowActions->map);
}

int eHelpWindow::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
	case eWidgetEvent::evtAction:
		if (event.action == &i_helpwindowActions->up)
		{
			if (doscroll && cur > 0 ) // valid it
			{
				--curPage;
				scrollbox->move(ePoint(0, -entryBeg[--cur]));
				updateScrollbar();
			}
		}
		else if (event.action == &i_helpwindowActions->down)
		{
			if (doscroll && cur < lastEntry ) // valid it
			{
				++curPage;
				scrollbox->move(ePoint(0, -entryBeg[++cur]));
				updateScrollbar();
			}
		}
		else if (event.action == &i_helpwindowActions->close)
			close(0);
		else
			break;
		return 1;
	default:
		break;
	}
	return eWindow::eventHandler(event);
}

void eHelpWindow::updateScrollbar()
{
	int total=(lastEntry+1)*visible->height();
	int start=curPage*visible->height()*100/total;
	int vis=visible->getSize().height()*100/total;
//	eDebug("total=%d, start = %d, vis = %d", total, start, vis);
	scrollbar->setParams(start,vis);
	scrollbar->show();
}

eHelpWindow::~eHelpWindow()
{
}

