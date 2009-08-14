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
 

#include <stdio.h>
#include <stdlib.h>

// this method is recommended for FreeType >2.0.x:
#include <ft2build.h>
#include FT_FREETYPE_H

#include <config.h>

#include <global.h>

#include "fontrenderer.h"

#include "system/debug.h"


FT_Error fontRenderClass::myFTC_Face_Requester(FTC_FaceID  face_id,
        FT_Library  library,
        FT_Pointer  request_data,
        FT_Face*    aface)
{
	return ((fontRenderClass*)request_data)->FTC_Face_Requester(face_id, aface);
}


fontRenderClass::fontRenderClass()
{
	dprintf(DEBUG_DEBUG, "[FONT] initializing core...\n");
	if (FT_Init_FreeType(&library))
	{
		dprintf(DEBUG_NORMAL, "[FONT] initializing core failed.\n");
		return;
	}
	dprintf(DEBUG_DEBUG, "[FONT] loading fonts...\n");
	fflush(stdout);
	font=0;

	int maxbytes= 4 *1024*1024;
	dprintf(DEBUG_INFO, "[FONT] Intializing font cache, using max. %dMB...", maxbytes/1024/1024);
	fflush(stdout);
	if (FTC_Manager_New(library, 10, 20, maxbytes, myFTC_Face_Requester, this, &cacheManager))
	{
		dprintf(DEBUG_NORMAL, "[FONT] manager failed!\n");
		return;
	}
	if (!cacheManager)
	{
		dprintf(DEBUG_NORMAL, "[FONT] error.\n");
		return;
	}
	if (FTC_SBit_Cache_New(cacheManager, &sbitsCache))
	{
		dprintf(DEBUG_NORMAL, "[FONT] sbit failed!\n");
		return;
	}
/*	if (FTC_ImageCache_New(cacheManager, &imageCache))
	{
		printf(" imagecache failed!\n");
	}
*/
	pthread_mutex_init( &render_mutex, NULL );
}

fontRenderClass::~fontRenderClass()
{
	FTC_Manager_Done(cacheManager);
	FT_Done_FreeType(library);
}

FT_Error fontRenderClass::FTC_Face_Requester(FTC_FaceID face_id, FT_Face* aface)
{
	fontListEntry *font=(fontListEntry *)face_id;
	if (!font)
		return -1;
	dprintf(DEBUG_DEBUG, "[FONT] FTC_Face_Requester (%s/%s)\n", font->family, font->style);

	int error;
	if ((error=FT_New_Face(library, font->filename, 0, aface)))
	{
		dprintf(DEBUG_NORMAL, "[FONT] FTC_Face_Requester (%s/%s) failed: %i\n", font->family, font->style, error);
		return error;
	}
	return 0;
}

FTC_FaceID fontRenderClass::getFaceID(const char *family, const char *style)
{
	for (fontListEntry *f=font; f; f=f->next)
	{
		if ((!strcmp(f->family, family)) && (!strcmp(f->style, style)))
			return (FTC_FaceID)f;
	}
	return 0;
}

FT_Error fontRenderClass::getGlyphBitmap(FTC_Image_Desc *font, FT_ULong glyph_index, FTC_SBit *sbit)
{
	return FTC_SBit_Cache_Lookup(sbitsCache, font, glyph_index, sbit);
}

int fontRenderClass::AddFont(const char *filename)
{
	fflush(stdout);
	int error;
	fontListEntry *n=new fontListEntry;

	FT_Face face;
	if ((error=FT_New_Face(library, filename, 0, &face)))
	{
		dprintf(DEBUG_NORMAL, "[FONT] adding font %s, failed: %i\n", filename, error);
		return error;
	}
	strcpy(n->filename=new char[strlen(filename)], filename);
	strcpy(n->family=new char[strlen(filename)], face->family_name);
	strcpy(n->style=new char[strlen(filename)], face->style_name);
	FT_Done_Face(face);

	n->next=font;
	dprintf(DEBUG_DEBUG, "[FONT] adding font %s... ok\n", filename);
	font=n;
	return 0;
}

fontRenderClass::fontListEntry::~fontListEntry()
{
	delete[] filename;
	delete[] family;
	delete[] style;
}

Font *fontRenderClass::getFont(const char *family, const char *style, int size)
{
	FTC_FaceID id=getFaceID(family, style);
	if (!id)
		return 0;
	return new Font(this, id, size);
}

