/*
 * semi crypt a string. Or actually obscure a string.
 *
 * (C) 2006 Mirakels 
 *
 * GPL
 *
 * This tool will read text from stdin and outputs a scrambled string
 * declaration to be used in your program.
 * Pass the name of the const char * variable to hold the scrambled 
 * string as command line argument.
 *
 * e.g.: echo "Hello" | crypt hellovar
 * will output:
 *
 *   / * crypting 2 6:
 *   Hello
 *
 *   * /
 *   const char *hellovar =
 *       "\x61\xe8\x5f\x2d\x60\x60\x10\x96\x63\x98\xa2\x48\xf5\x76\x61\xec\x94\x6b\x5b\x5c"
 *       "\x64\x08\x6c\x59\xc3\x7f\x1c\x6b\x7a\xb0\x07\xc3\x5c\x03\x31\xc5";
 *
 * The 6 is the length of the input string. The 2 is a starting index in the encrypted byte.
 * Both are there just for reference.
 * Take special care for newlines at the end of your tekst...
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAXLEN 4096

hpr32(unsigned char * buf, int i, unsigned int val)
{
	buf[i*4 + 0] = (val >> 24) & 0xff;
	buf[i*4 + 1] = (val >> 16) & 0xff;
	buf[i*4 + 2] = (val >>  8) & 0xff;
	buf[i*4 + 3] =  val        & 0xff;
}


main(int argc, char ** argv)
{
	unsigned int si, startindex;
	unsigned int mask;
	unsigned int dummy;
	int i, val;
	unsigned char buf[MAXLEN*4];
	char * varname;
	char str[MAXLEN];
	int fd = open("/dev/urandom", O_RDONLY);

	if (fd < 0)
	{
		perror("/dev/urandom");
		fprintf(stderr, "No random data, no point in going on.\n");
		exit(1);
	}

	varname = argv[1] && *argv[1] ? argv[1] : "varname";
	
	i = 0;
	/* get a random startpoint */
	read(fd, &startindex, sizeof(startindex));
	startindex %= 4;
	si = startindex;

	read(fd, &dummy, sizeof(dummy));
	dummy = (dummy & 0xFFFFFFCF) | (startindex << 4);
	hpr32(buf, i, dummy);
	i = 1;

	/* save input length dummy this time*/
	read(fd, &dummy, sizeof(dummy));
	dummy = (dummy & 0xF0F0F0F0) | ((i & 0xF000) << 12) | ((i & 0xF00) << 8) | ((i & 0xF0) << 4) | (i & 0xF);
	hpr32(buf, i, dummy);
	/* now crypt each byte */

	i = 2;
	while ((val = getchar()) != EOF && i < MAXLEN-1)
	{
		str[i-2] = val;
		read(fd, &dummy, sizeof(dummy));
		mask = (0x0F << (8*startindex)) | ( 0xF0 << (8 * ((startindex+2)%4)) );
		dummy &= ~mask;
		dummy |= ( ((val & 0x0F) << (8*startindex)) | ((val & 0xF0) << (8 * ((startindex+2)%4))) );
		hpr32(buf, i, dummy);
		startindex = (startindex+1)%4;
		i++;
	}
	/* save end index counter as a sort of checksum */
	read(fd, &dummy, sizeof(dummy));
	hpr32(buf, i, (dummy & 0xFFFFFFCF) | (startindex << 4));
	/* save input length dummy this time*/
	i -= 2;
	read(fd, &dummy, sizeof(dummy));
	dummy = (dummy & 0xF0F0F0F0) | ((i & 0xF000) << 12) | ((i & 0xF00) << 8) | ((i & 0xF0) << 4) | (i & 0xF);
	hpr32(buf, 1, dummy);

	printf("/* crypting %d %d into %s:\n%s\n*/\n", si, i, varname, str);
	printf("const char *%s =\n    \"", varname);
	for (startindex = 0; startindex < 4*(i+3); startindex++)
	{
		if (startindex && startindex % 20 == 0)
			printf("\"\n    \"");
		printf("\\x%02x", buf[startindex]);
	}
	printf("\";\n");
}


