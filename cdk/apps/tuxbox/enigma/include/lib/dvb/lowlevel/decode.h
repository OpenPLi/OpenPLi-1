#ifndef __DECODE_H__
#define __DECODE_H__

#include <sys/types.h>

char *decode_fec (u_char fec);
char *decode_service_type (u_char type);
char *decode_stream_type (u_char type);
char *decode_descr (u_char index);

#endif
