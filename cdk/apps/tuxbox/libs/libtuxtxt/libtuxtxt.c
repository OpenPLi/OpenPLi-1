/******************************************************************************
 *                      <<< TuxTxt - Teletext Plugin >>>                      *
 *                                                                            *
 *             (c) Thomas "LazyT" Loewe 2002-2003 (LazyT@gmx.net)             *
 *                                                                            *
 *    TOP-Text Support 2004 by Roland Meier <RolandMeier@Siemens.com>         *
 *    Info entnommen aus videotext-0.6.19991029,                              *
 *    Copyright (c) 1994-96 Martin Buck  <martin-2.buck@student.uni-ulm.de>   *
 *                                                                            *
 ******************************************************************************/

#ifdef DEBUG
#undef DEBUG
#endif

#define DEBUG 0

#include <sys/ioctl.h>
#include <fcntl.h>

// __USE_GNU is needed for PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP

#ifndef __USE_GNU
    #define __USE_GNU
    #include <pthread.h>
    #undef __USE_GNU
#else
    #include <pthread.h>
#endif

#include "tuxtxt_common.h"

/******************************************************************************
 * Initialize                                                                 *
 ******************************************************************************/

static int tuxtxt_initialized=0;
static pthread_mutex_t tuxtxt_control_lock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

int tuxtxt_init()
{
	if ( tuxtxt_initialized )
		return 0;

	pthread_mutex_lock(&tuxtxt_control_lock);
	tuxtxt_initialized=1;

	/* init data */
	memset(&tuxtxt_cache.astCachetable, 0, sizeof(tuxtxt_cache.astCachetable));
	memset(&tuxtxt_cache.astP29, 0, sizeof(tuxtxt_cache.astP29));

	tuxtxt_clear_cache();
	tuxtxt_cache.receiving = 0;
	tuxtxt_cache.thread_starting = 0;
	tuxtxt_cache.vtxtpid = -1;
	tuxtxt_cache.thread_id = 0;
	tuxtxt_cache.dmx = -1;
	pthread_mutex_unlock(&tuxtxt_control_lock);
	return 1;//tuxtxt_init_demuxer();
}

/******************************************************************************
 * Interface to caller                                                        *
 ******************************************************************************/

int tuxtxt_stop()
{
	if (!tuxtxt_cache.receiving) return 1;
	tuxtxt_cache.receiving = 0;

	pthread_mutex_lock(&tuxtxt_control_lock);
	int res = tuxtxt_stop_thread();
	pthread_mutex_unlock(&tuxtxt_control_lock);
	return res;
}
int tuxtxt_start(int tpid)
{
	int ret = 1;
	pthread_mutex_lock(&tuxtxt_control_lock);
	if (tuxtxt_cache.vtxtpid != tpid)
	{
		tuxtxt_stop();
		tuxtxt_clear_cache();
		tuxtxt_cache.page = 0x100;
		tuxtxt_cache.vtxtpid = tpid;
		ret = tuxtxt_start_thread();
	}
	else if (!tuxtxt_cache.thread_starting && !tuxtxt_cache.receiving)
	{
		ret = tuxtxt_start_thread();
	}
	pthread_mutex_unlock(&tuxtxt_control_lock);
	return ret;
}

void tuxtxt_close()
{
	pthread_mutex_lock(&tuxtxt_control_lock);
#if DEBUG
	printf ("cleaning up\n");
#endif
	tuxtxt_stop();
	if (tuxtxt_cache.dmx != -1)
    	    close(tuxtxt_cache.dmx);
	tuxtxt_cache.dmx = -1;
	tuxtxt_clear_cache();
	tuxtxt_initialized=0;
	pthread_mutex_unlock(&tuxtxt_control_lock);
}
/******************************************************************************
 * Generating HTML-Code                                                       *
 ******************************************************************************/
