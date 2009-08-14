#include <lib/gui/slider.h>

#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gui/guiactions.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>

inline void swap( gColor& a, gColor& b )
{
	gColor tmp = a;
	a = b;
	b = tmp;
}

eSlider::eSlider( eWidget* parent, const eWidget *descr, int min, int max )
	:eProgress( parent, 1), descr(descr)
{
	init_eSlider(min,max);
}
void eSlider::init_eSlider(int min,int max)
{
	activated_left = eSkin::getActive()->queryScheme( "eSlider.activated.left" );
	activated_right = eSkin::getActive()->queryScheme( "eSlider.activated.right" );
	setMin(min);
	setMax(max);
	incrementation = 4; // in Percent
	addActionMap(&i_cursorActions->map);
}

void eSlider::setMin( int i )
{
	min = i;
}

void eSlider::setMax( int i )
{
	max = (i <= min) ? 99 : i;
}

void eSlider::setValue( int i )
{
	if ( i >= min && i <= max )
//		setPerc( (int) round( i * (double)100/((max-min)+1) ) );
		setPerc((i-min) * 100 / (max-min));
	else
		setPerc( 0 );
}

int eSlider::getValue()
{	
	// int ret = min + (int) ( (double) perc / 100 * ( (max-min)+1));
	int ret = min + (max - min) * perc / 100;
	return (ret > max ? max : ret);
}

void eSlider::setIncrement( int i )
{
	incrementation = i ? i : 4;
}

int eSlider::setProperty( const eString &prop, const eString &val)
{
	if (prop == "leftColorActive")
		activated_left=eSkin::getActive()->queryColor( prop );
	else if (prop == "rightColorActive")
		activated_right=eSkin::getActive()->queryColor( prop );
	else if (prop == "incrementation")
		setIncrement( atoi( val.c_str() ) );
	else if (prop == "min")
		setMin( atoi(val.c_str()) );
	else if (prop == "max")
		setMax( atoi(val.c_str()) );
	else
		return eProgress::setProperty( prop, val );
	return 0;
}

void eSlider::gotFocus()
{
	have_focus++;
	swap( left, activated_left );
	swap( right, activated_right );

#ifndef DISABLE_LCD
	if (parent && parent->LCDElement)  // detect if LCD Avail
	{
		LCDTmp = new eProgress( parent->LCDElement );
		LCDTmp->hide();
		LCDTmp->setForegroundColor( 255 );
		((eSlider*)LCDTmp)->left=255;
		((eSlider*)LCDTmp)->right=0;	
		((eProgress*)LCDTmp)->setPerc( perc );
		eSize s = parent->LCDElement->getSize();

		if (descr)
		{
			LCDTmp->move(ePoint(0, s.height()/2 + s.height()/4 - 6 ));
			LCDTmp->resize( eSize(s.width(), 12) );
			tmpDescr = new eLabel(parent->LCDElement);
			tmpDescr->hide();
			tmpDescr->move(ePoint(0,0));
			tmpDescr->resize(eSize(s.width(), s.height()/2));
			tmpDescr->setText(descr->getText());
			tmpDescr->show();
		}
		else
		{
			LCDTmp->resize( eSize(s.width(), 8) );
			LCDTmp->move(ePoint(0, s.height() / 2 - 6));
		}
		((eProgress*)LCDTmp)->setPerc( perc );
		LCDTmp->show();
	}
#endif // DISABLE_LCD
	redraw();	
}

void eSlider::lostFocus()
{
// swap back;
	swap( left, activated_left );
	swap( right, activated_right );

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
	invalidate();
}

int eSlider::eventHandler( const eWidgetEvent& event )
{
	switch (event.type)
	{
		case eWidgetEvent::evtAction:
		{
			if(event.action == &i_cursorActions->right)
			{
			 	if ( (perc += incrementation) > 100 )
					perc = 100;
			}
			else if(event.action == &i_cursorActions->left)
			{
			 	if ( (perc-=incrementation) < 0 )
					perc = 0;
			}
			else
				break;
			redraw();
			/* emit */ changed( getValue() );
#ifndef DISABLE_LCD
			if (LCDTmp)
				((eProgress*)LCDTmp)->setPerc( perc );
#endif
			return 1;
		}
		default:
			break;
	}
	return eProgress::eventHandler( event );
}

static eWidget *create_eSlider(eWidget *parent)
{
	return new eSlider(parent);
}

class eSliderSkinInit
{
public:
	eSliderSkinInit()
	{
		eSkin::addWidgetCreator("eSlider", create_eSlider);
	}
	~eSliderSkinInit()
	{
		eSkin::removeWidgetCreator("eSlider", create_eSlider);
	}
};

eAutoInitP0<eSliderSkinInit> init_eSliderSkinInit(eAutoInitNumbers::guiobject, "eSlider");
