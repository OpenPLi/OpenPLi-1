#ifndef __eavswitch_h
#define __eavswitch_h

#include <lib/base/ebase.h>

enum eAVAspectRatio
{
	rUnknown, r43, r169
};

enum eAVColorFormat
{
	cfNull,
	cfCVBS, 	// "FBAS"
	cfRGB,
	cfYC,			// "SVideo"
	cfYPbPr		// "Component"
};

enum eVSystem
{
	vsPAL,
	vsNTSC,
};

class eAVSwitch
{
	static eAVSwitch *instance;
	int volume, VCRVolume, mute, useOst, audioChannel;

	int avsfd, saafd;
	eVSystem system;
	eAVAspectRatio aspect;
	eAVColorFormat colorformat;

	int setVolume(int vol);	// 0..65535
	void muteAvsAudio(bool);
	void muteOstAudio(bool);
	bool loadScartConfig();
	void init_eAVSwitch();
protected:
	enum {NOKIA, SAGEM, PHILIPS} Type;
	int scart[6];
	int dvb[6];
	int active, input;
	void init();
public:
	eAVSwitch();
	~eAVSwitch();
	Signal2<void, int, int> volumeChanged;

	static eAVSwitch *getInstance();
	int getVolume() { return volume; }
	int getMute() { return mute; }

	void sendVolumeChanged();
	void reloadSettings();

	/**
	 * \brief Changes the volume.
	 *
	 * \param abs What to change:
	 * \arg \c 0 Volume, relative
	 * \arg \c 1 Volume, absolute
	 * \param vol The volume/muteflag to set. In case of volume, 0 means max and 63 means min.
	 */
	void changeVolume(int abs, int vol);

	int setTVPin8CheckVCR(int vol);
	int setTVPin8(int vol);
	int setColorFormat(eAVColorFormat cf);
	int setAspectRatio(eAVAspectRatio as);
	void setVSystem(eVSystem system);
	eVSystem getVSystem() { return system; }
	int setActive(int active);
	int setInput(int v);	// 0: dbox, 1: vcr
	int getInput() { return input; }
	void selectAudioChannel( int chan ); // 0 MonoLeft, 1 Stereo, 2 MonoRight
	int getAudioChannel() const { return audioChannel; }
	void changeVCRVolume(int abs, int vol);
	void toggleMute();
	void setVideoFormat( int );
	int toggleAspect();
};

class eAVSwitchNokia: public eAVSwitch
{
public:
	eAVSwitchNokia()
	{
		Type = NOKIA;
		scart[0] = 3;
		scart[1] = 2;
		scart[2] = 1;
		scart[3] = 0;
		scart[4] = 1;
		scart[5] = 1;
		dvb[0] = 5;
		dvb[1] = 1;
		dvb[2] = 1;
		dvb[3] = 1;
		dvb[4] = 1;
		dvb[5] = 1;
		init();
	}
};

class eAVSwitchPhilips: public eAVSwitch
{
public:
	eAVSwitchPhilips()
	{
		Type = PHILIPS;
		scart[0] = 3;
		scart[1] = 3;
		scart[2] = 2;
		scart[3] = 2;
		scart[4] = 3;
		scart[5] = 2;
		dvb[0] = 1;
		dvb[1] = 1;
		dvb[2] = 1;
		dvb[3] = 1;
		dvb[4] = 1;
		dvb[5] = 1;
		init();
	}
};

class eAVSwitchSagem: public eAVSwitch
{
public:
	eAVSwitchSagem()
	{
		Type = SAGEM;
		scart[0] = 2;
		scart[1] = 1;
		scart[2] = 0;
		scart[3] = 0;
		scart[4] = 0;
		scart[5] = 0;
		dvb[0] = 0;
		dvb[1] = 0;
		dvb[2] = 0;
		dvb[3] = 0;
		dvb[4] = 0;
		dvb[5] = 0;
		init();
	}
};

#endif
