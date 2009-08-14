#include <lib/gui/listbox.h>
#include <lib/gui/eprogress.h>
#include <lib/gui/numberactions.h>
#include <lib/system/econfig.h>
#include <lib/gdi/font.h>

gFont eListBoxEntryText::font;
gFont eListBoxEntryTextStream::font;

eListBoxBase::eListBoxBase(eWidget* parent, const eWidget* descr, int takefocus, int item_height, const char *deco )
:   eDecoWidget(parent, takefocus, deco),
		removed_height_pixel(0), descr(descr),
#ifndef DISABLE_LCD
		tmpDescr(0),
#endif
		colorActiveB(eSkin::getActive()->queryScheme("listbox.selected.background")),
		colorActiveF(eSkin::getActive()->queryScheme("listbox.selected.foreground")),
		movemode(0), MaxEntries(0), flags(0), item_height(item_height),
		columns(1), in_atomic(0), entries(0), currentPos(-1), top(childs.end()), bottom(childs.end()), current(childs.end())
{
	init_eListBoxBase();
}
void eListBoxBase::init_eListBoxBase()
{
	scrollbar = new eProgress(this);
	scrollbar->setDirection(1);
	childs.setAutoDelete(false);	// machen wir selber
	addActionMap(&i_cursorActions->map);
	addActionMap(&i_listActions->map);
	if ( !colorActiveB )
		colorActiveB=eSkin::getActive()->queryScheme("global.selected.background");
	if ( !colorActiveF )
		colorActiveF=eSkin::getActive()->queryScheme("global.selected.foreground");
	gColor col = eSkin::getActive()->queryScheme("listbox.normal.background");
	if (col)
		backgroundColor=col;
	col = eSkin::getActive()->queryScheme("listbox.normal.foreground");
	if (col)
			foregroundColor=col;
}

eListBoxBase::~eListBoxBase()
{
	ePtrList<eListBoxEntry>::iterator it(childs.begin());
	while (it != childs.end())
	{
		it->clearLB();
		delete *it;
		childs.erase(it++);
	}
}

void eListBoxBase::setFlags(int _flags)	
{
	flags |= _flags;	
	if (_flags == flagHasShortcuts)
		addActionMap(&i_shortcutActions->map);
}

void eListBoxBase::removeFlags(int _flags)	
{
	flags &= ~_flags;	
	if (_flags == flagHasShortcuts)
		removeActionMap(&i_shortcutActions->map);
}

void eListBoxBase::recalcMaxEntries()
{
		// MaxEntries is PER COLUMN
	int decoheight=0;
	if (deco_selected && have_focus)
	{
		MaxEntries = crect_selected.height();
		decoheight = height() - crect_selected.height();
	}
	else if (deco)
	{
		MaxEntries = crect.height();
		decoheight = height() - crect.height();
	}
	else
		MaxEntries = height();
	int tmp = MaxEntries;
	MaxEntries /= item_height;
/*	eDebug("height = %d, MaxEntries = %d, item height = %d",
		tmp, MaxEntries, item_height);*/
	// das ist echt mal komischer code hier.. aber funktioniert :)
	// damit werden listboxen automatisch auf die höhe resized,
	// die benötigt wird damit alle entrys genau sichtbar sind..
	// und kein Rand bleibt..
	if ( tmp - ( MaxEntries*item_height ) > 0 )
	{
		if ( !removed_height_pixel )
		{
			removed_height_pixel = height() - ((MaxEntries*item_height) + decoheight);
			resize( eSize( size.width(), height()-removed_height_pixel ) );
		}
		else
		{
			int newMax = (tmp + removed_height_pixel) / item_height;
			if ( newMax > MaxEntries )
			{
				removed_height_pixel -= (newMax*item_height) - tmp;
				resize( eSize( size.width(), newMax*item_height+decoheight ) );
			}
			else
			{
				int tmp = height() - ((MaxEntries*item_height) + decoheight);
				resize( eSize( size.width(), height() - tmp ) );
				removed_height_pixel += tmp;
			}
		}
	}
/*	else
		eDebug("is ok .. do nothing");*/
}

eRect eListBoxBase::getEntryRect(int pos)
{
	bool sbar =
		( entries > (unsigned int)(MaxEntries*columns) && MaxEntries*columns > 1 );

	int lme=MaxEntries;
			// in case we show partial last lines (which only works in single-column),
			// we increase MaxEntries by one since we don't want the last line
			// one the next (invisible) column
	if ( (columns == 1) && (flags & flagShowPartial))
		++lme;
	if ( deco_selected && have_focus )
		return eRect( ePoint( deco_selected.borderLeft + ( ( pos / lme) * ( crect_selected.width() / columns ) ) , deco_selected.borderTop + ( pos % lme) * item_height ), eSize( (crect_selected.width() / columns) - (sbar?scrollbar->width()+5:columns>1?5:0), item_height ) );
	else if (deco)
		return eRect( ePoint( deco.borderLeft + ( ( pos / lme ) * ( crect.width() / columns ) ) , deco.borderTop + ( pos % lme) * item_height ), eSize( (crect.width() / columns) - (sbar?scrollbar->width()+5:columns>1?5:0), item_height ) );
	else if ( deco_selected )
		return eRect( ePoint( deco_selected.borderLeft + ( ( pos / lme) * ( crect_selected.width() / columns ) ) , deco_selected.borderTop + ( pos % lme) * item_height ), eSize( (crect_selected.width() / columns) - (sbar?scrollbar->width()+5:columns>1?5:0), item_height ) );
	else
		return eRect( ePoint( ( ( pos / lme ) * ( size.width() / columns ) ) , ( pos % lme) * item_height ), eSize( (size.width() / columns) - (sbar?scrollbar->width()+5:columns>1?5:0), item_height ) );
}

