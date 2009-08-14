#include "pnm_file.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
extern "C"
{
#include "draw.h"
}

static int		n_images = 0;
static int 		n_last_empty  = 0;
static IMAGE* 		images = NULL;

static unsigned char Index[6][7][6];

void Set_8Bit_Pal(void)
{
	int r,g,b;
	int ctr=0;
	for (r=0;r<6;r++)
		for (g=0;g<7;g++)
			for (b=0;b<6;b++)
			{
				FBSetColorEx( ctr, (r*51),(unsigned char)(g*42.5),(b*51),
					( r == ((int)(0/57)) && g == ((int)(100/42.5)) && b == ((int)(70/51)) )?60:0 );
				Index[r][g][b]=ctr;
				ctr++;
			}

	FBSetupColors();

}


unsigned char Convert_24_8(unsigned char r, unsigned char g, unsigned char b)
{
  r/=51;
  g = (unsigned char)(g / 42.5);
  b/=51;
  return Index[r][g][b];
}

static PNM_FILE* pnm_open( const char* _file_name )
{
	unsigned char i = 0;

	for( i = 0; pnm_files[i].file_name; i++ )
	{
		if( 0 == strcmp( _file_name, pnm_files[i].file_name ) )
		{
			PNM_FILE* file = (PNM_FILE*)malloc( sizeof( PNM_FILE ) );

			*file = pnm_files[i];

			return file;
		}
	}

	return NULL;
}


int pnm_getc( PNM_FILE* _file )
{
	int tmp_counter = _file->rle_counter > 0x80?_file->rle_counter-0x80:_file->rle_counter;


	if( NULL == _file ) return -2;

	if( _file->lenght == _file->current_position )
	{
		return PNM_EOF;
	}

	if( tmp_counter == _file->current_rle_pos )
	{
		if( _file->rle_counter > 0x80 )
			_file->current_position++;

		if( _file->lenght == _file->current_position )
		{
			return PNM_EOF;
		}

		_file->rle_counter = (int)_file->body[ _file->current_position++ ];
		_file->current_rle_pos = 0;
	}

	if( _file->rle_counter > 0x80 )
	{
		_file->current_rle_pos++;
		return (int)_file->body[ _file->current_position ];
	}
	else
	{
		_file->current_rle_pos++;
		return (int)_file->body[ _file->current_position++ ];
	}
}


int pnm_close( PNM_FILE* _file )
{
	if( NULL == _file ) return -2;

	free( _file );

	return 0;
}

IMAGE*	read_image( const char* _file_name )
{

	if(  n_images == 0 )
	{

		for( n_images = 0; pnm_files[n_images].file_name; n_images++ );

		images = new IMAGE[n_images];
	}

	PNM_FILE* pf = pnm_open( _file_name );

	if( pf == NULL ) return NULL;

	//search in image pool
	for( int i = 0; i < n_last_empty; i++ )
	{
		if( 0 == strcmp(pf->file_name, images[i].pnm_file->file_name ) )
		{
			return &images[i];
		}
	}

	memset( &images[n_last_empty], 0, sizeof( IMAGE ) );

	images[n_last_empty].pnm_file = pf;

	unsigned char i, j;
	long cur = 0;
	long file_size = 0;
	int buffer, r,g,b;

	char str[5];

	i = 0;
	while( PNM_EOF != ( buffer = pnm_getc( pf ) ) )
	{
		if( buffer <= 0x20 || i == 5 )
		{
			str[i] = '\0';
			break;
		}

		str[i++] = buffer;
	}

	if( strcmp( "P6", str ) )
	{
		pnm_close( pf );
		return NULL; // error, expected "P6"
	}

	//read WIDTH value
	i = 0;
	while( PNM_EOF != ( buffer = pnm_getc( pf ) ) )
	{
		if( buffer <= 0x20 || i == 5 )
		{
			str[i] = '\0';
//			some_changes = true;
			break;
		}
		str[i++] = buffer;
	}

	images[n_last_empty].width = atoi( str );

	//read HEIGHT value
	i = 0;
	while( PNM_EOF != ( buffer = pnm_getc( pf ) ) )
	{
		if( buffer <= 0x20 || i == 5 )
		{
			str[i] = '\0';
			break;
		}
		str[i++] = buffer;
	}

	images[n_last_empty].height = atoi( str );

	//read MAXCOLOR value
	i = 0;
	while( PNM_EOF != ( buffer = pnm_getc( pf ) ) )
	{
		if( buffer <= 0x20 || i == 5 )
		{
			str[i] = '\0';
			break;
		}
		str[i++] = buffer;
	}

	if( false == images[n_last_empty].AllocateRawData() )
	{
		return NULL;
	}

	cur = 0;
	while( PNM_EOF != ( r = pnm_getc( pf ) ) &&
		PNM_EOF != ( g = pnm_getc( pf ) ) &&
		PNM_EOF != ( b = pnm_getc( pf ) )
		)
	{
		unsigned char cidx = Convert_24_8( r,g,b );
		images[n_last_empty].raw_data[ cur++ ] = cidx;
	}

	return &images[n_last_empty++];
}


