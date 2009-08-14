/*
 * $Id: pictureviewer.cpp,v 1.47 2009/02/03 18:53:43 dbluelle Exp $
 *
 * (C) 2005 by digi_casi <digi_casi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */
#include <stdio.h>
 
#include <config.h>
#include <lib/driver/streamwd.h>
#include <lib/system/info.h>
#include <lib/system/init.h>
#include <lib/system/init_num.h>
#include <lib/system/econfig.h>
#include <lib/gdi/fb.h>
#include <lib/gdi/grc.h>
#include <lib/base/estring.h>
#include <lib/gui/actions.h>
#include <lib/gui/guiactions.h>
#include <lib/gui/numberactions.h>
#include <lib/dvb/servicedvb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <src/enigma_main.h>
#include <lib/gui/eskin.h>
#include <lib/gui/elabel.h>
#include <lib/gdi/font.h>
#include <lib/picviewer/pictureviewer.h>
#include "fb_display.h"
#include <lib/picviewer/format_config.h>
#include <lib/driver/eavswitch.h>
#include <lib/base/eerror.h>


/* resize.cpp */
extern unsigned char *simple_resize(unsigned char *orgin, int ox, int oy, int dx, int dy, int rotate);
extern unsigned char *color_average_resize(unsigned char *orgin, int ox, int oy, int dx, int dy, int rotate);
extern unsigned char *simple_rotate(unsigned char *orgin, int ox, int oy, int rotate);

#ifdef FBV_SUPPORT_GIF
extern int fh_gif_getsize(const char *, int *, int *);
extern int fh_gif_load(const char *, unsigned char *, int, int);
extern int fh_gif_id(const char *);
#endif
#ifdef FBV_SUPPORT_JPEG
extern int fh_jpeg_getsize(const char *, int *, int *);
extern int fh_jpeg_load(const char *, unsigned char *, int, int);
extern int fh_jpeg_id(const char *);
#endif
#ifdef FBV_SUPPORT_PNG
extern int fh_png_getsize(const char *, int *, int *);
extern int fh_png_load(const char *, unsigned char *, int, int);
extern int fh_png_id(const char *);
#endif
#ifdef FBV_SUPPORT_BMP
extern int fh_bmp_getsize(const char *, int *, int *);
extern int fh_bmp_load(const char *, unsigned char *, int, int);
extern int fh_bmp_id(const char *);
#endif
#ifdef FBV_SUPPORT_CRW
extern int fh_crw_getsize(const char *, int *, int *);
extern int fh_crw_load(const char *, unsigned char *, int, int);
extern int fh_crw_id(const char *);
#endif

bool doscale = false;
int rotate = 0;

void ePictureViewer::nextPicture()
{
	doscale = false;
	rotate = 0;
	if (++myIt == slideshowList.end())
		myIt = slideshowList.begin();
}

void ePictureViewer::previousPicture()
{
	doscale = false;
	rotate = 0;
	if (myIt == slideshowList.begin())
		myIt = slideshowList.end();
	myIt--;
}

void ePictureViewer::showNameOnLCD(const eString& filename)
{
	unsigned int pos = filename.find_last_of("/");
	pos = (pos == eString::npos) ? 0 : pos + 1;
	eString name = filename.substr(pos, filename.length() - 1);
#ifndef DISABLE_LCD
	pLCD->lcdMain->ServiceName->setText(name);
	pLCD->lcdMain->show();
#endif
}

