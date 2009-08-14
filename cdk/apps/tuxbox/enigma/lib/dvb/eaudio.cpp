#include <lib/dvb/eaudio.h>

#include <lib/dvb/edvb.h>
#include <lib/dvb/dvbservice.h>
#include <lib/system/init.h>
#include <lib/system/econfig.h>

eAudio *eAudio::instance;

eAudio::eAudio()
{
	instance=this;

	reloadSettings();
}

eAudio::~eAudio()
{
	instance=0;
}

eAudio *eAudio::getInstance()
{
	return instance;
}

void eAudio::setAC3default(int a)
{
	ac3default=a;

	eDVB *dvb = eDVB::getInstance();

	PMT *pmt=dvb->tPMT.ready()?dvb->tPMT.getCurrent():0;
	if ( pmt )
	{
		eDVBServiceController *sapi=eDVB::getInstance()->getServiceAPI();
		if ( sapi )
			sapi->scanPMT(pmt);
		pmt->unlock();
	}
}

void eAudio::saveSettings()
{
	eConfig::getInstance()->setKey("/elitedvb/audio/ac3default", ac3default);
	eConfig::getInstance()->flush();
}

void eAudio::reloadSettings()
{
	if (eConfig::getInstance()->getKey("/elitedvb/audio/ac3default", ac3default))
		ac3default=0;
}

eAutoInitP0<eAudio> init_eAudio(1, "EAUDIO");
