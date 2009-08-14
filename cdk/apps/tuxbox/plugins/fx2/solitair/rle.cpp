// rle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

void Usage()
{
	printf(
				"Usage:\n\n"
				"rle -c|-d <file_in>\n"
				"\nwhere \t-c  compress"
				"\n\t-d  decompress\n" );
}

int	Decompress( const char* filename );

int	Compress( const char* filename )
{
	if( NULL == filename || 0 == strlen( filename ) ) 
	{
		Usage();
		return 2;
	}
	
	FILE* out = NULL;
	FILE* in = fopen( filename, "rb" );
	
	char* filename_out;

	if( NULL == in )
	{
		printf("Cannot open [%s]\n", filename );
		return 2;
	}

	filename_out = (char*) malloc( strlen( filename + 4 ) );

	if( filename_out == NULL )
	{
		printf("Memory out!\n");
		return 2;
	}
	strcpy( filename_out, filename );
	strcat( filename_out, ".rle" );

	out = fopen( filename_out, "wb" );

	if( NULL == out )
	{
		printf("Cannot create [%s] for output\n", filename_out );
		return 2;
	}
	
	fwrite( filename, strlen( filename ) + 1, 1, out );

///////////////
//compress now
///////////////

	
	
	// 0xC0
	unsigned char	cursor = 0;

	unsigned char	bytes_to_read = 63;
	unsigned char	bytes[63];

	int				rep_byte = 0;
	bool			repeat = false;

	unsigned char	counter;

	while( 1 )
	{
		rep_byte = fgetc( in );
		
		if( rep_byte == EOF )
		{
			if( cursor == counter )
			{
				counter += 128;
				fputc( counter, out );
				fwrite( bytes, 1, counter - 128 , out );
	
				cursor = 1;
				bytes[0] = rep_byte;
				bytes[1] = 0;
			}
			else 
			{
				counter += 192;
				fputc( counter, out );
				fputc( bytes[0], out );
	
				cursor = 1;
				bytes[0] = rep_byte;
				bytes[1] = 0;
			}
		
			break;
		} 
		else
		if( cursor == 63 )
		{
			counter += 128;
			fputc( counter, out );
			fwrite( bytes, 1, 63, out );

			counter = 1;
			cursor = 1;
			bytes[0] = rep_byte;
			bytes[1] = 0;
		}
		else if( counter == 63 )
			{
				counter += 192;
				fputc( counter, out );
				fputc( bytes[0], out );
	
				cursor = 1;
				bytes[0] = rep_byte;
				bytes[1] = 0;
				counter = 1;
			}
			else if( cursor == 0 )
				{
					bytes[0] = rep_byte;
					bytes[1] = 0;
					counter = 1;
					cursor = 1;
				}
				else
				{
					if( rep_byte == bytes[cursor-1] && cursor == 1 )
					{
						counter++;
					}
					else if( rep_byte == bytes[cursor-1] )
					{
						// changed situation we have to flush out data, 
						// next we have repeated data to handle
				
						fputc( counter - 1 + 128 , out );
						fwrite( bytes, 1, counter - 1, out );

						bytes[0] = rep_byte;
						bytes[1] = 0;
						cursor = 1;
						counter = 2;
					}
					else if( cursor == 1 && counter > 1 )
					{
						fputc( counter + 192 , out );
						fputc( bytes[0], out );

						bytes[0] = rep_byte;
						bytes[1] = 0;
						cursor = 1;
						counter = 1;
					
					} else if( rep_byte != bytes[cursor-1] ) 
						{
					
							bytes[cursor] = rep_byte;
							bytes[cursor+1]  = 0;
							counter++;
							cursor++;
						}
						else
						{
							fputc( counter + 192 , out );
							fputc( bytes[0], out );

							bytes[0] = rep_byte;
							bytes[1] = 0;
							cursor = 1;
							counter = 1;
						}
				}
	}
	


	fclose( in );
	fclose( out );

	Decompress( filename_out );
	free(filename_out);

	return 0;
}

int Decompress( const char* filename )
{
	FILE* in = fopen( filename, "rb" );

	if( NULL == in )
	{
		printf("Cannot open [%s]\n", filename );
		return 2;
	}

	char filename_out[255];
	unsigned char cursor = 0;

	while( 
			cursor < 255 &&
			EOF != ( filename_out[cursor] = fgetc( in ) ) && 
			( filename_out[cursor] != 0 ) ) cursor++;
	
	if( cursor == 255 || EOF == filename_out[cursor] )
	{
		printf( "Decompress: invalid file format!\n" );
		fclose( in );
		return 3;
	}


	strcat( filename_out, ".out" );

	FILE* out = fopen( filename_out, "wb" );

	if( NULL == out )
	{
		printf("Cannot create [%s] for output\n", filename_out );
		fclose( in );
		return 2;
	}

	int byte;

	while( EOF != ( byte = fgetc( in ) ) )
	{
		char counter;
		
		if( !(byte&128) )
		{
			printf("Some error while decompressing!");
			fclose( in );
			fclose( out );
			return 3;
		}


		if( !(byte & 64) )
		{
			counter = byte - 128;

			while( EOF != ( byte = fgetc( in ) ) )
			{
				fputc( byte, out );
				if( counter-- == 1 ) break;
			}
		}
		else
		{
			counter = byte - 192;		
			byte = fgetc( in );
			
			while( counter-- > 0 ) fputc( byte, out );
		}
	}

	fclose( in );
	fclose( out );

	return 0;
}



int main(int argc, char* argv[])
{

	if( argc < 3 )
	{
		Usage();
		return 1;
	}

	if( 
		(argv[1][0] != '-') && 
		(argv[1][1] != 'c') && 
		(argv[1][1] != 'd') )
	{
		Usage();
		return -1;
	}


	switch( argv[1][1] )
	{
	case 'c': return Compress( argv[2] );
	case 'd': return Decompress( argv[2] );
	}

	return 0;
}