void eListBoxBase::setColumns(int col)
{
	if (col)
		columns=col;
}

int eListBoxBase::setProperty(const eString &prop, const eString &value)
{
	if (prop == "noPageMovement")
	{
		if (value == "off")
			flags &= ~flagNoPageMovement;
		else
			flags |= flagNoPageMovement;
	}
	else if (prop == "noUpDownMovement")
	{
		if (value == "off")
			flags &= ~flagNoUpDownMovement;
		else
			flags |= flagNoUpDownMovement;
	}
	else if (prop=="activeForegroundColor")
		colorActiveF=eSkin::getActive()->queryScheme(value);
	else if (prop=="activeBackgroundColor")
		colorActiveB=eSkin::getActive()->queryScheme(value);
	else if (prop=="showEntryHelp")
		setFlags( flagShowEntryHelp );
	else if (prop=="columns")
		setColumns( value?atoi(value.c_str()):1 );
	else
		return eDecoWidget::setProperty(prop, value);

	return 0;
}

void eListBoxBase::recalcScrollBar()
{
	int scrollbarwidth=0;
	eRect rc;
	if ( have_focus && deco_selected )
		rc = crect_selected;
	else if ( deco )
		rc = crect;
	else
		rc = clientrect;
	scrollbarwidth=rc.width()/16;
	if ( scrollbarwidth < 18 )
		scrollbarwidth=18;
	if ( scrollbarwidth > 22 )
		scrollbarwidth=22;
	scrollbar->move( ePoint(rc.right() - scrollbarwidth, rc.top()) );
	scrollbar->resize( eSize( scrollbarwidth, (MaxEntries*item_height) ) );
	scrollbar->hide();
}

void eListBoxBase::recalcClientRect()
{
	if (deco)
	{
		crect.setLeft( deco.borderLeft );
		crect.setTop( deco.borderTop );
		crect.setRight( width() - deco.borderRight );
		crect.setBottom( height()  - deco.borderBottom );
	}
	if (deco_selected)
	{
		crect_selected.setLeft( deco_selected.borderLeft );
		crect_selected.setTop( deco_selected.borderTop );
		crect_selected.setRight( width() - deco_selected.borderRight );
		crect_selected.setBottom( height() - deco_selected.borderBottom );
	}
	eWidget::recalcClientRect();
}

int eListBoxBase::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
		case eWidgetEvent::changedSize:
			eWidget::eventHandler(event);
			recalcMaxEntries();
			recalcScrollBar();
			init();
			return 1;
		break;
		case eWidgetEvent::evtAction:
			if ((flags & flagHasShortcuts) && eventHandlerShortcuts(event))
				return 1;
			else if ((event.action == &i_listActions->pageup) && !(flags & flagNoPageMovement))
				moveSelection(dirPageUp);
			else if ((event.action == &i_listActions->pagedown) && !(flags & flagNoPageMovement))
				moveSelection(dirPageDown);
			else if ( entries && current->eventHandler(event) )
				return 1;
			else if ((event.action == &i_cursorActions->up) && !(flags & flagNoUpDownMovement) && !(flags&flagLostFocusOnFirst && current == childs.begin()) )
				moveSelection(dirUp);
			else if ((event.action == &i_cursorActions->down) && !(flags & flagNoUpDownMovement) && !(flags&flagLostFocusOnLast && current == --childs.end()) )
				moveSelection(dirDown);
			else if (event.action == &i_cursorActions->ok)
			{
				if ( current == childs.end() )
					/*emit*/ SendSelected(0);
				else
					/*emit*/ SendSelected(*current);
			}
			else if (event.action == &i_cursorActions->cancel && MaxEntries > 1 )
				/*emit*/ SendSelected(0);
			else
				break;
		return 1;
		default:
		break;
	}
	return eWidget::eventHandler(event);
}

void eListBoxBase::invalidateContent()
{
	if ( have_focus && deco_selected )
		invalidate( crect_selected );
	else if ( deco )
		invalidate( crect );
	else
		invalidate();
}

int eListBoxBase::newFocus()
{
	if (deco && deco_selected)
	{
		recalcMaxEntries();
		recalcScrollBar();

		if (isVisible())
			invalidate();

		return 1;
	}
	return 0;
}

int eListBoxBase::setCurrent(const eListBoxEntry *c, bool sendSelected )
{
/*	if (childs.empty() || ((current != childs.end()) && (*current == c)))  // no entries or current is equal the entry to search
		return E_ALLREADY_SELECTED;	// do nothing*/

	if ( !entries )
		return E_COULDNT_FIND;

	ePtrList<eListBoxEntry>::iterator item(childs.begin()), it(childs.begin());

	int cnt=0;
	for ( ; item != childs.end(); ++item, ++cnt)
		if ( *item == c )
			break;

	if ( item == childs.end() ) // entry not in listbox... do nothing
		return E_COULDNT_FIND;

	currentPos=cnt;

	int newCurPos=-1;
	int oldCurPos=-1;
	ePtrList<eListBoxEntry>::iterator oldCur(current);

	int i = 0;

	for (it=top; it != bottom; ++it, ++i)  // check if entry to set between bottom and top
	{
		if (it == item)
		{
			newCurPos=i;
			current = it;
		}
		if ( it == oldCur)
			oldCurPos=i;
	}

	if (newCurPos != -1)	// found on current screen, so redraw only old and new
	{
		if (isVisible())
		{
			if (in_atomic)
			{
				if (atomic_redraw == arNothing)
				{
					atomic_redraw = arCurrentOld;
					atomic_old = oldCurPos;
				}
				if (atomic_redraw == arCurrentOld)
					atomic_new = newCurPos;
			}else
			{
				invalidateEntry(newCurPos);
				if (oldCurPos != -1)
					invalidateEntry(oldCurPos);
			}
		}
	} else // then we start to search from begin
	{
		bottom = childs.begin();

		while (newCurPos == -1 && MaxEntries)  // MaxEntries is already checked above...
		{
			if ( bottom != childs.end() )
				top = bottom;		// nächster Durchlauf

			for (i = 0; (i < (MaxEntries*columns) ) && (bottom != childs.end()); ++bottom, ++i)
			{
				if (bottom == item)
				{
					current = bottom;  // we have found
					++newCurPos;
				}
			}
		}
		if (isVisible())
		{
			if (!in_atomic)
				invalidateContent();   // Draw all
			else
				atomic_redraw=arAll;
		}
	}

	if (!in_atomic)
	{
		/*emit*/ SendSelChanged(*current);
		if ( sendSelected )
			/*emit*/ SendSelected(*current);
	}else
	{
		atomic_selchanged=1;
		if ( sendSelected )
			atomic_selected=1;
	}

	return OK;
}

