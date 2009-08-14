/*
** initial coding by fx2
*/


#include <stdio.h>
#include <stdlib.h>

#include <draw.h>
#include <sys/time.h>
#include <rcinput.h>
#include <colors.h>
#include <fcntl.h>
#include <zlib.h>
#include <malloc.h>

#include <pics.h>
#define	COMPSZ	8033

extern	int		doexit;

extern	unsigned short	realcode;
extern	unsigned short	actcode;

typedef struct _Pic
{
	unsigned char	*pic_data;
	unsigned char	*pic_flip;
	int				ani;		// how many animation levels in pic
	int				width;
	int				height;
} Pic;

#define	NUMPICS		36

static	Pic	pics[] = {
{ 0,	0,	2,	14,	14 },		// 0 cursor
{ 0,	0,	9,	41,	22 },		// 1 tor
{ 0,	0,	6,	6,	5 },		// 2 flamme
{ 0,	0,	7,	6,	10 },		// 3 lemming1
{ 0,	0,	11,	10,	10 },		// 4 lemming2
{ 0,	0,	12,	12,	12 },		// 5 lemming3
{ 0,	0,	5,	5,	11 },		// 6 lemming4
{ 0,	0,	2,	41,	25 },		// 7 haus
{ 0,	0,	1,	16,	16 },		// 8 explosion
{ 0,	0,	3,	32,	14 },		// 9 feuer
{ 0,	0,	1,	32,	32 },
{ 0,	0,	1,	41,	3 },
{ 0,	0,	1,	9,	18 },
{ 0,	0,	1,	26,	8 },
{ 0,	0,	1,	11,	26 },
{ 0,	0,	1,	66,	159 },
{ 0,	0,	1,	287,110 },
{ 0,	0,	1,	181,82 },
{ 0,	0,	1,	47,	160 },
{ 0,	0,	1,	16,	24 },
{ 0,	0,	1,	32,	24 },
{ 0,	0,	1,	33,	11 },
{ 0,	0,	1,	13,	17 },
{ 0,	0,	1,	13,	17 },
{ 0,	0,	1,	54,	13 },
{ 0,	0,	1,	34,	1 },
{ 0,	0,	1,	17,	1 },
{ 0,	0,	1,	25,	6 },
{ 0,	0,	1,	32,	32 },
{ 0,	0,	5,	9,	13 },	// 29 menu-icons (no ani)
{ 0,	0,	1,	14, 14 }, 	// 30 level6 short mask
{ 0,	0,  17, 9,  13 },	// 31 lemming - builder
{ 0,	0,  8, 	8,  11 },	// 32 lemming - builder - ende
{ 0,	0,	22,	17,	17 },	// 33 lemming - hacke
{ 0,	0,	1,	6,	14 },	// 34 maske hacke
{ 0,	0,	8,	9,	15 }	// 35 lemming - rutscher
};

