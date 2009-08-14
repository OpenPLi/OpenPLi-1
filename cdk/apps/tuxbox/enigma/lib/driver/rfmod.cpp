#ifdef ENABLE_RFMOD

#include <lib/driver/rfmod.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <lib/system/econfig.h>
#include <lib/dvb/edvb.h>

#define RFMOD_DEV "/dev/rfmod0"
#define IOCTL_SET_CHANNEL           0
#define IOCTL_SET_TESTMODE          1
#define IOCTL_SET_SOUNDENABLE       2
#define IOCTL_SET_SOUNDSUBCARRIER   3
#define IOCTL_SET_FINETUNE          4
#define IOCTL_SET_STANDBY           5

#define C0	3
#define C1	2
#define FL	1
#define FM	0

eRFmod *eRFmod::instance=0;

eRFmod::eRFmod()
{
	if (!instance)
		instance=this;

	rfmodfd=open(RFMOD_DEV, O_RDWR);
}

void eRFmod::init()
{
	if (eConfig::getInstance()->getKey("/elitedvb/rfmod/ssc", soundsubcarrier))
	{
		soundsubcarrier=5500;
		eConfig::getInstance()->setKey("/elitedvb/rfmod/ssc", soundsubcarrier);
	}

	if (eConfig::getInstance()->getKey("/elitedvb/rfmod/so", soundenable))
	{
		soundenable=0;
		eConfig::getInstance()->setKey("/elitedvb/rfmod/so", soundenable);	
	}

	if (eConfig::getInstance()->getKey("/elitedvb/rfmod/channel", channel))
	{
		channel=36;
		eConfig::getInstance()->setKey("/elitedvb/rfmod/channel", channel);	
	}

	if (eConfig::getInstance()->getKey("/elitedvb/rfmod/finetune", finetune))
	{
		finetune=0;
		eConfig::getInstance()->setKey("/elitedvb/rfmod/finetune", finetune);
	}

	if (eConfig::getInstance()->getKey("/elitedvb/rfmod/standby", standby))
	{
		standby=0;
		eConfig::getInstance()->setKey("/elitedvb/rfmod/standby", standby);
	}
	setSoundSubCarrier(soundsubcarrier);
	setSoundEnable(soundenable);
	setChannel(channel);
	setFinetune(finetune);
	setStandby(standby);
}

eRFmod *eRFmod::getInstance()
{
	return instance;
}

eRFmod::~eRFmod()
{
	save();
	
	if (instance==this)
		instance=0;

	if (rfmodfd>=0)
		close(rfmodfd);
}	

int	eRFmod::save()
{
	eConfig::getInstance()->setKey("/elitedvb/rfmod/ssc", soundsubcarrier);
	eConfig::getInstance()->setKey("/elitedvb/rfmod/so", soundenable);
	eConfig::getInstance()->setKey("/elitedvb/rfmod/channel", channel);
	eConfig::getInstance()->setKey("/elitedvb/rfmod/finetune", finetune);
	eConfig::getInstance()->setKey("/elitedvb/rfmod/standby", standby);
	eConfig::getInstance()->flush();
	return 0;
}


int	eRFmod::setSoundEnable(int val)
{
	soundenable = val;

	if(rfmodfd > 0)
		ioctl(rfmodfd,IOCTL_SET_SOUNDENABLE,&soundenable);
		
	return 0;	
}

int	eRFmod::setStandby(int val)
{
	standby = val;

	if(rfmodfd > 0)
		ioctl(rfmodfd,IOCTL_SET_STANDBY,&standby);

	return 0;
}

int eRFmod::setChannel(int val)
{
	channel = val;

	if(rfmodfd > 0)
		ioctl(rfmodfd,IOCTL_SET_CHANNEL,&channel);
		
	return 0;	
}

int	eRFmod::setFinetune(int val)
{
	finetune = val;

	if(rfmodfd > 0)
		ioctl(rfmodfd,IOCTL_SET_FINETUNE,&finetune);
		
	return 0;	
}		

int	eRFmod::setTestPattern(int val)
{
	if(rfmodfd > 0)
		ioctl(rfmodfd,IOCTL_SET_TESTMODE,&val);
		
	return 0;	
}		


int eRFmod::setSoundSubCarrier(int freq)			//freq in KHz
{
	int sfd=0;

	soundsubcarrier=freq;
	
	switch(freq)
	{
		case 4500:
			sfd=0;
			break;
		case 5500:
			sfd=1;
			break;
		case 6000:
			sfd=2;
			break;
		case 6500:
			sfd=3;
			break;
		default:
			eDebug("eRFMOD: unsupported Sound sub carrier Frequency");	
			return -1;
	}		

	if(rfmodfd > 0)
		ioctl(rfmodfd,IOCTL_SET_SOUNDSUBCARRIER,&sfd);

	return 0;
}

#endif // ENABLE_RFMOD