ePictureViewer::ePictureViewer(const eString &filename)
	:eWidget(0, 1), slideshowTimer(eApp), filename(filename)
{
	// eDebug("picviewer: Constructor...");
	init_ePictureViewer();
}
void ePictureViewer::init_ePictureViewer()
{

	addActionMap(&i_cursorActions->map);
	addActionMap(&i_shortcutActions->map);
	addActionMap(&i_numberActions->map);


	move(ePoint(70, 50));
	resize(eSize(590, 470));
	eLabel *l = new eLabel(this);
	l->move(ePoint(150, clientrect.height() / 2));
	l->setFont(eSkin::getActive()->queryFont("epg.title"));
	l->resize(eSize(clientrect.width() - 100, 30));
	l->setText(_("Loading slide... please wait."));

#ifndef DISABLE_LCD
	pLCD = eZapLCD::getInstance();
#endif

	fh_root = NULL;
	m_scaling = COLOR;

	m_NextPic_Name = "";
	m_NextPic_Buffer = NULL;// buffer for original image read from disk
	m_NextPic_Image = NULL;	// buffer holding processed image to show (partly) on display
	m_Img_X = 0;		// size of image in m_NextPic_Image
	m_Img_Y = 0;
	m_Img_XPos = 0;		// starting location on screen
	m_Img_YPos = 0;
	m_Img_XPan = 0;		// location withing m_NextPic_Image from where we start displaying
	m_Img_YPan = 0;

	m_startx = 20, m_starty = 20, m_endx = 699, m_endy = 555;
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/left", m_startx); // left
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/top", m_starty); // top
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/right", m_endx); // right
	eConfig::getInstance()->getKey("/enigma/plugins/needoffsets/bottom", m_endy); // bottom

	int showbusysign = 1;
	eConfig::getInstance()->getKey("/picviewer/showbusysign", showbusysign);
	showBusySign = (showbusysign == 1);
	unsigned int v_pin8 = 0;
	eConfig::getInstance()->getKey("/elitedvb/video/pin8", v_pin8);
	switchto43 = false;
	if ((v_pin8 == 3) || // always 16:9
		((v_pin8 > 1) && (eServiceInterface::getInstance()->getService()->getAspectRatio() == 1)))
	{
		m_aspect = 16.0 / 9;
	}
	else
	{
		m_aspect = 4.0 / 3;
		switchto43 = true;
	}

	format169 = 0;
	eConfig::getInstance()->getKey("/picviewer/format169", format169);
	if (format169)
	{
		eAVSwitch::getInstance()->setAspectRatio(r169);
		m_aspect = 16.0 / 9;
	}
	m_busy_buffer = NULL;

	init_handlers();

	CONNECT(slideshowTimer.timeout, ePictureViewer::slideshowTimeout);
	// eDebug("picviewer: Constructor done.");
}

ePictureViewer::~ePictureViewer()
{
	// eDebug("picviewer: closing class. Cleanup.");
	if (m_busy_buffer != NULL)
	{
		delete [] m_busy_buffer;
		m_busy_buffer = NULL;
	}
	if (m_NextPic_Image != m_NextPic_Buffer && m_NextPic_Image != NULL)
	{
		delete [] m_NextPic_Image;
		m_NextPic_Image = NULL;
	}
	if (m_NextPic_Buffer != NULL)
	{
		delete [] m_NextPic_Buffer;
		m_NextPic_Buffer = NULL;
	}

	CFormathandler *tmp = NULL;
	while(fh_root)
	{
		tmp = fh_root;
		fh_root = fh_root->next;
		delete tmp;
	}
// restore original screen size
	eStreamWatchdog::getInstance()->reloadSettings(); 
}

void ePictureViewer::add_format(int (*picsize)(const char *, int *, int *), int (*picread)(const char *, unsigned char *, int, int), int (*id)(const char*), int fmt)
{
	CFormathandler *fhn;
	fhn = new CFormathandler;
	fhn->get_size = picsize;
	fhn->get_pic = picread;
	fhn->id_pic = id;
	fhn->next = fh_root;
	fhn->fmt = fmt;
	fh_root = fhn;
}

void ePictureViewer::init_handlers(void)
{
#ifdef FBV_SUPPORT_GIF
	add_format(fh_gif_getsize, fh_gif_load, fh_gif_id, FMT_GIF);
#endif
#ifdef FBV_SUPPORT_JPEG
	add_format(fh_jpeg_getsize, fh_jpeg_load, fh_jpeg_id, FMT_JPG);
#endif
#ifdef FBV_SUPPORT_PNG
	add_format(fh_png_getsize, fh_png_load, fh_png_id, FMT_PNG);
#endif
#ifdef FBV_SUPPORT_BMP
	add_format(fh_bmp_getsize, fh_bmp_load, fh_bmp_id, FMT_BMP);
#endif
#ifdef FBV_SUPPORT_CRW
	add_format(fh_crw_getsize, fh_crw_load, fh_crw_id, FMT_CRW);
#endif
}

ePictureViewer::CFormathandler *ePictureViewer::fh_getsize(const char *name, int *x, int *y)
{
	CFormathandler *fh;
	for (fh = fh_root; fh != NULL; fh = fh->next)
	{
		if (fh->id_pic(name))
			if (fh->get_size(name, x, y) == FH_ERROR_OK)
				return fh;
	}
	return NULL;
}