void eListBoxBase::append(eListBoxEntry* entry, bool holdCurrent, bool front)
{
	eListBoxEntry* cur = 0;
	if (holdCurrent)
		cur = current;

	if ( front )
		childs.push_front(entry);
	else
		childs.push_back(entry);
	++entries;
	init();

	if (cur)
		setCurrent(cur);
}

void eListBoxBase::take(eListBoxEntry* entry, bool holdCurrent)
{
	eListBoxEntry *cur = 0;

	if (holdCurrent && current != entry)
		cur = current;

	childs.take(entry);
	if ( entries != childs.size() )
	{
		--entries;
		init();
	}

	if (cur)
		setCurrent(cur);
}

void eListBoxBase::clearList()
{
	ePtrList<eListBoxEntry>::iterator it(childs.begin());
	while (entries)
	{
		it->clearLB();
		delete *it;
		childs.erase(it++);
		--entries;
	}
	current = top = bottom = childs.end();
	if (!in_atomic)
	{
		/*emit*/ SendSelChanged(0);
		invalidateContent();
	} else
	{
		atomic_selected=0;
		atomic_selchanged=0;
		atomic_redraw=arAll;
		atomic_new=0;
		atomic_old=0;
	}
	currentPos=-1;
}

eListBoxEntry *eListBoxBase::goNext()
{
	moveSelection(dirDown);
	return current!=childs.end() ? *current : 0;
}

eListBoxEntry* eListBoxBase::goPrev()
{
	moveSelection(dirUp);
	return current!=childs.end() ? *current : 0;
}

void eListBoxBase::redrawWidget(gPainter *target, const eRect &where)
{
	// redraw scrollbar only...
	if ( where.size() == scrollbar->getSize() &&
		where.topLeft() == scrollbar->getPosition() )
		return;

	eRect rc;

	if (deco_selected && have_focus)
	{
		deco_selected.drawDecoration(target, ePoint(width(), height()));
		rc = crect_selected;
	}
	else if (deco)
	{
		deco.drawDecoration(target, ePoint(width(), height()));
		rc = crect;
	}
	else
		rc = where;

	if ( currentPos == -1 )
	{
		currentPos = 0;
		ePtrList<eListBoxEntry>::iterator it = childs.begin();
		for (; it != childs.end() ;++it, ++currentPos )
			if ( it == current )
				break;
	}

	if ( entries > (unsigned int)(MaxEntries*columns) && MaxEntries*columns > 1 )
	{
		int pages = entries / (MaxEntries*columns);
		if ( (unsigned int)(pages*MaxEntries*columns) < entries )
			pages++;
		int start=(currentPos/(MaxEntries*columns)*MaxEntries*columns*100)/(pages*MaxEntries*columns);
		int vis=MaxEntries*columns*100/(pages*MaxEntries*columns);
		if (vis < 3)
			vis=3;
		scrollbar->setParams(start,vis);
		scrollbar->show();
	}
	else
		scrollbar->hide();

	int i=0;
	for (ePtrList<eListBoxEntry>::iterator entry(top); ((flags & flagShowPartial) || (entry != bottom)) && (entry != childs.end()); ++entry)
	{
		eRect rect = getEntryRect(i);
		if ( rc.intersects(rect) )
		{
			target->clip(rect & rc);
			if ( entry == current )
			{
#ifndef DISABLE_LCD
				if ( LCDTmp ) // LCDTmp is only valid, when we have the focus
					LCDTmp->setText( entry->redraw(target, rect, colorActiveB, colorActiveF, getBackgroundColor(), getForegroundColor(), 1 ) );
				else if ( parent->LCDElement && have_focus )
					parent->LCDElement->setText( entry->redraw(target, rect, colorActiveB, colorActiveF, getBackgroundColor(), getForegroundColor(), 1 ) );
				else
#endif
					entry->redraw(target, rect, colorActiveB, colorActiveF, getBackgroundColor(), getForegroundColor(), ( have_focus ? 1 : ( MaxEntries > 1 ? 2 : 0 ) )	);
			}
			else
				entry->redraw(target, rect, colorActiveB, colorActiveF, getBackgroundColor(), getForegroundColor(), 0 /*( have_focus ? 0 : ( MaxEntries > 1 ? 2 : 0 ) )*/	);
			target->clippop();
		}
				// special case for "showPartial": as bottom is set to the
				// last, half visible entry we want to redraw this, too.
		if (flags & flagShowPartial)
			if (entry == bottom)
				break;

		++i;
	}
}

