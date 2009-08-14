#include <src/audio_dynamic.h>
#include <lib/driver/audiodynamic.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>

eProgressWithMark::eProgressWithMark(eWidget *parent): eProgress(parent)
{
	indexmarkcolor = eSkin::getActive()->queryColor("indexmark");
}

void eProgressWithMark::redrawWidget(gPainter *target, const eRect &area)
{
	eProgress::redrawWidget(target, area);

	if ((m_mark>0) && (m_range>0))
	{
		target->setForegroundColor(indexmarkcolor);
		int pos  = m_mark * size.width() / m_range;
		target->fill(eRect(pos - 2, 0, 4, size.height()));
	}
}

void eProgressWithMark::setRange(int max)
{
	m_range = max;
}

void eProgressWithMark::setMark(int val)
{
	m_mark = val;
}

void eProgressWithMark::setValue(int val)
{
	if (m_range)
		setPerc(val * 100 / m_range);
}

eAudioDynamicConfig::eAudioDynamicConfig(eWidget *parent): eWidget(parent), m_update(eApp)
{
	CONNECT(m_update.timeout, eAudioDynamicConfig::update);
	m_slider = new eProgressWithMark(this);
	m_slider->resize(eSize(240, 20));
	m_slider->move(ePoint(0, 30));
	m_slider->setRange(50000);
	
	m_label = new eLabel(this);
	m_label->resize(eSize(240, 30));
	m_label->move(ePoint(0, 0));
	m_label->setText(_("Audio Level:"));
	setShortcut("yellow");
	m_label->setShortcutPixmap("yellow");
}

void eAudioDynamicConfig::toggle()
{
	eDebug("toggle!");
	if (eAudioDynamicCompression::getInstance())
	{
		eAudioDynamicCompression::getInstance()->setEnable(!eAudioDynamicCompression::getInstance()->getEnable());
	}
	update(); 
}

void eAudioDynamicConfig::update()
{
	if (eAudioDynamicCompression::getInstance())
	{
		if (eAudioDynamicCompression::getInstance()->getEnable())
			m_slider->setMark(eAudioDynamicCompression::getInstance()->getMax());
		else
			m_slider->setMark(-1);
		m_slider->setValue(eAudioDynamicCompression::getInstance()->getCurrent());
	}
}

int eAudioDynamicConfig::eventHandler(const eWidgetEvent &event)
{
	eDebug("event handler!!");
	switch (event.type)
	{
	case eWidgetEvent::evtShortcut:
		toggle();
		eDebug("toggle!");
		return 0; // don't set focus
	case eWidgetEvent::willShow:
		m_update.start(100, 0);
		update();
		break;
	case eWidgetEvent::willHide:
		m_update.stop();
		break;
	default:
		break;
	}
	return eWidget::eventHandler(event);
}
