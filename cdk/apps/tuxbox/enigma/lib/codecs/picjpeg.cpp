#ifndef DISABLE_FILE

#include <lib/codecs/picjpeg.h>
#include <lib/base/eerror.h>
#include <lib/base/esize.h>
#include <lib/gdi/gpixmap.h>

ePictureDecoderJPEG::ePictureDecoderJPEG(eIOBuffer &input): ePictureDecoder(input)
{
	// set source!
	cinfo.err=jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
}

ePictureDecoderJPEG::~ePictureDecoderJPEG()
{
	jpeg_destroy_decompress(&cinfo);
}

int ePictureDecoderJPEG::decodeMore(int maxscanlines)
{
	int decodedscanlines=0;
	while (decodedscanlines < maxscanlines)
	{
		int code;
		switch (state)
		{
		case stateHeader:
			code=jpeg_read_header(&cinfo, TRUE);
			if (code == JPEG_SUSPENDED)
				return 1024;
			if (code == JPEG_HEADER_OK)
			{
				jpeg_calc_output_dimensions(&cinfo);
					// allocate buffer for decompression

				row_stride=cinfo.output_width * cinfo.output_components;
				buffer=(*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride*16, 1);

					// allocate a gImage for the output
				if ((cinfo.output_width > 1024)	|| (cinfo.output_height > 1024))
				{
					eDebug("Image too big: %d x %d", cinfo.output_width, cinfo.output_height);
					return -1;
				}
				int bpp=cinfo.output_components*8;
				if (bpp == 24)
					bpp=32;
				result=new gImage(eSize(cinfo.output_width, cinfo.output_height), bpp);
				eDebug("JPEG reader: created gImage (%d x %d x %d bpp)", cinfo.output_width, cinfo.output_height, bpp);
				state=stateStartDecompression;
				continue;
			}
			eDebug("jpeg_read_header returned: %d", code);
			return -1;
			break;
		case stateStartDecompression:
			if (!jpeg_start_decompress(&cinfo))
			{
				eDebug("jpeg_start_decompress - suspended.");
				return 1024;
			}
			state=stateOutput;
			break;
		case stateOutput:
		{
			if (cinfo.output_scanline >= cinfo.output_height)
			{
				eDebug("finished decompression.");
				state=stateFinishDecompression;
				break;
			}
			int maxlines=maxscanlines - decodedscanlines;
			if (maxlines > 16)
				maxlines=16;
			int d, start=cinfo.output_scanline;
			
			d=jpeg_read_scanlines(&cinfo, buffer, maxlines);
			if (!d)
			{
				eDebug("need more data..");
				return 1024; // need more data..
			}
			if (d < 0)
			{
				eDebug("fatal error %d while decompressing.", d);
				if (result)
					delete result;
				result=0;
				return -1;
			}
			
			if (cinfo.output_components == 1) // grayscale
			{
				for (int i=0; i<d; ++i)
					memcpy(((__u8*)result->data)+result->stride*(start+d), buffer[i], cinfo.output_width);
			} else if (cinfo.output_components == 3) // true color
			{
				for (register int i=0; i<d; ++i)
				{
					__u32 *dst=(__u32*)(((__u8*)result->data)+result->stride*(start+i));
					JSAMPLE *smp=buffer[i];
					{
						for (register int x=0; x<result->x; ++x)
							*dst++=smp[x*3] | (smp[x*3+1]<<8) | (smp[x*3+2]<<16);
					}
				}
			} else
			{
				eDebug("unsupported output_components %d", cinfo.output_components);
				if (result)
					delete result;
				result=0;
				return -1;
			}
			decodedscanlines+=d;
			break;
		}
		case stateFinishDecompression:
		{
			if (jpeg_finish_decompress(&cinfo))
				return 1024;
			state=stateEnd;
			break;
		}
		case stateEnd:
		{
			eDebug("we reached the holy end..");
			return -1;
		}
		}
	}
	return 0;
}

#endif //DISABLE_FILE
