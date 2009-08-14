#include <lib/gdi/gpixmap.h>
#include <lib/gdi/gfbdc.h>
#include <zlib.h>

gLookup::gLookup()
	:size(0), lookup(0)
{
}

gLookup::gLookup(int size, const gPalette &pal, const gRGB &start, const gRGB &end)
	:size(0), lookup(0)
{
	build(size, pal, start, end);
}

void gLookup::build(int _size, const gPalette &pal, const gRGB &start, const gRGB &end)
{
	if (lookup)
	{
		delete [] lookup;
		lookup=0;
		size=0;
	}
	size=_size;
	if (!size)
		return;
	lookup=new gColor[size];
	
	for (int i=0; i<size; i++)
	{
		gRGB col;
		if (i)
		{
			int rdiff=-start.r+end.r;
			int gdiff=-start.g+end.g;
			int bdiff=-start.b+end.b;
			int adiff=-start.a+end.a;
			rdiff*=i; rdiff/=(size-1);
			gdiff*=i; gdiff/=(size-1);
			bdiff*=i; bdiff/=(size-1);
			adiff*=i; adiff/=(size-1);
			col.r=start.r+rdiff;
			col.g=start.g+gdiff;
			col.b=start.b+bdiff;
			col.a=start.a+adiff;
		} else
			col=start;
		lookup[i]=pal.findColor(col);
	}
}

gPixmap *gPixmap::lock()
{
	contentlock.lock(1);
	return this;
}

void gPixmap::unlock()
{
	contentlock.unlock(1);
}

void gPixmap::fill(const eRect &area, const gColor &color, int round)
{
	if ((area.height()<=0) || (area.width()<=0))
		return;
	// eDebug("gPixmap::fill: pos=%d,%d, size=%d,%d bpp=%d col=%d", area.left(), area.top(), area.width(), area.height(), bpp, (int) color);
	if ((area.width() < round << 1) || (area.height() < round << 1)) round = 0; // dont round small areas
	int roundbot = area.bottom() - round - 1; // last row not influenced by corner rounding

	if (bpp == 8)
		for (int y = area.top(); y < area.bottom(); y++)
		{
			int x = area.width();
			if (round) // need some corner rounding
			{
				x -= (round << 1);
			}
			memset(((__u8*)data) + y * stride + area.left() + round, color.color, x);
			if (round) 
			{
				if (y < roundbot) round--; // at the bottom corner we need to count up...
			}
			if (y >= roundbot) round++; // check for top or bottom corner
		}
	else if (bpp == 32)
	{
		__u32 col;
		if (clut.data && color < clut.colors)
			//col = 	(gFBDC::getInstance()->getRampAlpha(clut.data[color].a)<<24) |
			col = 	((clut.data[color].a == 0xFF ? 0xFF : 
			               clut.data[color].a == 0 ? 0 : gFBDC::getInstance()->getRampAlpha(254))<<24) |
				(gFBDC::getInstance()->getRamp(clut.data[color].r)<<16) |
				(gFBDC::getInstance()->getRamp(clut.data[color].g)<<8) |
				(gFBDC::getInstance()->getRamp(clut.data[color].b));
		else
			col=0x10101*color;
		col^=0xFF000000;			
		for (int y = area.top(); y < area.bottom(); y++)
		{
			__u32 *dst = (__u32*)(((__u8*)data) + y*stride + (area.left()<<2));
			int x = area.width();
			if (round) // need some corner rounding
			{
				dst += round;
				x -= (round << 1);
			}
			while (x--)
				*dst++ = col;
			if (round) 
			{
				dst += round;
				if (y < roundbot) round--; // at the bottom corner we need to count up...
			}
			if (y >= roundbot) round++; // check for top or bottom corner
		}
	}
	else
		eWarning("couldn't fill %d bpp", bpp);
}

