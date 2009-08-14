#ifndef __gpixmap_h
#define __gpixmap_h

#include <pthread.h>
#include <lib/base/estring.h>
#include <lib/base/erect.h>
#include <lib/gdi/fb.h>
#include <lib/system/elock.h>

struct gColor
{
	int color;
	gColor(int color): color(color)
	{
	}
	gColor(): color(0)
	{
	}
	operator int() const { return color; }
	bool operator==(const gColor &o) const { return o.color==color; }
};

struct gRGB
{
	int b, g, r, a;
	gRGB(int r, int g, int b, int a=0): b(b), g(g), r(r), a(a)
	{
	}
	gRGB(unsigned long val): b(val&0xFF), g((val>>8)&0xFF), r((val>>16)&0xFF), a((val>>24)&0xFF)		// ARGB
	{
	}
	gRGB()
	{
	}
	bool operator < (const gRGB &c) const
	{
		if (b < c.b)
			return 1;
		if (b == c.b)
		{
			if (g < c.g)
				return 1;
			if (g == c.g)
			{
				if (r < c.r)
					return 1;
				if (r == c.r)
					return a < c.a;
			}
		}
		return 0;
	}
	bool operator==(const gRGB &c) const
	{
		return (b == c.b) && (g == c.g) && (r == c.r) && (a == c.a);
	}
};

struct gPalette
{
	int start, colors;
	gRGB *data;
	gColor findColor(const gRGB &rgb) const;
};

struct gLookup
{
	int size;
	gColor *lookup;
	gLookup(int size, const gPalette &pal, const gRGB &start, const gRGB &end);
	gLookup();
	~gLookup() { delete [] lookup; }
	void build(int size, const gPalette &pal, const gRGB &start, const gRGB &end);
};

/**
 * \brief A softreference to a font.
 *
 * The font is specified by a name and a size.
 * \c gFont is part of the \ref gdi.
 */
struct gFont
{
	eString family;
	int pointSize;
	
	/**
	 * \brief Constructs a font with the given name and size.
	 * \param family The name of the font, for example "NimbusSansL-Regular Sans L Regular".
	 * \param pointSize the size of the font in PIXELS.
	 */
	gFont(const eString &family, int pointSize):
			family(family), pointSize(pointSize)
	{
	}
	
	gFont()
		:pointSize(0)
	{
	}
};


struct gPixmap
{
	int x, y, bpp, bypp, stride;
	void *data;
	
	gPalette clut;
	
	eLock contentlock;
	int final;
	bool cancompress;
	int compressedsize;

	void compressdata();
	void uncompressdata();
	__u8* uncompressdatanoreplace() const;
	
	gPixmap *lock();
	void unlock();
	
	eSize getSize() const { return eSize(x, y); }
	
	void fill(const eRect &area, const gColor &color, int round=0);
	
	enum
	{
		blitAlphaTest=1
	};
	void blit(const gPixmap &src, ePoint pos, const eRect &clip=eRect(), int flags=0);
	void resize(eSize size);
	void mergePalette(const gPixmap &target);
	void line(ePoint start, ePoint end, gColor color);
	void finalLock();
	gPixmap();
	virtual ~gPixmap();
};

struct gImage: gPixmap
{
	gImage(eSize size, int bpp);
	~gImage();
};

#endif
