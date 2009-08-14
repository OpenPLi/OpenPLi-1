#ifndef __my_lcd__
#define __my_lcd__

#include <dbox/lcd-ks0713.h>
#include <string>

#define USEFREETYPELCD
#define LCD_DEV "/dev/dbox/lcd0"


#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H
#define FONT "/share/fonts/pakenham.ttf"


typedef unsigned char raw_display_t[LCD_ROWS*8][LCD_COLS];

class CLCDDisplay
{
 #ifdef USEFREETYPELCD
	FT_Library		library;
	FTC_Manager		manager;
	FTC_SBitCache		cache;
	FTC_SBit		sbit;
#if FREETYPE_MAJOR  == 2 && FREETYPE_MINOR == 0
	FTC_ImageDesc		desc;
#else
	FTC_ImageTypeRec	desc;
#endif
	FT_Face			face;
	FT_UInt			prev_glyphindex;
	int use_kerning;
	int RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int state);
	int GetStringLen(unsigned char *string);
#endif
	raw_display_t raw;
	unsigned char lcd[LCD_ROWS][LCD_COLS];
	int fd;
	bool available;
	void draw_point(const int x, const int y, const int state);
	int invalid_col(int x);
	int invalid_row(int y);
	void convert_data();

 public:
	enum {PIXEL_ON  = LCD_PIXEL_ON, PIXEL_OFF = LCD_PIXEL_OFF, PIXEL_INV = LCD_PIXEL_INV };
	enum {LEFT, CENTER, RIGHT};

	CLCDDisplay();
	~CLCDDisplay();

#ifdef USEFREETYPELCD
	FT_Error FTC_Face_Requester(FTC_FaceID face_id, FT_Face* aface);
	void RenderString(std::string word, int sx, int sy, int maxwidth, int layout, int size, int state);
#else
	void draw_char(int x, int y, char c);
	void draw_string(int x, int y, const char *string);
#endif

	void clear();
	void update();
	void draw_line(const int x1, const int y1, const int x2, const int y2, int state);
	void draw_fill_rect (int left,int top,int right,int bottom, int state);
	bool load_png(const char * const filename);
};

#endif