bool ePictureViewer::ProcessImage(bool unscaled, int rotate)
{
	int x, y, xs, ys, imx, imy, scrx, scry, needscale;

	// Show green block for "next ready" in view state
	if (showBusySign)
		showBusy(m_startx + 3, m_starty + 3, 10, 0, 0xff, 0);

	getCurrentRes(&xs, &ys);
	eDebug("picviewer: processImage: unscale=%d, rotate=%d xs=%d, ys=%d buf=%lx imgbuf was=%lx", unscaled, rotate, xs, ys, m_NextPic_Buffer,  m_NextPic_Image);
	if (m_NextPic_Image != m_NextPic_Buffer && m_NextPic_Image != NULL)
	{
		// Free image memory when we did some transformation on the original
		delete [] m_NextPic_Image;
		m_NextPic_Image = NULL;
	}
	
	scrx = m_endx - m_startx;
	scry = m_endy - m_starty;
	if (rotate % 2)
	{
		x = m_NextPic_Y;
		y = m_NextPic_X;
	}
	else
	{
		x = m_NextPic_X;
		y = m_NextPic_Y;
	}
	needscale = x > scrx || y > scry;
	// It seems enigma crashes when images larger than screensize are shown unscaled...
	eDebug("picviewer: processImage: x,y=%d,%d, needscale=%d m_scaling=%d", x, y, needscale, m_scaling);
	if ( m_scaling != NONE && ((needscale && !unscaled) || (!needscale && unscaled)) )
//	if ( m_scaling != NONE && (needscale || (!needscale && unscaled)) )
	{
		// perform scaling AND rotation
		double aspect_ratio_correction = m_aspect / ((double)xs / ys);
		if ((aspect_ratio_correction * y * scrx) <= scry * x)
		{
			imx = scrx;
			imy = (int)(aspect_ratio_correction * y * scrx / x);
		}
		else
		{
			imx = (int)((1.0 / aspect_ratio_correction) * x * scry / y);
			imy = scry;
		}
		int x1, y1;
		if (rotate % 2)
		{
			x1 = imy;
			y1 = imx;
		}
		else
		{
			x1 = imx;
			y1 = imy;
		}
		if (m_scaling == SIMPLE)
			m_NextPic_Image = simple_resize(m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y, x1, y1, rotate);
		else
			m_NextPic_Image = color_average_resize(m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y, x1, y1, rotate);
		if (m_NextPic_Image != m_NextPic_Buffer)
		{ // we have a new picture, set x/y to the new dimensions
			x = imx;
			y = imy;
		}
	}
	else if (rotate)
		// perform rotation ONLY
		m_NextPic_Image = simple_rotate(m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y, rotate);
	else
		m_NextPic_Image = m_NextPic_Buffer;

	if (m_NextPic_Image == NULL)
		return false;

	m_Img_X = x;
	m_Img_Y = y;

	// center on screen when smaller than screen size
	if (m_Img_X < scrx)
		m_Img_XPos = (scrx - m_Img_X) / 2 + m_startx;
	else
		m_Img_XPos = m_startx;
	if (m_Img_Y < scry)
		m_Img_YPos = (scry - m_Img_Y) / 2 + m_starty;
	else
		m_Img_YPos = m_starty;

	// show center of image on screen when image is bigger than screen size
	if (m_Img_X > scrx)
		m_Img_XPan = (m_Img_X - scrx) / 2;
	else
		m_Img_XPan = 0;
	if (m_Img_Y > scry)
		m_Img_YPan = (m_Img_Y - scry) / 2;
	else
		m_Img_YPan = 0;

	eDebug("picviewer: processImage: decoded to picsize=%d,%d pos=%d,%d panpos=%d,%d",
		m_Img_X, m_Img_Y, m_Img_XPos, m_Img_YPos, m_Img_XPan, m_Img_YPan);
	hideBusy();
	return true;
}

