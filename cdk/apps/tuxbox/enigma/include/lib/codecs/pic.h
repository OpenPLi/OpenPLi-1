#ifndef DISABLE_FILE

#ifndef __include_lib_codecs_pic_h
#define __include_lib_codecs_pic_h

class eIOBuffer;
class gImage;

class ePictureDecoder
{
protected:
	eIOBuffer &input;
	gImage *result;
public:
	ePictureDecoder(eIOBuffer &input);
	virtual ~ePictureDecoder();
	virtual int decodeMore(int maxscanlines)=0; // return: -1 if fatal error/done (check result == 0), 0 if done, >0 if more data requested (number of bytes)
	gImage *fetchResult();
};

#endif

#endif //DISABLE_FILE