static	int		piccolors[] = {
	0x00EE00,
	0x00AA00,
	0xFFFF00,
	0x660000,
	0x000033,
	0xCC7700,
	0xAA5511,
	0x993311,
	0x444455,
	0x666677,
	0x888888,
	0x226622,
	0x4444EE,
	0x779900,
	0xFFDDDD,
	0xFF2222,
	0x00BB00,
	0xDDBB88,
	0x010101,
	0xDDBB44,
	0xFFFF66,
	0xBB8822,
	0xAA5500,
	0x883311,
	0x550000,
	0xCC0000,
	0x665555,
	0x880000,
	0x887777,
	0x554444,
	0x442222,
	0xFFFFFF,
	0x5E1A1A,
	0x995544,
	0xC49022,
	0xBB881A,
	0x884433,
	0x66221A,
	0x904C3C,
	0xBB9990,
	0x3C0808,
	0x773C2A,
	0xE6D46E,
	0xDDDDDD,
	0x2A0000,
	0x330000,
	0x883C33,
	0x441108,
	0x80332A,
	0x330808,
	0x772222,
	0x552211,
	0x6E3322,
	0x662A22,
	0x4C1A11,
	0xB27711,
	0x4C2A22,
	0x2A1108,
	0x3C1111,
	0x3C1108,
	0x220000,
	0x770000,
	0xDD6600,
	0xDD2A00,
	0xEE9000,
	0xEECC00,
	0x44221A,
	0x1A0000,
	0x4C1111,
	0xEE5500,
	0x882A00,
	0x220808,
	0x901108,
	0x800000,
	0x551111,
	0x880808,
	0xB29088,
	0xBB4C1A,
	0x441111,
	0xC45522,
	0x55221A,
	0x304536,
	0x30593E,
	0x305236,
	0x3E6745,
	0x303E30,
	0x303630,
	0x303030,
	0x304C30,
	0x304C36,
	0x3E673E,
	0x36674C,
	0x3E674C,
	0x36593E,
	0x366145,
	0x304530,
	0x36613E,
	0x365236,
	0x365936,
	0x364C36,
	0x3E6E4C,
	0x36523E,
	0x3E7559,
	0x457552,
	0x364C3E,
	0x457559,
	0x457C61,
	0x3E6E52,
	0xDD8822 };

#if 0
#include <tools.c.mayk>
#include <i33.h>
#endif

int	LoadPics( void )
{
	unsigned long	i;
	unsigned char	*data;
	int				sz;

	data = malloc(UNZSIZE+100);
	i=UNZSIZE+100;
	if ( uncompress(data,&i,pic_img_gz,COMPSZ) != Z_OK )
	{
		free(data);
		return -1;
	}

	pics[0].pic_data=data;
	sz=pics[0].width*pics[0].height*pics[0].ani;
	for( i=1; i<NUMPICS; i++ )
	{
		data+=sz;
		pics[i].pic_data=data;
		sz=pics[i].width*pics[i].height*pics[i].ani;
	}

#if 0
// add new
	pics[35].pic_data=px_data;
	WritePics();
#endif

	return 0;
}

unsigned char	*GetAniPic( int idx, int ani, int *width, int *height )
{
	int		sz;

	*width = pics[idx].width;
	*height = pics[idx].height;

	if ( !ani )
		return pics[idx].pic_data;

	sz=pics[idx].width*pics[idx].height;

	return pics[idx].pic_data+(ani*sz);
}

unsigned char	*GetPic( int idx, int *maxani, int *width, int *height )
{
	*width = pics[idx].width;
	*height = pics[idx].height;
	*maxani = pics[idx].ani-1;
	return pics[idx].pic_data;
}

void	RemovePics( void )
{
	int		i;

	for( i=0; i<NUMPICS; i++ )
		if ( pics[i].pic_flip )
			free( pics[i].pic_flip );
	if ( pics[0].pic_data )
		free( pics[0].pic_data );		// all in one !
}

void	PicSetupColors( void )
{
	int		i;
	int		num = sizeof(piccolors)/sizeof(int);
	int		r, g, b;

	for( i=0; i<num; i++ )
	{
		r=(piccolors[i] >> 16) & 0xff;
		g=(piccolors[i] >> 8) & 0xff;
		b=piccolors[i] & 0xff;
		FBSetColor( i+10, r, g, b );
	}
}

unsigned char *GetMirrorPic( int picid )
{
	int				sz;
	int				y;
	int				x;
	int				width;
	int				height;
	unsigned char	*data1;
	unsigned char	*data;

	if ( pics[picid].pic_flip )
		return pics[picid].pic_flip;

	width=pics[picid].width;
	height=pics[picid].height;
	sz=pics[picid].width*pics[picid].height;
	pics[picid].pic_flip = malloc(sz*pics[picid].ani);
	data=pics[picid].pic_flip;
	data1=pics[picid].pic_data;
	for( y=0; y < height * pics[picid].ani; y++ )
		for( x=0; x < width; x++ )
			data[ y*width+x ] = data1[ y*width+width-x-1 ];

	return data;
}