bool ePictureViewer::DecodeImage(const std::string& name)
{
	eDebug("picviewer: DecodeImage %s", name.c_str());

	// Show red block for "next ready" in view state
	if (showBusySign)
		showBusy(m_startx + 3, m_starty + 3, 10, 0xff, 0, 0);

	// Free image memory when we did some transformation on the original
	if (m_NextPic_Image != m_NextPic_Buffer && m_NextPic_Image != NULL) {
		delete [] m_NextPic_Image;
	}
	m_NextPic_Image = NULL;
	if (m_NextPic_Buffer != NULL)
	{
		delete [] m_NextPic_Buffer;
		m_NextPic_Buffer = NULL;
	}

	CFormathandler *fh;
	bool havepic = false;
	fh = fh_getsize(name.c_str(), &m_NextPic_X, &m_NextPic_Y);
	if (fh)
	{
		if (fh->fmt == FMT_JPG)
		{
			int scrx, scry;
			scrx = m_endx - m_startx;
			scry = m_endy - m_starty;
			eDebug("picviewer: DecodeImage raw resolution: %dx%d ", m_NextPic_X, m_NextPic_Y);
			// check if picture is to big, and use jpg lib to resize a bit
			if (m_NextPic_X > m_NextPic_Y)
			{
				if (m_NextPic_X > 12*scrx)
				{
					m_NextPic_X /= 8;
					m_NextPic_Y /= 8;
				}
				else if (m_NextPic_X > 6*scrx)
				{
					m_NextPic_X /= 4;
					m_NextPic_Y /= 4;
				}
				else if (m_NextPic_X > 3*scrx)
				{
					m_NextPic_X /= 2;
					m_NextPic_Y /= 2;
				}
			}
			else
			{
				if (m_NextPic_Y > 12*scry)
				{
					m_NextPic_X /= 8;
					m_NextPic_Y /= 8;
				}
				else if (m_NextPic_Y > 6*scry)
				{
					m_NextPic_X /= 4;
					m_NextPic_Y /= 4;
				}
				else if (m_NextPic_Y > 3*scry)
				{
					m_NextPic_X /= 2;
					m_NextPic_Y /= 2;
				}
			}
			eDebug("picviewer: DecodeImage limit resolution: %dx%d ", m_NextPic_X, m_NextPic_Y);
		}
		m_NextPic_Buffer = new unsigned char[m_NextPic_X * m_NextPic_Y * 3];
		if (m_NextPic_Buffer == NULL)
		{
			eDebug("picviewer: DecodeImage Error: malloc picture buffer");
		}
		else if (fh->get_pic(name.c_str(), m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y) == FH_ERROR_OK)
		{
			havepic = true;
		}
		else
		{
			delete [] m_NextPic_Buffer;
			m_NextPic_Buffer = NULL;
//			eDebug("picviewer: DecodeImage: Unable to read file !");
		}
	}

	if (!havepic)
	{
		eDebug("picviewer: DecodeImage: Unable to read file or format not recognized!");
		m_NextPic_Buffer = new unsigned char[3];
		if (m_NextPic_Buffer == NULL)
		{
			eDebug("picviewer: DecodeImage Error: malloc nofile picture buffer");
			return false;
		}
		memset(m_NextPic_Buffer, 0 , 3);
		m_Img_X = 1;
		m_Img_Y = 1;
		m_Img_XPos = 0;
		m_Img_YPos = 0;
		m_Img_XPan = 0;
		m_Img_YPan = 0;
	}
	m_NextPic_Name = name;
	hideBusy();
	eDebug("picviewer: DecodeImage done");
	return (m_NextPic_Buffer != NULL);
}

