#ifndef DRAW_H
#define DRAW_H

typedef unsigned char	uchar;

extern	void	FBSetColor( int idx, uchar r, uchar g, uchar b );
extern	void	FBSetColorEx( int idx, uchar r, uchar g, uchar b, uchar transp );
extern	void	FBSetupColors( void );
extern	int	FBInitialize( int xRes, int yRes, int bpp, int extfd );
extern	void	FBClose( void );
extern	void	FBPaintPixel( int x, int y, unsigned char col );
extern	unsigned char	FBGetPixel( int x, int y );
extern	void	FBDrawLine( int xa, int ya, int xb, int yb, unsigned char col );
extern	void	FBDrawHLine( int x, int y, int dx, unsigned char col );
extern	void	FBDrawVLine( int x, int y, int dy, unsigned char col );
extern	void	FBFillRect( int x, int y, int dx, int dy, unsigned char col );
extern	void	FBDrawRect( int x, int y, int dx, int dy, unsigned char col );
extern	void	FBCopyImage( int x, int y, int dx, int dy, unsigned char *src );
extern	void	FB2CopyImage( int x, int y, int dx, int dy, unsigned char *src,
					int dbl );
extern	void	FBOverlayImage(int x, int y, int dx, int dy, int relx, int rely,
				unsigned char c1,
				unsigned char *src,
				unsigned char *under,
				unsigned char *right,
				unsigned char *bottom );
extern	void	FBCopyImageCol( int x, int y, int dx, int dy, unsigned char col,
					unsigned char *src );
extern	void	FBBlink( int x, int y, int dx, int dy, int count );
extern	void	FBMove( int x, int y, int x2, int y2, int dx, int dy );
extern	void	FBPrintScreen( void );
extern	void	FBPause( void );
extern	int	FBDrawString( int xpos, int ypos, int height, char *msg,
				unsigned char col,		/* text color */
				unsigned char backcol );/* background 0==transp */
extern	void	FBDrawFx2Logo( int x, int y );
extern	char	*FBEnterWord( int xpos, int ypos, int height,int len,
				unsigned char col);
extern	void	FBGetImage( int x1, int y1, int width, int height,
				unsigned char *to );
#ifdef USEX
extern	void	FBFlushGrafic( void );
#endif

#define BNR0		0
#define BLACK		1
#define	WHITE		2
#define	RED		3
#define	RESERVED	254

#endif
