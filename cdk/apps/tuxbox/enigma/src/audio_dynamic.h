#ifndef __audio_dynamic_h
#define __audio_dynamic_h

#include <lib/gui/ewidget.h>
#include <lib/gui/eprogress.h>

class eLabel;

class eProgressWithMark: public eProgress
{
	int m_mark, m_range;
	gColor indexmarkcolor;
public:
	eProgressWithMark(eWidget *parent);
	void redrawWidget(gPainter *target, const eRect &area);
	void setRange(int max);
	void setMark(int val);
	void setValue(int val);
};

class eAudioDynamicConfig: public eWidget
{
	eTimer m_update;
	eProgressWithMark *m_slider;
	eLabel *m_label;
	void toggle();
	void update();
protected:
	int eventHandler(const eWidgetEvent &event);
public:
	eAudioDynamicConfig(eWidget *parent);
};

#endif
