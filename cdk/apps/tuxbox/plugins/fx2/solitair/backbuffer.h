#ifndef __BACKBUFFER__
#define __BACKBUFFER__

#define WIDTH		720
#define HEIGHT		576

void 	BBCreate();
void 	BBFree();

extern unsigned char*	backbuffer;

inline void	BBPutPixel( unsigned char color, int x, int y  )
{
	backbuffer[ y * WIDTH + x ] = color;
}

void	BBSetBackground( unsigned char color );
void	BBFillRect( int _x, int _y, int _width, int _height, unsigned char color );
unsigned char* 	BBGetData( );

#endif /*__BACKBUFFER__*/