void gPixmap::blit(const gPixmap &src, ePoint pos, const eRect &clip, int flag)
{
	eRect area=eRect(pos, src.getSize());
	area&=clip;
	area&=eRect(ePoint(0, 0), getSize());
	if ((area.width()<0) || (area.height()<0))
		return;

	eRect srcarea=area;
	srcarea.moveBy(-pos.x(), -pos.y());

	__u8 *srcdata=src.uncompressdatanoreplace();
	__u8 *srcptr=srcdata;
	__u8 *dstptr=(__u8*)data;

	// eDebug("gPixmap::blit: pos=%d,%d, stride=%d size=%d,%d bpp=%d srcbpp=%d flag=0x%x srcarea=%d,%d stride=%d",
	//	area.left(), area.top(), stride, area.width(), area.height(), bpp, src.bpp, flag, srcarea.left(), srcarea.top(), src.stride);

	if ((bpp == 8) && (src.bpp==8))
	{
	
		srcptr += srcarea.left() + srcarea.top()*src.stride; // bypp == 1
		dstptr += area.left() + area.top()*stride; // bypp == 1
		for (int y=0; y<area.height(); y++)
		{
			if (flag & blitAlphaTest)
			{
  	      			// no real alphatest yet
				int width=area.width();
				unsigned char *srcp=(unsigned char*)srcptr;
				unsigned char *dstp=(unsigned char*)dstptr;
				// use duff's device here!
				while (width--)
				{
					if (*srcp)
					{
						*dstp = *srcp;
					}
					srcp++;
					dstp++;
				}
			} else
				memcpy(dstptr, srcptr, area.width()); // bypp == 1
			srcptr += src.stride;
			dstptr += stride;
		}
	}
	else if ((bpp == 32) && (src.bpp==8))
	{
		int byshift = 2;
		__u32 pal[256];
		
		for (int i=0; i<256; ++i)
		{
			if (src.clut.data && (i<src.clut.colors))
			{
				// images often have random data in transparent pixels. We just define
				// pixels having an alpha bigger that 0x80 as being completely
				// transparent so the random data is irrelevant.
				if ((src.clut.data[i].a & 0x80) == 0)
				{
					pal[i] = (gFBDC::getInstance()->getRampAlpha(254) ^ 0xFF) << 24 | // getRampAlpha = 0...alpha
						gFBDC::getInstance()->getRamp(src.clut.data[i].r) << 16 |
						gFBDC::getInstance()->getRamp(src.clut.data[i].g) <<  8 |
						gFBDC::getInstance()->getRamp(src.clut.data[i].b);
				}
				else
					pal[i] = 0;
			}
			else
				pal[i] = 0xFF000000 | i < 16 | i < 8 | i;
		}
	
		srcptr += (srcarea.left()) + srcarea.top()*src.stride;
		dstptr += (area.left()<<byshift) + area.top()*stride;
		for (int y=0; y<area.height(); y++)
		{
			int width=area.width();
			__u8 *srcp=(__u8*)srcptr;
			__u32 *dstp=(__u32*)dstptr;
			if (flag & blitAlphaTest)
			{
  	      			// no real alphatest yet
				// use duff's device here!
				while (width--)
				{
					//if (pal[*srcp] & 0x80000000)
					if (pal[*srcp] & 0xFF000000)
						*dstp = pal[*srcp];
					srcp++;
					dstp++;
				}
			}
			else
			{
				while (width--)
					*dstp++ = pal[*srcp++];
			}
			srcptr += src.stride;
			dstptr += stride;
		}
	}
	else if ((bpp == 32) && (src.bpp == 32))
	{
		int byshift = 2;
		__u8 *srcptr=(__u8*)src.data;
		__u8 *dstptr=(__u8*)data; // !!
		
		srcptr += (srcarea.left()<<byshift) + srcarea.top()*src.stride;
		dstptr += (area.left()<<byshift) + area.top()*stride;
		for (int y=0; y<area.height(); y++)
		{
			int width=area.width();
			__u32 *srcp=(__u32*)srcptr;
			__u32 *dstp=(__u32*)dstptr;
			if (flag & blitAlphaTest)
			{
  	      			// no real alphatest yet
				// use duff's device here!
				while (width--)
				{
					if ((*srcp & 0x80000000) == 0)
					{
						// *dstp = ((gFBDC::getInstance()->getRampAlpha((*srcp >> 24 & 0xFF)) ^ 0xFF ) << 24) |
						*dstp =	(gFBDC::getInstance()->getRampAlpha(254) ^ 0xFF) << 24 |
							gFBDC::getInstance()->getRamp((*srcp >> 16 & 0xFF)) << 16 |
							gFBDC::getInstance()->getRamp((*srcp >>  8 & 0xFF)) <<  8 |
							gFBDC::getInstance()->getRamp(*srcp & 0xFF);
					}
					srcp++;
					dstp++;
				}
			} 
			else
			{
				while (width--)
				{
					if ((*srcp & 0x80000000) == 0)
					{
						*dstp++ = (gFBDC::getInstance()->getRampAlpha(254) ^ 0xFF) << 24 | // getRampAlpha = 0...alpha
							gFBDC::getInstance()->getRamp(*srcp >> 16 & 0xFF) << 16 |
							gFBDC::getInstance()->getRamp(*srcp >>  8 & 0xFF) <<  8 |
							gFBDC::getInstance()->getRamp(*srcp & 0xFF);
					}
					else
						*dstp++ = 0;
					srcp++;
				}
			}
			srcptr += src.stride;
			dstptr += stride;
		}
	} else
		eWarning("cannot blit %dbpp from %dbpp. Please choos a different skin", bpp, src.bpp);
	if (src.compressedsize)
		delete[] srcdata;
}