bool ePictureViewer::ShowImage(const std::string& filename, bool unscaled)
{
//	eDebug("Show Image {");
	unsigned int pos = filename.find_last_of("/");
	if (pos == eString::npos)
		pos = filename.length() - 1;
	eString directory = pos ? filename.substr(0, pos) : "";
//	eDebug("---directory: %s", directory.c_str());
	slideshowList.clear();
	int includesubdirs = 0;
	eConfig::getInstance()->getKey("/picviewer/includesubdirs", includesubdirs);
	listDirectory(directory, includesubdirs);
	for (myIt = slideshowList.begin(); myIt != slideshowList.end(); myIt++)
	{
		eString tmp = *myIt;
//		eDebug("[PICTUREVIEWER] comparing: %s:%s", tmp.c_str(), filename.c_str());
		if (tmp == filename)
			break;
	}
	// eDebug("[PICTUREVIEWER] To DecodeImage %s, scale=%d", filename.c_str(), unscaled);

	struct fb_var_screeninfo *screenInfo = fbClass::getInstance()->getScreenInfo();

	switch( eSystemInfo::getInstance()->getHwType() )
	{
	case eSystemInfo::DM7000:
	case eSystemInfo::DM7020:
		want_bpp = 32;
		break;
	case eSystemInfo::DM500:
	case eSystemInfo::DM5600:
	case eSystemInfo::DM5620:
		want_bpp = 8;
		break;
	default:
		want_bpp = 16;
		break;
	}

	prev_bpp = screenInfo->bits_per_pixel;
	fbClass::getInstance()->lock(); // lock framebuffer ==> will finish the gDC queue
	if (screenInfo->bits_per_pixel != want_bpp)
	{
		fbClass::getInstance()->SetMode(720, 576, want_bpp);
#if HAVE_DVB_API_VERSION == 3
		fbClass::getInstance()->setTransparency(0);
#endif
		eDebug("picviewer: ShowImage: Switch to %d-bit color mode", want_bpp);
	}
#ifndef DISABLE_LCD
	pLCD->lcdMenu->hide();
#endif
	if (DecodeImage(filename))
		ProcessImage(unscaled);
	showNameOnLCD(filename);
	DisplayNextImage();
	// eDebug("Show Image }");
	return true;
}

void ePictureViewer::slideshowTimeout()
{
	eString tmp = *myIt;
//	eDebug("[PICTUREVIEWER] slideshowTimeout: show %s", tmp.c_str());
	if (DecodeImage(*myIt))
		ProcessImage(false);
	showNameOnLCD(tmp);
	DisplayNextImage();
	nextPicture();
	int timeout = 5;
	eConfig::getInstance()->getKey("/picviewer/slideshowtimeout", timeout);
	slideshowTimer.start(timeout * 1000, true);
}