void eListBoxBase::gotFocus()
{
#ifndef DISABLE_LCD
	if (parent && parent->LCDElement)  // detect if LCD Avail
		if (descr)
		{
			parent->LCDElement->setText("");
			LCDTmp = new eLabel(parent->LCDElement);
			LCDTmp->hide();
			eSize s = parent->LCDElement->getSize();
			LCDTmp->move(ePoint(0,s.height()/2));
			LCDTmp->resize(eSize(s.width(), s.height()/2));
			LCDTmp->show();
			tmpDescr = new eLabel(parent->LCDElement);
			tmpDescr->hide();
			tmpDescr->move(ePoint(0,0));
			tmpDescr->resize(eSize(s.width(), s.height()/2));
			tmpDescr->setText( descr->getText() );
			tmpDescr->show();
		}
#endif
	++have_focus;
	if (entries)
	{
		if ( newFocus() )   // recalced ?
		{
			ePtrList<eListBoxEntry>::iterator it = current;
			init();
			setCurrent(it);
		}
		else if ( isVisible() )
		{
			int i=0;
			for (ePtrList<eListBoxEntry>::iterator entry(top); entry != bottom; ++i, ++entry)
				if (entry == current)
					invalidateEntry(i);
		}
	}
	if (flags & flagShowEntryHelp)
		setHelpText( current != childs.end() ? current->getHelpText(): eString(" ")); // eString(_("no description available")));
}

void eListBoxBase::lostFocus()
{
#ifndef DISABLE_LCD
	if ( descr )
	{
		delete LCDTmp;
		LCDTmp=0;
		delete tmpDescr;
		tmpDescr=0;
	}
#endif
	--have_focus;
	if (entries)
		if ( newFocus() ) //recalced ?
		{
			ePtrList<eListBoxEntry>::iterator it = current;
			init();
			setCurrent(it);
		}
		else if ( isVisible() )
		{
			int i = 0;
			for (ePtrList<eListBoxEntry>::iterator entry(top); entry != bottom; ++i, ++entry)
				if (entry == current)
					invalidateEntry(i);
		}
#ifndef DISABLE_LCD
	if (parent && parent->LCDElement)
		parent->LCDElement->setText("");
#endif
}

void eListBoxBase::invalidateCurrent()
{
	int n=0;
	for (ePtrList<eListBoxEntry>::iterator i(top); i != bottom; ++i, ++n)
		if ( i == current )
			invalidate(getEntryRect(n));
}

void eListBoxBase::init()
{
	currentPos=entries?0:-1;
	current = top = bottom = childs.begin();
	for (int i = 0; i < (MaxEntries*columns); ++i, ++bottom)
		if (bottom == childs.end() )
			break;

	/* when the first item isn't a selectable item, keep scrolling down till we find one */
	while ( !current->isSelectable() && current != --childs.end())
	{
		++current;
		++currentPos;
	}

	if (!in_atomic)
	{
		invalidateContent();
	}
	else
	{
		atomic_redraw=arAll;
	}
}

