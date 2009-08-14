#ifndef DISABLE_FILE

#include <lib/codecs/codec.h>

eAudioDecoder::eAudioDecoder()
	: speed(1),filelength(0), sec_duration(-1), sec_currentpos(-1)
{
}

eAudioDecoder::~eAudioDecoder()
{
}

#endif //DISABLE_FILE