int ePictureViewer::eventHandler(const eWidgetEvent &evt)
{
	static eString serviceName;
	fflush(stdout);
	switch(evt.type)
	{
		case eWidgetEvent::evtAction:
		{

			if (evt.action == &i_cursorActions->cancel)
			{
				// eDebug("pictureviewer: event cancel");
				close(0);
			}
			else
			if (evt.action == &i_numberActions->key1)
			{
				// eDebug("pictureviewer: event 1");
				if (Move(-30, -30))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_numberActions->key2)
			{
				// eDebug("pictureviewer: event 2");
				if (Move(0, -30))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_numberActions->key3)
			{
				// eDebug("pictureviewer: event 3");
				if (Move(30, -30))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_numberActions->key4)
			{
				// eDebug("pictureviewer: event 4");
				if (Move(-30, 0))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_numberActions->key6)
			{
				// eDebug("pictureviewer: event 6");
				if (Move(30, 0))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_numberActions->key5)
			{ // centre image on screen
				// eDebug("pictureviewer: event 5");
				int scrx, scry;
				scrx = m_endx - m_startx;
				scry = m_endy - m_starty;
				if (m_Img_X > scrx || m_Img_Y > scry)
				{
					if (m_Img_X > scrx)
						m_Img_XPan = (m_Img_X - scrx) / 2;
					else
						m_Img_XPan = 0;
					if (m_Img_Y > scry)
						m_Img_YPan = (m_Img_Y - scry) / 2;
					else
						m_Img_YPan = 0;
					DisplayNextImage();
				}
			}
			else
			if (evt.action == &i_numberActions->key7)
			{
				// eDebug("pictureviewer: event 7");
				if (Move(-30, 30))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_numberActions->key8)
			{
				// eDebug("pictureviewer: event 8");
				if (Move(0, 30))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_numberActions->key9)
			{
				// eDebug("pictureviewer: event 9");
				if (Move(30, 30))
					DisplayNextImage();
			}
			else
			if (evt.action == &i_shortcutActions->yellow)
			{
				// eDebug("pictureviewer: event yellow");
				slideshowTimer.stop();
				previousPicture();
			}
			else
			if (evt.action == &i_shortcutActions->green)
			{
				// eDebug("pictureviewer: event green");
				nextPicture();
				slideshowTimeout();
			}
			else
			if (evt.action == &i_shortcutActions->red)
			{
				// eDebug("pictureviewer: event red");
				rotate = (rotate + 3) % 4;
				if (m_NextPic_Buffer != NULL)
					ProcessImage(doscale, rotate);
				DisplayNextImage();
			}
			else
			if (evt.action == &i_shortcutActions->blue)
			{
				// eDebug("pictureviewer: event blue");
				rotate = (rotate + 1) % 4;
				if (m_NextPic_Buffer != NULL)
					ProcessImage(doscale, rotate);
				DisplayNextImage();
			}
			else
			if (evt.action == &i_cursorActions->help)
			{
				// eDebug("pictureviewer: event help");
				doscale = doscale == true ? false : true;
				if (m_NextPic_Buffer != NULL)
					ProcessImage(doscale, rotate);
				DisplayNextImage();
			}
			else
			if (evt.action == &i_cursorActions->down ||
			    evt.action == &i_cursorActions->right)
			{
				// eDebug("pictureviewer: event down/right");
				nextPicture();
				if (DecodeImage(*myIt))
					ProcessImage(false, rotate);
				showNameOnLCD(*myIt);
				DisplayNextImage();
			}
			else
			if (evt.action == &i_cursorActions->up ||
			    evt.action == &i_cursorActions->left)
			{
				// eDebug("pictureviewer: event up/left");
				previousPicture();
				if (DecodeImage(*myIt))
					ProcessImage(false, rotate);
				showNameOnLCD(*myIt);
				DisplayNextImage();
			}
			break;
		}
		case eWidgetEvent::execBegin:
		{
			// eDebug("pictureviewer: event execBegin");
#ifndef DISABLE_LCD
			serviceName = pLCD->lcdMain->ServiceName->getText();
#endif
			ShowImage(filename, false);
			break;
		}
		case eWidgetEvent::execDone:
		{
			int cpp = 0;
			// eDebug("picviewer: event execDone. restore");
			switch (want_bpp) {
				case 8:  cpp = 1; break;
				case 15:
				case 16:  cpp = 2; break;
				case 24:  
				case 32:  cpp = 4; break;
			}
			clearFB(want_bpp, cpp, 0);
			if (prev_bpp != want_bpp)
				fbClass::getInstance()->SetMode(720, 576, prev_bpp);
			fbClass::getInstance()->unlock();
			if (switchto43 && format169)
				eAVSwitch::getInstance()->setAspectRatio(r43);
			showNameOnLCD(serviceName);
#ifndef DISABLE_LCD
			pLCD->lcdMain->hide();
			pLCD->lcdMenu->show();
#endif
			break;
		}
		default:
			// eDebug("picviewer: event other.");
			return eWidget::eventHandler(evt);
	}
	return 1;
}

void ePictureViewer::listDirectory(eString directory, int includesubdirs)
{
//	eDebug("[PICTUREVIEWER] listDirectory: dir %s", directory.c_str());
	std::list<eString> piclist;
	std::list<eString>::iterator picIt;
	std::list<eString> dirlist;
	std::list<eString>::iterator dirIt;
	piclist.clear();
	dirlist.clear();
	DIR *d = opendir(directory.c_str());
	if (d)
	{
		while (struct dirent *e = readdir(d))
		{
			eString filename = eString(e->d_name);
//			eDebug("[PICTUREVIEWER] listDirectory: processing %s", filename.c_str());
			if ((filename != ".") && (filename != ".."))
			{
				struct stat s;
				eString fullFilename = directory + "/" + filename;
				if (lstat(fullFilename.c_str(), &s) < 0)
				{
//					eDebug("--- file no good :(");
					continue;
				}

				if (S_ISREG(s.st_mode))
				{
					if (filename.right(4).upper() == ".JPG" ||
					    filename.right(5).upper() == ".JPEG" ||
					    filename.right(4).upper() == ".CRW" ||
					    filename.right(4).upper() == ".PNG" ||
					    filename.right(4).upper() == ".BMP" ||
					    filename.right(4).upper() == ".GIF")
					{
						eString tmp = directory + "/" + filename;
						piclist.push_back(tmp);
					}
				}
				else
				if ((includesubdirs == 1) && (S_ISDIR(s.st_mode) || S_ISLNK(s.st_mode)))
				{
					eString tmp = directory + "/" + filename;
					dirlist.push_back(tmp);
				}
			}
		}
		closedir(d);
	}
	if (!piclist.empty())
	{
		piclist.sort();
		for (picIt = piclist.begin(); picIt != piclist.end(); picIt++)
		{
			eString tmp = *picIt;
			slideshowList.push_back(tmp);
		}
	}
	if (!dirlist.empty())
	{
		dirlist.sort();
		for (dirIt = dirlist.begin(); dirIt != dirlist.end(); dirIt++)
		{
			eString tmp = *dirIt;
			listDirectory(tmp, includesubdirs);
		}
	}
}