Font::Font(fontRenderClass *render, FTC_FaceID faceid, int isize)
{
	frameBuffer 		= CFrameBuffer::getInstance();
	renderer 		= render;
	font.font.face_id 	= faceid;
	font.font.pix_width  	= isize;
	font.font.pix_height 	= isize;
	font.image_type 	= ftc_image_grays;
	font.image_type 	|= ftc_image_flag_autohinted;

	if (FTC_Manager_Lookup_Size(renderer->cacheManager, &font.font, &face, &size)<0)
	{
		dprintf(DEBUG_NORMAL, "FTC_Manager_Lookup_Size failed!\n");
		return;
	}
	// hack begin (this is a hack to get correct font metrics, didn't find any other way which gave correct values)
	FTC_SBit glyph;
	int index;

	index=FT_Get_Char_Index(face, 'M'); // "M" gives us ascender
	getGlyphBitmap(index, &glyph);
	int tM=glyph->top;

	index=FT_Get_Char_Index(face, 'g'); // "g" gives us descender
	getGlyphBitmap(index, &glyph);
	int hg=glyph->height;
	int tg=glyph->top;

	ascender=tM;
	descender=tg-hg; //this is a negative value!
	int halflinegap= -(descender>>1); // |descender/2| - we use descender as linegap, half at top, half at bottom
	upper = halflinegap+ascender+3;   // we add 3 at top
	lower = -descender+halflinegap+1; // we add 1 at bottom
	height=upper+lower;               // this is total height == distance of lines
	// hack end

	//printf("glyph: hM=%d tM=%d hg=%d tg=%d ascender=%d descender=%d height=%d linegap/2=%d upper=%d lower=%d\n",
	//       hM,tM,hg,tg,ascender,descender,height,halflinegap,upper,lower);
	//printf("font metrics: height=%ld\n", (size->metrics.height+32) >> 6);
}

FT_Error Font::getGlyphBitmap(FT_ULong glyph_index, FTC_SBit *sbit)
{
	return renderer->getGlyphBitmap(&font, glyph_index, sbit);
}

int Font::getHeight(void)
{
	return height;
}

