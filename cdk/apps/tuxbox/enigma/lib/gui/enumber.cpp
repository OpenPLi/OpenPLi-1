#include <lib/gui/enumber.h>
#include <lib/driver/rc.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gdi/fb.h>
#include <lib/gdi/grc.h>
#include <lib/gdi/font.h>
#include <lib/gui/guiactions.h>
#include <math.h>

void eNumber::unpack(__u32 l, int *t)
{
	for (int i=0; i<4; i++)
		*t++=(l>>((3-i)*8))&0xFF;
}

void eNumber::pack(__u32 &l, int *t)
{
	l=0;
	for (int i=0; i<4; i++)
		l|=(*t++)<<((3-i)*8);
}

eRect eNumber::getNumberRect(int n)
{
	if (deco_selected && have_focus)
		return eRect( deco_selected.borderLeft + n * space_selected, deco_selected.borderTop, dspace, crect_selected.height() );
	else if (deco)
		return eRect( deco.borderLeft + n * dspace, deco.borderTop, dspace, crect.height() );
	else
		return eRect( n * dspace, 0, dspace, height() );
}

void eNumber::redrawNumber(gPainter *p, int n, const eRect &area)
{
	eRect pos(getNumberRect(n));

	if (!area.intersects(pos))
		return;

	p->setForegroundColor((have_focus && n==active)?cursorB:normalB);
	p->fill(pos);
	p->setFont(font);

	// eDebug ("Font pointsize=%d HxW=%dx%d crect height=%d, fontheigt2=%d top=%d num=%d at pos %d,%d:%d,%d in area %d,%d:%d,%d", font.pointSize, height(), width(), crect.height(), fontYSize, fontTop, number[n],
	// pos.left(), pos.top(), pos.right(), pos.bottom(),
	// area.left(), area.top(), area.right(), area.bottom());

	eString t;
	if (flags & flagFillWithZeros || ( (flags & flagFixedNum) && n ))
	{
		eString s = "%0"+eString().setNum(maxdigits)+(base==10?"d":"X");
		const char* p = s.c_str();
		char* tmp = new char[10];
		strcpy( tmp, p );
		t.sprintf(tmp, number[n]);
		delete [] tmp;
	}
	else
	{
		if (flags&flagHideInput)
			t="*";
		else if (base==10)
			t.sprintf("%d", number[n]);
		else if (base==0x10)
			t.sprintf("%X", number[n]);
	}

	if (!n && (flags & flagPosNeg) && neg)
		t="-"+t;

	if (n && (flags & flagTime))
		t=":"+t;

	else if (n && ( (flags & flagDrawPoints) || (flags & flagFixedNum)) )
		t="."+t;

	p->setForegroundColor((have_focus && n==active)?cursorF:normalF);
	p->setBackgroundColor((have_focus && n==active)?cursorB:normalB);

	p->clip( pos );

	int yOffs;
	if (flags & flagVCenter)
		yOffs = (pos.height() - fontYSize) / 2 - fontTop;
	else
		yOffs = 0;


	eTextPara *para = new eTextPara( pos );
	para->setFont( font );
	para->renderString( t );
	if (!n && len==2 && ((flags & flagFixedNum) || (flags & flagTime)) ) // first element...
		para->realign( eTextPara::dirRight );
	else
		para->realign( eTextPara::dirLeft );
	p->renderPara( *para, ePoint(0, yOffs) );
	p->clippop();
	para->destroy();
		
}

double eNumber::getFixedNum()
{
	if (flags & flagFixedNum)
	{
		if ((flags & flagPosNeg) && neg)
		{
			double d = -((double)number[0]+(double)number[1]/1000);
			// eDebug("getFixedNum %lf", d);
			return d;
		}
		else
		{
			float d = (double)number[0]+(double)number[1]/1000;
			// eDebug("getFixedNum %lf", d);
			return d;
		}
	}
	else
		return 0;
}

void eNumber::setFixedNum(double d)
{
	// eDebug("setFixedNum %lf", d);
	if (flags & flagPosNeg)
		neg=d<0;
	else
		neg=0;

	d=fabs(d);

	if (flags & flagFixedNum)
	{
		number[0]=(int)d;
		number[1]=(int)round(( ( d - number[0] ) * 1000) );
	}
	else
		eDebug("eNumber bug... the Number %s is not a fixed Point number", name.c_str());
}

void eNumber::redrawWidget(gPainter *p, const eRect &area)
{
	if (flags & flagVCenter)
	{
		eTextPara *tmp = new eTextPara( eRect(0, 0, 200, 60) );
		tmp->setFont( font );
		tmp->renderString("1690");
		fontYSize = tmp->getBoundBox().height();
		fontTop = tmp->getBoundBox().top();
		tmp->destroy();
	}

	for (int i=0; i<len; i++)
		redrawNumber(p, i, area);

	if (deco_selected && have_focus)
		deco_selected.drawDecoration(p, ePoint(width(), height()));
	else if (deco)
		deco.drawDecoration(p, ePoint(width(), height()));
}

