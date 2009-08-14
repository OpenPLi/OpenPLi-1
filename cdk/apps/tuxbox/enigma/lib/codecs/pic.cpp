#ifndef DISABLE_FILE

#include <lib/codecs/pic.h>
#include <lib/gdi/gpixmap.h>

ePictureDecoder::ePictureDecoder(eIOBuffer &input): input(input)
{
	result=0;
}

ePictureDecoder::~ePictureDecoder()
{
	if (result)	
		delete result;
}

gImage *ePictureDecoder::fetchResult()
{
	gImage *r=result;
	result=0;
	return r;
}

#endif //DISABLE_FILE
