#ifndef __gfbdc_h
#define __gfbdc_h

#include "fb.h"
#include "gpixmap.h"
#include "grc.h"

class gFBDC: public gPixmapDC
{
	fbClass *fb;
	static gFBDC *instance;
	void exec(gOpcode *opcode);
	unsigned char ramp[256], rampalpha[256]; // RGB ramp 0..255
	int brightness, gamma, alpha;
	int colorDepth;
	void calcRamp();
	void setPalette();
	void setMode();
	void init_gFBDC();
public:
	void reloadSettings();
	void setAlpha(int alpha);
	void setBrightness(int brightness);
	void setGamma(int gamma);
	void setColorDepth(int depth);

	int getAlpha();
	int getBrightness() { return brightness; }
	int getGamma() { return gamma; }
	unsigned char getRamp(int i) { return ramp[i]; }
	unsigned char getRampAlpha(int i) { return rampalpha[i]; }

	void saveSettings();

	gFBDC();
	~gFBDC();
	static gFBDC *getInstance();
	int islocked() { return fb->islocked(); }
};


#endif