void eNumber::invalidateNum()
{
	if ( have_focus && deco_selected )
		invalidate( crect_selected );
	else if ( deco )
		invalidate( crect );
	else
		invalidate();
}

int eNumber::eventHandler(const eWidgetEvent &event)
{
#ifndef DISABLE_LCD
	if (LCDTmp)
		((eNumber*) LCDTmp)->eventHandler(event);
#endif
	switch (event.type)
	{
	case eWidgetEvent::changedSize:
		if (deco)
			dspace = (crect.width()) / len;
		else
			dspace = (size.width()) / len;	
		if (deco_selected)
			space_selected = (crect_selected.width()) / len;
		break;
	case eWidgetEvent::evtAction:
		if ( len > 1 && event.action == &i_cursorActions->left)
		{
			int oldac=active;
			active--;
			invalidate(getNumberRect(oldac));
			if (active<0)
				active=len-1;
			if (active!=oldac)
				invalidate(getNumberRect(active));
			digit=0;
		} else if ( len > 1 && (event.action == &i_cursorActions->right) || (event.action == &i_cursorActions->ok))
		{
			int oldac=active;
			active++;
			invalidate(getNumberRect(oldac));
			if (active>=len)
			{
				if (event.action == &i_cursorActions->ok)
				/*emit*/ selected(number);
				active=0;
			}
			if (active!=oldac)
				invalidate(getNumberRect(active));
			digit=0;
		} else
				break;
		return 1;
	default:
		break;
	}
	return eDecoWidget::eventHandler(event);
}

// isactive is the digit (always in the first field )
// that ist active after get the first focus ! 

eNumber::eNumber(eWidget *parent, int _len, int _min, int _max, int _maxdigits, int *init, int isactive, eWidget* descr, int grabfocus, const char *deco)
 :eDecoWidget(parent, grabfocus, deco ),
	active(0), 
	cursorB(eSkin::getActive()->queryScheme("number.selected.background")),
	cursorF(eSkin::getActive()->queryScheme("number.selected.foreground")),
	normalB(eSkin::getActive()->queryScheme("number.normal.background")),
	normalF(eSkin::getActive()->queryScheme("number.normal.foreground")),
	have_focus(0), digit(isactive), isactive(isactive), flags(0), descr(descr), tmpDescr(0),
	neg(false), saved_keyboard_mode(0), fontYSize(0), fontTop(0)
{
	init_eNumber(_len, _min, _max, _maxdigits, init);
}
void eNumber::init_eNumber(int _len, int _min, int _max, int _maxdigits, int *init)
{
	setNumberOfFields(_len);
	setLimits(_min, _max);
	setMaximumDigits(_maxdigits);
	setBase(10);
	for (int i=0; init && i<len; i++)
		number[i]=init[i];
	addActionMap(&i_cursorActions->map);
	if ( !cursorF )
		cursorF=eSkin::getActive()->queryScheme("global.selected.foreground");
	if ( !cursorB )
		cursorB=eSkin::getActive()->queryScheme("global.selected.background");
	if ( !normalF )
		normalF=eSkin::getActive()->queryScheme("global.normal.foreground");
	if ( !normalB )
		normalB=eSkin::getActive()->queryScheme("global.normal.background");
	setBackgroundColor(normalB);
	setForegroundColor(normalF);
}                 
             
eNumber::~eNumber()
{
}

int eNumber::keyDown(int key)
{
#ifndef DISABLE_LCD
	if (LCDTmp)
		((eNumber*) LCDTmp)->keyDown(key);
#endif
	// eDebug("eNumber: key: %d", key);
	switch (key)
	{
	case KEY_1 ... KEY_0:
	{
		int nn=(digit!=0)?number[active]*10:0;
		nn+=(key == KEY_0) ? 0 : (key - KEY_1 + 1);
		if (flags & flagTime)
		{
			if ( active )
				max = 59;
			else {
				int clktype = 0;
				eConfig::getInstance()->getKey("/elitedvb/extra/clocktype", clktype);
				if (clktype)
				{
					min = 1;
					max = 12;
				}
				else
					max = 23;
			}
		}
		else if (flags & flagFixedNum)
		{
			if (active)
				max=999;
			else
				max=oldmax;
		}
		if (nn>=min && nn<=max)
		{
			number[active]=nn;
			invalidate(getNumberRect(active));
			digit++;
			if ((digit>=maxdigits) || (nn==0) || ((digit+1==maxdigits) && (nn*10>max)))
			{        
				active++;
				invalidate(getNumberRect(active-1));
				digit=0;
				/*emit*/ numberChanged();
				if (active>=len)
				{
					/*emit*/ selected(number);
					active=0;
				}
				else
					invalidate(getNumberRect(active));
			}
		}
		break;

	break;
	}
	case KEY_KPPLUS:
		if ((flags & flagPosNeg) && neg )
		{
			neg=false;
			invalidate(getNumberRect(0));
		}
	break;

	case KEY_KPMINUS:
		if ((flags & flagPosNeg) && !neg )
		{
			neg=true;
			invalidate(getNumberRect(0));
		}
	default:
		return 0;
	}
	return 1;
}