void Font::RenderString(int x, int y, int width, const char *text, unsigned char color, int boxheight, const bool utf8_encoded)
{
	if (!frameBuffer->getActive())
		return;

	pthread_mutex_lock( &renderer->render_mutex );

	if (FTC_Manager_Lookup_Size(renderer->cacheManager, &font.font, &face, &size)<0)
	{
		dprintf(DEBUG_NORMAL, "FTC_Manager_Lookup_Size failed!\n");
		return;
	}

	int use_kerning=FT_HAS_KERNING(face);

	int left=x;
	int step_y=height;

	// ----------------------------------- box upper end (this is NOT a font metric, this is our method for y centering)
	//
	// **  -------------------------------- y=baseline-upper
	// ||
	// |u  --------------------*----------- y=baseline+ascender
	// |p                     * *
	// hp                    *   *
	// ee     *        *    *     *
	// ir      *      *     *******
	// g|       *    *      *     *
	// h|        *  *       *     *
	// t*  -------**--------*-----*-------- y=baseline
	// |l         *
	// |o        *
	// |w  -----**------------------------- y=baseline+descender   // descender is a NEGATIVE value
	// |r
	// **  -------------------------------- y=baseline+lower == YCALLER
	//
	// ----------------------------------- box lower end (this is NOT a font metric, this is our method for y centering)

	// height = ascender + -1*descender + linegap           // descender is negative!

	// now we adjust the given y value (which is the line marked as YCALLER) to be the baseline after adjustment:
	y -= lower;
	// y coordinate now gives font baseline which is used for drawing

	// caution: this only works if we print a single line of text
	// if we have multiple lines, don't use boxheight or specify boxheight==0.
	// if boxheight is !=0, we further adjust y, so that text is y-centered in the box
	if(boxheight)
	{
		if(boxheight>step_y)			// this is a real box (bigger than text)
			y -= ((boxheight-step_y)>>1);
		else if(boxheight<0)			// this normally would be wrong, so we just use it to define a "border"
			y += (boxheight>>1);		// half of border value at lower end, half at upper end
	}

	int lastindex=0; // 0 == missing glyph (never has kerning values)
	FT_Vector kerning;
	int pen1=-1; // "pen" positions for kerning, pen2 is "x"

	int coff=(color+ 2)%8;
	unsigned char colors[256];

	for (int i= 0; i< 8; i++ )
	{
		int c = i- coff; // c = 0..7
		if (c< 0)
			c= 0;
		c= color + c;
		for (int j= i*32; j< (i+1)*32; j++)
			colors[j]= c;
	}

	for (; *text; text++)
	{
		FTC_SBit glyph;
		if (*text=='\n')
		{
			x  = left;
			y += step_y;
		}

		int unicode_value;

		if (utf8_encoded && ((((unsigned char)(*text)) & 0x80) != 0))
		{
			int remaining_unicode_length;
			if ((((unsigned char)(*text)) & 0xf8) == 0xf0)
			{
				unicode_value = ((unsigned char)(*text)) & 0x07;
				remaining_unicode_length = 3;
			}
			else if ((((unsigned char)(*text)) & 0xf0) == 0xe0)
			{
				unicode_value = ((unsigned char)(*text)) & 0x0f;
				remaining_unicode_length = 2;
			}
			else if ((((unsigned char)(*text)) & 0xe0) == 0xc0)
			{
				unicode_value = ((unsigned char)(*text)) & 0x1f;
				remaining_unicode_length = 1;
			}
			else                     // cf.: http://www.cl.cam.ac.uk/~mgk25/unicode.html#utf-8
				break;           // corrupted character or a character with > 4 bytes utf-8 representation

			for (int i = 0; i < remaining_unicode_length; i++)
			{
				text++;
				if (((*text) & 0xc0) != 0x80)
				{
					remaining_unicode_length = -1;
					break;           // incomplete or corrupted character
				}
				unicode_value <<= 6;
				unicode_value |= ((unsigned char)(*text)) & 0x3f;
			}
			if (remaining_unicode_length == -1)
				break;           // incomplete or corrupted character
		}
		else
		    unicode_value = (unsigned char)(*text);
		    
		int index = FT_Get_Char_Index(face, unicode_value);

		if (!index)
			continue;
		if (getGlyphBitmap(index, &glyph))
		{
			dprintf(DEBUG_NORMAL, "failed to get glyph bitmap.\n");
			continue;
		}

		// width clip
		if(x+glyph->xadvance > left+width)
		{
			pthread_mutex_unlock( &renderer->render_mutex );
			return;
		}

		//kerning
		if(use_kerning)
		{
			FT_Get_Kerning(face,lastindex,index,0,&kerning);
			x+=(kerning.x+32)>>6; // kerning!
		}

		int rx=x+glyph->left;
		int ry=y-glyph->top;

		__u8 *d=frameBuffer->getFrameBufferPointer() + frameBuffer->getStride()*ry + rx;
		__u8 *s=glyph->buffer;



		int w =	glyph->width;
		int h =	glyph->height;
		int stride= frameBuffer->getStride();
		int pitch = glyph->pitch;
		for (int ay=0; ay<h; ay++)
		{
			__u8 *td=d;

			int ax;
			for (ax=0; ax<w; ax++)
			{
/*				int c = (*s++>>5)- coff; // c = 0..7
				if (c< 0)
					c= 0;
				*td++=color + c;   // we use color as "base color" plus 7 consecutive colors for anti-aliasing
*/
				*td++= colors[*s++];
			}
			s+= pitch- ax;
			d+= stride;
		}
		x+=glyph->xadvance+1;
		if(pen1>x)
			x=pen1;
		pen1=x;
		lastindex=index;
	}

    //printf("RenderStat: %d %d %d \n", renderer->cacheManager->num_nodes, renderer->cacheManager->num_bytes, renderer->cacheManager->max_bytes);
	pthread_mutex_unlock( &renderer->render_mutex );
}

void Font::RenderString(int x, int y, int width, string text, unsigned char color, int boxheight)
{
	RenderString( x, y, width, text.c_str(), color, boxheight);
}

int Font::getRenderWidth(const char *text)
{
	pthread_mutex_lock( &renderer->render_mutex );

	int use_kerning=FT_HAS_KERNING(face);
	if (FTC_Manager_Lookup_Size(renderer->cacheManager, &font.font, &face, &size)<0)
	{
		dprintf(DEBUG_NORMAL, "FTC_Manager_Lookup_Size failed!\n");
		return -1;
	}

	int x=0;
	int lastindex=0; // 0==missing glyph (never has kerning)
	FT_Vector kerning;
	int pen1=-1; // "pen" positions for kerning, pen2 is "x"
	for (; *text; text++)
	{
		FTC_SBit glyph;

		int index=FT_Get_Char_Index(face, *text);
		if (!index)
			continue;
		if (getGlyphBitmap(index, &glyph))
		{
			dprintf(DEBUG_NORMAL, "failed to get glyph bitmap.\n");
			continue;
		}
		//kerning
		if(use_kerning)
		{
			FT_Get_Kerning(face,lastindex,index,0,&kerning);
			x+=(kerning.x+32)>>6; // kerning!
		}

		x+=glyph->xadvance+1;
		if(pen1>x)
			x=pen1;
		pen1=x;
		lastindex=index;
	}
	pthread_mutex_unlock( &renderer->render_mutex );

	return x;
}

int Font::getRenderWidth(string text)
{
	getRenderWidth( text.c_str() );
	return 0;
}
