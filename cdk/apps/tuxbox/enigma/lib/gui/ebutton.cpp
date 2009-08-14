#include <lib/gui/ebutton.h>
#include <lib/gui/eskin.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/gdi/font.h>
#include <lib/gui/guiactions.h>

eButton::eButton(eWidget *parent, eLabel* desc, int takefocus, const char *deco)
	:eLabel(parent, 0, takefocus, deco),
#ifndef DISABLE_LCD
	tmpDescr(0),
#endif
	focusB(eSkin::getActive()->queryScheme("button.selected.background")),
	focusF(eSkin::getActive()->queryScheme("button.selected.foreground")),
	normalB(eSkin::getActive()->queryScheme("button.normal.background")),
	normalF(eSkin::getActive()->queryScheme("button.normal.foreground")),
	descr(desc)
{
	init_eButton();
}
void eButton::init_eButton()
{
	align=eTextPara::dirCenter;
	flags |= eLabel::flagVCenter;
	addActionMap(&i_cursorActions->map);
	if ( !focusF )
		focusF=eSkin::getActive()->queryScheme("global.selected.foreground");
	if ( !focusB )
		focusB=eSkin::getActive()->queryScheme("global.selected.background");
	if ( !normalF )
		normalF=eSkin::getActive()->queryScheme("global.normal.foreground");
	if ( !normalB )
		normalB=eSkin::getActive()->queryScheme("global.normal.background");
	setBackgroundColor(normalB);
	setForegroundColor(normalF);
}

int eButton::setProperty(const eString &prop, const eString &value)
{
	if (prop=="foregroundColor")
	{
		normalF=eSkin::getActive()->queryColor(value);
		setForegroundColor(normalF);
	}
	else if (prop=="backgroundColor")
	{
		normalB=eSkin::getActive()->queryColor(value);
		setBackgroundColor(normalB);
	}
	if (prop=="activeForegroundColor")
		focusF=eSkin::getActive()->queryColor(value);
	else if (prop=="activeBackgroundColor")
		focusB=eSkin::getActive()->queryColor(value);
	else
		return eLabel::setProperty(prop, value);
	return 0;
}

void eButton::gotFocus()
{
#ifndef DISABLE_LCD
	if (parent && parent->LCDElement)
	{
		if (descr)
		{
			LCDTmp = new eLabel(parent->LCDElement);
			LCDTmp->hide();
			eSize s = parent->LCDElement->getSize();
			LCDTmp->move(ePoint(0,s.height()/2));
			LCDTmp->resize(eSize(s.width(), s.height()/2));
			LCDTmp->setText(text);
			LCDTmp->setBackgroundColor(255);
			LCDTmp->show();
			tmpDescr = new eLabel(parent->LCDElement);
			tmpDescr->hide();
			tmpDescr->move(ePoint(0,0));
			tmpDescr->resize(eSize(s.width(), s.height()/2));
			tmpDescr->setText(descr->getText());
			tmpDescr->show();
		}
		else
			parent->LCDElement->setText(text);
	}
#endif
	setForegroundColor(focusF,false);
	setBackgroundColor(focusB);
}

void eButton::lostFocus()
{
#ifndef DISABLE_LCD
	if (parent && parent->LCDElement)
	{
		if (LCDTmp)
		{
			delete LCDTmp;
			LCDTmp = 0;
			if (tmpDescr)
			{
				delete tmpDescr;
				tmpDescr=0;
			}
		}
		else
			parent->LCDElement->setText("");	
	}
#endif
	setForegroundColor(normalF,false);
	setBackgroundColor(normalB);
}

int eButton::eventHandler(const eWidgetEvent &event)
{
	switch (event.type)
	{
    		case eWidgetEvent::evtAction:
			if (event.action == &i_cursorActions->ok)
			{
				/*emit*/ selected_id(this);
				/*emit*/ selected();
		
/*#ifndef DISABLE_LCD
				if (parent && parent->LCDElement)
				{
					if (LCDTmp)
						LCDTmp->setText(text);
					else
						parent->LCDElement->setText(text);
				}
#endif*/
			}
			else
				return eLabel::eventHandler(event);
			break;
		case eWidgetEvent::evtShortcut:
			/*emit*/ selected();
			/*emit*/ selected_id(this);
			return 0;
			break;
		default:
			return eLabel::eventHandler(event);
		break;
	}
	return 1;
}

static eWidget *create_eButton(eWidget *parent)
{
	return new eButton(parent);
}

class eButtonSkinInit
{
public:
	eButtonSkinInit()
	{
		eSkin::addWidgetCreator("eButton", create_eButton);
	}
	~eButtonSkinInit()
	{
		eSkin::removeWidgetCreator("eButton", create_eButton);
	}
};

eAutoInitP0<eButtonSkinInit> init_eButtonSkinInit(eAutoInitNumbers::guiobject, "eButton");