int eListBoxBase::moveSelection(int dir, bool sendSelected)
{
	int direction=0, forceredraw=0;

	if (!entries)
		return 0;

	ePtrList<eListBoxEntry>::iterator oldptr=current, oldtop=top;

	switch (dir)
	{
		case dirPageDown:
			direction=+1;
			for (int i = 0; i < MaxEntries; ++i)
			{
				++currentPos;
				if (++current == bottom) // unten (rechts) angekommen? page down
				{
					if (bottom == childs.end()) // einzige ausnahme: unten (rechts) angekommen
					{
						--currentPos;
						--current;
						break;
					}
					for (int i = 0; i < MaxEntries * columns; ++i)
					{
						if (bottom != childs.end())
							++bottom;
						if (top != childs.end())
							++top;
					}
				}
			}
			/* when we didn't find a selectable item, keep scrolling down till we find one */
			while ( !current->isSelectable() && current != --childs.end())
			{
				++current;
				++currentPos;
			}
			/* when we couldn't find a selectable item below, find the nearest one above */
			while ( !current->isSelectable() && current != childs.begin())
			{
				--current;
				--currentPos;
			}
		break;

		case dirPageUp:
			direction=-1;
			for (int i = 0; i < MaxEntries; ++i)
			{
				if (current == childs.begin())
					break;
				--currentPos;
				if (current-- == top/* && current != childs.begin()*/ )	// oben (links) angekommen? page up
				{
					for (int i = 0; i < MaxEntries * columns; ++i)
					{
						if (--top == childs.begin()) 		// einzige ausnahme: oben (links) angekommen
							break;
					}

					// und einmal bottom neuberechnen :)
					bottom=top;
					for (int i = 0; i < MaxEntries*columns; ++i)
						if (bottom != childs.end())
							++bottom;
				}
			}
			/* when we didn't find a selectable item, keep scrolling up till we find one */
			while ( !current->isSelectable() && current != childs.begin())
			{
				--current;
				--currentPos;
			}
			/* when we couldn't find a selectable item above, find the nearest one below */
			while ( !current->isSelectable() && current != --childs.end())
			{
				++current;
				++currentPos;
			}
			break;

		case dirUp:
		{
			bool first=true;
			while (first || !current->isSelectable() )
			{
				first=false;
				if ( current == childs.begin() )				// wrap around?
				{
					direction=+1;
					top = bottom = current = childs.end();
					--current;
					currentPos=entries-1;
					int cnt = entries%(MaxEntries*columns);
					for (int i = 0; i < (cnt?cnt:MaxEntries*columns); ++i, --top)
						if (top == childs.begin())
							break;
				}
				else
				{
					direction=-1;
					--currentPos;
					if (current-- == top) // new top must set
					{
						for (int i = 0; i < MaxEntries*columns; ++i, --top)
							if (top == childs.begin())
								break;
						bottom=top;
						for (int i = 0; i < MaxEntries*columns; ++i, ++bottom)
							if (bottom == childs.end())
								break;
					}
				}
			}
		}
		break;

		case dirDown:
		{
			bool first=true;
			while (first || !current->isSelectable() )
			{
				first=false;
				if ( current == --ePtrList<eListBoxEntry>::iterator(childs.end()) )				// wrap around?
				{
					direction=-1;
					currentPos=0;
					top = current = bottom = childs.begin(); 	// goto first
					for (int i = 0; i < MaxEntries * columns; ++i, ++bottom)
						if ( bottom == childs.end() )
							break;
				}
				else
				{
					direction=+1;
					++currentPos;
					if (++current == bottom)   // ++current ??
					{
						for (int i = 0; i<MaxEntries * columns; ++i)
						{
							if (bottom != childs.end() )
								++bottom;
							if (top != childs.end() )
								++top;
						}
					}
				}
			}
			break;
		}
		case dirFirst:
			direction=-1;
			currentPos=0;
			top = current = bottom = childs.begin(); 	// goto first;
			for (int i = 0; i < MaxEntries * columns; ++i, ++bottom)
				if ( bottom == childs.end() )
					break;
			/* when the first item isn't a selectable item, keep scrolling down till we find one */
			while ( !current->isSelectable() && current != --childs.end())
			{
				++current;
				++currentPos;
			}
			break;
		case dirLast:
		{
			direction=1;
			top = bottom = current = childs.end();
			--current;
			currentPos=entries-1;
			int cnt = entries%(MaxEntries*columns);
			for (int i = 0; i < (cnt?cnt:MaxEntries*columns); ++i, --top)
				if (top == childs.begin())
					break;
			/* when the last item isn't a selectable item, keep scrolling up till we find one */
			while ( !current->isSelectable() && current != childs.begin())
			{
				--current;
				--currentPos;
			}
			break;
		}
		default:
			return 0;
	}

	if (current != oldptr)  // current has changed
	{
		if (movemode)
		{
				// feel free to rewrite using stl::copy[_backward], but i didn't succeed.
			std::list<eListBoxEntry*>::iterator o=oldptr,
																				c=current,
																				curi=current,
																				oldi=oldptr;
			int count=0;

			eListBoxEntry* old=*o;

			if (direction > 0)
			{
				++o;
				++c;
				while (o != c)
				{
					*oldi++=*o++;
					++count;
				}
			} else
			{
				while (o != curi)
				{
					*oldi--=*--o;
					++count;
				}
			}

			if (count > 1)
				forceredraw=1;

			*curi=old;
		}
	}

	if (flags & flagShowEntryHelp)
		setHelpText( current != childs.end() ? current->getHelpText():eString(_("no description available")));

	if (!in_atomic)
	{
		/*emit*/ SendSelChanged(*current);
		if ( sendSelected )
			/*emit*/ SendSelected(*current);
	}
	else
	{
		atomic_selchanged=1;
		if ( sendSelected )
			atomic_selected=1;
	}

	if (isVisible())
	{
		if ((oldtop != top) || forceredraw)
		{
			if (in_atomic)
				atomic_redraw=arAll;
			else
				invalidateContent();
		} else if ( current != oldptr)
		{
			int i=0;
			int old=-1, cur=-1;

			for (ePtrList<eListBoxEntry>::iterator entry(top); entry != bottom; ++i, ++entry)
				if ( entry == oldptr)
					old=i;
				else if ( entry == current )
					cur=i;

			if (in_atomic)
			{
				if (atomic_redraw == arNothing)
				{
					atomic_old=old;
					atomic_redraw = arCurrentOld;
				}
				if (atomic_redraw == arCurrentOld)
					atomic_new=cur;
			} else
			{
				if (old != -1)
					invalidateEntry(old);
				if (cur != -1)
					invalidateEntry(cur);
			}
		}
	}
	return 1;
}

void eListBoxBase::setActiveColor(gColor back, gColor front)
{
	if (back)
    		colorActiveB=back;
	if (front)
		colorActiveF=front;

	if ((back || front) && current != childs.end())
	{
		int i = 0;
		for (ePtrList<eListBoxEntry>::iterator it(top); it != bottom; ++i, ++it)
		{
			if (it == current)
			{
				invalidateEntry(i);
				break;
			}
		}
	}
}

void eListBoxBase::beginAtomic()
{
	if (!in_atomic++)
	{
		atomic_redraw=arNothing;
		atomic_selchanged=0;
		atomic_selected=0;
		atomic_new=-1;
	}
}

void eListBoxBase::endAtomic()
{
	if (!--in_atomic)
	{
		if (atomic_redraw == arAll)
			invalidateContent();
		else if (atomic_redraw == arCurrentOld)
		{
			if (atomic_new != -1)
				invalidateEntry(atomic_new);
			if (atomic_old != -1)
				invalidateEntry(atomic_old);
		}
		if (atomic_selchanged)
			if (!entries)
				/*emit*/ SendSelChanged(0);
			else
				/*emit*/ SendSelChanged(*current);

		if (atomic_selected)
			if (!entries)
				/*emit*/ SendSelected(0);
			else
				/*emit*/ SendSelected(*current);
	}
}

void eListBoxBase::sort()
{
	eListBoxEntry* cur = current;
	childs.sort();

	init();

	if (cur)
		setCurrent(cur);
}

int eListBoxBase::getShortcut(eListBoxEntry* e)
{
	int i = 0;
	for (ePtrList<eListBoxEntry>::iterator it(childs.begin());
		it != childs.end(); ++it )
	{
		if ( it->isSelectable()&2)
		{
			i++;
			if ( it == e )
				return i;
		}
	}
	return -1;	
}

