#ifndef __PNM_FILE_H__
#define __PNM_FILE_H__

#define PNM_EOF -1

#include <stdlib.h>



struct PNM_FILE
{
   char*            file_name;
   unsigned char*   body;
   unsigned int     lenght;
   unsigned int     current_position;
   unsigned char	current_rle_pos;
   unsigned char	rle_counter;
};

struct IMAGE
{
	int			width;
	int			height;
	unsigned char*		raw_data;
	PNM_FILE*		pnm_file;

	IMAGE( )
	{
		width = 0;
		height = 0;
		raw_data = NULL;
		pnm_file = NULL;
	}

	~IMAGE( )
	{
		if( raw_data != NULL )
		{
			delete [] raw_data;

		}
	}

	bool	AllocateRawData()
	{
		if( width == 0 || height == 0 ) return false;

		if( raw_data != NULL )
		{
			delete [] raw_data;

		}

		raw_data = new unsigned char[ width*height];

		if( raw_data == NULL ) return false;
	}
};

int       	pnm_getc( PNM_FILE* _file );
int       	pnm_close( PNM_FILE* _file );
IMAGE*		read_image( const char* file_name );

extern PNM_FILE pnm_files[];
void Set_8Bit_Pal(void);
unsigned char Convert_24_8(unsigned char r, unsigned char g, unsigned char b);

#endif /* __CARDS_H__ */