void eNumber::gotFocus()
{
	have_focus++;
	digit=isactive;

	 if (deco && deco_selected)
		invalidate();
	else
		invalidate(getNumberRect(active));

	saved_keyboard_mode = eRCInput::getInstance()->getKeyboardMode();
	eRCInput::getInstance()->setKeyboardMode(eRCInput::kmNone);
#ifndef DISABLE_LCD
	if (parent && parent->LCDElement)  // detect if LCD Avail
	{
		LCDTmp = new eNumber(parent->LCDElement, len, min, max, maxdigits, &(number[0]), isactive, 0, 0);
		LCDTmp->hide();
		((eNumber*)LCDTmp)->setFlags(flags);
		eSize s = parent->LCDElement->getSize();

		if (descr)
		{
			LCDTmp->move(ePoint(0,s.height()/2));
			LCDTmp->resize(eSize(s.width(), s.height()/2));
			tmpDescr = new eLabel(parent->LCDElement);
			tmpDescr->hide();
			tmpDescr->move(ePoint(0,0));
			tmpDescr->resize(eSize(s.width(), s.height()/2));
			tmpDescr->setText(descr->getText());
			tmpDescr->show();
		}
		else
		{
			LCDTmp->resize(s);
			LCDTmp->move(ePoint(0,0));
		}
		((eNumber*)LCDTmp)->digit=digit;
		((eNumber*)LCDTmp)->active=active;
		((eNumber*)LCDTmp)->normalF=255;
		((eNumber*)LCDTmp)->normalB=0;
		((eNumber*)LCDTmp)->cursorF=0;
		((eNumber*)LCDTmp)->cursorB=255;
		((eNumber*)LCDTmp)->have_focus=1;
		LCDTmp->show();
	}
#endif //DISABLE_LCD
}

void eNumber::lostFocus()
{
#ifndef DISABLE_LCD
	if (LCDTmp)
	{
		delete LCDTmp;
		LCDTmp=0;
		if (tmpDescr)
		{
			delete tmpDescr;
			tmpDescr=0;
		}
	}
#endif
	have_focus--;

	if (deco && deco_selected)
		invalidate();
	else
		invalidate(getNumberRect(active));
	isactive=0;
	eRCInput::getInstance()->setKeyboardMode(saved_keyboard_mode);
}

int eNumber::setProperty(const eString &prop, const eString &value)
{
	if (prop=="foregroundColor")
	{
		normalF=eSkin::getActive()->queryColor(value);
		setForegroundColor(normalF);
	}
	else if (prop=="backgroundColor")
	{
		normalB=eSkin::getActive()->queryColor(value);
		setForegroundColor(normalB);
	}
	else if (prop=="activeForegroundColor")
		cursorF=eSkin::getActive()->queryColor(value);
	else if (prop=="activeBackgroundColor")
		cursorB=eSkin::getActive()->queryColor(value);
        else if (prop=="vcenter")
                setFlags( flags | flagVCenter );
	else
		return eDecoWidget::setProperty(prop, value);
	return 0;
}

void eNumber::setNumber(int f, int n)
{
	if (flags & flagPosNeg)
	{
		if(!f && n<0)
			neg=true;
		else
			neg=false;
	}
	else
		neg=false;

	if ((f>=0) && (f<len))
		number[f]=abs(n);

	invalidate(getNumberRect(f));
}

void eNumber::setLimits(int _min, int _max)
{
	min=_min;
	max=_max;
	oldmax=max;
}

void eNumber::setNumberOfFields(int n)
{
	len=n;
}

void eNumber::setMaximumDigits(int n)
{
	if (n > 16)
		n=16;
	maxdigits=n;
	if (digit >= maxdigits)
		digit=0;
}

void eNumber::setFlags(int _flags)
{
  if (flags&flagFixedNum)
		len=2;
		
	flags=_flags;
}

void eNumber::setBase(int _base)
{
	base=_base;
}

void eNumber::setNumber(int n)
{
	if ( flags&flagPosNeg )
		neg = n < 0;
	else
		neg=0;

	if( len == 1 )
		number[0]=abs(n);
	else
		for (int i=len-1; i>=0; --i)
		{
			number[i]=n%base;
			n/=base;
		}
	invalidateNum();
}

int eNumber::getNumber()
{
	int n=0;
	for (int i=0; i<len; i++)
	{
		n*=base;
		n+=number[i];
	}
	return flags&flagPosNeg && neg ? -n : n;
}
