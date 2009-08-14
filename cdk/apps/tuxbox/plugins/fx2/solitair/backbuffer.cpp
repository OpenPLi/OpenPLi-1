#include "backbuffer.h"
#include "string.h"


unsigned char*	backbuffer = NULL;

void 	BBCreate()
{
	backbuffer = new unsigned char[WIDTH * HEIGHT];
}

void 	BBFree()
{
	delete [] backbuffer;
}


void	BBSetBackground( unsigned char color )
{
	memset( backbuffer, color, WIDTH*HEIGHT );
}

void	BBFillRect( int _x, int _y, int _width, int _height, unsigned char color )
{
	int spos = _y * WIDTH + _x;

	for( int i = 0; i < _height; i++, spos += WIDTH )
	{
		if( spos >= (WIDTH*HEIGHT) || (spos + _width) >= ( WIDTH*HEIGHT ) ) return;

		memset( backbuffer + spos, color, _width );
	}
}

unsigned char* 	BBGetData( )
{
	return backbuffer;	
}
