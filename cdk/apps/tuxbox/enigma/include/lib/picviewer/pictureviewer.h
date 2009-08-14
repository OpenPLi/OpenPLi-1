/*
 * $Id: pictureviewer.h,v 1.22 2009/02/03 18:52:08 dbluelle Exp $
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
 
#ifndef __pictureviewer_h
#define __pictureviewer_h

#include <lib/base/estring.h>
#include <lib/gui/ewidget.h>
#include <src/enigma_main.h>

#define FH_ERROR_OK 0
#define FH_ERROR_FILE 1		/* read/access error */
#define FH_ERROR_FORMAT 2	/* file format error */
#define FH_ERROR_MALLOC 3	/* error during malloc */

#define dbout(fmt, args...) {struct timeval tv; gettimeofday(&tv, NULL); \
        printf( "PV[%ld|%02ld] " fmt, (long)tv.tv_sec, (long)tv.tv_usec / 10000, ## args);}

class ePictureViewer: public eWidget
{
	eTimer slideshowTimer;
	std::list<eString> slideshowList;
	std::list<eString>::iterator myIt;
	struct cformathandler 
	{
		struct cformathandler *next;
		int (*get_size)(const char *, int *, int *);
		int (*get_pic)(const char *, unsigned char *, int, int);
		int (*id_pic)(const char *);
		int fmt;
	};
	typedef  struct cformathandler CFormathandler;
	eString filename;
	int eventHandler(const eWidgetEvent &evt);
	void listDirectory(eString, int);
	void nextPicture();
	void previousPicture();
	void showNameOnLCD(const eString& filename);
	bool DecodeImage(const std::string& name);
	bool ProcessImage(bool unscaled = false, int rotate = 0);
	bool Move(int dx, int dy);
	bool DisplayNextImage();
	bool showBusySign;
	bool switchto43;
	void init_ePictureViewer();
	int format169;
	unsigned int want_bpp, prev_bpp;
#ifndef DISABLE_LCD
	eZapLCD* pLCD;
#endif
public:
	ePictureViewer(const eString &filename);
	~ePictureViewer();

	enum ScalingMode
	{
		NONE = 0,
		SIMPLE = 1,
		COLOR = 2
	};

	bool ShowImage(const std::string& filename, bool unscaled = false);
	void SetScaling(ScalingMode s) {m_scaling = s;}
	void SetAspectRatio(float aspect_ratio) {m_aspect = aspect_ratio;}
	void showBusy(int sx, int sy, int width, char r, char g, char b);
	void hideBusy();
//	void Zoom(float factor);
//	void Move(int dx, int dy);
	void slideshowTimeout();
	eString GetCurrentFile() { return *myIt; }
private:
	CFormathandler *fh_root;
	ScalingMode m_scaling;
	float m_aspect;
	std::string m_NextPic_Name;
	unsigned char *m_NextPic_Buffer;
	int m_NextPic_X;
	int m_NextPic_Y;

	unsigned char *m_NextPic_Image;
	int m_Img_X;
	int m_Img_Y;
	int m_Img_XPos;
	int m_Img_YPos;
	int m_Img_XPan;
	int m_Img_YPan;

	unsigned char *m_busy_buffer;
	int m_busy_x;
	int m_busy_y;
	int m_busy_width;
	int m_busy_cpp;

	int m_startx;
	int m_starty;
	int m_endx;
	int m_endy;
	
	enum { FMT_GIF, FMT_JPG, FMT_PNG, FMT_BMP, FMT_CRW };
	CFormathandler * fh_getsize(const char *name, int *x, int *y);
	void init_handlers(void);
	void add_format(int (*picsize)(const char *, int *, int *), int (*picread)(const char *, unsigned char *, int , int), int (*id)(const char *), int fmt);
};
#endif