int eListBoxBase::eventHandlerShortcuts(const eWidgetEvent &event)
{
	int num=-1;
	int ColorShortcutsFirst = (getFlags() & flagColorShortcutsFirst ? 1 : 0);

	switch (event.type)
	{
	case eWidgetEvent::evtAction:
		if (event.action == &i_shortcutActions->number0)
			num=9 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number1)
			num=0 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number2)
			num=1 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number3)
			num=2 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number4)
			num=3 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number5)
			num=4 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number6)
			num=5 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number7)
			num=6 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number8)
			num=7 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->number9)
			num=8 + ColorShortcutsFirst*4;
		else if (event.action == &i_shortcutActions->red)
			num=10 - ColorShortcutsFirst*10;
		else if (event.action == &i_shortcutActions->green)
			num=11 - ColorShortcutsFirst*10;
		else if (event.action == &i_shortcutActions->yellow)
			num=12 - ColorShortcutsFirst*10;
		else if (event.action == &i_shortcutActions->blue)
			num=13 - ColorShortcutsFirst*10;
		else
			break;
		if (num != -1)
		{
			for (ePtrList<eListBoxEntry>::iterator i(childs.begin()); i!=childs.end(); ++i)
				if (i->isSelectable()&2 && !num--)
				{
					SendSelected(i);
					return 1;
				}
		}
	default:
		break;
	}
	return 0;
}

void eListBoxBase::shuffle()
{
	unsigned i = 0 , j;
	unsigned int listsize = childs.size();

	if (listsize<2) return;

	ePtrList<eListBoxEntry>::iterator it(childs.begin());

	while (it !=current && it != childs.end())
	{
		it++;
		i++;
	}

	if (it == childs.end()) return;

	srand(time(0));

	for (;i<listsize;i++)
	{
		ePtrList<eListBoxEntry>::iterator swapit=it;

		unsigned int r=rand();	

		for (j=0;j<(r%(listsize-i));j++)
		{
			swapit++;
		}
		eListBoxEntry* temp = *swapit;
		*swapit  = *it;
		*it = temp;

		it++;
	}
}

eListBoxBaseExt::eListBoxBaseExt(eWidget* parent, const eWidget* descr, int takefocus, int item_height, const char *deco)
	:eListBoxBase(parent, descr, takefocus, item_height, deco), browseTimer(eApp)
{
	CONNECT(browseTimer.timeout, eListBoxBaseExt::browseTimeout);
	addActionMap(&i_numberActions->map);
}

void eListBoxBaseExt::gotFocus()
{
	eListBoxBase::gotFocus();
	eRCInput::getInstance()->setKeyboardMode(eRCInput::kmAscii);
}

int eListBoxBaseExt::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
		case eWidgetEvent::evtAction:
			if (event.action == &i_cursorActions->ok)
			{
				browseText="";
				browseHistory.clear();
				return eListBoxBase::eventHandler(event);
			}
			else if (event.action == &i_numberActions->keyBackspace)
			{
				if ( browseText )
					browseText.erase(browseText.length()-1,1);
				if ( browseHistory.size() )
				{
					setCurrent(browseHistory.front(),false);
					browseHistory.pop_front();
				}
				browseTimer.start(2*1000,true);
			}
			else if (event.action == &i_cursorActions->down && browseTimer.isActive())
			{
				const char *browseBuf = browseText.c_str();
				int len = browseText.length();
				ePtrList<eListBoxEntry>::iterator it(current);
				++it;
				for (;it != childs.end(); ++it )
				{
					if ( !strncasecmp(it->getText().c_str(), browseBuf, len) )
					{
						if ( it != current )
						{
							setCurrent(*it,false);
							browseTimer.start(2*1000,true);
						}
						return 1;
					}
				}
				it=childs.begin();
				for (;it != childs.end(); ++it)
				{
					if ( !strncasecmp(it->getText().c_str(), browseBuf, len) )
					{
						if ( it != current )
						{
							setCurrent(*it,false);
							browseTimer.start(2*1000,true);
						}
						return 1;
					}
				}
			}
			else if (event.action == &i_cursorActions->up && browseTimer.isActive() )
			{
				const char *browseBuf = browseText.c_str();
				int len = browseText.length();
				ePtrList<eListBoxEntry>::reverse_iterator it((ePtrList<eListBoxEntry>::reverse_iterator&)current);
				for (;it != childs.rend(); ++it )
				{
					if ( !strncasecmp(it->getText().c_str(), browseBuf, len) )
					{
						if ( it != current )
						{
							setCurrent(*it,false);
							browseTimer.start(2*1000,true);
						}
						return 1;
					}
				}
				it=childs.rbegin();
				for (;it != childs.rend(); ++it )
				{
					if ( !strncasecmp(it->getText().c_str(), browseBuf, len) )
					{
						if ( it != current )
						{
							setCurrent(*it,false);
							browseTimer.start(2*1000,true);
						}
						return 1;
					}
				}
			}
			else
				break;
		return 1;
		default:
		break;
	}
	return eListBoxBase::eventHandler(event);
}

int eListBoxBaseExt::keyDown(int key)
{
	if (key >= KEY_ASCII)
	{
		browseTimer.start(2*1000,true);
		// TODO convert browseText to utf8 !!
		browseText+=(char)key;
		const char *browseBuf = browseText.c_str();
		int len = browseText.length();
		for (ePtrList<eListBoxEntry>::iterator it(childs.begin());
			it != childs.end(); ++it )
		{
			if ( !strncasecmp(it->getText().c_str(), browseBuf, len) )
			{
				if ( it != current )
				{
					browseHistory.push_front(current);
					setCurrent(*it,false);
				}
				return 1;
			}
		}
		browseText.erase(len-1,1);
	}
	return 0;
}

void eListBoxBaseExt::clearList()
{
	eListBoxBase::clearList();
	browseHistory.clear();
}

void eListBoxBaseExt::lostFocus()
{
	eListBoxBase::lostFocus();
	eRCInput::getInstance()->setKeyboardMode(eRCInput::kmNone);
	browseText="";
	browseHistory.clear();
}