bool ePictureViewer::DisplayNextImage()
{
	int scrx, scry;

	// eDebug("picviewer: DisplayNextImage");
	if (m_NextPic_Image == NULL)
	{
		eDebug("picviewer: DisplayNextImage: no picture...");
		return false;
	}

	scrx = m_endx - m_startx;
	scry = m_endy - m_starty;

	if (scrx > m_Img_X)
		scrx = m_Img_X;
	if (scry > m_Img_Y)
		scry = m_Img_Y;

	fb_display(m_NextPic_Image, m_Img_X, m_Img_Y, m_Img_XPan, m_Img_YPan, m_Img_XPos, m_Img_YPos, scrx, scry);

	// eDebug("picviewer: DisplayNextImage done");
	return true;
}

#if 0
void ePictureViewer::Zoom(float factor)
{
//	eDebug("Zoom %f {",factor);
	showBusy(m_startx + 3, m_starty + 3, 10, 0xff, 0xff, 0);

	int oldx = m_NextPic_X;
	int oldy = m_NextPic_Y;
	unsigned char *oldBuf = m_NextPic_Buffer;
	m_NextPic_X = (int)(factor * m_NextPic_X);
	m_NextPic_Y = (int)(factor * m_NextPic_Y);

	if (m_scaling == COLOR)
		m_NextPic_Buffer = color_average_resize(m_NextPic_Buffer, oldx, oldy, m_NextPic_X, m_NextPic_Y);
	else
		m_NextPic_Buffer = simple_resize(m_NextPic_Buffer, oldx, oldy, m_NextPic_X, m_NextPic_Y);

	if (m_NextPic_Buffer == oldBuf)
	{
		// resize failed
		hideBusy();
		return;
	}

	if (m_NextPic_X < (m_endx - m_startx))
		m_NextPic_XPos = (m_endx - m_startx - m_NextPic_X) / 2 + m_startx;
	else
		m_NextPic_XPos = m_startx;
	if (m_NextPic_Y < (m_endy - m_starty))
		m_NextPic_YPos = (m_endy - m_starty - m_NextPic_Y) / 2 + m_starty;
	else
		m_NextPic_YPos = m_starty;
	if (m_NextPic_X > (m_endx - m_startx))
		m_NextPic_XPan = (m_NextPic_X - (m_endx - m_startx)) / 2;
	else
		m_NextPic_XPan = 0;
	if (m_NextPic_Y > (m_endy - m_starty))
		m_NextPic_YPan = (m_NextPic_Y - (m_endy - m_starty)) / 2;
	else
		m_NextPic_YPan = 0;
	fb_display(m_NextPic_Buffer, m_NextPic_X, m_NextPic_Y, m_NextPic_XPan, m_NextPic_YPan, m_NextPic_XPos, m_NextPic_YPos);
//	eDebug("Zoom }");
}
#endif

bool ePictureViewer::Move(int dx, int dy)
{
	eDebug("picviewer: Move %d %d", dx, dy);

	int scrx, scry;
	int oldx, oldy;
	scrx = m_endx - m_startx;
	scry = m_endy - m_starty;
	
	if (m_Img_X <= scrx && m_Img_Y <= scry)
		return false;

	oldx = m_Img_XPan;
	oldy = m_Img_YPan;
	if (m_Img_X > scrx)
	{
		m_Img_XPan += dx;
		if (m_Img_XPan + scrx >= m_Img_X)
			m_Img_XPan = m_Img_X - scrx - 1;
		if (m_Img_XPan < 0)
			m_Img_XPan = 0;
	}

	if (m_Img_Y > scry)
	{
		m_Img_YPan += dy;
		if (m_Img_YPan + scry >= m_Img_Y)
			m_Img_YPan = m_Img_Y - scry - 1;
		if (m_Img_YPan < 0)
			m_Img_YPan = 0;
	}

	eDebug("picviewer: move: picsize=%d,%d pos=%d,%d panpos=%d,%d",
		m_Img_X, m_Img_Y, m_Img_XPos, m_Img_YPos, m_Img_XPan, m_Img_YPan);

//	eDebug("picviewer: Move done");
	return (oldx != m_Img_XPan || oldy != m_Img_YPan);
}