void tuxtxt_EndHTML(tstHTML* pHTML)
{
	if (pHTML->page_char) free(pHTML->page_char);
	if (pHTML->page_atrb) free(pHTML->page_atrb);
	free (pHTML);
	pHTML = NULL;
}
tstHTML* tuxtxt_InitHTML()
{
	tstHTML *pHTML = malloc(sizeof(tstHTML));
	if (pHTML)
	{
		pHTML->page_char = malloc(25*40);
		if (!pHTML->page_char)
		{
			tuxtxt_EndHTML(pHTML);
			return NULL;
		}
		pHTML->page_atrb = malloc(25*40*sizeof(tstPageAttr));
		if (!pHTML->page_atrb)
		{
			tuxtxt_EndHTML(pHTML);
			return NULL;
		}
		pHTML->row = 0;
		pHTML->col = 0;
		pHTML->stylecount_n=0;
		pHTML->stylecount_d=0;
		pHTML->stylecount_g=0;
		pHTML->stylecount_b=0;
		pHTML->pageinfo = tuxtxt_DecodePage(1, pHTML->page_char, pHTML->page_atrb, 0, 0);
		memset(pHTML->cstyles_n,0,1024*sizeof(unsigned short));
		memset(pHTML->cstyles_d,0,1024*sizeof(unsigned short));
		memset(pHTML->cstyles_g,0,32*sizeof(unsigned short));
		memset(pHTML->cstyles_b,0,32*sizeof(unsigned short));
		if (pHTML->pageinfo &&
			tuxtxt_cache.national_subset <= NAT_MAX_FROM_HEADER && 
			pHTML->pageinfo->nationalvalid) 
			tuxtxt_cache.national_subset = countryconversiontable[pHTML->pageinfo->national];
	}
	return pHTML;

}
void tuxtxt_color2string(const unsigned short * pcolormap, int color, char* colstr)
{
	if (!pcolormap)
		pcolormap = tuxtxt_defaultcolors;
	else
	{
		if (color < 16)
			pcolormap = tuxtxt_defaultcolors;
		else
			color %= 16;
	}
	int cval = (((int)(pcolormap[color]) & 0xf00))>> 4 | 
		   (((int)(pcolormap[color]) & 0x0f0))<< 8 | 
		   (((int)(pcolormap[color]) & 0x00f))<<20;
	sprintf(colstr,"#%06X",cval);
}

