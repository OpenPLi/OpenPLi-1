// (c) '96/97 Niklas Beisert
// hope it's okay to use it as GPL.
// this code was derived from a mpeg2 en/decoder source by the
// MPEG Software Simulation Group, 1994

// modified for ppcboot and dbox

static const unsigned char *bitrdbfr;
static int bitpos;
static char tabs[18432];

static unsigned char defqmatrix[64]=
{
	 8,16,16,19,16,19,22,22,22,22,22,22,26,24,26,27,
	27,27,26,26,26,26,27,27,27,29,29,29,34,34,34,29,
	29,29,27,27,29,29,32,32,34,34,37,38,37,35,35,34,
	35,38,38,40,40,40,48,48,46,46,56,56,58,69,69,83
};

static unsigned char zig_zag_scan[64]=
{
	0,1,8,16,9,2,3,10,17,24,32,25,18,11,4,5,
	12,19,26,33,40,48,41,34,27,20,13,6,7,14,21,28,
	35,42,49,56,57,50,43,36,29,22,15,23,30,37,44,51,
	58,59,52,45,38,31,39,46,53,60,61,54,47,55,62,63
};

static char DClumtab0[31][2] =
{
	{1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
	{0, 3}, {0, 3}, {0, 3}, {0, 3}, {3, 3}, {3, 3}, {3, 3}, {3, 3},
	{4, 3}, {4, 3}, {4, 3}, {4, 3}, {5, 4}, {5, 4}, {6, 5}
};

static char DClumtab1[16][2] =
{
	{7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6}, {7, 6},
	{8, 7}, {8, 7}, {8, 7}, {8, 7}, {9, 8}, {9, 8}, {10,9}, {11,9}
};

static char DCchromtab0[31][2] =
{
	{0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2}, {0, 2},
	{1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2}, {1, 2},
	{2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2}, {2, 2},
	{3, 3}, {3, 3}, {3, 3}, {3, 3}, {4, 4}, {4, 4}, {5, 5}
};

static char DCchromtab1[32][2] =
{
	{6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
	{6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6}, {6, 6},
	{7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7}, {7, 7},
	{8, 8}, {8, 8}, {8, 8}, {8, 8}, {9, 9}, {9, 9}, {10,10}, {11,10}
};

static char DCTtabnext[12][3] =
{
	{0,2,4},	{2,1,4},	{1,1,3},	{1,1,3},
	{64,0,2}, {64,0,2}, {64,0,2}, {64,0,2}, /* EOB */
	{0,1,2},	{0,1,2},	{0,1,2},	{0,1,2}
};

static char DCTtab0[60][3] =
{
	{65,0,6}, {65,0,6}, {65,0,6}, {65,0,6}, /* Escape */
	{2,2,7}, {2,2,7}, {9,1,7}, {9,1,7},
	{0,4,7}, {0,4,7}, {8,1,7}, {8,1,7},
	{7,1,6}, {7,1,6}, {7,1,6}, {7,1,6},
	{6,1,6}, {6,1,6}, {6,1,6}, {6,1,6},
	{1,2,6}, {1,2,6}, {1,2,6}, {1,2,6},
	{5,1,6}, {5,1,6}, {5,1,6}, {5,1,6},
	{13,1,8}, {0,6,8}, {12,1,8}, {11,1,8},
	{3,2,8}, {1,3,8}, {0,5,8}, {10,1,8},
	{0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
	{0,3,5}, {0,3,5}, {0,3,5}, {0,3,5},
	{4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
	{4,1,5}, {4,1,5}, {4,1,5}, {4,1,5},
	{3,1,5}, {3,1,5}, {3,1,5}, {3,1,5},
	{3,1,5}, {3,1,5}, {3,1,5}, {3,1,5}
};

static char DCTtab1[8][3] =
{
	{16,1,10}, {5,2,10}, {0,7,10}, {2,3,10},
	{1,4,10}, {15,1,10}, {14,1,10}, {4,2,10}
};

static char DCTtab2[16][3] =
{
	{0,11,12}, {8,2,12}, {4,3,12}, {0,10,12},
	{2,4,12}, {7,2,12}, {21,1,12}, {20,1,12},
	{0,9,12}, {19,1,12}, {18,1,12}, {1,5,12},
	{3,3,12}, {0,8,12}, {6,2,12}, {17,1,12}
};

static char DCTtab3[16][3] =
{
	{10,2,13}, {9,2,13}, {5,3,13}, {3,4,13},
	{2,5,13}, {1,7,13}, {1,6,13}, {0,15,13},
	{0,14,13}, {0,13,13}, {0,12,13}, {26,1,13},
	{25,1,13}, {24,1,13}, {23,1,13}, {22,1,13}
};

static char DCTtab4[16][3] =
{
	{0,31,14}, {0,30,14}, {0,29,14}, {0,28,14},
	{0,27,14}, {0,26,14}, {0,25,14}, {0,24,14},
	{0,23,14}, {0,22,14}, {0,21,14}, {0,20,14},
	{0,19,14}, {0,18,14}, {0,17,14}, {0,16,14}
};

static char DCTtab5[16][3] =
{
	{0,40,15}, {0,39,15}, {0,38,15}, {0,37,15},
	{0,36,15}, {0,35,15}, {0,34,15}, {0,33,15},
	{0,32,15}, {1,14,15}, {1,13,15}, {1,12,15},
	{1,11,15}, {1,10,15}, {1,9,15}, {1,8,15}
};

static char DCTtab6[16][3] =
{
	{1,18,16}, {1,17,16}, {1,16,16}, {1,15,16},
	{6,3,16}, {16,2,16}, {15,2,16}, {14,2,16},
	{13,2,16}, {12,2,16}, {11,2,16}, {31,1,16},
	{30,1,16}, {29,1,16}, {28,1,16}, {27,1,16}
};


unsigned long getswapdword(const void *p)		// not swapped since we're on BE
{
	return *(unsigned long*)p;
}

inline unsigned long showbits(int n)
{
	return (getswapdword(bitrdbfr+(bitpos>>3))<<(bitpos&7))>>(32-n);
}

inline void flushbits(int n)
{
	bitpos+=n;
}

inline unsigned long getbits(int n)
{
	unsigned long l=(getswapdword(bitrdbfr+(bitpos>>3))<<(bitpos&7))>>(32-n);
	bitpos+=n;
	return l;
}

inline unsigned long getbits1(void)
{
	int l=(bitrdbfr[(bitpos>>3)]>>(7-(bitpos&7)))&1;
	
	bitpos++;
	return l;
}

inline void startcode(void)
{
	bitpos=(bitpos+7)&~7;
	while (showbits(24)!=1)
		flushbits(8);
}


#define S2 181
#define W0 5793
#define W1 5681
#define W2 5352
#define W3 4816
#define W4 4096
#define W5 3218
#define W6 2217
#define W7 1130

inline void idctcol1(long *buf)
{
	long x0, x1, x2, x3, x4, x5, x6, x7, x8;

	x0 = buf[8*0];
	x1 = buf[8*4];
	x2 = buf[8*6];
	x3 = buf[8*2];
	x4 = buf[8*1];
	x5 = buf[8*7];
	x6 = buf[8*5];
	x7 = buf[8*3];
	x8 = W7*(x4+x5);
	x4 = x8 + (W1-W7)*x4;
	x5 = x8 - (W1+W7)*x5;
	x8 = W3*(x6+x7);
	x6 = x8 - (W3-W5)*x6;
	x7 = x8 - (W3+W5)*x7;

	x8 = ((x0+x1)<<12) + 16;
	x0 = ((x0-x1)<<12) + 16;
	x1 = W6*(x3+x2);
	x2 = x1 - (W2+W6)*x2;
	x3 = x1 + (W2-W6)*x3;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;

	x7 = x8 + x3;
	x8 -= x3;
	x3 = x0 + x2;
	x0 -= x2;
	x2 = (181*(x4+x5)+128)>>8;
	x4 = (181*(x4-x5)+128)>>8;

	buf[8*0] = (x7+x1)>>5;
	buf[8*1] = (x3+x2)>>5;
	buf[8*2] = (x0+x4)>>5;
	buf[8*3] = (x8+x6)>>5;
	buf[8*4] = (x8-x6)>>5;
	buf[8*5] = (x0-x4)>>5;
	buf[8*6] = (x3-x2)>>5;
	buf[8*7] = (x7-x1)>>5;
}

inline void idctrow2(long *buf, long *dst)
{
	long x0, x1, x2, x3, x4, x5, x6, x7, x8;

	x0 = buf[0];
	x1 = buf[4];
	x2 = buf[6];
	x3 = buf[2];
	x4 = buf[1];
	x5 = buf[7];
	x6 = buf[5];
	x7 = buf[3];
	x8 = W7*(x4+x5) + 2048;
	x4 = (x8+(W1-W7)*x4)>>12;
	x5 = (x8-(W1+W7)*x5)>>12;
	x8 = W3*(x6+x7) + 2048;
	x6 = (x8-(W3-W5)*x6)>>12;
	x7 = (x8-(W3+W5)*x7)>>12;

	x8 = x0 + x1;
	x0 -= x1;
	x1 = W6*(x3+x2) + 2048;
	x2 = (x1-(W2+W6)*x2)>>12;
	x3 = (x1+(W2-W6)*x3)>>12;
	x1 = x4 + x6;
	x4 -= x6;
	x6 = x5 + x7;
	x5 -= x7;

	x7 = x8 + x3 + 512;
	x8 += -x3 + 512;
	x3 = x0 + x2 + 512;
	x0 += -x2 + 512;
	x2 = (181*(x4+x5)+128)>>8;
	x4 = (181*(x4-x5)+128)>>8;

	dst[0] = (x7+x1)>>10;
	dst[1] = (x3+x2)>>10;
	dst[2] = (x0+x4)>>10;
	dst[3] = (x8+x6)>>10;
	dst[4] = (x8-x6)>>10;
	dst[5] = (x0-x4)>>10;
	dst[6] = (x3-x2)>>10;
	dst[7] = (x7-x1)>>10;
}


inline int getidctblock(long *bp, int comp, int *dctpred, const long *qmatrix, int qscale)
{
	int i, code, size, val;
	char *tab;

	comp=(comp<4)?0:(comp-3);

	for (i=0; i<64; i++)
		bp[i]=0;

	code=showbits(10);
	if (!comp)
	{
		if (code>=992)
			tab=DClumtab1[(code>>1)-496];
		else
			tab=DClumtab0[code>>5];
	}
	else
	{
		if (code>=992)
			tab=DCchromtab1[code-992];
		else
			tab=DCchromtab0[code>>5];
	}

	flushbits(tab[1]);
	size=tab[0];

	if (!size)
		val=0;
	else
	{
		val=getbits(size);
		if (!(val&(1<<(size-1))))
			val-=(1<<size)-1;
	}

	*bp=(dctpred[comp]+=val)<<3;

	i=1;
	while (1)
	{
		int sign;
		code=showbits(16);
		if (code>=16384)
			tab=DCTtabnext[(code>>12)-4];
		else if (code>=1024)
			tab=DCTtab0[(code>>8)-4];
		else if (code>=512)
			tab=DCTtab1[(code>>6)-8];
		else if (code>=256)
			tab=DCTtab2[(code>>4)-16];
		else if (code>=128)
			tab=DCTtab3[(code>>3)-16];
		else if (code>=64)
			tab=DCTtab4[(code>>2)-16];
		else if (code>=32)
			tab=DCTtab5[(code>>1)-16];
		else if (code>=16)
			tab=DCTtab6[code-16];
		else
			return 0;

		flushbits(tab[2]);

		if (tab[0]==64)
			return 1;

		if (tab[0]==65)
		{
			signed char tval;
			i+=getbits(6);
			tval=getbits(8);

			if (tval==0)
				val=getbits(8);
			else
			if (tval==-128)
				val=getbits(8)-256;
			else
				val=tval;
		}
		else
		{
			i+=tab[0];
			val=tab[1];
			if (getbits1())
				val=-val;
		}

		if (i>=64)
			return 0;

		sign=val<0;
		if (sign)
			val=-val;

		val=(val*qscale*qmatrix[i])>>3;

		val=(val-1)|1;
		bp[zig_zag_scan[i]]=sign?-val:val;
		i++;
	}
}


inline unsigned long clipcol(int r, int g, int b)
{
  return ((b<0)?0:(b>255)?255:b)|(((g<0)?0:(g>255)?255:g)<<8)|(((r<0)?0:(r>255)?255:r)<<16);
}
  
int decodestillmpg(void *pic, const void *src, int X_RESOLUTION, int Y_RESOLUTION)
{
	long *ubtab=(long*)(tabs+2048);
	long *ugtab=(long*)(tabs+6144);
	long *vrtab=(long*)(tabs+10320);
	long *vgtab=(long*)(tabs+14336);
	long *tmpblk=(long*)(tabs+16384);
	long *qmatrix=(long*)(tabs+17920);

	int i,j, owid, ohgt, mb_width, mb_height;
	int dctpred[3];
	int qscale;
#if 0
	int x,y,c;
#endif

	int MBA=0, MBAmax;
	for (i=-512; i<512; i++)
	{
		ubtab[i]=i*3629/2048;
		ugtab[i]=-i*11/32;
		vrtab[i]=i*11485/8192;
		vgtab[i]=-i*2925/4096;
	}
	for (i=0; i<64; i++)
		qmatrix[i]=defqmatrix[i];

	bitpos=0;
	bitrdbfr=(const unsigned char*)src;
	//for (i=0;i<10;i++) printf("%02x",bitrdbfr[i]); //ausgabe im ppcboot als test

	if (getbits(32)!=0x1B3)
		return 0;

	owid=getbits(12);
	ohgt=getbits(12);

	flushbits(38);
	if (getbits1())
		for (i=0; i<64; i++)
			qmatrix[i]=getbits(8);

	if (getbits1())
		flushbits(8*64);

	mb_width=(owid+15)>>4;
	 mb_height=(ohgt+15)>>4;

	while (1)
	{
		unsigned long code;
		startcode();
		code=getbits(32);
		if (code==0x100)
			break;
		else
		if (code==0x1B8)
		{
			flushbits(27);
			continue;
		}
		else
		if (code==0x1B2)
			continue;
		else
			return 0;
	}

	flushbits(10);
	if (getbits(3)!=1)
		return 0;
	flushbits(16);

	while (getbits1())
		flushbits(8);

	while (1)
	{
		int code;
		startcode();
		code=showbits(32);
		if (code!=0x1B2)
			break;
		flushbits(32);
	}

	MBAmax=mb_width*mb_height;
	
	while (1)
	{
		int bx, by;
		unsigned short *pp; 
		if (!showbits(23))
		{
			unsigned long code;
			if (MBA>=MBAmax)
				break;

again:
			startcode();

			code=getbits(32);
			
			if (code==0x1B5)
				goto again;

			if ((code<0x101)||(code>0x1AF))
				return 0;

			qscale=getbits(5);
			while (getbits1())
				flushbits(8);

			MBA=(code-0x101)*mb_width;

			dctpred[0]=dctpred[1]=dctpred[2]=0;
		}

		if (MBA>=MBAmax)
			return 0;

		if (!getbits1())
			return 0;

		if (!getbits1())
		{
			if (!getbits1())
				return 0;
			qscale=getbits(5);
		}

		for (i=0; i<6; i++)
		{
			int lx;
			long *rfp;
			if (!getidctblock(tmpblk+320,i,dctpred,qmatrix,qscale))
				return 0;
			for (j=0; j<8; j++)
				idctcol1(tmpblk+320+j);

			if (i<4)
			{
				lx=16;
				rfp=tmpblk+(i&2)*64+(i&1)*8;
			}
			else
			{
				lx=8;
				rfp=tmpblk+i*64;
			}
			for (j=0; j<8; j++)
				idctrow2(tmpblk+320+8*j, rfp+lx*j);
		}

		bx=16*(MBA%mb_width);
		by=16*(MBA/mb_width);

		for (j=0; j<8; j++)
			for (i=0; i<8; i++)
			{
				int u=tmpblk[256+8*j+i];
				int v=tmpblk[320+8*j+i];
				int r=vrtab[v];
				int g=ugtab[u]+vgtab[v];
				int b=ubtab[u];
				int y;
				long *p=tmpblk+16*2*j+2*i;
				y=128+p[0];
				p[0]=clipcol(y+r, y+g, y+b);
				y=128+p[1];
				p[1]=clipcol(y+r, y+g, y+b);
				y=128+p[16];
				p[16]=clipcol(y+r, y+g, y+b);
				y=128+p[17];
				p[17]=clipcol(y+r, y+g, y+b);
			}

		pp=((unsigned short*)pic)+by*X_RESOLUTION+bx;

		for (j=0; j<16; j++)
			for (i=0; i<16; i++)
			{
				int r, g, b;
				r=(tmpblk[j*16+i]>>16)&0xFF;
				g=(tmpblk[j*16+i]>>8)&0xFF;
				b=(tmpblk[j*16+i])&0xFF;
				pp[j*X_RESOLUTION+i]=(((r>>3)&0x1F)<<10)|(((g>>3)&0x1F)<<5)|(((b>>3)&0x1F));
			}

		MBA++;
	}
#if 0
	for (y=8; y<Y_RESOLUTION; y+=8)
		for (x=0; x<X_RESOLUTION; x++)
			for (c=0; c<3; c++)
			{
				char left=((char*)pic)[X_RESOLUTION*4*(y-1)+x*4+c];
				char right=((char*)pic)[X_RESOLUTION*4*y+x*4+c];
				char mid=(left+right+1)>>1;
				((char*)pic)[X_RESOLUTION*4*(y-1)+x*4+c]=(left+mid+1)>>1;
				((char*)pic)[X_RESOLUTION*4*y+x*4+c]=(right+mid+1)>>1;
			}
	for (y=0; y<Y_RESOLUTION; y++)
		for (x=8; x<X_RESOLUTION; x+=8)
			for (c=0; c<3; c++)
			{
				char left=((char*)pic)[X_RESOLUTION*4*y+4*(x-1)+c];
				char right=((char*)pic)[X_RESOLUTION*4*y+4*x+c];
				char mid=(left+right+1)>>1;
				((char*)pic)[X_RESOLUTION*4*y+4*(x-1)+c]=(left+mid+1)>>1;
				((char*)pic)[X_RESOLUTION*4*y+4*x+c]=(right+mid+1)>>1;
			}
#endif
	return 1;
}