void ePictureViewer::showBusy(int sx, int sy, int width, char r, char g, char b)
{
//	eDebug("Show Busy{");

	unsigned char rgb_buffer[3];
	unsigned char *fb_buffer=0;
	unsigned char *busy_buffer_wrk;
	int cpp;
	struct fb_var_screeninfo *var = fbClass::getInstance()->getScreenInfo();

	bool yuv_fb = (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM500PLUS || eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM600PVR) && var->bits_per_pixel == 16;

	rgb_buffer[0] = r;
	rgb_buffer[1] = g;
	rgb_buffer[2] = b;

	if (!yuv_fb)
	{
		fb_buffer = (unsigned char *) convertRGB2FB(rgb_buffer, 1, var->bits_per_pixel, &cpp);
		if (fb_buffer == NULL)
		{
			eDebug("Error: malloc");
			return;
		}
	}
	else
		cpp=2;

	if (m_busy_buffer != NULL)
	{
		delete [] m_busy_buffer;
		m_busy_buffer = NULL;
	}
	m_busy_buffer = new unsigned char[width * width * cpp];
	if (m_busy_buffer == NULL)
	{
		eDebug("picviewer: showBusy: Error: malloc busybuffer");
		return;
	}
	busy_buffer_wrk = m_busy_buffer;
	unsigned char *fb = fbClass::getInstance()->lfb;
	unsigned int stride = fbClass::getInstance()->Stride();

	for (int y = sy ; y < sy + width; y++)
	{
		for(int x = sx ; x < sx + width; x++)
		{
			if (yuv_fb)
			{
				int offs = y * stride + x;
				busy_buffer_wrk[0]=fb[offs];  // save old Y
				busy_buffer_wrk[1]=fb[offs + 720 * 576]; // save old UV
				fb[offs]=(lut_r_y[r] + lut_g_y[g] + lut_b_y[b]) >> 8;
				if (x & 1)
					fb[offs + 720 * 576]=((lut_r_v[r] + lut_g_v[g] + lut_b_v[b]) >> 8) + 128;
				else
					fb[offs + 720 * 576]=((lut_r_u[r] + lut_g_u[g] + lut_b_u[b]) >> 8) + 128;
			}
			else
			{
				memcpy(busy_buffer_wrk, fb + y * stride + x, cpp);
				memcpy(fb + y * stride + x * cpp, fb_buffer, cpp);
			}
			busy_buffer_wrk += cpp;
		}
	}

	m_busy_x = sx;
	m_busy_y = sy;
	m_busy_width = width;
	m_busy_cpp = cpp;
	delete [] fb_buffer;
//	eDebug("Show Busy}");
}

void ePictureViewer::hideBusy()
{
//	eDebug("Hide Busy {");
	if (m_busy_buffer != NULL)
	{
		unsigned char *fb = fbClass::getInstance()->lfb;
		unsigned int stride = fbClass::getInstance()->Stride();
		unsigned char *busy_buffer_wrk = m_busy_buffer;
		struct fb_var_screeninfo *var = fbClass::getInstance()->getScreenInfo();
		bool yuv_fb = (eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM500PLUS || eSystemInfo::getInstance()->getHwType() == eSystemInfo::DM600PVR) && var->bits_per_pixel == 16;

		for (int y = m_busy_y; y < m_busy_y + m_busy_width; y++)
		{
			for (int x = m_busy_x; x < m_busy_x + m_busy_width; x++)
			{
				if (yuv_fb)
				{
					int offs = y * stride + x;
					fb[offs]=*(busy_buffer_wrk++);
					fb[offs + 720 * 576]=*(busy_buffer_wrk++);
				}
				else
				{
					memcpy(fb + y * stride + x, busy_buffer_wrk, m_busy_cpp);
					busy_buffer_wrk += m_busy_cpp;
				}
			}
		}
		delete [] m_busy_buffer;
		m_busy_buffer = NULL;
	}
//	eDebug("Hide Busy}");
}
