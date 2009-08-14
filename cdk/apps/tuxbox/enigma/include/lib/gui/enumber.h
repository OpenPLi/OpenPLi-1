#ifndef __enumber_h
#define __enumber_h

#include <lib/gui/ewidget.h>

class eLabel;
class gPainter;

/**
 * \brief A widget to enter a number.
 */
class eNumber: public eDecoWidget
{
private:
	void redrawNumber(gPainter *, int n, const eRect &rect);
	void redrawWidget(gPainter *, const eRect &rect);
	eRect getNumberRect(int n);
	int eventHandler(const eWidgetEvent &event);
	int number[32];
	int len, dspace, space_selected, active;
	gColor cursorB, cursorF, normalB, normalF;
	int oldmax;	
	int have_focus;
	int min, max, digit, maxdigits, isactive;
	int flags;
	int base;
	eWidget* descr;
	eLabel* tmpDescr; // used for description Label in LCD
	bool neg;
	int saved_keyboard_mode;
	int fontYSize, fontTop; // for Vcenter
	void init_eNumber(int _len, int _min, int _max, int _maxdigits, int *init);
protected:
	int getActive()	{ return active; }
	int keyDown(int key);
	void gotFocus();
	void lostFocus();
	int setProperty(const eString &prop, const eString &value);
public:
	static void unpack(__u32 l, int *t);
	static void pack(__u32 &l, int *t);
	void invalidateNum();
	Signal1<void, int*> selected;
	Signal0<void> numberChanged;
	eNumber(eWidget *parent, int len, int min, int max, int maxdigits, int *init, int isactive=0, eWidget* descr=0, int grabfocus=1, const char* deco="eNumber" );
	~eNumber();
	int getNumber(int f) { if ((f>=0) && (f<len)) return number[f]; return -1; }
	double getFixedNum();
	void setFixedNum(double);
	void setNumber(int f, int n);
	void setLimits(int min, int max);
	void setNumberOfFields(int n);
	void setMaximumDigits(int n);
	enum
	{
		flagDrawPoints=1,
		flagDrawBoxes=2,
		flagFillWithZeros=4,
		flagTime=8,
		flagPosNeg=16,
		flagHideInput=32,
		flagFixedNum=64,
		flagVCenter=128
	};
	void setFlags(int flags);
	void setBase(int base);
	
	void setNumber(int n);
	int getNumber();
	void setDescr( eWidget *d ) { descr = d; }
};

#endif
