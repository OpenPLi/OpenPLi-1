/* 
TuxTerm v0.2

Written 10/2006 by Seddi
Contact: seddi@ihad.tv / http://www.ihad.tv
*/

#ifdef HAVE_CONFIG_H
        #include "config.h"
#endif

#include "render.h"

// Freetype - MyFaceRequester
FT_Error MyFaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	FT_Error result;

	result = FT_New_Face(library, face_id, 0, aface);

	if(!result) printf("Freetype <Schrift \"%s\" geladen>\n", (char*)face_id);
	else        printf("Freetype <Schrift \"%s\" fehlgeschlagen>\n", (char*)face_id);

	return result;
}

// Freetype - RenderChar
int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, unsigned char *color)
{
	if (currentchar == 32) return;

#ifdef FB8BIT
		int bpp=1;
#else
		int bpp=4;
#endif		
		
	int row, pitch, bit, x = 0, y = 0;
	FT_UInt glyphindex;
	FT_Vector kerning;
	FT_Error error;
	int tmpcolor;

	if(!(glyphindex = FT_Get_Char_Index(face, currentchar)))
	{
		printf("Freetype <FT_Get_Char_Index> fuer Zeichen %x \"%c\" fehlgeschlagen\n", (int)currentchar,(int)currentchar);
		return 0;
	}

#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	FTC_Node anode;
	if((error = FTC_SBitCache_Lookup(cache, &desc, glyphindex, &sbit, &anode)))
#else
	if((error = FTC_SBit_Cache_Lookup(cache, &desc, glyphindex, &sbit)))
#endif
	{
		printf("Freetype <FTC_SBitCache_Lookup> fuer Zeichen %x \"%c\" fehlgeschlagen. Fehler: 0x%.2X>\n", (int)currentchar,(int)currentchar, error);
		return 0;
	}
	if(use_kerning)
	{
		FT_Get_Kerning(face, prev_glyphindex, glyphindex, ft_kerning_default, &kerning);

		prev_glyphindex = glyphindex;
		kerning.x >>= 6;
	}
	else
		kerning.x = 0;

//	if(color != -1)
//	{
		if(sx + sbit->xadvance >= ex) return -1;
		for(row = 0; row < sbit->height; row++)
		{
			for(pitch = 0; pitch < sbit->pitch; pitch++)
			{
				for(bit = 7; bit >= 0; bit--)
				{
					if(pitch*8 + 7-bit >= sbit->width) break; /* render needed bits only */
					if((sbit->buffer[row * sbit->pitch + pitch]) & 1<<bit) {
#ifdef FB8BIT
					memset(lfb + StartX*bpp + sx*bpp + (sbit->left + kerning.x + x)*bpp + fix_screeninfo.line_length*(StartY + sy - sbit->top + y), (int)color[bpp], bpp);
#else
					memcpy(lfb + StartX*bpp + sx*bpp + (sbit->left + kerning.x + x)*bpp + fix_screeninfo.line_length*(StartY + sy - sbit->top + y), color, bpp);
#endif
					}
					x++;
				}
			}
			x = 0;
			y++;
		}
//	}

		return sbit->xadvance + kerning.x;
}

// Freetype - Stringlaenge
int GetStringLen(const char *string, int size)
{
	int stringlen = 0;

	switch (size)
	{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	case VERY_SMALL: desc.width = desc.height = FONTHEIGHT_VERY_SMALL; break;
	case SMALL     : desc.width = desc.height = FONTHEIGHT_SMALL     ; break;
	case BIG       : desc.width = desc.height = FONTHEIGHT_BIG       ; break;
#else
	case VERY_SMALL: desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_VERY_SMALL; break;
	case SMALL     : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_SMALL     ; break;
	case BIG       : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_BIG       ; break;
#endif
	}
	prev_glyphindex = 0;
	while(*string != '\0')
	{
		stringlen += RenderChar(*string, -1, -1, -1, "");
		string++;
	}

	return stringlen;
}

