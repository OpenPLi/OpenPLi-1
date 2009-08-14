#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/* crypting 2 23 into varname:
hello in %d chars long

*/
const char *str =
    "\xe6\xff\x5c\xe9\x80\xa0\xa1\xe7\x98\x68\x07\x63\xa5\x8c\x6b\x77\x23\x68\x5a\x7c"
    "\x6e\x83\x3c\xb8\xf4\xff\x58\x64\xb0\x9d\x27\x30\x98\x67\x25\x49\x6f\x17\x7e\x78"
    "\x5c\xf0\x70\x22\xb5\xaa\x29\x4f\xa7\x69\x5d\x24\x2c\x3c\x60\xa2\x7c\xd3\x50\x62"
    "\x08\xb2\x64\xdb\xa4\x67\x3b\xa1\x7a\x18\xb2\xbd\xce\x43\x23\x7d\x90\x1a\x2c\x7c"
    "\xb9\x62\xf4\xec\x66\xda\x8f\x31\x96\x6e\x1c\x66\xd7\x30\x61\x87\x91\x0b\x44\xba"
    "\x43\x18\x88\xde";


#define fuzz_len(buf) ((buf[4] & 0x0F) << 12) | ((buf[5] & 0x0F) << 8) | ((buf[6] & 0x0F) << 4) | (buf[7] & 0x0F);

void fuzz(const char * buf)
{
	unsigned int si, i, j, len, val;

	si = (buf[3] & 0x30) >> 4;
        // len = ((buf[4] & 0x0F) << 12) | ((buf[5] & 0x0F) << 8) | ((buf[6] & 0x0F) << 4) | (buf[7] & 0x0F);
        len = fuzz_len(buf);

	// printf("startindex = %d (%0x), len = %d\n", startindex, buf[3], len);

	if ((si+len) % 4 != (buf[len*4+8+3] & 0x30) >> 4)
	{
		fprintf(stderr, "invalid string\n");
		return;
	}

	for (i = 0; i < len; i++)
	{
		j = 8 + 4*i;
		val = (buf[j + 3 - si] & 0x0F) | (buf[j + 3 - ((si+2)%4)] & 0xF0);
		putchar(val);
		si = (si+1)%4;
	}
}

void fuzz_str(const char * buf, char * res)
{
	unsigned int si, i, j, len, val;

	si = (buf[3] & 0x30) >> 4;
        // len = ((buf[4] & 0x0F) << 12) | ((buf[5] & 0x0F) << 8) | ((buf[6] & 0x0F) << 4) | (buf[7] & 0x0F);
        len = fuzz_len(buf);

	// printf("startindex = %d (%0x), len = %d\n", startindex, buf[3], len);

	if ((si+len) % 4 != (buf[len*4+8+3] & 0x30) >> 4)
	{
		fprintf(stderr, "invalid string\n");
		return;
	}

	for (i = 0; i < len; i++)
	{
		j = 8 + 4*i;
		val = (buf[j + 3 - si] & 0x0F) | (buf[j + 3 - ((si+2)%4)] & 0xF0);
		res[i] = val;
		si = (si+1)%4;
	}
	res[len] = 0;
}

void prfuzz(const char *buf)
{
	fuzz(buf);
}


main(int argc, char ** argv)
{
	int len;
	char * res;

	prfuzz(str);

	len = fuzz_len(str);
	res = calloc(len + 1, sizeof(char));
	fuzz_str(str, res);

	printf(res, len);
}
