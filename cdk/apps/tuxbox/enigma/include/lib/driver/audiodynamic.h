#ifndef __lib_driver_audiodynamic_h
#define __lib_driver_audiodynamic_h

class eAudioDynamicCompression: public Object
{
	struct arg_s
	{
		int num;
		int clear;
		unsigned long long dst;
		int dst_n;
	};
	eTimer pollTimer;
	int fd;
	int current_value;
	
	int last_val[100]; // the last 10 seconds
	int last_ptr;
	
	int read_rms(int i);
	int hyst_low, hyst_hi;
	int current_fast, current_slow;
	int enabled;
	
	void doPoll();
	static eAudioDynamicCompression *instance;
public:
	static eAudioDynamicCompression *getInstance()
	{
		return instance;
	}
	
	int getCurrent() { if (!enabled) doPoll(); return current_value; }
	void setEnable(int enable);
	int getEnable() { return enabled; }
	int getMax() { return hyst_hi; }
	void setMax(int h) { hyst_hi = h; hyst_low = h / 2; }
	
	eAudioDynamicCompression();
	~eAudioDynamicCompression();
};

#endif