void eListBoxEntry::drawEntryBorder(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF )
{
	rc->setForegroundColor(coActiveB);
	rc->line( ePoint(rect.left(), rect.bottom()-1), ePoint(rect.right()-1, rect.bottom()-1) );
	rc->line( ePoint(rect.left(), rect.top()), ePoint(rect.right()-1, rect.top()) );
	rc->line( ePoint(rect.left(), rect.top()), ePoint(rect.left(), rect.bottom()-1) );
	rc->line( ePoint(rect.right()-1, rect.top()), ePoint(rect.right()-1, rect.bottom()-1) );
	rc->line( ePoint(rect.left()+1, rect.bottom()-2), ePoint(rect.right()-2, rect.bottom()-2) );
	rc->line( ePoint(rect.left()+1, rect.top()+1), ePoint(rect.right()-2, rect.top()+1) );
	rc->line( ePoint(rect.left()+1, rect.top()+2), ePoint(rect.left()+1, rect.bottom()-3) );
	rc->line( ePoint(rect.right()-2, rect.top()+2), ePoint(rect.right()-2, rect.bottom()-3) );
	rc->line( ePoint(rect.left()+2, rect.bottom()-3), ePoint(rect.right()-3, rect.bottom()-3) );
	rc->line( ePoint(rect.left()+2, rect.top()+2), ePoint(rect.right()-3, rect.top()+2) );
	rc->line( ePoint(rect.left()+2, rect.top()+3), ePoint(rect.left()+2, rect.bottom()-4) );
	rc->line( ePoint(rect.right()-3, rect.top()+3), ePoint(rect.right()-3, rect.bottom()-4) );
}

void eListBoxEntry::drawEntryRect(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state )
{
	if ( (coNormalB != -1 && !state) || (state && coActiveB != -1) )
	{
		rc->setForegroundColor(state?coActiveB:coNormalB);
		rc->fill(rect);
		rc->setBackgroundColor(state?coActiveB:coNormalB);
	}
	else
	{
		eWidget *w=listbox->getNonTransparentBackground();
		rc->setForegroundColor(w->getBackgroundColor());
		rc->fill(rect);
		rc->setBackgroundColor(w->getBackgroundColor());
	}
	rc->setForegroundColor(state?coActiveF:coNormalF);
}

eListBoxEntryText::~eListBoxEntryText()
{
	if (para)
	{
		para->destroy();
		para = 0;
	}
}

int eListBoxEntryText::getEntryHeight()
{
	if ( !font.pointSize)
		font = eSkin::getActive()->queryFont("eListBox.EntryText.normal");

	return calcFontHeight( font ) + 4;
}

int eListBoxEntryTextStream::getEntryHeight()
{
	if ( !font.pointSize)
		font = eSkin::getActive()->queryFont("eListBox.EntryText.normal");

	return calcFontHeight( font ) + 4;
}

int calcFontHeight( const gFont& font)
{
	eTextPara *test;
	test = new eTextPara( eRect(0,0,100,50) );
	test->setFont( font );
	test->renderString("Mjdyl");
	int i =  test->getBoundBox().height();
	test->destroy();
	return i;
}

const eString& eListBoxEntryText::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state)
{
	bool b;

	if ( (b = (state == 2)) )
		state = 0;

	drawEntryRect( rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );

	int lft = rect.left();
	if (listbox->getFlags() & eListBoxBase::flagHasShortcuts)
	{
		int hideshortcuts=1; // dAF2000: Default no shortcuts
		if (eConfig::getInstance()->getKey("/ezap/osd/hideshortcuts", hideshortcuts))
		{
			eConfig::getInstance()->setKey("/ezap/osd/hideshortcuts", hideshortcuts);
		}
		if (!hideshortcuts)
		{
			int shortcut = listbox->getShortcut(this);
			if (listbox->getFlags() & eListBoxBase::flagColorShortcutsFirst)
			{
				if (shortcut < 5) 
					shortcut += 10;
				else if (shortcut < 15)
					shortcut -= 4;
			}
		
			eString strShortcut;
			switch (shortcut)
			{
				case 10: shortcut = 0;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
					strShortcut = eString().sprintf("shortcut.%d",shortcut);
					break;
				case 11:
					strShortcut = eString().sprintf("shortcut.red");
					break;
				case 12:
					strShortcut = eString().sprintf("shortcut.green");
					break;
				case 13:
					strShortcut = eString().sprintf("shortcut.yellow");
					break;
				case 14:
					strShortcut = eString().sprintf("shortcut.blue");
					break;
				default:
					shortcut=-1;
			}
			if (shortcut >= 0)
			{
					
				gPixmap* pm = eSkin::getActive()->queryImage(strShortcut);
				eRect left = rect;
				lft = rect.left() + pm->x + 10;
				left.setRight( lft );
				left.setLeft(rect.left()+5);
				rc->clip(left);
	
				eSize psize = pm->getSize(),
				esize = rect.size();
	
				int yOffs = rect.top()+((esize.height() - psize.height()) / 2);
	
				rc->blit(*pm, ePoint(left.left(), yOffs), left, gPixmap::blitAlphaTest );
	
				rc->clippop();
			}
		}
	}
	if (!para)
	{
		para = new eTextPara( eRect( 0, 0, rect.width()-(lft-rect.left()), rect.height() ) );
		para->setFont( font );
		para->renderString(text);
		para->realign(align);
//		yOffs = ((rect.height() - para->getBoundBox().height()) / 2 + 0) - para->getBoundBox().top() ;
		yOffs=0;
	}
	rc->renderPara(*para, ePoint( lft, rect.top()+yOffs ) );

	if (b)
		drawEntryBorder( rc, rect, coActiveB, coActiveF, coNormalB, coNormalF );

	return text;
}

const eString &eListBoxEntryTextStream::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state)
{
	rc->setFont( font );

	drawEntryRect( rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );

	rc->setForegroundColor(state?coActiveF:coNormalF);
	rc->renderText(rect, text.str());

	static eString ret;
	return ret = text.str();
}