void gPixmap::mergePalette(const gPixmap &target)
{
	if ((!clut.colors) || (!target.clut.colors))
		return;
	gColor *lookup=new gColor[clut.colors];

	for (int i=0; i<clut.colors; i++)
		lookup[i].color=target.clut.findColor(clut.data[i]);
	
	delete [] clut.data;
	clut.colors=target.clut.colors;
	clut.data=new gRGB[clut.colors];
	memcpy(clut.data, target.clut.data, sizeof(gRGB)*clut.colors);

	bool compressed = compressedsize;
	if (compressed) uncompressdata();

	__u8 *dstptr=(__u8*)data;

	for (int ay=0; ay<y; ay++)
	{
		for (int ax=0; ax<x; ax++)
			dstptr[ax]=lookup[dstptr[ax]];
		dstptr+=stride;
	}
	
	delete [] lookup;

	if (compressed) compressdata();
}
void gPixmap::compressdata()
{
	if (cancompress && !compressedsize)
	{
		uLongf comprlen = x*y*bypp;
		if (comprlen > 1000) // only compress "big" images
		{
			lock();
			__u8 compressed[comprlen];
			if (compress2(compressed,&comprlen,(__u8*)data,comprlen,Z_BEST_SPEED) == Z_OK)
			{
				delete[] (char*)data;
				data = new __u8[comprlen];
				memcpy(data,compressed,comprlen);
				compressedsize=comprlen;
			}
			unlock();
		}
	}
}
void gPixmap::uncompressdata()
{
	if (compressedsize)
	{
		lock();
		uLongf uncomprlen = x*y*bypp;
		__u8* uncompressed = new __u8[uncomprlen];
		uncompress(uncompressed,&uncomprlen,(__u8*)data,compressedsize);
		delete[] (char*)data;
		data = uncompressed;
		compressedsize = 0;
		unlock();
	}
}
__u8* gPixmap::uncompressdatanoreplace() const
{
	if (compressedsize)
	{
		uLongf uncomprlen = x*y*bypp;
		__u8* uncompressed = new __u8[uncomprlen];
		uncompress(uncompressed,&uncomprlen,(__u8*)data,compressedsize);
		return uncompressed;
	}
	return (__u8*)data;
}

void gPixmap::resize(eSize size)
{
	int dmx = 1000 * x / size.width();
	int dmy = 1000 * y / size.height();
	int oldstride = stride;

	if (dmx > 980 && dmx < 1020 || dmy > 980 && dmy < 1020)
	{
		/* close enough to the original size, and we're not intending to mess with the aspect ratio */
		return;
	}

	bool compressed = compressedsize;
	if (compressed) uncompressdata();

	x = size.width();
	y = size.height();
	stride = x * bypp;

	void *newdata = new char[x * y * bypp];
	__u8 *dst = (__u8*)newdata;
	__u8 *src = (__u8*)data;

	for (int ay = 0; ay < y; ay++)
	{
		__u8 *dstptr = &dst[ay * stride];
		__u8 *srcptr = &src[(ay * dmy / 1000) * oldstride];
		for (int ax = 0; ax < x; ax++)
		{
			dstptr[ax] = srcptr[ax * dmx / 1000];
		}
	}
	delete [] (char*)data;
	data = newdata;

	if (compressed) compressdata();
}

