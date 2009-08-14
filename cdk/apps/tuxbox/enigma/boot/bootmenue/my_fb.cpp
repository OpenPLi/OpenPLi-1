#ifdef HAVE_DREAMBOX_HARDWARE
#include "my_fb.h"

#define SAA_MODE_RGB    0
#define SAA_MODE_FBAS   1
#define SAA_MODE_SVIDEO 2
#define SAA_MODE_COMPONENT 3


inline static unsigned char make8color(unsigned char r, unsigned char g, unsigned char b)
{
    return ((((r >> 5) & 7) << 5) | (((g >> 5) & 7) << 2) | ((b >> 6) & 3));
}
inline static unsigned short make15color(unsigned char r, unsigned char g, unsigned char b)
{
    return ((((r >> 3) & 31) << 10) | (((g >> 3) & 31) << 5) | ((b >> 3) & 31));
}
inline static unsigned short make16color(unsigned char r, unsigned char g, unsigned char b)
{
    return ((((r >> 3) & 31) << 11) | (((g >> 2) & 63) << 5)  | ((b >> 3) & 31));
}

void* convertRGB2FB(int fh, unsigned char *rgbbuff, unsigned long count, int bpp, int *cpp)
{
	unsigned long i;
	void *fbbuff = NULL;
	u_int8_t  *c_fbbuff;
	u_int16_t *s_fbbuff;
	u_int32_t *i_fbbuff;

	switch(bpp)
	{
		case 8:
			*cpp = 1;
			c_fbbuff = (unsigned char *) malloc(count * sizeof(unsigned char));
			for(i = 0; i < count; i++)
				c_fbbuff[i] = make8color(rgbbuff[i*3], rgbbuff[i*3+1], rgbbuff[i*3+2]);
			fbbuff = (void *) c_fbbuff;
			break;
		case 15:
			*cpp = 2;
			s_fbbuff = (unsigned short *) malloc(count * sizeof(unsigned short));
			for(i = 0; i < count ; i++)
				s_fbbuff[i] = make15color(rgbbuff[i*3], rgbbuff[i*3+1], rgbbuff[i*3+2]);
			fbbuff = (void *) s_fbbuff;
			break;
		case 16:
			*cpp = 2;
			s_fbbuff = (unsigned short *) malloc(count * sizeof(unsigned short));
			for(i = 0; i < count ; i++)
				s_fbbuff[i] = make16color(rgbbuff[i*3], rgbbuff[i*3+1], rgbbuff[i*3+2]);
			fbbuff = (void *) s_fbbuff;
			break;
		case 24:
		case 32:
			*cpp = 4;
			i_fbbuff = (unsigned int *) malloc(count * sizeof(unsigned int));
			for(i = 0; i < count ; i++)
				i_fbbuff[i] = ((rgbbuff[i*3] << 16) & 0xFF0000) | ((rgbbuff[i*3+1] << 8) & 0xFF00) | (rgbbuff[i*3+2] & 0xFF);
			fbbuff = (void *) i_fbbuff;
			break;
		default:
			fprintf(stderr, "Unsupported video mode! You've got: %dbpp\n", bpp);
			exit(1);
	}
	return fbbuff;
}

#ifdef USEFREETYPEFB
FT_Error FB_FaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face *aface)
{
	return ((fbClass*)request_data)->FTC_Face_Requester(face_id, aface);
}

FT_Error fbClass::FTC_Face_Requester(FTC_FaceID	face_id, FT_Face* aface)
{
	int error;
	if ((error=FT_New_Face(library, FONT, 0, aface)))
	{
		printf(" failed: %s", strerror(error));
		return error;
	}
	FT_Select_Charmap(*aface, ft_encoding_unicode);
	return 0;
}
#endif

fbClass *fbClass::instance;

fbClass *fbClass::getInstance()
{
	return instance;
}

fbClass::fbClass()
{
	instance=this;
	int maxbytes=4*1024*1024;
	available=0;

	fd=open(FB_DEV, O_RDWR);
	if (fd<0)
	{
		perror(FB_DEV);
		goto nolfb;
	}
	if (ioctl(fd, FBIOGET_VSCREENINFO, &screeninfo)<0)
	{
		perror("FBIOGET_VSCREENINFO");
		goto nolfb;
	}

	memcpy(&oldscreen, &screeninfo, sizeof(screeninfo));

	fb_fix_screeninfo fix;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0)
	{
		perror("[Framebuffer] <FBIOGET_FSCREENINFO>");
		goto nolfb;
	}

	available=fix.smem_len;
	//printf("[Framebuffer] %dk video memory\n", available/1024);
	lfb=(unsigned char*)mmap(0, available, PROT_WRITE|PROT_READ, MAP_SHARED, fd, 0);
	if (!lfb)
	{
		perror("mmap");
		goto nolfb;
	}

#ifdef USEFREETYPEFB
	FT_Error error;

	if (error = FT_Init_FreeType(&library))
	{
		printf("[FONT] <FT_Init_FreeType failed with Errorcode 0x%.2X>", error);
		munmap(lfb, available);
	}
	if((error = FTC_Manager_New(library, 10, 20, maxbytes, FB_FaceRequester, this, &manager)))
	{
		printf("[FONT] <FTC_Manager_New failed with Errorcode 0x%.2X>\n", error);
		FT_Done_FreeType(library);
		munmap(lfb, available);
	}
	if((error = FTC_SBitCache_New(manager, &cache)))
	{
		printf("[FONT] <FTC_SBitCache_New failed with Errorcode 0x%.2X>\n", error);
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		munmap(lfb, available);
	}

	if((FTC_Manager_Lookup_Face(manager, (char*)FONT, &face)))
	{
		printf("[FONT] <FTC_Manager_Lookup_Face failed with Errorcode 0x%.2X>\n", error);
		FTC_Manager_Done(manager);
		FT_Done_FreeType(library);
		munmap(lfb, available);
	}

	use_kerning = FT_HAS_KERNING(face);

	desc.font.face_id = (char*)FONT;
#if FREETYPE_MAJOR  == 2 && FREETYPE_MINOR == 0
	desc.type = ftc_image_mono;
#else
	desc.flags = FT_LOAD_MONOCHROME;
#endif
#endif//USEFREETYPEFB

	showConsole(1);
	printf("[Framebuffer] available.\n");
	return;
nolfb:
	lfb=0;
	printf("[Framebuffer] <not available>\n");
	return;
}

fbClass::~fbClass()
{
	if(c_rect_buffer!=NULL) { free(c_rect_buffer); c_rect_buffer=NULL; }

	if (available)
		ioctl(fd, FBIOPUT_VSCREENINFO, &oldscreen);
	if (lfb)
		munmap(lfb, available);

#ifdef USEFREETYPEFB
	FTC_Manager_Done(manager);
	FT_Done_FreeType(library);
#endif
}

int fbClass::SetMode(unsigned int nxRes, unsigned int nyRes, unsigned int nbpp)
{
	screeninfo.xres_virtual=screeninfo.xres=nxRes;
	screeninfo.yres_virtual=screeninfo.yres=nyRes;
	screeninfo.height=0;
	screeninfo.width=0;
	screeninfo.xoffset=screeninfo.yoffset=0;
	screeninfo.bits_per_pixel=nbpp;
	if (ioctl(fd, FBIOPUT_VSCREENINFO, &screeninfo)<0)
	{
		perror("[FB-SET] <FBIOPUT_VSCREENINFO>");
		return -1;
	}

	if ((screeninfo.xres!=nxRes) && (screeninfo.yres!=nyRes) && (screeninfo.bits_per_pixel!=nbpp))
	{
		printf("[FB-SET] <SetMode failed: wanted: %dx%dx%d, got %dx%dx%d>\n", nxRes, nyRes, nbpp,
			screeninfo.xres,screeninfo.yres, screeninfo.bits_per_pixel);
	}
	xRes=screeninfo.xres;
	yRes=screeninfo.yres;
	printf("[Framebuffer] xRes:%d yRes:%d bpp:%d\n", xRes, yRes, screeninfo.bits_per_pixel);
	bpp=screeninfo.bits_per_pixel;
	fb_fix_screeninfo fix;
	if (ioctl(fd, FBIOGET_FSCREENINFO, &fix)<0) perror("[FB-SET] <FBIOGET_FSCREENINFO>");

	stride=fix.line_length;
	x_stride = (fix.line_length*8)/screeninfo.bits_per_pixel;
	memset(lfb, 0, stride*yRes);

	palette();
	return 0;
}

void fbClass::fb_display(unsigned char *rgbbuff,unsigned char * alpha,int x_size,int y_size,int x_pan,int y_pan,int x_offs,int y_offs)
{
	/* correct panning */
	if(x_pan > x_size - (int)x_stride) x_pan = 0;
	if(y_pan > y_size - (int)yRes) y_pan = 0;
	/* correct offset */
	if(x_offs + x_size > (int)x_stride) x_offs = 0;
	if(y_offs + y_size > (int)yRes) y_offs = 0;

	/* blit buffer 2 fb */
	int cpp = 0;
	unsigned short *fbbuff = (unsigned short *) convertRGB2FB(fd, rgbbuff, x_size * y_size, screeninfo.bits_per_pixel, &cpp);
	if(fbbuff==NULL) return;
	blit2FB(fbbuff, alpha, x_size, y_size, x_stride, yRes, x_pan, y_pan, x_offs, y_offs, cpp);
	free(fbbuff);
}

