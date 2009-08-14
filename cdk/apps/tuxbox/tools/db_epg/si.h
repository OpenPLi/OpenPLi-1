#ifndef __si_h
#define __si_h

#include <time.h>
typedef unsigned char __u8;
typedef unsigned int __u32;

time_t parseDVBtime(__u8 t1, __u8 t2, __u8 t3, __u8 t4, __u8 t5);
int fromBCD(int bcd);
unsigned int crc32_be(unsigned int crc, unsigned char const *data, unsigned int len);

#endif
