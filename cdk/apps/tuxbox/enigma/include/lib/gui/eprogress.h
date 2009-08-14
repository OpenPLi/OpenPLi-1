#ifndef __eprogress_h
#define __eprogress_h

#include <lib/gui/ewidget.h>
#include <lib/gdi/grc.h>

/**
 * \brief A progressbar.
 *
 * Useful for displaying a progress or stuff.
 */
class eProgress: public eWidget
{
protected:
	int perc, border, start;
	int direction;
	gColor left, right;
	gPixmap *sliderPixmap;
	void init_eProgress();
public:
	eProgress(eWidget *parent, int takeFocus=0);
	~eProgress();
	
	/**
	 * \brief Sets the value. 
	 *
	 * \param perc The range is \c 0..100
	 */
	void setPerc(int perc);
	void setStart(int perc);
	bool setParams( int _start, int _perc );
	void redrawWidget(gPainter *target, const eRect &area);
	
	/**
	 * \brief Sets a property.
	 *
	 * Valid, eProgress specific properties are:
	 * \arg \c leftColor, the color of the left part
	 * \arg \c rightColor, the color of the right part
	 * \arg \c border, the size of the border (in pixels)
	 * \arg \c direction, direction (0 for horiz., 1 for vert.)
	 * \sa eWidget::setProperty
	 */
	int setProperty(const eString &prop, const eString &value);
	
	void setDirection(int d) { direction = d; }
	void setBorder( int b ) { border = b; }
	void setLeftColor( const gColor& c )	{ left = c; }
	void setRightColor( const gColor& c )	{ right = c; }
	void setSliderPixmap(gPixmap *pmap) { sliderPixmap = pmap; }
};

class eMultiProgress : public eMultiWidget
{
public:
	void setPerc(int perc)
	{
		unsigned int i;
		for (i = 0; i < size(); i++)
		{
			((eProgress*)((*this)[i]))->setPerc(perc);
		}
	}
	void setStart(int perc)
	{
		unsigned int i;
		for (i = 0; i < size(); i++)
		{
			((eProgress*)((*this)[i]))->setStart(perc);
		}
	}
	int setProperty(const eString &prop, const eString &value)
	{
		int retval = 0;
		unsigned int i;
		for (i = 0; i < size(); i++)
		{
			retval = ((eProgress*)((*this)[i]))->setProperty(prop, value);
		}
		return retval;
	}
};

#endif