void fbClass::blit2FB(void *fbbuff,unsigned char *alpha,unsigned int pic_xs,unsigned int pic_ys,unsigned int scr_xs,unsigned int scr_ys, unsigned int xp, unsigned int yp,unsigned int xoffs, unsigned int yoffs,int cpp)
{
	int xc = (pic_xs > scr_xs) ? scr_xs : pic_xs;
	int yc = (pic_ys > scr_ys) ? scr_ys : pic_ys;


	unsigned char *fb = (unsigned char*) mmap(NULL, scr_xs * scr_ys * cpp, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
	if(fb == MAP_FAILED) { perror("mmap"); return; }

	if(cpp == 1) make332map();

	unsigned char *fbptr = fb + (yoffs * scr_xs + xoffs) * cpp;
	unsigned char *imptr = (unsigned char*) fbbuff + (yp * pic_xs + xp) * cpp;

	for(int i = 0; i < yc; i++, fbptr += scr_xs * cpp, imptr += pic_xs * cpp)
		memcpy(fbptr, imptr, xc * cpp);

	if(cpp == 1) make8map();

	munmap(fb, scr_xs * scr_ys * cpp);
}

void fbClass::make8map()
{
	struct fb_cmap colormap;
	colormap.start=0;
	colormap.len=256;
	colormap.red=red;
	colormap.green=green;
	colormap.blue=blue;
	colormap.transp=trans;

	ioctl(fd, FBIOPUTCMAP, &colormap);
}

void fbClass::make332map()
{
	int r = 8, g = 8, b = 4;

	struct fb_cmap colormap;
	colormap.start=0;
	colormap.len=256;
	colormap.red=red;
	colormap.green = green;
	colormap.blue = blue;
	colormap.transp=trans;

	int rs = 256 / (r - 1);
	int gs = 256 / (g - 1);
	int bs = 256 / (b - 1);

	for (int i = 0; i < 256; i++) {
		colormap.red[i]   = (rs * ((i / (g * b)) % r)) * 255;
		colormap.green[i] = (gs * ((i / b) % g)) * 255;
		colormap.blue[i]  = (bs * ((i) % b)) * 255;
	}

	ioctl(fd, FBIOPUTCMAP, &colormap);
}

//----------------------------------------------------------------------------------------

void fbClass::W_buffer(int sx, int sy, int width, int height)
{
	if(c_rect_buffer!=NULL)
	{
		unsigned char* busy_buffer_wrk = c_rect_buffer;

		for(int y=sy ; y < sy+height; y++)
		{
			for(int x=sx ; x< sx+width; x++)
			{
				memcpy(lfb + y * stride + x * c_rect_buffer_cpp, busy_buffer_wrk, c_rect_buffer_cpp);
				busy_buffer_wrk += c_rect_buffer_cpp;
			}
		}
	}
}

void fbClass::Fill_buffer(int sx, int sy, int width, int height)
{
	unsigned char rgb_buffer[3];
	unsigned char* fbbuff;
	unsigned char* busy_buffer_wrk;
	int cpp = 0;

	rgb_buffer[0]=0;
	rgb_buffer[1]=0;
	rgb_buffer[2]=0;

	fbbuff = (unsigned char *) convertRGB2FB(fd, rgb_buffer, 1, screeninfo.bits_per_pixel, &cpp);
	if(fbbuff==NULL) { printf("malloc_1\n"); return; }

	if(c_rect_buffer!=NULL) { free(c_rect_buffer); c_rect_buffer=NULL; }
	c_rect_buffer = (unsigned char*) malloc(width*width*cpp);
	if(c_rect_buffer==NULL) { printf("malloc_2\n"); return; }
	busy_buffer_wrk = c_rect_buffer;

	for(int y=sy ; y < sy+height; y++)
	{
		for(int x=sx ; x< sx+width; x++)
		{
			memcpy(busy_buffer_wrk, lfb + y * stride + x*cpp, cpp);
			busy_buffer_wrk += cpp;
		}
	}

	free(fbbuff);
	c_rect_buffer_cpp=cpp;
}

void fbClass::palette()
{
	unsigned char rgb_buffer[3];
	rgb_buffer[0]=0;
	rgb_buffer[1]=0;
	rgb_buffer[2]=0;
	int cpp = 0;

	unsigned char* fbbuff = (unsigned char *) convertRGB2FB(fd, rgb_buffer, 1, screeninfo.bits_per_pixel, &cpp);
	if(fbbuff==NULL) { printf("malloc\n"); return; }
	blit2FB(fbbuff, NULL, 1, 1, x_stride, yRes, 0, 0, 0, 0, cpp);
	free(fbbuff);
}

//----------------------------------------------------------------------------------------
char circle[] =
{
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
	0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,
	0,0,0,1,1,1,1,1,1,1,1,1,0,0,0,
	0,0,0,0,0,1,1,1,1,1,0,0,0,0,0
};

void fbClass::RenderCircle(int sx, int sy, int r, int g, int b)
{

	unsigned char rgb_buffer[3];
	unsigned char* fbbuff;
	int cpp = 0;

	rgb_buffer[0]=r;
	rgb_buffer[1]=g;
	rgb_buffer[2]=b;

	fbbuff = (unsigned char *) convertRGB2FB(fd, rgb_buffer, 1, screeninfo.bits_per_pixel, &cpp);
	if(fbbuff==NULL) { printf("malloc\n"); return; }

	for(int y = 0; y < 15; y++)
		for(int x = 0; x < 15; x++)
			if(circle[x + y*15])
				memcpy(lfb + (sy+y) * stride + (sx+x)*cpp, fbbuff, cpp);

	free(fbbuff);
}

void fbClass::FillRect(int sx, int sy, int width, int height, int r, int g, int b)
{
	unsigned char rgb_buffer[3];
	rgb_buffer[0]=r;
	rgb_buffer[1]=g;
	rgb_buffer[2]=b;
	int cpp = 0;

	unsigned char* fbbuff = (unsigned char *) convertRGB2FB(fd, rgb_buffer, 1, screeninfo.bits_per_pixel, &cpp);
	if(fbbuff==NULL) { printf("malloc\n"); return; }

	for(int y=sy ; y < sy+height; y++)
	{
		for(int x=sx ; x< sx+width; x++)
		{
			memcpy(lfb + y * stride + x*cpp, fbbuff, cpp);
		}
	}

	free(fbbuff);
}

void fbClass::PaintPixel(int x, int y, int r, int g, int b)
{
	unsigned char rgb_buffer[3];
	unsigned char* fbbuff;
	int cpp = 0;
	rgb_buffer[0]=r;
	rgb_buffer[1]=g;
	rgb_buffer[2]=b;

	fbbuff = (unsigned char *) convertRGB2FB(fd, rgb_buffer, 1, screeninfo.bits_per_pixel, &cpp);
	if(fbbuff==NULL) { printf("malloc\n"); return; }

	memcpy(lfb + y * stride + x*cpp, fbbuff, cpp);
	free(fbbuff);
}

void fbClass::DrawRect( int x, int y, int width, int height, int r, int g, int b)
{
	DrawHLine( x, y, width, r,g,b );
	DrawHLine( x, y+height, width, r,g,b );
	DrawVLine( x, y, height, r,g,b );
	DrawVLine( x+width, y, height, r,g,b );
}

void fbClass::DrawVLine(int x, int y, int sy, int r, int g, int b)
{
	while (sy--)
		PaintPixel(x, y+sy, r, g, b);

}

void fbClass::DrawHLine(int x, int y, int sx, int r, int g, int b)
{
	while (sx--)
		PaintPixel(x+sx, y, r, g, b);
}

#ifdef USEFREETYPEFB

void fbClass::RenderString(std::string word, int sx, int sy, int maxwidth, int layout, int size, int r, int g, int b)
{
	int stringlen, ex, charwidth;
	unsigned char *string = ((unsigned char*)word.c_str());

	desc.font.pix_width = desc.font.pix_height = size;

	if(layout != LEFT)
	{
		stringlen = GetStringLen(string);
		switch(layout)
		{
			case CENTER:	if(stringlen < maxwidth) sx += (maxwidth - stringlen)/2;
					break;

			case RIGHT:	if(stringlen < maxwidth) sx += maxwidth - stringlen;
					break;
		}
	}
	//reset kerning
	prev_glyphindex = 0;
	//render string
	ex = sx + maxwidth;
	while(*string != '\0')
	{
		if((charwidth = RenderChar(*string, sx, sy, ex, r,g,b)) == -1) return; /* string > maxwidth */
		sx += charwidth;
		string++;
	}
}

int fbClass::RenderChar(FT_ULong currentchar, int sx, int sy, int ex, int r, int g, int b)
{
	unsigned char rgb_buffer[3];
	unsigned char* fbbuff;
	int cpp = 0;
	rgb_buffer[0]=r;
	rgb_buffer[1]=g;
	rgb_buffer[2]=b;

	fbbuff = (unsigned char *) convertRGB2FB(fd, rgb_buffer, 1, screeninfo.bits_per_pixel, &cpp);
	if(fbbuff==NULL) { printf("malloc\n"); return 0; }

	int row, pitch, bit, x = 0, y = 0;
	FT_Error error;
	FT_UInt glyphindex;
	FT_Vector kerning;
	FTC_Node anode;

	//load char
	if(!(glyphindex = FT_Get_Char_Index(face, currentchar)))
	{
		printf("TuxMail <FT_Get_Char_Index for Char \"%c\" failed: \"undefined character code\">\n", (int)currentchar);return 0;
	}

	if((error = FTC_SBitCache_Lookup(cache, &desc, glyphindex, &sbit, &anode)))
	{
		printf("TuxMail <FTC_SBitCache_Lookup for Char \"%c\" failed with Errorcode 0x%.2X>\n", (int)currentchar, error);return 0;
	}

	if(use_kerning)
	{
		FT_Get_Kerning(face, prev_glyphindex, glyphindex, ft_kerning_default, &kerning);
		prev_glyphindex = glyphindex;
		kerning.x >>= 6;
	}
	else kerning.x = 0;

	//render char
	if(r != -1)
	{
		if(sx + sbit->xadvance >= ex) return -1; /* limit to maxwidth */

		for(row = 0; row < sbit->height; row++)
		{
			for(pitch = 0; pitch < sbit->pitch; pitch++)
			{
				for(bit = 7; bit >= 0; bit--)
				{
					if(pitch*8 + 7-bit >= sbit->width) break; /* render needed bits only */

					if((sbit->buffer[row * sbit->pitch + pitch]) & 1<<bit)
					{
						memcpy(lfb + (sy - sbit->top + y ) * stride + (sx + sbit->left + kerning.x + x)*cpp, fbbuff, cpp);
					}
					x++;
				}
			}
			x = 0;
			y++;
		}
	}

	free(fbbuff);
	return sbit->xadvance + kerning.x;
}

int fbClass::GetStringLen(unsigned char *string)
{
	int stringlen = 0;
	//reset kerning
	prev_glyphindex = 0;
	//calc len
	while(*string != '\0')
	{
		stringlen += RenderChar(*string, -1, -1, -1, -1,-1,-1);
		string++;
	}
	return stringlen;
}

#else//USEFREETYPEFB

#define MAXMSG 1024
#define DWIDTH 132
#define NCHARS 128
#define NBYTES 9271

/* Pointers into data_table for each ASCII char */
const int asc_ptr[NCHARS] = {
   0,      0,      0,      0,      0,      0,      0,      0,
   0,      0,      0,      0,      0,      0,      0,      0,
   0,      0,      0,      0,      0,      0,      0,      0,
   0,      0,      0,      0,      0,      0,      0,      0,
   1,      3,     50,     81,    104,    281,    483,    590,
 621,    685,    749,    851,    862,    893,    898,    921,
1019,   1150,   1200,   1419,   1599,   1744,   1934,   2111,
2235,   2445,   2622,   2659,      0,   2708,      0,   2715,
2857,   3072,   3273,   3403,   3560,   3662,   3730,   3785,
3965,   4000,   4015,   4115,   4281,   4314,   4432,   4548,
4709,   4790,   4999,   5188,   5397,   5448,   5576,   5710,
5892,   6106,   6257,      0,      0,      0,      0,      0,
  50,   6503,   6642,   6733,   6837,   6930,   7073,   7157,
7380,   7452,   7499,   7584,   7689,   7702,   7797,   7869,
7978,   8069,   8160,   8222,   8381,   8442,   8508,   8605,
8732,   8888,   9016,      0,      0,      0,      0,      0
};

const unsigned char data_table[NBYTES] = {
   129,  227,  130,   34,    6,   90,   19,  129,   32,   10,
    74,   40,  129,   31,   12,   64,   53,  129,   30,   14,
    54,   65,  129,   30,   14,   53,   67,  129,   30,   14,
    54,   65,  129,   31,   12,   64,   53,  129,   32,   10,
    74,   40,  129,   34,    6,   90,   19,  129,  194,  130,
    99,    9,  129,   97,   14,  129,   96,   18,  129,   95,
    22,  129,   95,   16,  117,    2,  129,   95,   14,  129,
    96,   11,  129,   97,    9,  129,   99,    6,  129,  194,
   129,   87,    4,  101,    4,  131,   82,   28,  131,   87,
     4,  101,    4,  133,   82,   28,  131,   87,    4,  101,
     4,  131,  193,  129,   39,    1,   84,   27,  129,   38,
     3,   81,   32,  129,   37,    5,   79,   35,  129,   36,
     5,   77,   38,  129,   35,    5,   76,   40,  129,   34,
     5,   75,   21,  103,   14,  129,   33,    5,   74,   19,
   107,   11,  129,   32,    5,   73,   17,  110,    9,  129,
    32,    4,   73,   16,  112,    7,  129,   31,    4,   72,
    15,  114,    6,  129,   31,    4,   72,   14,  115,    5,
   129,   30,    4,   71,   15,  116,    5,  129,   27,   97,
   131,   30,    4,   69,   14,  117,    4,  129,   30,    4,
    68,   15,  117,    4,  132,   30,    4,   68,   14,  117,
     4,  129,   27,   97,  131,   30,    5,   65,   15,  116,
     5,  129,   31,    4,   65,   14,  116,    4,  129,   31,
     6,   64,   15,  116,    4,  129,   32,    7,   62,   16,
   115,    4,  129,   32,    9,   61,   17,  114,    5,  129,
    33,   11,   58,   19,  113,    5,  129,   34,   14,   55,
    21,  112,    5,  129,   35,   40,  111,    5,  129,   36,
    38,  110,    5,  129,   37,   35,  109,    5,  129,   38,
    32,  110,    3,  129,   40,   27,  111,    1,  129,  193,
   129,   30,    4,  103,    9,  129,   30,    7,  100,   15,
   129,   30,   10,   99,   17,  129,   33,   10,   97,    6,
   112,    6,  129,   36,   10,   96,    5,  114,    5,  129,
    39,   10,   96,    4,  115,    4,  129,   42,   10,   95,
     4,  116,    4,  129,   45,   10,   95,    3,  117,    3,
   129,   48,   10,   95,    3,  117,    3,  129,   51,   10,
    95,    4,  116,    4,  129,   54,   10,   96,    4,  115,
     4,  129,   57,   10,   96,    5,  114,    5,  129,   60,
    10,   97,    6,  112,    6,  129,   63,   10,   99,   17,
   129,   66,   10,  100,   15,  129,   69,   10,  103,    9,
   129,   39,    9,   72,   10,  129,   36,   15,   75,   10,
   129,   35,   17,   78,   10,  129,   33,    6,   48,    6,
    81,   10,  129,   32,    5,   50,    5,   84,   10,  129,
    32,    4,   51,    4,   87,   10,  129,   31,    4,   52,
     4,   90,   10,  129,   31,    3,   53,    3,   93,   10,
   129,   31,    3,   53,    3,   96,   10,  129,   31,    4,
    52,    4,   99,   10,  129,   32,    4,   51,    4,  102,
    10,  129,   32,    5,   50,    5,  105,   10,  129,   33,
     6,   48,    6,  108,   10,  129,   35,   17,  111,   10,
   129,   36,   15,  114,    7,  129,   40,    9,  118,    4,
   129,  193,  129,   48,   18,  129,   43,   28,  129,   41,
    32,  129,   39,   36,  129,   37,   40,  129,   35,   44,
   129,   34,   46,  129,   33,   13,   68,   13,  129,   32,
     9,   73,    9,  129,   32,    7,   75,    7,  129,   31,
     6,   77,    6,  129,   31,    5,   78,    5,  129,   30,
     5,   79,    5,  129,   20,   74,  132,   30,    4,   80,
     4,  129,   31,    3,   79,    4,  129,   31,    4,   79,
     4,  129,   32,    3,   78,    4,  129,   32,    4,   76,
     6,  129,   33,    4,   74,    7,  129,   34,    4,   72,
     8,  129,   35,    5,   72,    7,  129,   37,    5,   73,
     4,  129,   39,    4,   74,    1,  129,  129,  193,  130,
   111,    6,  129,  109,   10,  129,  108,   12,  129,  107,
    14,  129,   97,    2,  105,   16,  129,   99,   22,  129,
   102,   18,  129,  105,   14,  129,  108,    9,  129,  194,
   130,   63,   25,  129,   57,   37,  129,   52,   47,  129,
    48,   55,  129,   44,   63,  129,   41,   69,  129,   38,
    75,  129,   36,   79,  129,   34,   83,  129,   33,   28,
    90,   28,  129,   32,   23,   96,   23,  129,   32,   17,
   102,   17,  129,   31,   13,  107,   13,  129,   30,    9,
   112,    9,  129,   30,    5,  116,    5,  129,   30,    1,
   120,    1,  129,  194,  130,   30,    1,  120,    1,  129,
    30,    5,  116,    5,  129,   30,    9,  112,    9,  129,
    31,   13,  107,   13,  129,   32,   17,  102,   17,  129,
    32,   23,   96,   23,  129,   33,   28,   90,   28,  129,
    34,   83,  129,   36,   79,  129,   38,   75,  129,   41,
    69,  129,   44,   63,  129,   48,   55,  129,   52,   47,
   129,   57,   37,  129,   63,   25,  129,  194,  129,   80,
     4,  130,   80,    4,  129,   68,    2,   80,    4,   94,
     2,  129,   66,    6,   80,    4,   92,    6,  129,   67,
     7,   80,    4,   90,    7,  129,   69,    7,   80,    4,
    88,    7,  129,   71,    6,   80,    4,   87,    6,  129,
    72,   20,  129,   74,   16,  129,   76,   12,  129,   62,
    40,  131,   76,   12,  129,   74,   16,  129,   72,   20,
   129,   71,    6,   80,    4,   87,    6,  129,   69,    7,
    80,    4,   88,    7,  129,   67,    7,   80,    4,   90,
     7,  129,   66,    6,   80,    4,   92,    6,  129,   68,
     2,   80,    4,   94,    2,  129,   80,    4,  130,  193,
   129,   60,    4,  139,   41,   42,  131,   60,    4,  139,
   193,  130,   34,    6,  129,   32,   10,  129,   31,   12,
   129,   30,   14,  129,   20,    2,   28,   16,  129,   22,
    22,  129,   24,   19,  129,   27,   15,  129,   31,    9,
   129,  194,  129,   60,    4,  152,  193,  130,   34,    6,
   129,   32,   10,  129,   31,   12,  129,   30,   14,  131,
    31,   12,  129,   32,   10,  129,   34,    6,  129,  194,
   129,   30,    4,  129,   30,    7,  129,   30,   10,  129,
    33,   10,  129,   36,   10,  129,   39,   10,  129,   42,
    10,  129,   45,   10,  129,   48,   10,  129,   51,   10,
   129,   54,   10,  129,   57,   10,  129,   60,   10,  129,
    63,   10,  129,   66,   10,  129,   69,   10,  129,   72,
    10,  129,   75,   10,  129,   78,   10,  129,   81,   10,
   129,   84,   10,  129,   87,   10,  129,   90,   10,  129,
    93,   10,  129,   96,   10,  129,   99,   10,  129,  102,
    10,  129,  105,   10,  129,  108,   10,  129,  111,   10,
   129,  114,    7,  129,  117,    4,  129,  193,  129,   60,
    31,  129,   53,   45,  129,   49,   53,  129,   46,   59,
   129,   43,   65,  129,   41,   69,  129,   39,   73,  129,
    37,   77,  129,   36,   79,  129,   35,   15,  101,   15,
   129,   34,   11,  106,   11,  129,   33,    9,  109,    9,
   129,   32,    7,  112,    7,  129,   31,    6,  114,    6,
   129,   31,    5,  115,    5,  129,   30,    5,  116,    5,
   129,   30,    4,  117,    4,  132,   30,    5,  116,    5,
   129,   31,    5,  115,    5,  129,   31,    6,  114,    6,
   129,   32,    7,  112,    7,  129,   33,    9,  109,    9,
   129,   34,   11,  106,   11,  129,   35,   15,  101,   15,
   129,   36,   79,  129,   37,   77,  129,   39,   73,  129,
    41,   69,  129,   43,   65,  129,   46,   59,  129,   49,
    53,  129,   53,   45,  129,   60,   31,  129,  193,  129,
    30,    4,  129,   30,    4,  100,    1,  129,   30,    4,
   100,    3,  129,   30,    4,  100,    5,  129,   30,   76, 
/* 1170 */   129,   30,   78,  129,   30,   80,  129,   30,   82,  129, 
/* 1180 */    30,   83,  129,   30,   85,  129,   30,   87,  129,   30, 
/* 1190 */    89,  129,   30,   91,  129,   30,    4,  132,  193,  129, 
/* 1200 */    30,    3,  129,   30,    7,  129,   30,   10,  112,    1, 
/* 1210 */   129,   30,   13,  112,    2,  129,   30,   16,  112,    3, 
/* 1220 */   129,   30,   18,  111,    5,  129,   30,   21,  111,    6, 
/* 1230 */   129,   30,   23,  112,    6,  129,   30,   14,   47,    8, 
/* 1240 */   113,    6,  129,   30,   14,   49,    8,  114,    5,  129, 
/* 1250 */    30,   14,   51,    8,  115,    5,  129,   30,   14,   53, 
/* 1260 */     8,  116,    4,  129,   30,   14,   55,    8,  116,    5, 
/* 1270 */   129,   30,   14,   56,    9,  117,    4,  129,   30,   14, 
/* 1280 */    57,    9,  117,    4,  129,   30,   14,   58,   10,  117, 
/* 1290 */     4,  129,   30,   14,   59,   10,  117,    4,  129,   30, 
/* 1300 */    14,   60,   11,  117,    4,  129,   30,   14,   61,   11, 
/* 1310 */   116,    5,  129,   30,   14,   62,   11,  116,    5,  129, 
/* 1320 */    30,   14,   63,   12,  115,    6,  129,   30,   14,   64, 
/* 1330 */    13,  114,    7,  129,   30,   14,   65,   13,  113,    8, 
/* 1340 */   129,   30,   14,   65,   15,  111,    9,  129,   30,   14, 
/* 1350 */    66,   16,  109,   11,  129,   30,   14,   67,   17,  107, 
/* 1360 */    12,  129,   30,   14,   68,   20,  103,   16,  129,   30, 
/* 1370 */    14,   69,   49,  129,   30,   14,   70,   47,  129,   30, 
/* 1380 */    14,   71,   45,  129,   30,   14,   73,   42,  129,   30, 
/* 1390 */    15,   75,   38,  129,   33,   12,   77,   34,  129,   36, 
/* 1400 */    10,   79,   30,  129,   40,    6,   82,   23,  129,   44, 
/* 1410 */     3,   86,   15,  129,   47,    1,  129,  193,  129,  129, 
/* 1420 */    38,    3,  129,   37,    5,  111,    1,  129,   36,    7, 
/* 1430 */   111,    2,  129,   35,    9,  110,    5,  129,   34,    8, 
/* 1440 */   110,    6,  129,   33,    7,  109,    8,  129,   32,    7, 
/* 1450 */   110,    8,  129,   32,    6,  112,    7,  129,   31,    6, 
/* 1460 */   113,    6,  129,   31,    5,  114,    6,  129,   30,    5, 
/* 1470 */   115,    5,  129,   30,    5,  116,    4,  129,   30,    4, 
/* 1480 */   117,    4,  131,   30,    4,  117,    4,  129,   30,    4, 
/* 1490 */    79,    2,  117,    4,  129,   30,    5,   78,    4,  117, 
/* 1500 */     4,  129,   30,    5,   77,    6,  116,    5,  129,   30, 
/* 1510 */     6,   76,    8,  115,    6,  129,   30,    7,   75,   11, 
/* 1520 */   114,    6,  129,   30,    8,   73,   15,  112,    8,  129, 
/* 1530 */    31,    9,   71,   19,  110,    9,  129,   31,   11,   68, 
/* 1540 */    26,  107,   12,  129,   32,   13,   65,   14,   82,   36, 
/* 1550 */   129,   32,   16,   61,   17,   83,   34,  129,   33,   44, 
/* 1560 */    84,   32,  129,   34,   42,   85,   30,  129,   35,   40, 
/* 1570 */    87,   27,  129,   36,   38,   89,   23,  129,   38,   34, 
/* 1580 */    92,   17,  129,   40,   30,   95,   11,  129,   42,   26, 
/* 1590 */   129,   45,   20,  129,   49,   11,  129,  193,  129,   49, 
/* 1600 */     1,  129,   49,    4,  129,   49,    6,  129,   49,    8, 
/* 1610 */   129,   49,   10,  129,   49,   12,  129,   49,   14,  129, 
/* 1620 */    49,   17,  129,   49,   19,  129,   49,   21,  129,   49, 
/* 1630 */    23,  129,   49,   14,   65,    9,  129,   49,   14,   67, 
/* 1640 */     9,  129,   49,   14,   69,    9,  129,   49,   14,   71, 
/* 1650 */    10,  129,   49,   14,   74,    9,  129,   49,   14,   76, 
/* 1660 */     9,  129,   49,   14,   78,    9,  129,   49,   14,   80, 
/* 1670 */     9,  129,   49,   14,   82,    9,  129,   49,   14,   84, 
/* 1680 */     9,  129,   30,    4,   49,   14,   86,   10,  129,   30, 
/* 1690 */     4,   49,   14,   89,    9,  129,   30,    4,   49,   14, 
/* 1700 */    91,    9,  129,   30,    4,   49,   14,   93,    9,  129, 
/* 1710 */    30,   74,  129,   30,   76,  129,   30,   78,  129,   30, 
/* 1720 */    81,  129,   30,   83,  129,   30,   85,  129,   30,   87, 
/* 1730 */   129,   30,   89,  129,   30,   91,  129,   30,    4,   49, 
/* 1740 */    14,  132,  193,  129,   37,    1,  129,   36,    3,   77, 
/* 1750 */     3,  129,   35,    5,   78,   11,  129,   34,    7,   78, 
/* 1760 */    21,  129,   33,    7,   79,   29,  129,   32,    7,   79, 
/* 1770 */    38,  129,   32,    6,   80,    4,   92,   29,  129,   31, 
/* 1780 */     6,   80,    5,  102,   19,  129,   31,    5,   80,    6, 
/* 1790 */   107,   14,  129,   31,    4,   81,    5,  107,   14,  129, 
/* 1800 */    30,    5,   81,    6,  107,   14,  129,   30,    4,   81, 
/* 1810 */     6,  107,   14,  130,   30,    4,   81,    7,  107,   14, 
/* 1820 */   129,   30,    4,   80,    8,  107,   14,  130,   30,    5, 
/* 1830 */    80,    8,  107,   14,  129,   30,    5,   79,    9,  107, 
/* 1840 */    14,  129,   31,    5,   79,    9,  107,   14,  129,   31, 
/* 1850 */     6,   78,   10,  107,   14,  129,   32,    6,   76,   11, 
/* 1860 */   107,   14,  129,   32,    8,   74,   13,  107,   14,  129, 
/* 1870 */    33,   10,   71,   16,  107,   14,  129,   33,   15,   67, 
/* 1880 */    19,  107,   14,  129,   34,   51,  107,   14,  129,   35, 
/* 1890 */    49,  107,   14,  129,   36,   47,  107,   14,  129,   37, 
/* 1900 */    45,  107,   14,  129,   39,   41,  107,   14,  129,   41, 
/* 1910 */    37,  107,   14,  129,   44,   32,  107,   14,  129,   47, 
/* 1920 */    25,  111,   10,  129,   51,   16,  115,    6,  129,  119, 
/* 1930 */     2,  129,  193,  129,   56,   39,  129,   51,   49,  129, 
/* 1940 */    47,   57,  129,   44,   63,  129,   42,   67,  129,   40, 
/* 1950 */    71,  129,   38,   75,  129,   37,   77,  129,   35,   81, 
/* 1960 */   129,   34,   16,   74,    5,  101,   16,  129,   33,   11, 
/* 1970 */    76,    5,  107,   11,  129,   32,    9,   77,    5,  110, 
/* 1980 */     9,  129,   32,    7,   79,    4,  112,    7,  129,   31, 
/* 1990 */     6,   80,    4,  114,    6,  129,   31,    5,   81,    4, 
/* 2000 */   115,    5,  129,   30,    5,   82,    4,  116,    5,  129, 
/* 2010 */    30,    4,   82,    4,  116,    5,  129,   30,    4,   82, 
/* 2020 */     5,  117,    4,  131,   30,    5,   82,    5,  117,    4, 
/* 2030 */   129,   31,    5,   81,    6,  117,    4,  129,   31,    6, 
/* 2040 */    80,    7,  117,    4,  129,   32,    7,   79,    8,  117, 
/* 2050 */     4,  129,   32,    9,   77,    9,  116,    5,  129,   33, 
/* 2060 */    11,   75,   11,  116,    4,  129,   34,   16,   69,   16, 
/* 2070 */   115,    5,  129,   35,   49,  114,    5,  129,   37,   46, 
/* 2080 */   113,    5,  129,   38,   44,  112,    6,  129,   40,   41, 
/* 2090 */   112,    5,  129,   42,   37,  113,    3,  129,   44,   33, 
/* 2100 */   114,    1,  129,   47,   27,  129,   51,   17,  129,  193, 
/* 2110 */   129,  103,    2,  129,  103,    6,  129,  104,    9,  129, 
/* 2120 */   105,   12,  129,  106,   15,  129,  107,   14,  135,   30, 
/* 2130 */    10,  107,   14,  129,   30,   17,  107,   14,  129,   30, 
/* 2140 */    25,  107,   14,  129,   30,   31,  107,   14,  129,   30, 
/* 2150 */    37,  107,   14,  129,   30,   42,  107,   14,  129,   30, 
/* 2160 */    46,  107,   14,  129,   30,   50,  107,   14,  129,   30, 
/* 2170 */    54,  107,   14,  129,   30,   58,  107,   14,  129,   59, 
/* 2180 */    32,  107,   14,  129,   64,   30,  107,   14,  129,   74, 
/* 2190 */    23,  107,   14,  129,   81,   18,  107,   14,  129,   86, 
/* 2200 */    16,  107,   14,  129,   91,   14,  107,   14,  129,   96, 
/* 2210 */    25,  129,  100,   21,  129,  104,   17,  129,  107,   14, 
/* 2220 */   129,  111,   10,  129,  114,    7,  129,  117,    4,  129, 
/* 2230 */   120,    1,  129,  193,  129,   48,   13,  129,   44,   21, 
/* 2240 */   129,   42,   26,  129,   40,   30,   92,   12,  129,   38, 
/* 2250 */    34,   88,   20,  129,   36,   37,   86,   25,  129,   35, 
/* 2260 */    39,   84,   29,  129,   34,   13,   63,   12,   82,   33, 
/* 2270 */   129,   33,   11,   67,    9,   80,   36,  129,   32,    9, 
/* 2280 */    70,    7,   79,   38,  129,   31,    8,   72,   46,  129, 
/* 2290 */    30,    7,   74,   22,  108,   11,  129,   30,    6,   75, 
/* 2300 */    19,  111,    9,  129,   30,    5,   75,   17,  113,    7, 
/* 2310 */   129,   30,    5,   74,   16,  114,    6,  129,   30,    4, 
/* 2320 */    73,   16,  115,    6,  129,   30,    4,   72,   16,  116, 
/* 2330 */     5,  129,   30,    4,   72,   15,  117,    4,  129,   30, 
/* 2340 */     4,   71,   16,  117,    4,  129,   30,    5,   70,   16, 
/* 2350 */   117,    4,  129,   30,    5,   70,   15,  117,    4,  129, 
/* 2360 */    30,    6,   69,   15,  116,    5,  129,   30,    7,   68, 
/* 2370 */    17,  115,    5,  129,   30,    9,   67,   19,  114,    6, 
/* 2380 */   129,   30,   10,   65,   22,  113,    6,  129,   31,   12, 
/* 2390 */    63,   27,  110,    9,  129,   32,   14,   60,   21,   84, 
/* 2400 */     9,  106,   12,  129,   33,   47,   85,   32,  129,   34, 
/* 2410 */    45,   86,   30,  129,   35,   43,   88,   26,  129,   36, 
/* 2420 */    40,   90,   22,  129,   38,   36,   93,   17,  129,   40, 
/* 2430 */    32,   96,   10,  129,   42,   28,  129,   44,   23,  129, 
/* 2440 */    48,   15,  129,  193,  129,   83,   17,  129,   77,   27, 
/* 2450 */   129,   36,    1,   74,   33,  129,   35,    3,   72,   37, 
/* 2460 */   129,   34,    5,   70,   41,  129,   33,    6,   69,   44, 
/* 2470 */   129,   33,    5,   68,   46,  129,   32,    5,   67,   49, 
/* 2480 */   129,   31,    5,   66,   17,  101,   16,  129,   31,    5, 
/* 2490 */    66,   11,  108,   10,  129,   30,    4,   65,    9,  110, 
/* 2500 */     9,  129,   30,    4,   64,    8,  112,    7,  129,   30, 
/* 2510 */     4,   64,    7,  114,    6,  129,   30,    4,   64,    6, 
/* 2520 */   115,    5,  129,   30,    4,   64,    5,  116,    5,  129, 
/* 2530 */    30,    4,   64,    5,  117,    4,  131,   30,    4,   65, 
/* 2540 */     4,  117,    4,  129,   30,    5,   65,    4,  116,    5, 
/* 2550 */   129,   31,    5,   66,    4,  115,    5,  129,   31,    6, 
/* 2560 */    67,    4,  114,    6,  129,   32,    7,   68,    4,  112, 
/* 2570 */     7,  129,   32,    9,   69,    5,  110,    9,  129,   33, 
/* 2580 */    11,   70,    5,  107,   11,  129,   34,   16,   72,    5, 
/* 2590 */   101,   16,  129,   35,   81,  129,   37,   77,  129,   38, 
/* 2600 */    75,  129,   40,   71,  129,   42,   67,  129,   44,   63, 
/* 2610 */   129,   47,   57,  129,   51,   49,  129,   56,   39,  129, 
/* 2620 */   193,  130,   34,    6,   74,    6,  129,   32,   10,   72, 
/* 2630 */    10,  129,   31,   12,   71,   12,  129,   30,   14,   70, 
/* 2640 */    14,  131,   31,   12,   71,   12,  129,   32,   10,   72, 
/* 2650 */    10,  129,   34,    6,   74,    6,  129,  194,  130,   34, 
/* 2660 */     6,   74,    6,  129,   32,   10,   72,   10,  129,   31, 
/* 2670 */    12,   71,   12,  129,   30,   14,   70,   14,  129,   20, 
/* 2680 */     2,   28,   16,   70,   14,  129,   22,   22,   70,   14, 
/* 2690 */   129,   24,   19,   71,   12,  129,   27,   15,   72,   10, 
/* 2700 */   129,   31,    9,   74,    6,  129,  194,  129,   53,    4, 
/* 2710 */    63,    4,  152,  193,  130,   99,    7,  129,   97,   13, 
/* 2720 */   129,   96,   16,  129,   96,   18,  129,   96,   19,  129, 
/* 2730 */    97,   19,  129,   99,    6,  110,    7,  129,  112,    6, 
/* 2740 */   129,  114,    5,  129,   34,    6,   57,    5,  115,    4, 
/* 2750 */   129,   32,   10,   54,   12,  116,    4,  129,   31,   12, 
/* 2760 */    53,   16,  117,    3,  129,   30,   14,   52,   20,  117, 
/* 2770 */     4,  129,   30,   14,   52,   23,  117,    4,  129,   30, 
/* 2780 */    14,   52,   25,  117,    4,  129,   31,   12,   52,   27, 
/* 2790 */   117,    4,  129,   32,   10,   53,   10,   70,   11,  116, 
/* 2800 */     5,  129,   34,    6,   55,    5,   73,   10,  115,    6, 
/* 2810 */   129,   74,   11,  114,    7,  129,   75,   12,  112,    9, 
/* 2820 */   129,   76,   13,  110,   10,  129,   77,   16,  106,   14, 
/* 2830 */   129,   78,   41,  129,   80,   38,  129,   81,   36,  129, 
/* 2840 */    82,   34,  129,   84,   30,  129,   86,   26,  129,   88, 
/* 2850 */    22,  129,   92,   14,  129,  194,  129,   55,   15,  129, 
/* 2860 */    50,   25,  129,   47,   32,  129,   45,   13,   70,   12, 
/* 2870 */   129,   43,    9,   76,   10,  129,   42,    6,   79,    8, 
/* 2880 */   129,   41,    5,   81,    7,  129,   40,    4,   84,    6, 
/* 2890 */   129,   39,    4,   59,   12,   85,    6,  129,   38,    4, 
/* 2900 */    55,   19,   87,    5,  129,   37,    4,   53,   23,   88, 
/* 2910 */     4,  129,   36,    4,   51,    8,   71,    6,   89,    4, 
/* 2920 */   129,   36,    4,   51,    6,   73,    4,   89,    4,  129, 
/* 2930 */    36,    4,   50,    6,   74,    4,   90,    3,  129,   35, 
/* 2940 */     4,   50,    5,   75,    3,   90,    4,  129,   35,    4, 
/* 2950 */    50,    4,   75,    4,   90,    4,  131,   35,    4,   50, 
/* 2960 */     5,   75,    4,   90,    4,  129,   36,    4,   51,    5, 
/* 2970 */    75,    4,   90,    4,  129,   36,    4,   51,    6,   75, 
/* 2980 */     4,   90,    4,  129,   36,    4,   53,   26,   90,    4, 
/* 2990 */   129,   37,    4,   54,   25,   90,    4,  129,   37,    4, 
/* 3000 */    52,   27,   90,    3,  129,   38,    4,   52,    4,   89, 
/* 3010 */     4,  129,   39,    4,   51,    4,   88,    4,  129,   40, 
/* 3020 */     4,   50,    4,   87,    5,  129,   41,    4,   50,    4, 
/* 3030 */    86,    5,  129,   42,    4,   50,    4,   85,    5,  129, 
/* 3040 */    43,    3,   50,    4,   83,    6,  129,   44,    2,   51, 
/* 3050 */     5,   80,    7,  129,   46,    1,   52,    6,   76,    9, 
/* 3060 */   129,   54,   28,  129,   56,   23,  129,   60,   16,  129, 
/* 3070 */   193,  129,   30,    4,  132,   30,    5,  129,   30,    8, 
/* 3080 */   129,   30,   12,  129,   30,   16,  129,   30,    4,   37, 
/* 3090 */    12,  129,   30,    4,   41,   12,  129,   30,    4,   44, 
/* 3100 */    13,  129,   30,    4,   48,   13,  129,   52,   13,  129, 
/* 3110 */    56,   12,  129,   58,   14,  129,   58,    4,   64,   12, 
/* 3120 */   129,   58,    4,   68,   12,  129,   58,    4,   72,   12, 
/* 3130 */   129,   58,    4,   75,   13,  129,   58,    4,   79,   13, 
/* 3140 */   129,   58,    4,   83,   13,  129,   58,    4,   87,   13, 
/* 3150 */   129,   58,    4,   91,   12,  129,   58,    4,   95,   12, 
/* 3160 */   129,   58,    4,   96,   15,  129,   58,    4,   93,   22, 
/* 3170 */   129,   58,    4,   89,   30,  129,   58,    4,   85,   36, 
/* 3180 */   129,   58,    4,   81,   38,  129,   58,    4,   77,   38, 
/* 3190 */   129,   58,    4,   73,   38,  129,   58,    4,   70,   37, 
/* 3200 */   129,   58,    4,   66,   37,  129,   58,   41,  129,   58, 
/* 3210 */    37,  129,   54,   38,  129,   30,    4,   50,   38,  129, 
/* 3220 */    30,    4,   46,   38,  129,   30,    4,   42,   38,  129, 
/* 3230 */    30,    4,   38,   39,  129,   30,   43,  129,   30,   39, 
/* 3240 */   129,   30,   35,  129,   30,   31,  129,   30,   27,  129, 
/* 3250 */    30,   24,  129,   30,   20,  129,   30,   16,  129,   30, 
/* 3260 */    12,  129,   30,    8,  129,   30,    5,  129,   30,    4, 
/* 3270 */   132,  193,  129,   30,    4,  117,    4,  132,   30,   91, 
/* 3280 */   137,   30,    4,   80,    4,  117,    4,  138,   30,    4, 
/* 3290 */    80,    5,  116,    5,  129,   30,    5,   79,    6,  116, 
/* 3300 */     5,  130,   30,    6,   78,    8,  115,    6,  129,   31, 
/* 3310 */     6,   77,    9,  115,    6,  129,   31,    7,   76,   11, 
/* 3320 */   114,    6,  129,   31,    8,   75,   14,  112,    8,  129, 
/* 3330 */    32,    8,   74,   16,  111,    9,  129,   32,    9,   73, 
/* 3340 */    19,  109,   10,  129,   33,   10,   71,   24,  106,   13, 
/* 3350 */   129,   33,   13,   68,   12,   83,   35,  129,   34,   16, 
/* 3360 */    64,   15,   84,   33,  129,   35,   43,   85,   31,  129, 
/* 3370 */    36,   41,   86,   29,  129,   37,   39,   88,   25,  129, 
/* 3380 */    38,   37,   90,   21,  129,   40,   33,   93,   15,  129, 
/* 3390 */    42,   29,   96,    9,  129,   45,   24,  129,   49,   16, 
/* 3400 */   129,  193,  129,   63,   25,  129,   57,   37,  129,   53, 
/* 3410 */    45,  129,   50,   51,  129,   47,   57,  129,   45,   61, 
/* 3420 */   129,   43,   65,  129,   41,   69,  129,   39,   73,  129, 
/* 3430 */    38,   25,   92,   21,  129,   36,   21,   97,   18,  129, 
/* 3440 */    35,   18,  102,   14,  129,   34,   16,  106,   11,  129, 
/* 3450 */    33,   14,  108,   10,  129,   32,   12,  111,    8,  129, 
/* 3460 */    32,   10,  113,    6,  129,   31,   10,  114,    6,  129, 
/* 3470 */    31,    8,  115,    5,  129,   30,    8,  116,    5,  129, 
/* 3480 */    30,    7,  116,    5,  129,   30,    6,  117,    4,  130, 
/* 3490 */    30,    5,  117,    4,  131,   31,    4,  116,    5,  129, 
/* 3500 */    32,    4,  116,    4,  129,   32,    5,  115,    5,  129, 
/* 3510 */    33,    4,  114,    5,  129,   34,    4,  112,    6,  129, 
/* 3520 */    35,    4,  110,    7,  129,   37,    4,  107,    9,  129, 
/* 3530 */    39,    4,  103,   12,  129,   41,    4,  103,   18,  129, 
/* 3540 */    43,    4,  103,   18,  129,   45,    5,  103,   18,  129, 
/* 3550 */    48,    5,  103,   18,  129,   51,    1,  129,  193,  129, 
/* 3560 */    30,    4,  117,    4,  132,   30,   91,  137,   30,    4, 
/* 3570 */   117,    4,  135,   30,    5,  116,    5,  130,   30,    6, 
/* 3580 */   115,    6,  130,   31,    6,  114,    6,  129,   31,    7, 
/* 3590 */   113,    7,  129,   32,    7,  112,    7,  129,   32,    8, 
/* 3600 */   111,    8,  129,   33,    9,  109,    9,  129,   33,   12, 
/* 3610 */   106,   12,  129,   34,   13,  104,   13,  129,   35,   15, 
/* 3620 */   101,   15,  129,   36,   19,   96,   19,  129,   37,   24, 
/* 3630 */    90,   24,  129,   39,   73,  129,   40,   71,  129,   42, 
/* 3640 */    67,  129,   44,   63,  129,   46,   59,  129,   49,   53, 
/* 3650 */   129,   52,   47,  129,   56,   39,  129,   61,   29,  129, 
/* 3660 */   193,  129,   30,    4,  117,    4,  132,   30,   91,  137, 
/* 3670 */    30,    4,   80,    4,  117,    4,  140,   30,    4,   79, 
/* 3680 */     6,  117,    4,  129,   30,    4,   77,   10,  117,    4, 
/* 3690 */   129,   30,    4,   73,   18,  117,    4,  132,   30,    4, 
/* 3700 */   117,    4,  130,   30,    5,  116,    5,  130,   30,    7, 
/* 3710 */   114,    7,  129,   30,    8,  113,    8,  129,   30,   11, 
/* 3720 */   110,   11,  129,   30,   18,  103,   18,  132,  193,  129, 
/* 3730 */    30,    4,  117,    4,  132,   30,   91,  137,   30,    4, 
/* 3740 */    80,    4,  117,    4,  132,   80,    4,  117,    4,  136, 
/* 3750 */    79,    6,  117,    4,  129,   77,   10,  117,    4,  129, 
/* 3760 */    73,   18,  117,    4,  132,  117,    4,  130,  116,    5, 
/* 3770 */   130,  114,    7,  129,  113,    8,  129,  110,   11,  129, 
/* 3780 */   103,   18,  132,  193,  129,   63,   25,  129,   57,   37, 
/* 3790 */   129,   53,   45,  129,   50,   51,  129,   47,   57,  129, 
/* 3800 */    45,   61,  129,   43,   65,  129,   41,   69,  129,   39, 
/* 3810 */    73,  129,   38,   25,   92,   21,  129,   36,   21,   97, 
/* 3820 */    18,  129,   35,   18,  102,   14,  129,   34,   16,  106, 
/* 3830 */    11,  129,   33,   14,  108,   10,  129,   32,   12,  111, 
/* 3840 */     8,  129,   32,   10,  113,    6,  129,   31,   10,  114, 
/* 3850 */     6,  129,   31,    8,  115,    5,  129,   30,    8,  116, 
/* 3860 */     5,  129,   30,    7,  116,    5,  129,   30,    6,  117, 
/* 3870 */     4,  130,   30,    5,  117,    4,  131,   30,    5,   75, 
/* 3880 */     4,  116,    5,  129,   31,    5,   75,    4,  116,    4, 
/* 3890 */   129,   31,    6,   75,    4,  115,    5,  129,   32,    7, 
/* 3900 */    75,    4,  114,    5,  129,   32,    9,   75,    4,  112, 
/* 3910 */     6,  129,   33,   11,   75,    4,  110,    7,  129,   34, 
/* 3920 */    15,   75,    4,  107,    9,  129,   35,   44,  103,   12, 
/* 3930 */   129,   36,   43,  103,   18,  129,   38,   41,  103,   18, 
/* 3940 */   129,   39,   40,  103,   18,  129,   41,   38,  103,   18, 
/* 3950 */   129,   44,   35,  129,   48,   31,  129,   52,   27,  129, 
/* 3960 */    61,   18,  129,  193,  129,   30,    4,  117,    4,  132, 
/* 3970 */    30,   91,  137,   30,    4,   80,    4,  117,    4,  132, 
/* 3980 */    80,    4,  140,   30,    4,   80,    4,  117,    4,  132, 
/* 3990 */    30,   91,  137,   30,    4,  117,    4,  132,  193,  129, 
/* 4000 */    30,    4,  117,    4,  132,   30,   91,  137,   30,    4, 
/* 4010 */   117,    4,  132,  193,  129,   44,    7,  129,   40,   13, 
/* 4020 */   129,   37,   17,  129,   35,   20,  129,   34,   22,  129, 
/* 4030 */    33,   23,  129,   32,   24,  129,   32,   23,  129,   31, 
/* 4040 */     6,   41,   13,  129,   31,    5,   42,   11,  129,   30, 
/* 4050 */     5,   44,    7,  129,   30,    4,  132,   30,    5,  130, 
/* 4060 */    31,    5,  129,   31,    6,  117,    4,  129,   31,    8, 
/* 4070 */   117,    4,  129,   32,    9,  117,    4,  129,   33,   11, 
/* 4080 */   117,    4,  129,   34,   87,  129,   35,   86,  129,   36, 
/* 4090 */    85,  129,   37,   84,  129,   38,   83,  129,   40,   81, 
/* 4100 */   129,   42,   79,  129,   45,   76,  129,   50,   71,  129, 
/* 4110 */   117,    4,  132,  193,  129,   30,    4,  117,    4,  132, 
/* 4120 */    30,   91,  137,   30,    4,   76,    8,  117,    4,  129, 
/* 4130 */    30,    4,   73,   13,  117,    4,  129,   30,    4,   70, 
/* 4140 */    18,  117,    4,  129,   30,    4,   67,   23,  117,    4, 
/* 4150 */   129,   65,   26,  129,   62,   31,  129,   59,   35,  129, 
/* 4160 */    56,   29,   89,    7,  129,   53,   29,   91,    7,  129, 
/* 4170 */    50,   29,   93,    7,  129,   47,   29,   95,    6,  129, 
/* 4180 */    30,    4,   45,   29,   96,    7,  129,   30,    4,   42, 
/* 4190 */    29,   98,    7,  129,   30,    4,   39,   30,  100,    6, 
/* 4200 */   129,   30,    4,   36,   30,  101,    7,  129,   30,   33, 
/* 4210 */   103,    7,  117,    4,  129,   30,   30,  105,    6,  117, 
/* 4220 */     4,  129,   30,   27,  106,    7,  117,    4,  129,   30, 
/* 4230 */    25,  108,    7,  117,    4,  129,   30,   22,  110,   11, 
/* 4240 */   129,   30,   19,  111,   10,  129,   30,   16,  113,    8, 
/* 4250 */   129,   30,   13,  115,    6,  129,   30,   11,  116,    5, 
/* 4260 */   129,   30,    8,  117,    4,  129,   30,    5,  117,    4, 
/* 4270 */   129,   30,    4,  117,    4,  130,   30,    4,  130,  193, 
/* 4280 */   129,   30,    4,  117,    4,  132,   30,   91,  137,   30, 
/* 4290 */     4,  117,    4,  132,   30,    4,  144,   30,    5,  130, 
/* 4300 */    30,    7,  129,   30,    8,  129,   30,   11,  129,   30, 
/* 4310 */    18,  132,  193,  129,   30,    4,  117,    4,  132,   30, 
/* 4320 */    91,  132,   30,    4,  103,   18,  129,   30,    4,   97, 
/* 4330 */    24,  129,   30,    4,   92,   29,  129,   30,    4,   87, 
/* 4340 */    34,  129,   81,   40,  129,   76,   45,  129,   70,   49, 
/* 4350 */   129,   65,   49,  129,   60,   49,  129,   55,   49,  129, 
/* 4360 */    50,   48,  129,   44,   49,  129,   39,   48,  129,   33, 
/* 4370 */    49,  129,   30,   47,  129,   34,   37,  129,   40,   26, 
/* 4380 */   129,   46,   19,  129,   52,   19,  129,   58,   19,  129, 
/* 4390 */    64,   19,  129,   70,   19,  129,   76,   19,  129,   82, 
/* 4400 */    19,  129,   30,    4,   88,   18,  129,   30,    4,   94, 
/* 4410 */    18,  129,   30,    4,  100,   18,  129,   30,    4,  106, 
/* 4420 */    15,  129,   30,   91,  137,   30,    4,  117,    4,  132, 
/* 4430 */   193,  129,   30,    4,  117,    4,  132,   30,   91,  132, 
/* 4440 */    30,    4,  107,   14,  129,   30,    4,  104,   17,  129, 
/* 4450 */    30,    4,  101,   20,  129,   30,    4,   99,   22,  129, 
/* 4460 */    96,   25,  129,   93,   28,  129,   91,   28,  129,   88, 
/* 4470 */    29,  129,   85,   29,  129,   82,   29,  129,   79,   29, 
/* 4480 */   129,   76,   29,  129,   74,   29,  129,   71,   29,  129, 
/* 4490 */    68,   29,  129,   65,   29,  129,   62,   29,  129,   60, 
/* 4500 */    29,  129,   57,   29,  129,   54,   29,  129,   51,   29, 
/* 4510 */   129,   49,   28,  129,   46,   29,  129,   43,   29,  129, 
/* 4520 */    40,   29,  117,    4,  129,   37,   29,  117,    4,  129, 
/* 4530 */    35,   29,  117,    4,  129,   32,   29,  117,    4,  129, 
/* 4540 */    30,   91,  132,  117,    4,  132,  193,  129,   63,   25, 
/* 4550 */   129,   57,   37,  129,   53,   45,  129,   50,   51,  129, 
/* 4560 */    47,   57,  129,   45,   61,  129,   43,   65,  129,   41, 
/* 4570 */    69,  129,   39,   73,  129,   38,   21,   92,   21,  129, 
/* 4580 */    36,   18,   97,   18,  129,   35,   14,  102,   14,  129, 
/* 4590 */    34,   11,  106,   11,  129,   33,   10,  108,   10,  129, 
/* 4600 */    32,    8,  111,    8,  129,   32,    6,  113,    6,  129, 
/* 4610 */    31,    6,  114,    6,  129,   31,    5,  115,    5,  129, 
/* 4620 */    30,    5,  116,    5,  130,   30,    4,  117,    4,  132, 
/* 4630 */    30,    5,  116,    5,  130,   31,    5,  115,    5,  129, 
/* 4640 */    31,    6,  114,    6,  129,   32,    6,  113,    6,  129, 
/* 4650 */    32,    8,  111,    8,  129,   33,   10,  108,   10,  129, 
/* 4660 */    34,   11,  106,   11,  129,   35,   14,  102,   14,  129, 
/* 4670 */    36,   18,   97,   18,  129,   38,   21,   92,   21,  129, 
/* 4680 */    39,   73,  129,   41,   69,  129,   43,   65,  129,   45, 
/* 4690 */    61,  129,   47,   57,  129,   50,   51,  129,   53,   45, 
/* 4700 */   129,   57,   37,  129,   63,   25,  129,  193,  129,   30, 
/* 4710 */     4,  117,    4,  132,   30,   91,  137,   30,    4,   80, 
/* 4720 */     4,  117,    4,  132,   80,    4,  117,    4,  134,   80, 
/* 4730 */     5,  116,    5,  131,   80,    6,  115,    6,  130,   81, 
/* 4740 */     6,  114,    6,  129,   81,    8,  112,    8,  129,   81, 
/* 4750 */     9,  111,    9,  129,   82,   10,  109,   10,  129,   82, 
/* 4760 */    13,  106,   13,  129,   83,   35,  129,   84,   33,  129, 
/* 4770 */    85,   31,  129,   86,   29,  129,   88,   25,  129,   90, 
/* 4780 */    21,  129,   93,   15,  129,   96,    9,  129,  193,  129, 
/* 4790 */    63,   25,  129,   57,   37,  129,   53,   45,  129,   50, 
/* 4800 */    51,  129,   47,   57,  129,   45,   61,  129,   43,   65, 
/* 4810 */   129,   41,   69,  129,   39,   73,  129,   38,   21,   92, 
/* 4820 */    21,  129,   36,   18,   97,   18,  129,   35,   14,  102, 
/* 4830 */    14,  129,   34,   11,  106,   11,  129,   33,   10,  108, 
/* 4840 */    10,  129,   32,    8,  111,    8,  129,   32,    6,  113, 
/* 4850 */     6,  129,   31,    6,  114,    6,  129,   31,    5,  115, 
/* 4860 */     5,  129,   30,    5,  116,    5,  130,   30,    4,   39, 
/* 4870 */     2,  117,    4,  129,   30,    4,   40,    4,  117,    4, 
/* 4880 */   129,   30,    4,   41,    5,  117,    4,  129,   30,    4, 
/* 4890 */    41,    6,  117,    4,  129,   30,    5,   40,    8,  116, 
/* 4900 */     5,  129,   30,    5,   39,   10,  116,    5,  129,   31, 
/* 4910 */     5,   38,   11,  115,    5,  129,   31,   18,  114,    6, 
/* 4920 */   129,   32,   17,  113,    6,  129,   32,   16,  111,    8, 
/* 4930 */   129,   33,   15,  108,   10,  129,   33,   14,  106,   11, 
/* 4940 */   129,   32,   17,  102,   14,  129,   31,   23,   97,   18, 
/* 4950 */   129,   31,   28,   92,   21,  129,   30,   82,  129,   30, 
/* 4960 */    80,  129,   30,   11,   43,   65,  129,   30,   10,   45, 
/* 4970 */    61,  129,   31,    8,   47,   57,  129,   32,    6,   50, 
/* 4980 */    51,  129,   33,    5,   53,   45,  129,   35,    4,   57, 
/* 4990 */    37,  129,   38,    2,   63,   25,  129,  193,  129,   30, 
/* 5000 */     4,  117,    4,  132,   30,   91,  137,   30,    4,   76, 
/* 5010 */     8,  117,    4,  129,   30,    4,   73,   11,  117,    4, 
/* 5020 */   129,   30,    4,   70,   14,  117,    4,  129,   30,    4, 
/* 5030 */    67,   17,  117,    4,  129,   65,   19,  117,    4,  129, 
/* 5040 */    62,   22,  117,    4,  129,   59,   25,  117,    4,  129, 
/* 5050 */    56,   28,  117,    4,  129,   53,   31,  117,    4,  129, 
/* 5060 */    50,   34,  117,    4,  129,   47,   29,   80,    5,  116, 
/* 5070 */     5,  129,   30,    4,   45,   29,   80,    5,  116,    5, 
/* 5080 */   129,   30,    4,   42,   29,   80,    5,  116,    5,  129, 
/* 5090 */    30,    4,   39,   30,   80,    6,  115,    6,  129,   30, 
/* 5100 */     4,   36,   30,   80,    6,  115,    6,  129,   30,   33, 
/* 5110 */    81,    6,  114,    6,  129,   30,   30,   81,    8,  112, 
/* 5120 */     8,  129,   30,   27,   81,    9,  111,    9,  129,   30, 
/* 5130 */    25,   82,   10,  109,   10,  129,   30,   22,   82,   13, 
/* 5140 */   106,   13,  129,   30,   19,   83,   35,  129,   30,   16, 
/* 5150 */    84,   33,  129,   30,   13,   85,   31,  129,   30,   11, 
/* 5160 */    86,   29,  129,   30,    8,   88,   25,  129,   30,    5, 
/* 5170 */    90,   21,  129,   30,    4,   93,   15,  129,   30,    4, 
/* 5180 */    96,    9,  129,   30,    4,  130,  193,  129,   30,   18, 
/* 5190 */   130,   30,   18,   89,   15,  129,   30,   18,   85,   23, 
/* 5200 */   129,   34,   11,   83,   27,  129,   34,    9,   81,   31, 
/* 5210 */   129,   33,    8,   79,   35,  129,   33,    6,   78,   16, 
/* 5220 */   106,    9,  129,   32,    6,   77,   15,  109,    7,  129, 
/* 5230 */    32,    5,   76,   14,  111,    6,  129,   31,    5,   75, 
/* 5240 */    14,  113,    5,  129,   31,    4,   74,   15,  114,    5, 
/* 5250 */   129,   31,    4,   74,   14,  115,    4,  129,   30,    4, 
/* 5260 */    73,   15,  116,    4,  129,   30,    4,   73,   14,  116, 
/* 5270 */     4,  129,   30,    4,   73,   14,  117,    4,  129,   30, 
/* 5280 */     4,   72,   15,  117,    4,  130,   30,    4,   71,   15, 
/* 5290 */   117,    4,  130,   30,    4,   70,   15,  117,    4,  129, 
/* 5300 */    30,    5,   70,   15,  117,    4,  129,   30,    5,   69, 
/* 5310 */    15,  116,    5,  129,   30,    6,   68,   16,  115,    5, 
/* 5320 */   129,   31,    6,   67,   16,  114,    6,  129,   31,    7, 
/* 5330 */    66,   17,  113,    6,  129,   32,    7,   64,   18,  111, 
/* 5340 */     8,  129,   32,    8,   62,   19,  109,    9,  129,   33, 
/* 5350 */     9,   60,   20,  107,   10,  129,   34,   11,   57,   22, 
/* 5360 */   103,   13,  129,   35,   43,  103,   18,  129,   36,   41, 
/* 5370 */   103,   18,  129,   38,   38,  103,   18,  129,   39,   35, 
/* 5380 */   103,   18,  129,   41,   31,  129,   43,   27,  129,   46, 
/* 5390 */    22,  129,   49,   14,  129,  193,  129,  103,   18,  132, 
/* 5400 */   110,   11,  129,  113,    8,  129,  114,    7,  129,  116, 
/* 5410 */     5,  130,  117,    4,  132,   30,    4,  117,    4,  132, 
/* 5420 */    30,   91,  137,   30,    4,  117,    4,  132,  117,    4, 
/* 5430 */   132,  116,    5,  130,  114,    7,  129,  113,    8,  129, 
/* 5440 */   110,   11,  129,  103,   18,  132,  193,  129,  117,    4, 
/* 5450 */   132,   56,   65,  129,   50,   71,  129,   46,   75,  129, 
/* 5460 */    44,   77,  129,   42,   79,  129,   40,   81,  129,   38, 
/* 5470 */    83,  129,   36,   85,  129,   35,   86,  129,   34,   20, 
/* 5480 */   117,    4,  129,   33,   17,  117,    4,  129,   32,   15, 
/* 5490 */   117,    4,  129,   32,   13,  117,    4,  129,   31,   12, 
/* 5500 */   129,   31,   10,  129,   31,    9,  129,   30,    9,  129, 
/* 5510 */    30,    8,  130,   30,    7,  132,   31,    6,  130,   31, 
/* 5520 */     7,  129,   32,    6,  129,   32,    7,  129,   33,    7, 
/* 5530 */   129,   34,    7,  129,   35,    8,  129,   36,    9,  117, 
/* 5540 */     4,  129,   38,    9,  117,    4,  129,   40,   10,  117, 
/* 5550 */     4,  129,   42,   12,  117,    4,  129,   44,   77,  129, 
/* 5560 */    46,   75,  129,   50,   71,  129,   56,   43,  100,   21, 
/* 5570 */   129,  117,    4,  132,  193,  129,  117,    4,  132,  115, 
/* 5580 */     6,  129,  110,   11,  129,  105,   16,  129,  101,   20, 
/* 5590 */   129,   96,   25,  129,   92,   29,  129,   87,   34,  129, 
/* 5600 */    83,   38,  129,   78,   43,  129,   74,   47,  129,   70, 
/* 5610 */    42,  117,    4,  129,   65,   42,  117,    4,  129,   60, 
/* 5620 */    43,  117,    4,  129,   56,   42,  129,   51,   42,  129, 
/* 5630 */    46,   43,  129,   42,   43,  129,   37,   44,  129,   33, 
/* 5640 */    43,  129,   30,   42,  129,   33,   34,  129,   38,   25, 
/* 5650 */   129,   42,   16,  129,   47,   15,  129,   52,   15,  129, 
/* 5660 */    57,   15,  129,   61,   16,  129,   66,   16,  129,   71, 
/* 5670 */    16,  129,   76,   16,  129,   80,   16,  129,   85,   16, 
/* 5680 */   117,    4,  129,   90,   16,  117,    4,  129,   95,   16, 
/* 5690 */   117,    4,  129,  100,   21,  129,  105,   16,  129,  110, 
/* 5700 */    11,  129,  114,    7,  129,  117,    4,  132,  193,  129, 
/* 5710 */   117,    4,  132,  115,    6,  129,  110,   11,  129,  105, 
/* 5720 */    16,  129,  101,   20,  129,   96,   25,  129,   92,   29, 
/* 5730 */   129,   87,   34,  129,   83,   38,  129,   78,   43,  129, 
/* 5740 */    74,   47,  129,   70,   42,  117,    4,  129,   65,   42, 
/* 5750 */   117,    4,  129,   60,   43,  117,    4,  129,   56,   42, 
/* 5760 */   129,   51,   42,  129,   46,   43,  129,   42,   43,  129, 
/* 5770 */    37,   44,  129,   33,   43,  129,   30,   42,  129,   33, 
/* 5780 */    34,  129,   38,   25,  129,   42,   16,  129,   47,   15, 
/* 5790 */   129,   52,   15,  129,   57,   15,  129,   61,   16,  129, 
/* 5800 */    65,   17,  129,   60,   27,  129,   56,   36,  129,   51, 
/* 5810 */    42,  129,   46,   43,  129,   42,   43,  129,   37,   44, 
/* 5820 */   129,   33,   43,  129,   30,   42,  129,   33,   34,  129, 
/* 5830 */    38,   25,  129,   42,   16,  129,   47,   15,  129,   52, 
/* 5840 */    15,  129,   57,   15,  129,   61,   16,  129,   66,   16, 
/* 5850 */   129,   71,   16,  129,   76,   16,  129,   80,   16,  129, 
/* 5860 */    85,   16,  117,    4,  129,   90,   16,  117,    4,  129, 
/* 5870 */    95,   16,  117,    4,  129,  100,   21,  129,  105,   16, 
/* 5880 */   129,  110,   11,  129,  114,    7,  129,  117,    4,  132, 
/* 5890 */   193,  129,   30,    4,  117,    4,  132,   30,    4,  115, 
/* 5900 */     6,  129,   30,    4,  112,    9,  129,   30,    6,  109, 
/* 5910 */    12,  129,   30,    9,  106,   15,  129,   30,   11,  103, 
/* 5920 */    18,  129,   30,   14,  100,   21,  129,   30,    4,   38, 
/* 5930 */     9,   98,   23,  129,   30,    4,   40,   10,   95,   26, 
/* 5940 */   129,   30,    4,   43,    9,   92,   29,  129,   46,    9, 
/* 5950 */    89,   32,  129,   49,    8,   86,   28,  117,    4,  129, 
/* 5960 */    51,    9,   83,   28,  117,    4,  129,   54,    9,   80, 
/* 5970 */    28,  117,    4,  129,   57,    8,   77,   28,  117,    4, 
/* 5980 */   129,   59,    9,   74,   28,  129,   62,   37,  129,   64, 
/* 5990 */    33,  129,   66,   28,  129,   63,   28,  129,   60,   28, 
/* 6000 */   129,   57,   28,  129,   54,   33,  129,   51,   39,  129, 
/* 6010 */    48,   29,   83,    9,  129,   30,    4,   45,   29,   86, 
/* 6020 */     9,  129,   30,    4,   42,   29,   89,    9,  129,   30, 
/* 6030 */     4,   39,   29,   92,    8,  129,   30,    4,   36,   29, 
/* 6040 */    94,    9,  129,   30,   32,   97,    9,  129,   30,   29, 
/* 6050 */   100,    8,  117,    4,  129,   30,   26,  103,    8,  117, 
/* 6060 */     4,  129,   30,   23,  105,    9,  117,    4,  129,   30, 
/* 6070 */    20,  108,   13,  129,   30,   18,  111,   10,  129,   30, 
/* 6080 */    15,  113,    8,  129,   30,   12,  116,    5,  129,   30, 
/* 6090 */     9,  117,    4,  129,   30,    6,  117,    4,  129,   30, 
/* 6100 */     4,  117,    4,  132,  193,  129,  117,    4,  132,  114, 
/* 6110 */     7,  129,  111,   10,  129,  108,   13,  129,  105,   16, 
/* 6120 */   129,  102,   19,  129,  100,   21,  129,   96,   25,  129, 
/* 6130 */    93,   28,  129,   90,   31,  129,   87,   34,  129,   84, 
/* 6140 */    30,  117,    4,  129,   30,    4,   81,   30,  117,    4, 
/* 6150 */   129,   30,    4,   78,   30,  117,    4,  129,   30,    4, 
/* 6160 */    75,   30,  117,    4,  129,   30,    4,   72,   30,  129, 
/* 6170 */    30,   69,  129,   30,   66,  129,   30,   63,  129,   30, 
/* 6180 */    60,  129,   30,   57,  129,   30,   54,  129,   30,   51, 
/* 6190 */   129,   30,   48,  129,   30,   51,  129,   30,    4,   73, 
/* 6200 */    12,  129,   30,    4,   76,   12,  129,   30,    4,   80, 
/* 6210 */    12,  129,   30,    4,   83,   12,  129,   87,   12,  129, 
/* 6220 */    90,   12,  117,    4,  129,   94,   11,  117,    4,  129, 
/* 6230 */    97,   12,  117,    4,  129,  101,   12,  117,    4,  129, 
/* 6240 */   104,   17,  129,  108,   13,  129,  111,   10,  129,  115, 
/* 6250 */     6,  129,  117,    4,  134,  193,  129,   30,    1,  103, 
/* 6260 */    18,  129,   30,    4,  103,   18,  129,   30,    7,  103, 
/* 6270 */    18,  129,   30,    9,  103,   18,  129,   30,   12,  110, 
/* 6280 */    11,  129,   30,   15,  113,    8,  129,   30,   18,  114, 
/* 6290 */     7,  129,   30,   21,  116,    5,  129,   30,   24,  116, 
/* 6300 */     5,  129,   30,   27,  117,    4,  129,   30,   30,  117, 
/* 6310 */     4,  129,   30,   33,  117,    4,  129,   30,    4,   37, 
/* 6320 */    28,  117,    4,  129,   30,    4,   40,   28,  117,    4, 
/* 6330 */   129,   30,    4,   42,   29,  117,    4,  129,   30,    4, 
/* 6340 */    45,   29,  117,    4,  129,   30,    4,   48,   29,  117, 
/* 6350 */     4,  129,   30,    4,   51,   29,  117,    4,  129,   30, 
/* 6360 */     4,   54,   29,  117,    4,  129,   30,    4,   57,   29, 
/* 6370 */   117,    4,  129,   30,    4,   59,   30,  117,    4,  129, 
/* 6380 */    30,    4,   62,   30,  117,    4,  129,   30,    4,   65, 
/* 6390 */    30,  117,    4,  129,   30,    4,   68,   30,  117,    4, 
/* 6400 */   129,   30,    4,   71,   30,  117,    4,  129,   30,    4, 
/* 6410 */    74,   30,  117,    4,  129,   30,    4,   77,   30,  117, 
/* 6420 */     4,  129,   30,    4,   80,   30,  117,    4,  129,   30, 
/* 6430 */     4,   83,   30,  117,    4,  129,   30,    4,   86,   35, 
/* 6440 */   129,   30,    4,   89,   32,  129,   30,    4,   91,   30, 
/* 6450 */   129,   30,    4,   94,   27,  129,   30,    5,   97,   24, 
/* 6460 */   129,   30,    5,  100,   21,  129,   30,    7,  103,   18, 
/* 6470 */   129,   30,    8,  106,   15,  129,   30,   11,  109,   12, 
/* 6480 */   129,   30,   18,  112,    9,  129,   30,   18,  115,    6, 
/* 6490 */   129,   30,   18,  117,    4,  129,   30,   18,  120,    1, 
/* 6500 */   129,  193,  129,   42,    8,  129,   38,   16,  129,   36, 
/* 6510 */    20,  129,   34,   24,   71,    5,  129,   33,   26,   69, 
/* 6520 */    10,  129,   32,   28,   68,   13,  129,   31,   30,   68, 
/* 6530 */    14,  129,   31,    9,   52,    9,   68,   15,  129,   30, 
/* 6540 */     8,   54,    8,   69,   14,  129,   30,    7,   55,    7, 
/* 6550 */    71,    4,   78,    6,  129,   30,    6,   56,    6,   79, 
/* 6560 */     5,  129,   30,    6,   56,    6,   80,    4,  130,   31, 
/* 6570 */     5,   56,    5,   80,    4,  129,   31,    5,   56,    5, 
/* 6580 */    79,    5,  129,   32,    5,   55,    5,   78,    6,  129, 
/* 6590 */    33,    5,   54,    5,   77,    7,  129,   34,    6,   52, 
/* 6600 */     6,   74,    9,  129,   35,   48,  129,   33,   49,  129, 
/* 6610 */    32,   49,  129,   31,   49,  129,   30,   49,  129,   30, 
/* 6620 */    47,  129,   30,   45,  129,   30,   41,  129,   30,    6, 
/* 6630 */   129,   30,    4,  129,   30,    3,  129,   30,    2,  129, 
/* 6640 */   193,  129,   30,    4,  117,    4,  130,   31,   90,  136, 
/* 6650 */    37,    5,   72,    5,  129,   35,    5,   74,    5,  129, 
/* 6660 */    33,    5,   76,    5,  129,   32,    5,   77,    5,  129, 
/* 6670 */    31,    5,   78,    5,  129,   31,    4,   79,    4,  129, 
/* 6680 */    30,    5,   79,    5,  131,   30,    6,   78,    6,  129, 
/* 6690 */    30,    7,   77,    7,  129,   31,    8,   75,    8,  129, 
/* 6700 */    31,   11,   72,   11,  129,   32,   15,   67,   15,  129, 
/* 6710 */    33,   48,  129,   34,   46,  129,   35,   44,  129,   37, 
/* 6720 */    40,  129,   39,   36,  129,   42,   30,  129,   46,   22, 
/* 6730 */   129,  193,  129,   48,   18,  129,   43,   28,  129,   41, 
/* 6740 */    32,  129,   39,   36,  129,   37,   40,  129,   35,   44, 
/* 6750 */   129,   34,   46,  129,   33,   13,   68,   13,  129,   32, 
/* 6760 */     9,   73,    9,  129,   32,    7,   75,    7,  129,   31, 
/* 6770 */     6,   77,    6,  129,   31,    5,   78,    5,  129,   30, 
/* 6780 */     5,   79,    5,  129,   30,    4,   80,    4,  133,   31, 
/* 6790 */     3,   79,    4,  129,   31,    4,   79,    4,  129,   32, 
/* 6800 */     3,   78,    4,  129,   32,    4,   76,    6,  129,   33, 
/* 6810 */     4,   74,    7,  129,   34,    4,   72,    8,  129,   35, 
/* 6820 */     5,   72,    7,  129,   37,    5,   73,    4,  129,   39, 
/* 6830 */     4,   74,    1,  129,  129,  193,  129,   46,   22,  129, 
/* 6840 */    42,   30,  129,   39,   36,  129,   37,   40,  129,   35, 
/* 6850 */    44,  129,   34,   46,  129,   33,   48,  129,   32,   15, 
/* 6860 */    67,   15,  129,   31,   11,   72,   11,  129,   31,    8, 
/* 6870 */    75,    8,  129,   30,    7,   77,    7,  129,   30,    6, 
/* 6880 */    78,    6,  129,   30,    5,   79,    5,  131,   31,    4, 
/* 6890 */    79,    4,  129,   31,    5,   78,    5,  129,   32,    5, 
/* 6900 */    77,    5,  129,   33,    5,   76,    5,  129,   35,    5, 
/* 6910 */    74,    5,  117,    4,  129,   37,    5,   72,    5,  117, 
/* 6920 */     4,  129,   30,   91,  136,   30,    4,  130,  193,  129, 
/* 6930 */    48,   18,  129,   43,   28,  129,   41,   32,  129,   39, 
/* 6940 */    36,  129,   37,   40,  129,   35,   44,  129,   34,   46, 
/* 6950 */   129,   33,   13,   55,    4,   68,   13,  129,   32,    9, 
/* 6960 */    55,    4,   73,    9,  129,   32,    7,   55,    4,   75, 
/* 6970 */     7,  129,   31,    6,   55,    4,   77,    6,  129,   31, 
/* 6980 */     5,   55,    4,   78,    5,  129,   30,    5,   55,    4, 
/* 6990 */    79,    5,  129,   30,    4,   55,    4,   80,    4,  132, 
/* 7000 */    30,    4,   55,    4,   79,    5,  129,   31,    3,   55, 
/* 7010 */     4,   78,    5,  129,   31,    4,   55,    4,   77,    6, 
/* 7020 */   129,   32,    3,   55,    4,   75,    7,  129,   32,    4, 
/* 7030 */    55,    4,   73,    9,  129,   33,    4,   55,    4,   68, 
/* 7040 */    13,  129,   34,    4,   55,   25,  129,   35,    5,   55, 
/* 7050 */    24,  129,   37,    5,   55,   22,  129,   39,    4,   55, 
/* 7060 */    20,  129,   55,   18,  129,   55,   16,  129,   55,   11, 
/* 7070 */   129,  193,  129,   80,    4,  129,   30,    4,   80,    4, 
/* 7080 */   130,   30,   78,  129,   30,   82,  129,   30,   85,  129, 
/* 7090 */    30,   87,  129,   30,   88,  129,   30,   89,  129,   30, 
/* 7100 */    90,  130,   30,    4,   80,    4,  115,    6,  129,   30, 
/* 7110 */     4,   80,    4,  117,    4,  129,   80,    4,  105,    6, 
/* 7120 */   117,    4,  129,   80,    4,  103,   10,  116,    5,  129, 
/* 7130 */    80,    4,  102,   19,  129,   80,    4,  101,   19,  129, 
/* 7140 */   101,   19,  129,  101,   18,  129,  102,   16,  129,  103, 
/* 7150 */    12,  129,  105,    6,  129,  193,  129,   12,   10,   59, 
/* 7160 */    11,  129,    9,   16,   55,   19,  129,    7,   20,   53, 
/* 7170 */    23,  129,    6,    7,   23,    5,   32,    6,   51,   27, 
/* 7180 */   129,    4,    7,   25,   16,   50,   29,  129,    3,    6, 
/* 7190 */    27,   16,   49,   31,  129,    2,    6,   28,   16,   48, 
/* 7200 */    33,  129,    1,    6,   27,   18,   47,   35,  129,    1, 
/* 7210 */     6,   27,   31,   71,   12,  129,    1,    5,   26,   15, 
/* 7220 */    44,   10,   75,    8,  129,    1,    5,   25,   14,   45, 
/* 7230 */     7,   77,    7,  129,    1,    5,   25,   13,   45,    5, 
/* 7240 */    79,    5,  129,    1,    5,   24,   14,   45,    4,   80, 
/* 7250 */     4,  129,    1,    5,   24,   13,   45,    4,   80,    4, 
/* 7260 */   129,    1,    5,   23,   14,   45,    4,   80,    4,  129, 
/* 7270 */     1,    5,   23,   13,   45,    4,   80,    4,  129,    1, 
/* 7280 */     6,   22,   13,   45,    5,   79,    5,  129,    1,    6, 
/* 7290 */    21,   14,   45,    7,   77,    7,  129,    1,    7,   21, 
/* 7300 */    13,   46,    8,   75,    8,  129,    1,    8,   20,   13, 
/* 7310 */    46,   12,   71,   12,  129,    1,   10,   18,   15,   47, 
/* 7320 */    35,  129,    2,   30,   48,   33,  129,    3,   29,   49, 
/* 7330 */    32,  129,    4,   27,   50,   31,  129,    5,   25,   51, 
/* 7340 */    27,   80,    2,   86,    4,  129,    7,   21,   53,   23, 
/* 7350 */    80,    3,   85,    6,  129,    9,   17,   55,   19,   80, 
/* 7360 */    12,  129,   12,   12,   59,   11,   81,   11,  129,   82, 
/* 7370 */    10,  129,   84,    7,  129,   86,    4,  129,  193,  129, 
/* 7380 */    30,    4,  117,    4,  130,   30,   91,  136,   30,    4, 
/* 7390 */    72,    5,  129,   30,    4,   74,    5,  129,   75,    5, 
/* 7400 */   129,   76,    5,  129,   76,    6,  129,   77,    6,  130, 
/* 7410 */    77,    7,  130,   76,    8,  129,   30,    4,   75,    9, 
/* 7420 */   129,   30,    4,   72,   12,  129,   30,   54,  129,   30, 
/* 7430 */    53,  130,   30,   52,  129,   30,   51,  129,   30,   49, 
/* 7440 */   129,   30,   46,  129,   30,   42,  129,   30,    4,  130, 
/* 7450 */   193,  129,   30,    4,   80,    4,  129,   30,    4,   80, 
/* 7460 */     4,  100,    6,  129,   30,   54,   98,   10,  129,   30, 
/* 7470 */    54,   97,   12,  129,   30,   54,   96,   14,  131,   30, 
/* 7480 */    54,   97,   12,  129,   30,   54,   98,   10,  129,   30, 
/* 7490 */    54,  100,    6,  129,   30,    4,  130,  193,  129,    7, 
/* 7500 */     6,  129,    4,   11,  129,    3,   13,  129,    2,   14, 
/* 7510 */   129,    1,   15,  130,    1,    3,    6,    9,  129,    1, 
/* 7520 */     3,    7,    6,  129,    1,    3,  130,    1,    4,  129, 
/* 7530 */     1,    5,   80,    4,  129,    1,    7,   80,    4,  100, 
/* 7540 */     6,  129,    2,   82,   98,   10,  129,    3,   81,   97, 
/* 7550 */    12,  129,    4,   80,   96,   14,  129,    5,   79,   96, 
/* 7560 */    14,  129,    7,   77,   96,   14,  129,   10,   74,   97, 
/* 7570 */    12,  129,   14,   70,   98,   10,  129,   19,   65,  100, 
/* 7580 */     6,  129,  193,  129,   30,    4,  117,    4,  130,   30, 
/* 7590 */    91,  136,   30,    4,   57,    9,  129,   30,    4,   55, 
/* 7600 */    12,  129,   52,   17,  129,   50,   20,  129,   48,   24, 
/* 7610 */   129,   46,   27,  129,   44,   21,   69,    6,  129,   41, 
/* 7620 */    22,   70,    6,   80,    4,  129,   30,    4,   39,   21, 
/* 7630 */    72,    6,   80,    4,  129,   30,    4,   36,   22,   73, 
/* 7640 */    11,  129,   30,   26,   75,    9,  129,   30,   23,   76, 
/* 7650 */     8,  129,   30,   21,   78,    6,  129,   30,   19,   79, 
/* 7660 */     5,  129,   30,   16,   80,    4,  129,   30,   14,   80, 
/* 7670 */     4,  129,   30,   12,  129,   30,   10,  129,   30,    7, 
/* 7680 */   129,   30,    5,  129,   30,    4,  130,  193,  129,   30, 
/* 7690 */     4,  117,    4,  130,   30,   91,  136,   30,    4,  130, 
/* 7700 */   193,  129,   30,    4,   80,    4,  130,   30,   54,  136, 
/* 7710 */    30,    4,   72,    5,  129,   30,    4,   74,    5,  129, 
/* 7720 */    75,    5,  129,   76,    5,  129,   30,    4,   75,    7, 
/* 7730 */   129,   30,    4,   74,    9,  129,   30,   54,  132,   30, 
/* 7740 */    53,  129,   30,   52,  129,   30,   51,  129,   30,   48, 
/* 7750 */   129,   30,    4,   72,    5,  129,   30,    4,   74,    5, 
/* 7760 */   129,   75,    5,  129,   76,    5,  129,   30,    4,   75, 
/* 7770 */     7,  129,   30,    4,   74,    9,  129,   30,   54,  132, 
/* 7780 */    30,   53,  129,   30,   52,  129,   30,   51,  129,   30, 
/* 7790 */    48,  129,   30,    4,  130,  193,  129,   30,    4,   80, 
/* 7800 */     4,  130,   30,   54,  136,   30,    4,   72,    5,  129, 
/* 7810 */    30,    4,   74,    5,  129,   75,    5,  129,   76,    5, 
/* 7820 */   129,   76,    6,  129,   77,    6,  130,   77,    7,  130, 
/* 7830 */    76,    8,  129,   30,    4,   75,    9,  129,   30,    4, 
/* 7840 */    72,   12,  129,   30,   54,  129,   30,   53,  130,   30, 
/* 7850 */    52,  129,   30,   51,  129,   30,   49,  129,   30,   46, 
/* 7860 */   129,   30,   42,  129,   30,    4,  130,  193,  129,   48, 
/* 7870 */    18,  129,   43,   28,  129,   41,   32,  129,   39,   36, 
/* 7880 */   129,   37,   40,  129,   35,   44,  129,   34,   46,  129, 
/* 7890 */    33,   13,   68,   13,  129,   32,    9,   73,    9,  129, 
/* 7900 */    32,    7,   75,    7,  129,   31,    6,   77,    6,  129, 
/* 7910 */    31,    5,   78,    5,  129,   30,    5,   79,    5,  129, 
/* 7920 */    30,    4,   80,    4,  132,   30,    5,   79,    5,  130, 
/* 7930 */    31,    5,   78,    5,  129,   31,    6,   77,    6,  129, 
/* 7940 */    32,    7,   75,    7,  129,   32,    9,   73,    9,  129, 
/* 7950 */    33,   13,   68,   13,  129,   34,   46,  129,   35,   44, 
/* 7960 */   129,   37,   40,  129,   39,   36,  129,   41,   32,  129, 
/* 7970 */    43,   28,  129,   48,   18,  129,  193,  129,    1,    3, 
/* 7980 */    80,    4,  130,    1,   83,  137,   37,    5,   72,    5, 
/* 7990 */   129,   35,    5,   74,    5,  129,   33,    5,   76,    5, 
/* 8000 */   129,   32,    5,   77,    5,  129,   31,    5,   78,    5, 
/* 8010 */   129,   31,    4,   79,    4,  129,   30,    5,   79,    5, 
/* 8020 */   131,   30,    6,   78,    6,  129,   30,    7,   77,    7, 
/* 8030 */   129,   31,    8,   75,    8,  129,   31,   11,   72,   11, 
/* 8040 */   129,   32,   15,   67,   15,  129,   33,   48,  129,   34, 
/* 8050 */    46,  129,   35,   44,  129,   37,   40,  129,   39,   36, 
/* 8060 */   129,   42,   30,  129,   46,   22,  129,  193,  129,   46, 
/* 8070 */    22,  129,   42,   30,  129,   39,   36,  129,   37,   40, 
/* 8080 */   129,   35,   44,  129,   34,   46,  129,   33,   48,  129, 
/* 8090 */    32,   15,   67,   15,  129,   31,   11,   72,   11,  129, 
/* 8100 */    31,    8,   75,    8,  129,   30,    7,   77,    7,  129, 
/* 8110 */    30,    6,   78,    6,  129,   30,    5,   79,    5,  131, 
/* 8120 */    31,    4,   79,    4,  129,   31,    5,   78,    5,  129, 
/* 8130 */    32,    5,   77,    5,  129,   33,    5,   76,    5,  129, 
/* 8140 */    35,    5,   74,    5,  129,   37,    5,   72,    5,  129, 
/* 8150 */     1,   83,  136,    1,    3,   80,    4,  130,  193,  129, 
/* 8160 */    30,    4,   80,    4,  130,   30,   54,  136,   30,    4, 
/* 8170 */    68,    6,  129,   30,    4,   70,    6,  129,   71,    7, 
/* 8180 */   129,   72,    7,  129,   73,    7,  129,   74,    7,  129, 
/* 8190 */    74,    8,  129,   75,    8,  130,   69,   15,  129,   67, 
/* 8200 */    17,  129,   66,   18,  129,   65,   19,  130,   65,   18, 
/* 8210 */   130,   66,   16,  129,   67,   13,  129,   69,    8,  129, 
/* 8220 */   193,  129,   30,   13,   64,    8,  129,   30,   13,   61, 
/* 8230 */    14,  129,   30,   13,   59,   18,  129,   30,   13,   57, 
/* 8240 */    22,  129,   33,    8,   56,   24,  129,   32,    7,   55, 
/* 8250 */    26,  129,   32,    6,   54,   28,  129,   31,    6,   53, 
/* 8260 */    16,   77,    6,  129,   31,    5,   53,   14,   79,    4, 
/* 8270 */   129,   30,    5,   52,   14,   80,    4,  129,   30,    5, 
/* 8280 */    52,   13,   80,    4,  129,   30,    4,   52,   13,   80, 
/* 8290 */     4,  129,   30,    4,   52,   12,   80,    4,  129,   30, 
/* 8300 */     4,   51,   13,   80,    4,  130,   30,    4,   50,   13, 
/* 8310 */    79,    5,  129,   30,    4,   50,   13,   78,    5,  129, 
/* 8320 */    30,    5,   49,   14,   77,    6,  129,   31,    4,   49, 
/* 8330 */    13,   76,    6,  129,   31,    5,   48,   14,   75,    7, 
/* 8340 */   129,   32,    5,   47,   14,   73,    8,  129,   32,    6, 
/* 8350 */    45,   16,   71,   13,  129,   33,   27,   71,   13,  129, 
/* 8360 */    34,   26,   71,   13,  129,   35,   24,   71,   13,  129, 
/* 8370 */    37,   20,  129,   39,   16,  129,   43,    9,  129,  193, 
/* 8380 */   129,   80,    4,  131,   41,   56,  129,   37,   60,  129, 
/* 8390 */    35,   62,  129,   33,   64,  129,   32,   65,  129,   31, 
/* 8400 */    66,  129,   30,   67,  130,   30,   11,   80,    4,  129, 
/* 8410 */    30,    9,   80,    4,  129,   30,    8,   80,    4,  129, 
/* 8420 */    31,    7,   80,    4,  129,   31,    6,  129,   32,    5, 
/* 8430 */   129,   33,    5,  129,   35,    4,  129,   38,    3,  129, 
/* 8440 */   193,  129,   80,    4,  130,   42,   42,  129,   38,   46, 
/* 8450 */   129,   35,   49,  129,   33,   51,  129,   32,   52,  129, 
/* 8460 */    31,   53,  130,   30,   54,  129,   30,   12,  129,   30, 
/* 8470 */     9,  129,   30,    8,  129,   30,    7,  130,   31,    6, 
/* 8480 */   130,   32,    6,  129,   33,    5,  129,   34,    5,  129, 
/* 8490 */    35,    5,   80,    4,  129,   37,    5,   80,    4,  129, 
/* 8500 */    30,   54,  136,   30,    4,  130,  193,  129,   80,    4, 
/* 8510 */   130,   77,    7,  129,   74,   10,  129,   70,   14,  129, 
/* 8520 */    66,   18,  129,   62,   22,  129,   59,   25,  129,   55, 
/* 8530 */    29,  129,   51,   33,  129,   47,   37,  129,   44,   32, 
/* 8540 */    80,    4,  129,   40,   32,   80,    4,  129,   36,   32, 
/* 8550 */   129,   32,   33,  129,   30,   31,  129,   33,   24,  129, 
/* 8560 */    36,   17,  129,   40,   12,  129,   44,   12,  129,   48, 
/* 8570 */    12,  129,   51,   13,  129,   55,   13,  129,   59,   13, 
/* 8580 */    80,    4,  129,   63,   13,   80,    4,  129,   67,   17, 
/* 8590 */   129,   71,   13,  129,   74,   10,  129,   78,    6,  129, 
/* 8600 */    80,    4,  131,  193,  129,   80,    4,  130,   77,    7, 
/* 8610 */   129,   74,   10,  129,   70,   14,  129,   66,   18,  129, 
/* 8620 */    62,   22,  129,   59,   25,  129,   55,   29,  129,   51, 
/* 8630 */    33,  129,   47,   37,  129,   44,   32,   80,    4,  129, 
/* 8640 */    40,   32,   80,    4,  129,   36,   32,  129,   32,   33, 
/* 8650 */   129,   30,   31,  129,   33,   24,  129,   36,   17,  129, 
/* 8660 */    40,   12,  129,   44,   12,  129,   47,   13,  129,   44, 
/* 8670 */    20,  129,   40,   28,  129,   36,   31,  129,   32,   32, 
/* 8680 */   129,   30,   30,  129,   33,   24,  129,   36,   17,  129, 
/* 8690 */    40,   12,  129,   44,   12,  129,   48,   12,  129,   51, 
/* 8700 */    13,  129,   55,   13,  129,   59,   13,   80,    4,  129, 
/* 8710 */    63,   13,   80,    4,  129,   67,   17,  129,   71,   13, 
/* 8720 */   129,   74,   10,  129,   78,    6,  129,   80,    4,  131, 
/* 8730 */   193,  129,   30,    4,   80,    4,  130,   30,    4,   79, 
/* 8740 */     5,  129,   30,    5,   77,    7,  129,   30,    6,   74, 
/* 8750 */    10,  129,   30,    8,   72,   12,  129,   30,   11,   69, 
/* 8760 */    15,  129,   30,   13,   67,   17,  129,   30,    4,   37, 
/* 8770 */     8,   64,   20,  129,   30,    4,   39,    8,   62,   22, 
/* 8780 */   129,   41,    8,   59,   25,  129,   43,    8,   57,   27, 
/* 8790 */   129,   45,    8,   55,   22,   80,    4,  129,   47,   27, 
/* 8800 */    80,    4,  129,   49,   23,  129,   47,   22,  129,   44, 
/* 8810 */    23,  129,   42,   22,  129,   30,    4,   39,   27,  129, 
/* 8820 */    30,    4,   37,   31,  129,   30,   27,   62,    8,  129, 
/* 8830 */    30,   25,   64,    8,  129,   30,   22,   66,    8,   80, 
/* 8840 */     4,  129,   30,   20,   68,    8,   80,    4,  129,   30, 
/* 8850 */    17,   70,    8,   80,    4,  129,   30,   15,   73,   11, 
/* 8860 */   129,   30,   12,   75,    9,  129,   30,   10,   77,    7, 
/* 8870 */   129,   30,    7,   79,    5,  129,   30,    5,   80,    4, 
/* 8880 */   129,   30,    4,   80,    4,  130,  193,  129,    4,    5, 
    80,    4,  129,    2,    9,   80,    4,  129,    1,   11,
    77,    7,  129,    1,   12,   74,   10,  129,    1,   12,
    70,   14,  129,    1,   12,   66,   18,  129,    1,   11,
    62,   22,  129,    2,    9,   59,   25,  129,    4,   11,
    55,   29,  129,    7,   12,   51,   33,  129,   10,   12,
    47,   37,  129,   14,   12,   44,   32,   80,    4,  129,
    17,   13,   40,   32,   80,    4,  129,   21,   13,   36,
    32,  129,   25,   40,  129,   29,   32,  129,   33,   24,
   129,   36,   17,  129,   40,   12,  129,   44,   12,  129,
    48,   12,  129,   51,   13,  129,   55,   13,  129,   59,
    13,   80,    4,  129,   63,   13,   80,    4,  129,   67,
    17,  129,   71,   13,  129,   74,   10,  129,   78,    6,
   129,   80,    4,  131,  193,  129,   30,    1,   71,   13,
   129,   30,    3,   71,   13,  129,   30,    6,   71,   13,
   129,   30,    9,   75,    9,  129,   30,   11,   77,    7,
   129,   30,   14,   79,    5,  129,   30,   17,   79,    5,
   129,   30,   19,   80,    4,  129,   30,   22,   80,    4,
   129,   30,   25,   80,    4,  129,   30,   27,   80,    4,
   129,   30,    4,   36,   24,   80,    4,  129,   30,    4,
    38,   25,   80,    4,  129,   30,    4,   41,   24,   80,
     4,  129,   30,    4,   44,   24,   80,    4,  129,   30,
     4,   46,   25,   80,    4,  129,   30,    4,   49,   25,
    80,    4,  129,   30,    4,   52,   24,   80,    4,  129,
    30,    4,   54,   30,  129,   30,    4,   57,   27,  129,
    30,    4,   59,   25,  129,   30,    4,   62,   22,  129,
    30,    4,   65,   19,  129,   30,    5,   67,   17,  129,
    30,    5,   70,   14,  129,   30,    7,   73,   11,  129,
    30,    9,   76,    8,  129,   30,   13,   78,    6,  129,
    30,   13,   81,    3,  129,   30,   13,  129,  193,    2,
     9,   59,   25,  129,    4,   11,   55,   29,  129,    7,
    12,   51,   33,  129,   10,   12,   47,   37,  129,   14,
    12,   44,   32,   80,    4,  129,   17,   13,   40,   32,
    80,    4,  129,   21,   13,   36,   32,  129,   25,   40,
   129,   29,   32,  129,   33,   24,  129,   36,   17,  129,
    40,   12,  129,   44,   12,  129,   48,   12,  129,   51,
    13,  129,   55,   13,  129,   59,   13,   80,    4,  129,
    63,   13,   80,    4,  129,   67,   17,  129,   71,   13,
   129,   74,   10,  129,   78,    6,  129,   80,    4,  131,
   193
};

int fbClass::DrawString( int xpos, int ypos, int height, const char *msg, int r, int g, int b )
{
	char line[DWIDTH];
	char message[MAXMSG];
	char print[DWIDTH];
	int i, j, linen, max, nchars, pc, term, x, y;
	int gx = xpos;
	int gy = ypos + height -1;

	strcpy(message,msg);

	memset(print,0,DWIDTH);

	for (i = 0; i < height; i++)
	{
		j = i * 132 / height;
		print[j] = 1;
	}

	nchars = strlen(message);

	for (i = 0; i < nchars; i++)
	{
		if ((u_char) message[i] >= NCHARS || asc_ptr[(u_char) message[i]] == 0) return(gx-xpos);
	}

	for (i = 0; i < nchars; i++)
	{
		for (j = 0; j < DWIDTH; j++)
			line[j] = ' ';
		pc = asc_ptr[(u_char) message[i]];
		term = 0;
		max = 0;
		linen = 0;
		while (!term)
		{
			if (pc < 0 || pc > NBYTES) return(gx-xpos);

			x = data_table[pc] & 0377;
			if (x >= 128)
			{
				if (x>192) term++;
				x = x & 63;
				while (x--)
				{
					if (print[linen++])
					{
						for (j=0; j <= max; j++)
						{
							if (print[j])
							{
								if ( line[j] == '#' ) PaintPixel(gx,gy,r,g,b);
								gy--;
							}
						}
						gx++;
						gy=ypos+height-1;
					}
				}
				for (j = 0; j < DWIDTH; j++)
					line[j] = ' ';
				pc++;
			}
			else
			{
				y = data_table[pc+1];
				max = x+y;
				while (x < max) line[x++] = '#';
				pc += 2;
			}
		}
	}
	return(gx-xpos);
}
#endif //USEFREETYPEFB

int fbClass::showConsole(int state)
{
	int fd=open("/dev/vc/0", O_RDWR);
	if(fd>=0)
	{
		if(ioctl(fd, KDSETMODE, state?KD_TEXT:KD_GRAPHICS)<0)
			perror("[CONSOLE] <setting /dev/vc/0 status failed>");
		close(fd);
	}
	return 0;
}

int fbClass::SetSAA(int value)
{
	int arg, saafd;

	switch (value)
	{
		case 0: arg=SAA_MODE_RGB;	break;
		case 1: arg=SAA_MODE_FBAS;	break;
		case 2: arg=SAA_MODE_SVIDEO;	break;
		case 3: arg=SAA_MODE_COMPONENT; break;
	}

	if ((saafd=open("/dev/dbox/saa0", O_RDWR))<0)
	{
		perror("[SAA-Set] saa open"); return 1;
	}
	else
	{
		if((ioctl(saafd, 5, &arg))<0)
		{
			perror("[SAA-Set] saa set"); close(saafd); return 1;
		}
		close(saafd);
	}
	return 0;
}
#endif


