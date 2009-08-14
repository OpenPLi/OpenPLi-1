#ifndef __grc_h
#define __grc_h

/*
	gPainter ist die high-level version. die highlevel daten werden zu low level opcodes ueber
	die gRC-queue geschickt und landen beim gDC der hardwarespezifisch ist, meist aber auf einen
	gPixmap aufsetzt (und damit unbeschleunigt ist).
*/

#include <pthread.h>
#include <unistd.h>
#include <stack>
#include <list>

#include <lib/base/estring.h>
#include <lib/base/erect.h>
#include <lib/system/elock.h>
#include <lib/gdi/gpixmap.h>


class eTextPara;

class gDC;
struct gOpcode
{
	enum Opcode
	{
		begin,
		
		renderText,
		renderPara,
		
		fill,
		blit,

		setPalette,
		mergePalette,
		
		line,
		
		clip,
		
		flush,
		end,
		
		shutdown
	} opcode;

	union para
	{
		struct pbegin
		{
			eRect area;
			pbegin(const eRect &area): area(area) { }
		} *begin;
		
		struct pfill
		{
			eRect area;
			gColor color;
			int cornerRound;
			pfill(const eRect &area, gColor color, int &cornerRound): area(area), color(color), cornerRound(cornerRound) { }
		} *fill;

		struct prenderText
		{
			gFont font;
			eRect area;
			char *text;
			gRGB foregroundColor, backgroundColor;
			prenderText(const gFont &font, const eRect &area, const eString &text, const gRGB &foregroundColor, const gRGB &backgroundColor):
				font(font), area(area), text(text.length()?strdup(text.c_str()):0), foregroundColor(foregroundColor), backgroundColor(backgroundColor) { }
		} *renderText;

		struct prenderPara
		{
			ePoint offset;
			eTextPara *textpara;
			gRGB foregroundColor, backgroundColor;
			prenderPara(const ePoint &offset, eTextPara *textpara, const gRGB &foregroundColor, const gRGB &backgroundColor)
				: offset(offset), textpara(textpara), foregroundColor(foregroundColor), backgroundColor(backgroundColor) { }
		} *renderPara;

		struct psetPalette
		{
			gPalette *palette;
			psetPalette(gPalette *palette): palette(palette) { }
		} *setPalette;
		
		struct pblit
		{
			gPixmap *pixmap;
			ePoint position;
			eRect clip;
			pblit(gPixmap *pixmap, const ePoint &position, const eRect &clip)
				: pixmap(pixmap), position(position), clip(clip) { }
		} *blit;

		struct pmergePalette
		{
			gPixmap *target;
			pmergePalette(gPixmap *target): target(target) { }
		} *mergePalette;
		
		struct pline
		{
			ePoint start, end;
			gColor color;
			pline(const ePoint &start, const ePoint &end, gColor color): start(start), end(end), color(color) { }
		} *line;

		struct pclip
		{
			eRect clip;
			pclip(const eRect &clip): clip(clip) { }
		} *clip;
	} parm;

	int flags;
	
	gDC *dc;
};

#define MAXSIZE 1024

class gRC
{
	static void *thread_wrapper(void *ptr);
	void *thread();

	static gRC *instance;
	gOpcode queue[MAXSIZE];
	pthread_t the_thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int rp, wp;
public:
	bool mustDraw() { return rp != wp; }
	gRC();
	virtual ~gRC();

	void submit(const gOpcode &o)
	{
		while(1)
		{
			pthread_mutex_lock(&mutex);
			int tmp=wp+1;
			if ( tmp == MAXSIZE )
				tmp=0;
			if ( tmp == rp )
			{
				pthread_mutex_unlock(&mutex);
				//printf("render buffer full...\n");
				//fflush(stdout);
				usleep(1000);  // wait 1 msec 
				continue;
			}
			int free=rp-wp;
			if ( free <= 0 )
				free+=MAXSIZE;
			queue[wp++]=o;
			if ( wp == MAXSIZE )
				wp = 0;
			if (o.opcode==gOpcode::end||o.opcode==gOpcode::shutdown)
				pthread_cond_signal(&cond);
			pthread_mutex_unlock(&mutex);
			break;
		}
	}

	static gRC &getInstance();
};

class gDC
{
protected:
	eLock dclock;
public:
	virtual void exec(gOpcode *opcode)=0;
	virtual gPixmap &getPixmap()=0;
	virtual eSize getSize()=0;
	virtual const eRect &getClip()=0;
	virtual gRGB getRGB(gColor col)=0;
	virtual ~gDC();
	virtual int islocked() { return 0; }
	void lock() { dclock.lock(1); }
	void unlock() { dclock.unlock(1); }
};

class gPainter
{
	gDC &dc;
	gRC &rc;
	friend class gRC;

	gOpcode *beginptr;
			/* paint states */	
//	std::stack<eRect, std::list<eRect> > cliparea;
	std::stack<eRect> cliparea;
	gFont font;
	gColor foregroundColor, backgroundColor;
	int cornerRound;
	ePoint logicalZero;
	void begin(const eRect &rect);
	void end();
public:
	gPainter(gDC &dc, eRect rect=eRect());
	virtual ~gPainter();

	void setBackgroundColor(const gColor &color);
	void setForegroundColor(const gColor &color);
	void setRound(int r);

	void setFont(const gFont &font);
	void renderText(const eRect &position, const std::string &string, int flags=0);
	void renderPara(eTextPara &para, ePoint offset=ePoint(0, 0));

	void fill(const eRect &area);
	
	void clear();
	
	void gPainter::blit(gPixmap &pixmap, ePoint pos, eRect clip=eRect(), int flags=0)
	{
		if ( dc.islocked() )
			return;
		gOpcode o;
		o.dc=&dc;
		o.opcode=gOpcode::blit;
		pos+=logicalZero;
		clip.moveBy(logicalZero.x(), logicalZero.y());
		o.parm.blit=new gOpcode::para::pblit(pixmap.lock(), pos, clip);
		o.flags=flags;
		rc.submit(o);
	}

	void setPalette(gRGB *colors, int start=0, int len=256);
	void mergePalette(gPixmap &target);
	
	void line(ePoint start, ePoint end);

	void setLogicalZero(ePoint abs);
	void moveLogicalZero(ePoint rel);
	void resetLogicalZero();
	
	void clip(eRect clip);
	void clippop();

	void flush();
};

class gPixmapDC: public gDC
{
protected:
	gPixmap *pixmap;
	eRect clip;

	void exec(gOpcode *opcode);
	gPixmapDC();
public:
	gPixmapDC(gPixmap *pixmap);
	virtual ~gPixmapDC();
	gPixmap &getPixmap() { return *pixmap; }
	gRGB getRGB(gColor col);
	const eRect &getClip() { return clip; }
	virtual eSize getSize() { return eSize(pixmap->x, pixmap->y); }
};

#endif
