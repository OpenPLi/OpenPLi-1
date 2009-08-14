/*
	Neutrino-GUI  -   DBoxII-Project

	Copyright (C) 2001 Steffen Hehn 'McClean'
	Homepage: http://dbox.cyberphoria.org/

	Kommentar:

	Diese GUI wurde von Grund auf neu programmiert und sollte nun vom
	Aufbau und auch den Ausbaumoeglichkeiten gut aussehen. Neutrino basiert
	auf der Client-Server Idee, diese GUI ist also von der direkten DBox-
	Steuerung getrennt. Diese wird dann von Daemons uebernommen.


	License: GPL

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __FONTRENDERER__
#define __FONTRENDERER__

#include <pthread.h>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H

#include FT_CACHE_IMAGE_H
#include FT_CACHE_SMALL_BITMAPS_H

#include "framebuffer.h"


class Font;
class fontRenderClass
{
		struct fontListEntry
		{
			char *filename, *style, *family;
			fontListEntry *next;
			~fontListEntry();
		}
		*font;

		FT_Library	library;
		FTC_Manager	cacheManager;	/* the cache manager               */
		FTC_Image_Cache	imageCache;	/* the glyph image cache           */
		FTC_SBit_Cache	sbitsCache;	/* the glyph small bitmaps cache   */

		FTC_FaceID getFaceID(const char *family, const char *style);
		FT_Error getGlyphBitmap(FTC_Image_Desc *font, FT_ULong glyph_index, FTC_SBit *sbit);

	public:
		pthread_mutex_t     render_mutex;

		FT_Error FTC_Face_Requester(FTC_FaceID face_id, FT_Face* aface);


		static FT_Error myFTC_Face_Requester(FTC_FaceID  face_id,
		                                     FT_Library  library,
		                                     FT_Pointer  request_data,
		                                     FT_Face*    aface);

		//FT_Face getFace(const char *family, const char *style);
		Font *getFont(const char *family, const char *style, int size);

		int AddFont(const char *filename);

		fontRenderClass();
		~fontRenderClass();

		friend class Font;
};

class Font
{
		CFrameBuffer	*frameBuffer;
		FTC_Image_Desc	font;
		fontRenderClass *renderer;
		FT_Face		face;
		FT_Size		size;

		FT_Error getGlyphBitmap(FT_ULong glyph_index, FTC_SBit *sbit);

		// these are HACKED values, because the font metrics were unusable.
		int height,ascender,descender,upper,lower;

	public:
		void RenderString(int x, int y, int width, const char *text, unsigned char color, int boxheight=0, const bool utf8_encoded = false);
		void RenderString(int x, int y, int width, string text, unsigned char color, int boxheight=0);

		int getRenderWidth(const char *text);
		int getRenderWidth(string text);
		int getHeight(void);

		Font(fontRenderClass *render, FTC_FaceID faceid, int isize);
		~Font(){}
};

class FontsDef
{
	public:
		Font
		*menu,
		*menu_title,
		*menu_info,
		*epg_title,
		*epg_info1, // epg_info1 should be same size as info2, but italic!
		*epg_info2,
		*epg_date,

		*eventlist_title,
		*eventlist_itemLarge,
		*eventlist_datetime,
		*eventlist_itemSmall,

		*gamelist_itemLarge,
		*gamelist_itemSmall,

		*alert,
		*channellist,
		*channellist_descr,
		*channellist_number,
		*channel_num_zap,

		*infobar_number,
		*infobar_channame,
		*infobar_info,
		*infobar_small;
};


#endif
