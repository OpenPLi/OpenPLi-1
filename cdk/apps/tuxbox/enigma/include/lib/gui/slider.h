#ifndef __SRC_CORE_GUI_SLIDER_
#define __SRC_CORE_GUI_SLIDER_

#include <lib/gui/eprogress.h>

class eSlider: public eProgress
{
	int incrementation, max, min;
	gColor activated_left, activated_right;
	const eWidget *descr;
	int setProperty( const eString &prop, const eString &val);
	void gotFocus();
	void lostFocus();
	int eventHandler( const eWidgetEvent& event );
	eWidget *tmpDescr;
	void update();
	void init_eSlider(int min,int max);
public:
	void setMin( int i );
	void setMax( int i );
	void setIncrement( int i );
	void setValue( int i );
	int getValue();
	Signal1<void, int> changed;
	eSlider( eWidget *parent, const eWidget *descr=0, int min=0, int max=99 );
};

#endif // __SRC_CORE_GUI_SLIDER_
