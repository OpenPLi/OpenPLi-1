#include <lib/gdi/gfbdc.h>

#include <lib/dvb/edvb.h>
#include <lib/system/init.h>
#include <lib/system/info.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>

gFBDC *gFBDC::instance;

gFBDC::gFBDC()
{
	init_gFBDC();
}
void gFBDC::init_gFBDC()
{
	instance=this;
	fb=new fbClass;

	if (!fb->Available())
		eFatal("no framebuffer available");

	colorDepth = 8;
	pixmap = NULL;
	setMode();
}

gFBDC::~gFBDC()
{
	delete pixmap;
	delete fb;
	instance=0;
}

void gFBDC::setColorDepth(int depth)
{
	if (depth != colorDepth)
	{
		colorDepth = depth;
		setMode();
	}
}

void gFBDC::setMode()
{
	eDebug("FBDC colordepth %d-bit\n", colorDepth);
	fb->SetMode(720, 576, colorDepth);

	// make whole screen transparent
	memset(fb->lfb, 0x00, 576 * fb->Stride());

	gPixmap *newpixmap = NULL, *oldpixmap = NULL;
	newpixmap=new gPixmap();
	newpixmap->x = 720;
	newpixmap->y = 576;
	newpixmap->bpp = colorDepth;
	switch (colorDepth)
	{
		case 8:  newpixmap->bypp = 1; break;
		case 15:
		case 16: newpixmap->bypp = 2; break;
		case 24:
		case 32: newpixmap->bypp = 4; break;
	}
	newpixmap->stride=fb->Stride();
	newpixmap->data=fb->lfb;
	
	newpixmap->clut.colors=256;
	newpixmap->clut.data=new gRGB[newpixmap->clut.colors];
	memset(newpixmap->clut.data, 0, sizeof(*newpixmap->clut.data)*newpixmap->clut.colors);

	oldpixmap = pixmap;
	pixmap = newpixmap;
	if (oldpixmap) delete oldpixmap;

	reloadSettings();
}

void gFBDC::calcRamp()
{
#if 0
	float fgamma=gamma ? gamma : 1;
	fgamma/=10.0;
	fgamma=1/log(fgamma);
	for (int i=0; i<256; i++)
	{
		float raw=i/255.0; // IIH, float.
		float corr=pow(raw, fgamma) * 256.0;

		int d=corr * (float)(256-brightness) / 256 + brightness;
		if (d < 0)
			d=0;
		if (d > 255)
			d=255;
		ramp[i]=d;
		
		rampalpha[i]=i*alpha/256;
	}
#endif
	for (int i=0; i<256; i++)
	{
		int d;
		d=i;
		d=(d-128)*(gamma+64)/(128+64)+128;
		d+=brightness-128; // brightness correction
		if (d<0)
			d=0;
		if (d>255)
			d=255;
		ramp[i]=d;

		rampalpha[i]=i*alpha/256;
	}

	rampalpha[255]=255; // transparent BLEIBT bitte so.
}

void gFBDC::setPalette()
{
	if (!pixmap->clut.data)
		return;
	
	for (int i=0; i<256; ++i)
	{
		fb->CMAP()->red[i]=ramp[pixmap->clut.data[i].r]<<8;
		fb->CMAP()->green[i]=ramp[pixmap->clut.data[i].g]<<8;
		fb->CMAP()->blue[i]=ramp[pixmap->clut.data[i].b]<<8;
		fb->CMAP()->transp[i]=rampalpha[pixmap->clut.data[i].a]<<8;
		if (!fb->CMAP()->red[i])
			fb->CMAP()->red[i]=0x100;
	}
	fb->PutCMAP();
}

void gFBDC::exec(gOpcode *o)
{
	switch (o->opcode)
	{
	case gOpcode::setPalette:
	{
		gPixmapDC::exec(o);
		setPalette();
		break;
	}
	default:
		gPixmapDC::exec(o);
		break;
	}
}

gFBDC *gFBDC::getInstance()
{
	return instance;
}

void gFBDC::setAlpha(int a)
{
	alpha = (colorDepth == 32) ? a/10 : a;

	calcRamp();
	setPalette();
	// eDebug("setAlpha %d\n", a);
}

int gFBDC::getAlpha()
{
	return (colorDepth == 32) ?  alpha*10 : alpha;
}

void gFBDC::setBrightness(int b)
{
	brightness=b;

	calcRamp();
	setPalette();
}

void gFBDC::setGamma(int g)
{
	gamma=g;

	calcRamp();
	setPalette();
}

void gFBDC::saveSettings()
{
	eConfig::getInstance()->setKey("/ezap/osd/alpha", (colorDepth == 32) ? alpha*10 : alpha);
	eConfig::getInstance()->setKey("/ezap/osd/gamma", gamma);
	eConfig::getInstance()->setKey("/ezap/osd/brightness", brightness);
}

void gFBDC::reloadSettings()
{
	if (eConfig::getInstance()->getKey("/ezap/osd/alpha", alpha))
	{
		alpha=33;
	}
	if (colorDepth == 32)
	{
		alpha /= 10;
	}

	if (eConfig::getInstance()->getKey("/ezap/osd/gamma", gamma))
		gamma=128;
	if (eConfig::getInstance()->getKey("/ezap/osd/brightness", brightness))
		brightness=128;

	calcRamp();
	setPalette();
}

eAutoInitP0<gFBDC> init_gFBDC(eAutoInitNumbers::graphic-1, "GFBDC");
