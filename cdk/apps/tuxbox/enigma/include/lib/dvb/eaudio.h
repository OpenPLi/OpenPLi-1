#ifndef __eaudio_h
#define __eaudio_h

class eAudio
{
	static eAudio *instance;
	int ac3default;
public:
	void reloadSettings();
	void setAC3default(int a);
	
	int getAC3default() { return ac3default; }
	
	void saveSettings();
	
	eAudio();
	~eAudio();
	static eAudio *getInstance();
};


#endif