void eListBoxEntryText::SetText(const eString& txt)
{
	if(para){
		para->destroy();
		para=0;
	};
	text=txt;
}

const eString& eListBoxSeparator::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state )
{
	int x = 0;
	if ( pm )
	{
		rc->clip(rect);
		eSize psize = pm->getSize(),
					esize = rect.size();
		int yOffs = rect.top()+((esize.height() - psize.height()) / 2);
		do
		{
			rc->blit(*pm, ePoint(x, yOffs), eRect(x, yOffs, psize.width(), psize.height()), alphatest?gPixmap::blitAlphaTest:0 );
			x+=distance;
			x+=psize.width();
		}
		while (x<esize.width());
		rc->clippop();
	}
	static eString ret="separator";
	return ret;
}

// deprecated...
const eString& eListBoxEntrySeparator::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state )
{
	int x = 0;
	if ( pm )
	{
		rc->clip(rect);
		eSize psize = pm->getSize(),
					esize = rect.size();
		int yOffs = rect.top()+((esize.height() - psize.height()) / 2);
		do
		{
			rc->blit(*pm, ePoint(x, yOffs), eRect(x, yOffs, psize.width(), psize.height()), alphatest?gPixmap::blitAlphaTest:0 );
			x+=distance;
			x+=psize.width();
		}
		while (x<esize.width());
		rc->clippop();
	}
	static eString ret="separator";
	return ret;
}

eListBoxEntryCheck::eListBoxEntryCheck( eListBox<eListBoxEntryMenu> *lb, const char* text, const char* regkey, const eString& hlptxt )
	:eListBoxEntryMenu(lb, text, hlptxt, 0 )
	,pm(eSkin::getActive()->queryImage("eListBoxEntryCheck"))
	,regKey(regkey), checked(0)
{
	selectable=1;
	if ( regKey )
	{
		if ( eConfig::getInstance()->getKey( regKey.c_str(), checked ) )
			eConfig::getInstance()->setKey( regKey.c_str(), checked );
	}
}

eListBoxEntryCheck::eListBoxEntryCheck( eListBox<eListBoxEntryMenu> *lb, const eString& text, const eString& hlptxt, int align, void* key, int keytype )
	:eListBoxEntryMenu(lb, text, hlptxt, align, key, keytype )
	,pm(eSkin::getActive()->queryImage("eListBoxEntryCheck"))
	,regKey(0) ,checked(0)
{
	selectable=1;
}

void eListBoxEntryCheck::setChecked(int checkIt)
{
	checked = checkIt;
	listbox->invalidateCurrent();
}

void eListBoxEntryCheck::LBSelected(eListBoxEntry* t)
{
	if (t == this)
	{
		if(regKey)
		{
			checked^=1;
			eConfig::getInstance()->setKey( regKey.c_str(), checked );
			listbox->invalidateCurrent();
			/* emit */ selected((bool)checked);
		}
	}
}

const eString& eListBoxEntryCheck::redraw(gPainter *rc, const eRect& rect, gColor coActiveB, gColor coActiveF, gColor coNormalB, gColor coNormalF, int state )
{
	bool b;

	if ( (b = (state == 2)) )
		state = 0;

	eListBoxEntryText::redraw( rc, rect, coActiveB, coActiveF, coNormalB, coNormalF, state );

	if ( pm && checked )
	{
		eRect right = rect;
		right.setLeft( rect.right() - (pm->x + 10) );
		rc->clip(right);

		eSize psize = pm->getSize(),
					esize = rect.size();

		int yOffs = rect.top()+((esize.height() - psize.height()) / 2);

		rc->blit(*pm, ePoint(right.left(), yOffs), right, gPixmap::blitAlphaTest );

		rc->clippop();
	}

	return text;
}

eListBoxEntryMulti::eListBoxEntryMulti( eListBox<eListBoxEntryMenu> *lb, const char *hlptext )
	:eListBoxEntryMenu( lb, NULL, hlptext, (int)eTextPara::dirCenter ),
	cur(entrys.end())
{
	selectable=1;
}

void eListBoxEntryMulti::add( const char *text, int key )
{
	entrys.push_back( std::pair< int, eString>(key,text) );
}

void eListBoxEntryMulti::add( const eString &text, int key )
{
	entrys.push_back( std::pair< int, eString>(key,text) );
}

void eListBoxEntryMulti::setCurrent(int key)
{
	for ( std::list< std::pair< int, eString > >::iterator it(entrys.begin())
		;it != entrys.end(); ++it )
	{
		if ( it->first == key )
		{
			cur=it;
			text = cur->second;
			this->key = (void*) cur->first;
			if ( para )
			{
				para->destroy();
				para=0;
			}
			listbox->invalidateCurrent();
			listbox->selchanged(this);
			break;
		}
	}
}

int eListBoxEntryMulti::eventHandler( const eWidgetEvent &e )
{
	switch(e.type)
	{
		case eWidgetEvent::evtAction:
			if ( e.action == &i_listActions->pageup )
			{
				std::list< std::pair< int, eString > >::iterator it(cur);
				--it;
				if ( it != entrys.end() )
				{
					--cur;
					text = cur->second;
					key = (void*) cur->first;
					if ( para )
					{
						para->destroy();
						para=0;
						listbox->invalidateCurrent();
						listbox->selchanged(this);
					}
				}
				return 1;
			}
			else if ( e.action == &i_listActions->pagedown )
			{
				std::list< std::pair< int, eString > >::iterator it(cur);
				++it;
				if ( it != entrys.end() )
				{
					++cur;
					text = cur->second;
					key = (void*) cur->first;
					if ( para )
					{
						para->destroy();
						para=0;
						listbox->invalidateCurrent();
						listbox->selchanged(this);
					}
				}
				return 1;
			}
		default:
			break;
	}
	return 0;
}