void gPixmap::line(ePoint start, ePoint dst, gColor color)
{
// dieser code rult ganz ganz doll weil er ganz ganz fast ist und auch sehr gut dokumentiert is
// t. es handelt sich immerhin um den weltbekannten bresenham algorithmus der nicht nur
// sehr schnell ist sondern auch sehr gut dokumentiert und getestet wurde. nicht
// nur auf dem LCD der dbox, sondern auch ueberall anders. und auch auf der
// dbox mit LCD soll das teil nun tun, und ich denke das tut es. ausse
// rdem hat dieser algo den vorteil dass man fehler sehr leicht fi
// ndet und beheben kann. das liegt nicht zuletzt an den komment
// aren. und ausserdem, je kuerzer der code, desto weniger k
// ann daran falsch sein. erwaehnte ich schon, da
// s dieser tolle code wahnsinnig schnell
// ist? bye, tmbinc
// nun ist der tolle code leider zu ende. tut mir leid.

	__u32 col = 0;
	int highcol = 0;
	
	if (bpp == 32)
		highcol = 1;
		
	if (highcol)
	{
		if (clut.data && color < clut.colors)
			//col = 	(gFBDC::getInstance()->getRampAlpha(clut.data[color].a)<<24) |
			col = 	((clut.data[color].a == 0xFF ? 0xFF : 
			               clut.data[color].a == 0 ? 0 : gFBDC::getInstance()->getRampAlpha(254))<<24) |
				(gFBDC::getInstance()->getRamp(clut.data[color].r)<<16) |
				(gFBDC::getInstance()->getRamp(clut.data[color].g)<<8) |
				(gFBDC::getInstance()->getRamp(clut.data[color].b));
		else
			col=0x10101*color;
		col^=0xFF000000;			
	}

	int Ax=start.x(), Ay=start.y(), Bx=dst.x(), By=dst.y();
	int dX, dY, fbXincr, fbYincr, fbXYincr, dPr, dPru, P;
	__u8 *AfbAddr = &((__u8*)data)[Ay*stride + Ax*bypp];
	__u8 *BfbAddr = &((__u8*)data)[By*stride + Bx*bypp];

	// eDebug("gPixmap::line: from %d,%d to %d,%d bpp=%d", Ax, Ay, Bx, By, bpp);
	fbXincr = bypp;
	if ( (dX = Bx - Ax) >= 0)
		goto AFTERNEGX;
	dX = -dX;
	// fbXincr=-1;
	fbXincr = -fbXincr;
    AFTERNEGX:
	fbYincr = stride;
	if ( (dY = By - Ay) >= 0)
		goto AFTERNEGY;
	fbYincr=-stride;
	dY = -dY;
    AFTERNEGY:
	fbXYincr = fbXincr + fbYincr;
	if (dY > dX)
		goto YisIndependent;
	dPr = dY << 1 ;
	P = -dX;
	dPru = P << 1;
	dY = dX >> 1;
    XLOOP:
	if (highcol) {
		*AfbAddr     = (col>>24) & 0xFF;
		*(AfbAddr+1) = (col>>16) & 0xFF;
		*(AfbAddr+2) = (col>>8) & 0xFF;
		*(AfbAddr+3) = col & 0xFF;
		*BfbAddr     = (col>>24) & 0xFF;
		*(BfbAddr+1) = (col>>16) & 0xFF;
		*(BfbAddr+2) = (col>>8) & 0xFF;
		*(BfbAddr+3) = col & 0xFF;
	}
	else {
		*AfbAddr = color;
		*BfbAddr = color;
	}
	if ((P += dPr) > 0)
		goto RightAndUp;
	AfbAddr += fbXincr;
	BfbAddr -= fbXincr;
	if (--dY > 0)
		goto XLOOP;
	if (highcol) {
		*AfbAddr     = (col>>24) & 0xFF;
		*(AfbAddr+1) = (col>>16) & 0xFF;
		*(AfbAddr+2) = (col>>8) & 0xFF;
		*(AfbAddr+3) = col & 0xFF;
	}
	else
		*AfbAddr = color;
	if ((dX & 1) == 0)
		return;
	if (highcol) {
		*BfbAddr     = (col>>24) & 0xFF;
		*(BfbAddr+1) = (col>>16) & 0xFF;
		*(BfbAddr+2) = (col>>8) & 0xFF;
		*(BfbAddr+3) = col & 0xFF;
	}
	else
		*BfbAddr = color;
	return;

    RightAndUp:
	AfbAddr += fbXYincr;
	BfbAddr -= fbXYincr;
	P += dPru;
	if (--dY > 0)
		goto XLOOP;
	if (highcol) {
		*AfbAddr     = (col>>24) & 0xFF;
		*(AfbAddr+1) = (col>>16) & 0xFF;
		*(AfbAddr+2) = (col>>8) & 0xFF;
		*(AfbAddr+3) = col & 0xFF;
	}
	else
		*AfbAddr = color;
	if ((dX & 1) == 0)
		return;
	if (highcol) {
		*BfbAddr     = (col>>24) & 0xFF;
		*(BfbAddr+1) = (col>>16) & 0xFF;
		*(BfbAddr+2) = (col>>8) & 0xFF;
		*(BfbAddr+3) = col & 0xFF;
	}
	else
		*BfbAddr = color;
	return;

    YisIndependent:
	dPr = dX << 1;
	P = -dY;
	dPru = P << 1;
	dX = dY >> 1;
    YLOOP:
	if (highcol) {
		*AfbAddr     = (col>>24) & 0xFF;
		*(AfbAddr+1) = (col>>16) & 0xFF;
		*(AfbAddr+2) = (col>>8) & 0xFF;
		*(AfbAddr+3) = col & 0xFF;
		*BfbAddr     = (col>>24) & 0xFF;
		*(BfbAddr+1) = (col>>16) & 0xFF;
		*(BfbAddr+2) = (col>>8) & 0xFF;
		*(BfbAddr+3) = col & 0xFF;
	}
	else {
		*AfbAddr = color;
		*BfbAddr = color;
	}
	if ((P += dPr) > 0)
		goto RightAndUp2;
	AfbAddr += fbYincr;
	BfbAddr -= fbYincr;
	if (--dX > 0)
		goto YLOOP;
	if (highcol) {
		*AfbAddr     = (col>>24) & 0xFF;
		*(AfbAddr+1) = (col>>16) & 0xFF;
		*(AfbAddr+2) = (col>>8) & 0xFF;
		*(AfbAddr+3) = col & 0xFF;
	}
	else
		*AfbAddr = color;
	if ((dY & 1) == 0)
		return;
	if (highcol) {
		*BfbAddr     = (col>>24) & 0xFF;
		*(BfbAddr+1) = (col>>16) & 0xFF;
		*(BfbAddr+2) = (col>>8) & 0xFF;
		*(BfbAddr+3) = col & 0xFF;
	}
	else
		*BfbAddr = color;
	return;

    RightAndUp2:
	AfbAddr += fbXYincr;
	BfbAddr -= fbXYincr;
	P += dPru;
	if (--dX > 0)
		goto YLOOP;
	if (highcol) {
		*AfbAddr     = (col>>24) & 0xFF;
		*(AfbAddr+1) = (col>>16) & 0xFF;
		*(AfbAddr+2) = (col>>8) & 0xFF;
		*(AfbAddr+3) = col & 0xFF;
	}
	else
		*AfbAddr = color;
	if ((dY & 1) == 0)
		return;
	if (highcol) {
		*BfbAddr     = (col>>24) & 0xFF;
		*(BfbAddr+1) = (col>>16) & 0xFF;
		*(BfbAddr+2) = (col>>8) & 0xFF;
		*(BfbAddr+3) = col & 0xFF;
	}
	else
		*BfbAddr = color;
	return;
}

