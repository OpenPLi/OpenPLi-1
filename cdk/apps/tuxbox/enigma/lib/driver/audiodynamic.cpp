#include <lib/base/ebase.h>
#include <lib/base/eerror.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/driver/eavswitch.h>
#include <lib/driver/audiodynamic.h>
#include <lib/system/econfig.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <math.h>

int eAudioDynamicCompression::read_rms(int i)
{
	struct arg_s arg;
	arg.num = i;
	arg.clear = 1;
	if (ioctl(fd, 0, &arg))
		return -1;
	return (int)(sqrt(arg.dst / arg.dst_n) * 100000 / (17794890));
}
	
void eAudioDynamicCompression::doPoll()
{
	int sum = read_rms(0) + read_rms(1) + read_rms(4) + read_rms(5);
	last_val[last_ptr++] = sum;
	if (last_ptr == 100)
		last_ptr = 0;
	
	int result = 0;
	
	current_value = sum;
	
	current_fast = current_value;
	
	int i, maxval=0;
	for (i=0; i<100; ++i)
		if (last_val[i] > maxval)
			maxval = last_val[i];

	current_value = current_fast;
	
	if (maxval < hyst_low)
	{
		result = +1;
	} else if (current_fast > hyst_hi * 2)
	{
		result = -4;
	} else if (current_fast > hyst_hi)
	{
		result = -1;
 	}
	
//	eDebug("%d, %d (%d %d) %d %d", sum, result, hyst_low, hyst_hi, maxval, current_fast);
	eAVSwitch *avsw=eAVSwitch::getInstance();
	if (avsw && result && enabled && !avsw->getMute())
		avsw->changeVolume(0, -result);
}

eAudioDynamicCompression *eAudioDynamicCompression::instance;

eAudioDynamicCompression::eAudioDynamicCompression(): pollTimer(eApp)
{
	fd = ::open("/dev/audio", O_RDWR);
	if (fd < 0)
	{
		eWarning("can't open /dev/audio (%m) - disabling audio dynamic compression support.");
		return;
	}

//	CONNECT(pollTimer.timeout, eAudioDynamicCompression::doPoll);

	enabled = 0;
	eConfig::getInstance()->getKey("/elitedvb/audio/dynamicadjust", enabled);
	int val = 8000;;
	eConfig::getInstance()->getKey("/ezap/audio/dynamicadjust_value", val);
	setMax(val);
	setEnable(enabled);
	
	current_value = 0;
	instance = this;
	
	for (int i=0; i<100; ++i)
		last_val[i] = 0;
	last_ptr = 0;
}

void eAudioDynamicCompression::setEnable(int enable)
{
	enabled = enable;
	if (enabled)
		pollTimer.start(100, 0);
	else
		pollTimer.stop();
}

eAudioDynamicCompression::~eAudioDynamicCompression()
{
	if (fd >= 0)
		::close(fd);
	instance = 0;
	eConfig::getInstance()->setKey("/elitedvb/audio/dynamicadjust", enabled);
	eConfig::getInstance()->setKey("/ezap/audio/dynamicadjust_value", hyst_hi);
}

eAutoInitP0<eAudioDynamicCompression> init_eAudioDynamicCompression(eAutoInitNumbers::dvb, "eAudioDynamicCompression");