// Freetype - Render String
void RenderString(const char *string, int sx, int sy, int maxwidth, int layout, int size, unsigned char *color)
{
	int stringlen, ex, charwidth;
	switch (size)
	{
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	case VERY_SMALL: desc.width = desc.height = FONTHEIGHT_VERY_SMALL; break;
	case SMALL     : desc.width = desc.height = FONTHEIGHT_SMALL     ; break;
	case BIG       : desc.width = desc.height = FONTHEIGHT_BIG       ; break;
#else
	case VERY_SMALL: desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_VERY_SMALL; break;
	case SMALL     : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_SMALL     ; break;
	case BIG       : desc.font.pix_width = desc.font.pix_height = FONTHEIGHT_BIG       ; break;
#endif
	}
	if(layout != LEFT)
	{
		stringlen = GetStringLen(string, size);
		switch(layout)
		{
			case CENTER:	if(stringlen < maxwidth) sx += (maxwidth - stringlen)/2;
					break;
			case RIGHT:	if(stringlen < maxwidth) sx += maxwidth - stringlen;
		}
	}
	prev_glyphindex = 0;
	ex = sx + maxwidth;
	while(*string != '\0' && *string != '\n')
	{
		if((charwidth = RenderChar(*string, sx, sy, ex, color)) == -1) return; /* string > maxwidth */
		sx += charwidth;
		string++;
	}
}

// Render Box
void RenderBox(int sx, int sy, int ex, int ey, int mode, unsigned char *color)
{

#ifdef FB8BIT
		int bpp=1;
#else
		int bpp=4;
#endif		

	
	int loop;
	int tx;
	if(mode == FILL)
	{
		for(; sy <= ey; sy++)
		{
#ifdef FB8BIT
			memset(lfb + StartX*bpp + sx*bpp + fix_screeninfo.line_length*(StartY + sy),(int)color[bpp],bpp*(ex-sx+1));
#else
			memcpy(lfb + StartX*bpp + sx*bpp + fix_screeninfo.line_length*(StartY + sy),colorline[color[bpp]],bpp*(ex-sx+1));
#endif
//			for(tx=0; tx <= (ex-sx); tx++)	
//			{
//			memcpy(lfb + StartX*bpp + sx*bpp + (tx*bpp) + fix_screeninfo.line_length*(StartY + sy),color,bpp);
//			}
		}
	}
	else
	{
		for(loop = sx; loop <= ex; loop++)
		{
			memcpy(lfb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(sy+StartY), color, bpp);
			memcpy(lfb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(sy+1+StartY), color, bpp);
			memcpy(lfb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(ey-1+StartY), color, bpp);
			memcpy(lfb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(ey+StartY), color, bpp);
		}
		for(loop = sy; loop <= ey; loop++)
		{
			memcpy(lfb + StartX*bpp+sx*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
			memcpy(lfb + StartX*bpp+(sx+1)*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
			memcpy(lfb + StartX*bpp+(ex-1)*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
			memcpy(lfb + StartX*bpp+ex*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
		}
	}
}

// Render Box BB
void RenderBoxBB(int sx, int sy, int ex, int ey, int mode, unsigned char *color)
{

#ifdef FB8BIT
		int bpp=1;
#else
		int bpp=4;
#endif		
	
	int loop;
	int tx;
	if(mode == FILL)
	{
		for(; sy <= ey; sy++)
		{
#ifdef FB8BIT
			memset(lbb + StartX*bpp + sx*bpp + fix_screeninfo.line_length*(StartY + sy),(int)color[bpp],bpp*(ex-sx+1));
#else
			memcpy(lbb + StartX*bpp + sx*bpp + fix_screeninfo.line_length*(StartY + sy),colorline[color[bpp]],bpp*(ex-sx+1));
#endif
//			for(tx=0; tx <= (ex-sx); tx++)	
//			{
//			memcpy(lbb + StartX*bpp + sx*bpp + (tx*bpp) + fix_screeninfo.line_length*(StartY + sy),color,bpp);
//			}
		}
	}
	else
	{
		for(loop = sx; loop <= ex; loop++)
		{
			memcpy(lbb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(sy+StartY), color, bpp);
			memcpy(lbb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(sy+1+StartY), color, bpp);
			memcpy(lbb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(ey-1+StartY), color, bpp);
			memcpy(lbb + StartX*bpp+loop*bpp + fix_screeninfo.line_length*(ey+StartY), color, bpp);
		}
		for(loop = sy; loop <= ey; loop++)
		{
			memcpy(lbb + StartX*bpp+sx*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
			memcpy(lbb + StartX*bpp+(sx+1)*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
			memcpy(lbb + StartX*bpp+(ex-1)*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
			memcpy(lbb + StartX*bpp+ex*bpp + fix_screeninfo.line_length*(loop+StartY), color, bpp);
		}
	}
}
