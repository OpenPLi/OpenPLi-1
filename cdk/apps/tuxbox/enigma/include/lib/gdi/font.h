#ifndef __FONT_H
#define __FONT_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_IMAGE_H
#include FT_CACHE_SMALL_BITMAPS_H
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
typedef FTC_ImageCache FTC_Image_Cache;
typedef FTC_ImageTypeRec FTC_Image_Desc;
typedef FTC_SBitCache FTC_SBit_Cache;
#endif
#include <vector>


#include <lib/gdi/fb.h>
#include <lib/base/esize.h>
#include <lib/base/epoint.h>
#include <lib/base/erect.h>
#include <lib/base/estring.h>

class FontRenderClass;
class Font;
class gPixmapDC;
class gFont;
class gRGB;

class fontRenderClass
{ 
	friend class Font;
	friend class eTextPara;
	// fbClass *fb;
	struct fontListEntry
	{
		eString filename, face;
		int scale; // 100 is 1:1
		fontListEntry *next;
		~fontListEntry();
	} *font;

	FT_Library library;
	FTC_Manager			cacheManager;				/* the cache manager							 */
	FTC_Image_Cache	imageCache;					/* the glyph image cache					 */
	FTC_SBit_Cache	 sbitsCache;				/* the glyph small bitmaps cache	 */

	FTC_FaceID getFaceID(const eString &face);
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	FT_Error getGlyphBitmap(FTC_Image_Desc *font, FT_UInt glyph_index, FTC_SBit *sbit);
#else
	FT_Error getGlyphBitmap(FTC_Image_Desc *font, FT_ULong glyph_index, FTC_SBit *sbit);
#endif
	static fontRenderClass *instance;
	void init_fontRenderClass();
public:
	float getLineHeight(const gFont& font);
	eString AddFont(const eString &filename, const eString &name, int scale);
	static fontRenderClass *getInstance();
	FT_Error FTC_Face_Requester(FTC_FaceID	face_id,
															FT_Face*		aface);
	Font *getFont(const eString &face, int size, int tabwidth=-1);
	fontRenderClass();
	~fontRenderClass();
};

#define RS_WRAP		1
#define RS_DOT		2
#define RS_DIRECT	4
#define RS_FADE		8

#define GS_ISSPACE  1
#define GS_ISFIRST  2
#define GS_USED			4

struct pGlyph
{
	int x, y, w;
	Font *font;
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	FT_UInt glyph_index;
#else
	FT_ULong glyph_index;
#endif
	int flags;
	eRect bbox;
};

typedef std::vector<pGlyph> glyphString;

class Font;
class eLCD;

class eTextPara
{
	Font *current_font, *replacement_font;
	FT_Face current_face, replacement_face;
	int use_kerning;
	int previous;
	static eString replacement_facename;

	eRect area;
	ePoint cursor;
	eSize maximum;
	int left;
	glyphString glyphs;
	int refcnt;
	eRect boundBox;
	int bboxValid;

	int appendGlyph(Font *current_font, FT_Face current_face, FT_UInt glyphIndex, int flags, int rflags);
	void newLine(int flags);
	void setFont(Font *font, Font *replacement_font);
	void calc_bbox();
	void clear();
public:
	eTextPara(eRect area, ePoint start=ePoint(-1, -1))
		: current_font(0), replacement_font(0), current_face(0), replacement_face(0),
			area(area), cursor(start), maximum(0, 0), left(start.x()), refcnt(0), bboxValid(0)
	{
	}
	~eTextPara();
	
	static void setReplacementFont(eString font) { replacement_facename=font; }

	void destroy();
	eTextPara *grab();

	void setFont(const gFont &font);
	int renderString(const eString &string, int flags=0);

	void blit(gPixmapDC &dc, const ePoint &offset, const gRGB &background, const gRGB &foreground);

	enum
	{
		dirLeft, dirRight, dirCenter, dirBlock
	};

	void realign(int dir);

	const eRect & getBoundBox()
	{
		if (!bboxValid)
			calc_bbox();
		return boundBox;
	}

	const eRect& getGlyphBBox(int num) const
	{
		return glyphs[num].bbox;
	}
};

class Font
{
public:
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	FTC_ScalerRec scaler;
#endif
	FTC_Image_Desc font;
	fontRenderClass *renderer;
#if ((defined(FREETYPE_MAJOR)) && (((FREETYPE_MAJOR == 2) && (((FREETYPE_MINOR == 1) && (FREETYPE_PATCH >= 9)) || (FREETYPE_MINOR > 1))) || (FREETYPE_MAJOR > 2)))
	FT_Error getGlyphBitmap(FT_UInt glyph_index, FTC_SBit *sbit);
#else
	FT_Error getGlyphBitmap(FT_ULong glyph_index, FTC_SBit *sbit);
#endif
	int ref;
	FT_Face face;
	FT_Size size;
	
	int tabwidth;
	int height;
	Font(fontRenderClass *render, FTC_FaceID faceid, int isize, int tabwidth);
	~Font();
	
	void lock();
	void unlock();	// deletes if ref==0
};

extern fontRenderClass *font;

#endif