gColor gPalette::findColor(const gRGB &rgb) const
{
	int difference=1<<30, best_choice=0;
	for (int t=0; t<colors; t++)
	{
		int ttd;
		int td=(signed)(rgb.r-data[t].r); td*=td; td*=(255-data[t].a);
		ttd=td;
		if (ttd>=difference)
			continue;
		td=(signed)(rgb.g-data[t].g); td*=td; td*=(255-data[t].a);
		ttd+=td;
		if (ttd>=difference)
			continue;
		td=(signed)(rgb.b-data[t].b); td*=td; td*=(255-data[t].a);
		ttd+=td;
		if (ttd>=difference)
			continue;
		td=(signed)(rgb.a-data[t].a); td*=td; td*=255;
		ttd+=td;
		if (ttd>=difference)
			continue;
		difference=ttd;
		best_choice=t;
	}
	return best_choice;
}

void gPixmap::finalLock()
{
	if (!final)
		contentlock.lock();
	final=1;
}

gPixmap::gPixmap()
	:final(0),cancompress(false),compressedsize(0)
{
	data = NULL;
	clut.data = NULL;
}

gPixmap::~gPixmap()
{
	finalLock();
	delete[] clut.data;
}

gImage::gImage(eSize size, int _bpp)
{
	x=size.width();
	y=size.height();
	bpp=_bpp;
	switch (bpp)
	{
	case 8:
		bypp=1;
		break;
	case 15:
	case 16:
		bypp=2;
		break;
	case 24:		// never use 24bit mode
	case 32:
		bypp=4;
		break;
	default:
		bypp=(bpp+7)/8;
	}
	stride=x*bypp;
	clut.colors=0;
	clut.data=0;
	data=new char[x*y*bypp];
	cancompress = true;
}

gImage::~gImage()
{
	finalLock();
	delete[] (char*)data;
}