void tuxtxt_RenderGraphicHTML(unsigned char* graphbuffer,// buffer containing the graphics data (size must be >=xres*10)
			      int xres,// length of 1 line in graphbuffer (must be >= 12)
			      const char* fgclass, // CSS-class for foreground color
			      const char* bgclass, // CSS-class for background color
			      tstPageAttr* pAttr, // Attributes of Graphic
			      int col, // current column (0..39 )
			      char* result ) 
{

	unsigned char *p,*p1;
	int srow, scol;
	char tmpbuf[300];
	char classfg[20];
	sprintf(tmpbuf,"<td %s%s><table class=%s cellspacing=0 cellpadding=0><colgroup><col width=1 span=12></colgroup>",(pAttr->doublew ? " COLSPAN=2" : ""),(pAttr->doubleh ? " ROWSPAN=2" : ""),bgclass);
	strcat(result,tmpbuf);
	sprintf(classfg," class=%s",fgclass);
	for (srow = 0; srow < 10; srow++)
	{
		strcat(result,"<tr>");
		p = NULL;
		int pcount = 0;
		for (scol = 0; scol < 12; scol++)
		{
			p1 = &graphbuffer[srow*xres+scol];
			if (p)
			{
				if (*p1 == *p) 
					pcount++;
				else
				{
					strcat(result,"<td");
					if (*p == pAttr->fg)
						strcat(result,classfg);
					
					if (pcount>0) 
					{
						sprintf(tmpbuf," colspan=%d",pcount+1);
						strcat(result,tmpbuf);
					}
					strcat(result,"></td>");
					pcount = 0;
					p = p1;
				}
			}
			else 
				p = p1;
		}
		strcat(result,"<td");
		if (*p == pAttr->fg)
			strcat(result,classfg);
		if (pcount>0) 
		{
			sprintf(tmpbuf," colspan=%d",pcount+1);
			strcat(result,tmpbuf);
		}
		strcat(result,"></td></tr>\n");
	}
	strcat(result,"</table></td>");
}
void tuxtxt_RenderTextHTML(int Char,// Character to render
			   const char* class, // CSS-class 
			   tstPageAttr* pAttr, // Attributes of Graphic
			   int col, // current column (0..39 )
			   char* result ) // resulting html-string, buffersize must be >= 300
{
	char tmp[300];
	sprintf(tmp,"<td class=%s%s%s>%s%s",class,(pAttr->doublew ? " COLSPAN=2" : ""),(pAttr->doubleh ? " ROWSPAN=2" : ""),(pAttr->underline ? "<u>" : ""),(pAttr->flashing ? "<blink>":""));
	strcat(result,tmp);
	if (Char < 0)
		strcat(result,"&nbsp;");
	else
	{
		if (Char > 0x20 && Char <= 0x7f)
			result[strlen(result)] = (char)Char;
		else
		{
			sprintf(tmp,"&#%d;",Char);
			strcat(result,tmp);
		}
		if (pAttr->diacrit && pAttr->diacrit <16)
		{
			int dia[] ={0x20,0x300,0x301,0x302,0x303,0x304,0x306,0x307,0x308,0x323,0x30a,0x317,0x331,0x30b,0x316,0x30c};
			sprintf(tmp,"&#%d;",dia[pAttr->diacrit]);
			strcat(result,tmp);
		}
	}
	if (pAttr->underline) strcat(result,"</u>");
	if (pAttr->flashing) strcat(result,"</blink>");
	strcat(result,"</td>");
}
void tuxtxt_RenderLinkHTML(int number,// Pagenumber to render 
			   const char* class, // CSS-class
			   tstPageAttr* pAttr, // Attributes of Graphic
			   int col, // current column (0..39 )
			   char* result ) // resulting html-string, buffersize must be >= 300
{
	char tmpbuf[300];
	sprintf(tmpbuf,"<td class=%s%s COLSPAN=%d%s>%s<a href=\"javascript:setpage(%d,-1)\" class=%s>%d</a>%s</td>",class,(pAttr->doublew && pAttr->doubleh ? "D":""),(pAttr->doublew ? 6 : 3),(pAttr->doubleh ? " ROWSPAN=2" : ""),(pAttr->underline ? "<u>" : ""),number,class,number,(pAttr->underline ? "</u>" : ""));
	strcat(result,tmpbuf);
}
void tuxtxt_RenderStyleHTML(const char* colorstyle, // CSS-classname
			   const char* fcolor, // foreground color
			   const char* bcolor, // background color
			   int typ, // 0 = normal, 1 = double size, 2 = graphic char foreground,  3 = graphic char background
			   char* result ) // resulting html-string, buffersize must be >= 200
{
	switch(typ)
	{
		case 0:
		case 1:
			sprintf(result,".%s {font-family:Courier,monospace;font-weight:bold;font-size:%d;color:%s;background-color:%s;text-align:center;width:12;height:20}\n",colorstyle,1<<(4+typ),fcolor,bcolor);
			break;
		case 2:
			sprintf(result,".%s {background-color:%s;width:1;height:2}\n",colorstyle,fcolor);
			break;
		case 3:
			sprintf(result,".%s {background-color:%s;table-layout:fixed;width:12;height:20;border-style:none}\n",colorstyle,fcolor);
			break;
	}
	
}
void tuxtxt_RenderStylesHTML(tstHTML* pHTML,
		      char* style) // pointer to new generated css-style, must be >= 200
{
	if (!pHTML)
	{
		return;
	}	

	if (pHTML->row >23 || pHTML->col >= 40)
	{
		printf("tuxtxt_RenderStylesHTML:irregular row(%d) or column (%d)\n",pHTML->row,pHTML->col);
		return;
	}
	if (!pHTML->pageinfo)
	{
		pHTML->row = 24;
		pHTML->col = 0;
		return;
	}
	int stylenr;
	tstPageAttr* pAttr;
	int PosX = 0;
	char bcolor[10];
	char fcolor[10];
	char colorstyle[10];
	memset(style,0,200);
	unsigned char axdrcs[12+1+10+1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
	pAttr = &pHTML->page_atrb[pHTML->row*40+pHTML->col];
	int Char = tuxtxt_RenderChar(NULL, 24, pHTML->page_char[pHTML->row*40+pHTML->col], &PosX, 0, pAttr, 0, 12, 12, 10, 0, axdrcs,2);
	tuxtxt_color2string(tuxtxt_cache.colortable,pAttr->fg,fcolor);
	tuxtxt_color2string(tuxtxt_cache.colortable,pAttr->bg,bcolor);
	stylenr = ((pAttr->fg)<<5)|pAttr->bg;
	if (pAttr->doublew && pAttr->doubleh)
	{		
		if (pHTML->cstyles_d[stylenr] == 0)
		{
			pHTML->stylecount_d++;
			pHTML->cstyles_d[stylenr] = pHTML->stylecount_d;
			sprintf(colorstyle,"d%x",pHTML->cstyles_d[stylenr]);
			tuxtxt_RenderStyleHTML(colorstyle,fcolor,bcolor,1,style);
		}
	}
	else
	{
		if (Char == 0 && (pAttr->fg == pAttr->bg))
			Char = 0x20;
		if (Char == 0) // graphic character
		{
			stylenr = pAttr->fg;
			if (pHTML->cstyles_g[stylenr] == 0)
			{
				pHTML->stylecount_g++;
				pHTML->cstyles_g[stylenr] = pHTML->stylecount_g;
				sprintf(colorstyle,"g%x",pHTML->cstyles_g[stylenr]);
				tuxtxt_RenderStyleHTML(colorstyle,fcolor,bcolor,2,style);
			}
			stylenr = pAttr->bg;
			if (pHTML->cstyles_b[stylenr] == 0)
			{
				char tmpstyle[200];
				pHTML->stylecount_b++;
				pHTML->cstyles_b[stylenr] = pHTML->stylecount_b;
				sprintf(colorstyle,"b%x",pHTML->cstyles_b[stylenr]);
				tuxtxt_RenderStyleHTML(colorstyle,bcolor,fcolor,3,tmpstyle);
				strcat(style,tmpstyle);
			}
		}
		else
		{
			if (pHTML->cstyles_n[stylenr] == 0)
			{
				pHTML->stylecount_n++;
				pHTML->cstyles_n[stylenr] = pHTML->stylecount_n;
				sprintf(colorstyle,"n%x",pHTML->cstyles_n[stylenr]);
				tuxtxt_RenderStyleHTML(colorstyle,fcolor,bcolor,0,style);
			}
		}
	}
	if (pHTML->col < 39 && pAttr->doublew)
	{
		pHTML->page_char[40*pHTML->row+pHTML->col+1] = 0xff;
		pHTML->page_atrb[40*pHTML->row+pHTML->col+1].doubleh = 0;
		pHTML->page_atrb[40*pHTML->row+pHTML->col+1].doublew = 0;
	}

	if (pHTML->row < 23 && pAttr->doubleh)
	{
		pHTML->page_char[40*(pHTML->row+1)+pHTML->col] = 0xff;
		pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col].doubleh = 0;
		pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col].doublew = 0;
		if (pAttr->doublew && pHTML->col < 39)
		{
			pHTML->page_char[40*(pHTML->row+1)+pHTML->col+1] = 0xff;
			pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col+1].doubleh = 0;
			pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col+1].doublew = 0;
		}
	}
	pHTML->col++;
	if (pHTML->col == 40)
	{
		pHTML->row++;
		pHTML->col = 0;
	}
}
void tuxtxt_RenderHTML(tstHTML* pHTML,
		      char* result ) // resulting html-string, buffersize must be >= 4000
{
	if (!pHTML)
	{
		strcpy(result,"Teletext not available");
		return;
	}	

	if (pHTML->row >23 || pHTML->col >= 40)
	{
		printf("irregular row(%d) or column (%d)\n",pHTML->row,pHTML->col);
		return;
	}
	if (!pHTML->pageinfo)
	{
		strcpy(result,"Page not available");
		pHTML->row = 24;
		return;
	}
	memset (result,0,4000);
	if (pHTML->row == 0 && pHTML->col == 0)
	{	
		strcpy(result,"<table border=0 cellspacing=0 cellpadding=0 style=\"table-layout:fixed;height=450;width=480\" ><colgroup><col width=12 span=40 /></colgroup>");
	}
	if (pHTML->col == 0)
		strcat(result,"<tr>");
		
	int stylenr;
	int numberlink = 0;
	int linkcount = 0;
	tstPageAttr* pAttr;
	int PosX = 0;
	char colorstyle[10];
	char graphfgstyle[10];
	char graphbgstyle[10];
	char linkcolorstyle[10];
	char nonumberlink[300];
	char bcolor[10];
	char fcolor[10];
	unsigned char  charbuffer[24*20]; 
	unsigned char axdrcs[12+1+10+1] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23};
	memset(nonumberlink,0,300);
	while (1)
	{
		pAttr = &pHTML->page_atrb[pHTML->row*40+pHTML->col];
		memset(charbuffer,pAttr->bg,24*20);
		int Char = tuxtxt_RenderChar(charbuffer, 24, pHTML->page_char[pHTML->row*40+pHTML->col], &PosX, 0, pAttr, 0, 12, 12, 10, 0, axdrcs,2);
		tuxtxt_color2string(tuxtxt_cache.colortable,pAttr->fg,fcolor);
		tuxtxt_color2string(tuxtxt_cache.colortable,pAttr->bg,bcolor);
		stylenr = ((pAttr->fg)<<5)|pAttr->bg;
		if (pAttr->doublew && pAttr->doubleh)
		{		
			sprintf(colorstyle,"d%x",pHTML->cstyles_d[stylenr]);
		}
		else
		{
//			if (Char == 0 && (pAttr->fg == pAttr->bg || !memchr(charbuffer,pAttr->fg,24*20)))
			if (Char == 0 && (pAttr->fg == pAttr->bg))
				Char = 0x20;
			if (Char == 0) // graphic character
			{
				stylenr = pAttr->fg;
				sprintf(graphfgstyle,"g%x",pHTML->cstyles_g[stylenr]);
				stylenr = pAttr->bg;
				sprintf(graphbgstyle,"b%x",pHTML->cstyles_b[stylenr]);
			}
			else
			{
				sprintf(colorstyle,"n%x",pHTML->cstyles_n[stylenr]);
			}
		}
		if (pHTML->col < 39 && pAttr->doublew)
		{
			pHTML->page_char[40*pHTML->row+pHTML->col+1] = 0xff;
			pHTML->page_atrb[40*pHTML->row+pHTML->col+1].doubleh = 0;
			pHTML->page_atrb[40*pHTML->row+pHTML->col+1].doublew = 0;
		}

		if (pHTML->row < 23 && pAttr->doubleh)
		{
			pHTML->page_char[40*(pHTML->row+1)+pHTML->col] = 0xff;
			pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col].doubleh = 0;
			pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col].doublew = 0;
			if (pAttr->doublew && pHTML->col < 39)
			{
				pHTML->page_char[40*(pHTML->row+1)+pHTML->col+1] = 0xff;
				pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col+1].doubleh = 0;
				pHTML->page_atrb[40*(pHTML->row+1)+pHTML->col+1].doublew = 0;
			}
		}
		if (pHTML->row > 0 && Char>= 0x30 && Char <= 0x39) //Digits
		{
			if (linkcount == 0)
			{
				memset(nonumberlink,0,300);
				strcpy(linkcolorstyle,colorstyle);
			}
			numberlink= numberlink*10 + (Char&0x0f);
			tuxtxt_RenderTextHTML(Char,colorstyle, pAttr,pHTML->col,nonumberlink);
			
			linkcount++;
			pHTML->col++;
			if (pHTML->col == 40)
			{
				if (linkcount == 3 && numberlink >= 100 && numberlink < 900)
					tuxtxt_RenderLinkHTML(numberlink,linkcolorstyle, pAttr,pHTML->col,result);
				else
					strcat(result,nonumberlink);
				break;
			}
			
		}
		else
		{
			if (linkcount == 3 && numberlink >= 100 && numberlink < 900)
				tuxtxt_RenderLinkHTML(numberlink,linkcolorstyle, pAttr,pHTML->col,result);
			else
				strcat(result,nonumberlink);
			if (Char!=-1)
			{
				if (Char != 0)
					tuxtxt_RenderTextHTML(Char,colorstyle, pAttr,pHTML->col,result);
				else 				
					tuxtxt_RenderGraphicHTML(charbuffer,24,graphfgstyle,graphbgstyle,pAttr,pHTML->col, result);
			}
			pHTML->col++;
			break;
		}
	}
	if (pHTML->col == 40)
	{
		strcat(result,"</tr>\n");
		pHTML->col = 0;
		pHTML->row++;
		if (pHTML->row == 24)
			strcat(result,"</table>\n");
	}
}

/* Local Variables: */
/* indent-tabs-mode:t */
/* tab-width:3 */
/* c-basic-offset:3 */
/* comment-column:0 */
/* fill-column:120 */
/* End: */
