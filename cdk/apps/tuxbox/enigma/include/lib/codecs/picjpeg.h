#ifndef DISABLE_FILE

#ifndef __include_lib_codecs_picjpeg_h
#define __include_lib_codecs_picjpeg_h

#include <lib/base/eerror.h>
#include <jpeglib.h>
#include <lib/codecs/pic.h>

class ePictureDecoderJPEG: ePictureDecoder
{
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	JSAMPARRAY buffer;
	int row_stride;
	enum { stateHeader, stateStartDecompression, stateOutput, stateFinishDecompression, stateEnd } state;
public:
	ePictureDecoderJPEG(eIOBuffer &input);
	~ePictureDecoderJPEG();
	int decodeMore(int maxscanlines);
};

#endif

#endif // DISABLE_FILE