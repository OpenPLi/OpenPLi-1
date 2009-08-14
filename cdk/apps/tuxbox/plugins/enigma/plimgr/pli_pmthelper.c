/***************************************************************************
                          pmthelper.c  -  helper daemon which creates /tmp/pmt.tmp from enigma CAPMT objects
                             -------------------
    begin                : Nov 2005
    copyright            : (C) 2005 by pieterg
    email                : 
 ***************************************************************************/

#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/un.h>
#include <unistd.h>
#include <alloca.h>
#include <dirent.h>

#define PMT_SOCKET "/tmp/.listen.camd.socket"

#define VERSION "1.0"

#define PMT_LEN 12

/* crypting 1 301:
This is the PLi PMT helper tool.
This is aid softcams taht do not support the new PMT interface in which enigma is the server side.
Please advise your softcam supplier to update to the new PMT interface developped by the PLi team.
See the tech-notes section on http://www.pli-images.org/modules/wiki/index.php?wakka=caPMT

*/
const char *herald =
    "\x8f\x18\xbc\x5e\x40\xc1\x62\x9d\x51\xa6\xa4\xf8\xa9\x48\x7c\x66\xa9\x0c\x6b\xc0"
    "\xc5\x72\x71\x43\x2e\x82\xe0\xfb\x36\xd9\xc5\x6f\x73\x5b\x78\x81\xd8\x23\xe6\xf0"
    "\x73\x82\xb4\x0c\x21\x68\x5f\x64\xe5\x83\x6b\xd9\xd6\x22\x45\x70\x54\x72\xf0\x72"
    "\xfd\x1c\x1f\x43\x69\xc4\x4c\xbb\x2f\x25\xcd\xf0\x4f\x09\x85\xac\x59\x1d\xb3\x60"
    "\x35\xcc\x6e\xec\xcd\x7d\x87\xa2\x64\x11\xd1\x38\x38\xac\x85\x6c\xe4\x78\x61\xc9"
    "\x67\x20\x96\x00\x50\x6a\xf0\xb9\xa0\xad\x38\x4c\x94\xf6\x55\x46\x0d\x24\x90\x80"
    "\x6e\x6c\xf8\xd2\x93\xb5\x83\x6a\x7c\x97\x69\xeb\x6b\x7f\x6d\xf0\x62\x63\x15\x47"
    "\x37\xa2\x74\x7b\x90\x43\x24\x2d\xcc\x7e\x6c\x74\x63\x31\x4f\x62\xa1\x1f\x8f\x60"
    "\x8c\x9c\x6b\xcd\x1a\x28\x31\x1e\x0a\x6e\xfa\x67\x50\xd4\xb1\x5b\x18\xde\x67\x09"
    "\xa5\x65\xdb\xa9\x7e\x4e\x23\xee\xb1\x20\x1e\x26\x59\x53\x69\x29\xea\x79\x7b\xf3"
    "\x2f\xc3\x60\xc6\x70\x21\xfc\x64\x49\xf3\x62\xb3\x81\x67\x07\x34\x25\xce\xe0\x5e"
    "\x1f\x43\x3a\x78\x6f\x74\x67\x1c\xb0\x6d\x9f\x06\x71\x86\x64\xd7\x43\x13\xb8\x67"
    "\xc1\x7d\x60\xeb\x0b\x6c\xd4\x0d\x75\x0c\xe3\x68\x83\xf0\xd0\x29\x04\xa9\x77\x7b"
    "\x9b\x6e\xc6\xd1\x65\x18\x88\x7f\x64\xe4\x4c\x7e\xf0\xe9\x2b\x80\x6e\x69\xb3\x54"
    "\x61\x4e\xbf\xb4\x15\x40\x70\x20\x7e\x34\x60\x9c\x12\x6e\xa0\x6f\x77\x7e\x44\x1c"
    "\x13\x60\x0b\x20\x83\xe5\x74\x01\x4d\x70\x0a\x85\x76\x1f\x80\xd0\xc4\x90\x17\x74"
    "\x3f\xe2\x62\xd9\x3d\x7f\x70\xe2\x7d\xb5\x94\x16\x0e\xd0\x36\x2b\xd4\x8f\x75\x0f"
    "\x42\x67\x38\x38\x65\xf6\xb5\xd1\xb2\x70\xb3\x2c\x7e\x59\x67\xa3\xe2\x68\x0d\xb5"
    "\x7e\x13\xb7\xb7\x65\x30\x3b\x2f\xe0\x30\x5b\x60\x86\x46\x49\x4d\x51\xd6\x64\x75"
    "\x32\x90\x71\x23\xe9\x4c\x63\x7f\xa3\x61\x1f\x8e\x71\x52\x14\x88\xa6\xa5\xdc\x6a"
    "\xc2\xce\x75\x37\x78\x6c\x81\x76\x6b\x1e\x51\x32\x3b\xa3\x25\x6b\x55\xd0\x66\x7f"
    "\x62\x27\xf8\xc0\x66\xc8\x29\xd2\xfa\x6e\x55\x6e\xb0\x29\x2b\x9f\xbd\x74\x50\x97"
    "\x66\xe5\x78\xe5\x9f\x89\xcb\x66\xb3\x4c\x6e\x08\x17\x6e\xe9\x38\x2c\x52\xa0\x1f"
    "\x08\x95\x76\x6b\x4e\x53\x68\x1c\xf2\x65\xf4\x89\x6e\x01\x77\x3f\x61\x2d\xb5\x6a"
    "\x91\xa3\x6e\xfe\xf7\x21\xf5\x20\x67\x16\x09\xc4\x50\xa3\x72\x74\xb0\xac\x27\x3a"
    "\xae\x72\xd6\x84\x6c\xe3\x38\xe4\xbd\xa5\x7f\x66\xb0\x46\x2d\x38\xdc\x74\x07\x53"
    "\x69\x83\x75\x14\xa0\x72\x1b\x7e\x26\x8a\x7c\x39\xdc\x63\x41\xd5\x7e\x8a\x92\x52"
    "\x29\x80\xf5\x25\x53\x19\x78\x28\x65\x6b\xba\xf9\x64\xb5\x44\x80\x7f\x65\x7b\x63"
    "\xae\xf3\x24\xbf\xac\x06\xd6\x2a\x5b\x67\xf0\xbd\xae\xfc\xeb\x64\x05\xb3\x6c\x94"
    "\x9e\x6a\x31\xe1\x7d\x3d\x93\xfd\x1d\x55\x40\x60\xc0\x67\x2a\x14\xa2\x6b\x4f\x31"
    "\x68\xee\xa4\x86\x0e\xa6\x99\x7a\x09\x56\x69\xa8\x37\x7b\xd6\xa3\x61\x8b\x25\x22"
    "\xf0\x30\xad\x2a\x59\xea\x70\x31\x59\x67\xce\x6f\x7d\x2c\x65\x33\x84\x92\x3c\x75"
    "\xc0\x2f\x2f\x92\x41\x77\x28\x73\x6d\xfa\x2f\xa8\xd7\x56\x34\x64\x14\x3f\x71\xbc"
    "\xd9\x6b\x4b\xa3\x60\x42\xb1\x50\x63\x2d\x63\x6c\x90\x0e\x24\x9e\xd5\x77\x27\xb3"
    "\x7e\xce\x55\xa9\x83\xa0\x52\x73\x90\xb8\x7d\x28\xc5\x65\xbc\x7c\x6f\x90\xe9\x3a"
    "\xc7\xd5\xb3\x61\xe2\x38\x75\x9a\x5e\x2e\xac\xe0\x78\xa3\xf4\x2d\x11\xaf\xad\x60"
    "\x30\x9d\x20\x0c\x04\x74\x93\xd5\x70\x6c\xf0\x9b\xff\x54\x10\x65\xd1\xe7\x68\x59"
    "\xf5\x70\xb2\xa4\x6d\x2b\x95\x79\x7e\x00\xdb\x29\x64\xbf\x78\x73\x56\x6a\x32\x2f"
    "\x2e\xc3\x30\x2b\xac\xc4\xe8\x76\x18\xf6\x66\x06\x1a\x6c\x56\x45\x2a\xdf\xe0\x47"
    "\xb3\xce\x28\x6d\x75\xcf\x6f\x27\x19\x75\xe1\xd7\x29\x7a\xa0\x28\x97\x60\x5b\x5e"
    "\xbd\x6d\x4c\x9b\xaa\x5e\xc8\x44\x21\xd1\x80\x05\xc2\x99\x1c\x61\xfe\x63\x64\xee"
    "\x83\x74\xcf\xc4\x68\xdb\x65\x6c\x1e\x22\x28\x73\x46\xfc\x6b\xfd\x23\x62\xce\x41"
    "\x6b\xc9\xa3\x28\x8a\x35\xa9\x62\x80\xe8\x2a\x32\xfc\x62\xe8\xc4\x64\xf8\x55\xfc"
    "\x32\x16\xac\x7e\xf5\x9f\x62\x42\x3b\x6d\xb7\xfc\x6b\x27\xef\xe2\xf2\x00\xea\x72"
    "\x00\xc3\x77\xd7\x13\x68\x25\xa5\x6d\x52\x24\xa3\x4f\xd0\x75\x26\xf2\x4f\x63\xd3"
    "\x17\x79\x85\xb9\x2a\xd6\x80\xa7\xe4\x54\x23\x7a\x98\x00\x64\x47\xec\x61\x27\x95"
    "\x29\xe0\x10\x46\xe1\xc0\x6f\x58\xbc\x48\x49\x12\x43\x6c\xb6\xf9\x29\x5c\x60\x16"
    "\x09\x44\x36\x76\xa5\x9d\x65\xe4\xcb\x6b\xec\xd1\x62\x63\x5d\xb0\xb7\xfe\xf8\x20"
    "\xfa\x08\x06\xee\x05\x5e\x66\x23\x6a\x90\x55\x35\x30\xe5\x01\x60\x60\x23\x21\xd5"
    "\xcf\x72\x0f\xb4\x6d\x87\xd8\x9e\xef\xf5\xfb\x61\x90\x18\x27\x06\x01\x73\x45\xf4"
    "\x61\x9f\x35\x5d\x15\x83\x50\x61\x78\xd6\x63\xd4\x17\x2f\xca\x3d\x68\x46\x1e\xc3"
    "\x53\x6f\x7f\x60\xa4\x79\x7f\x7c\x74\x6e\x7f\x25\x7d\xd3\x23\x6f\xf6\x80\xbe\x29"
    "\x43\x09\x79\xaa\xc5\x6d\x92\xa5\x6e\xa0\x73\x8c\xf5\xe4\x7c\x78\x09\x25\x6b\xd3"
    "\x0a\x68\x66\x9f\x6a\x9d\x1e\x1c\x58\xe0\x2a\x21\x2f\x3f\x62\x84\xb5\x64\x92\x9e"
    "\x27\x48\x30\xf9\x20\xf8\xc7\x6a\x24\xc9\x7e\x7b\x98\x74\x26\xb4\x79\xfa\x90\x0d"
    "\xcb\x2a\x86\x34\x0f\x64\x27\x84\xfd\x27\x35\x1f\x78\x7f\xd0\xb5\xfe\x6c\x85\x6f"
    "\x49\x58\x69\x6e\xe3\x21\x30\xce\x63\xd3\xd4\x4a\xad\x72\x5c\x79\x85\x56\x65\x74"
    "\xe7\x66\x8d\x01\x60\x02\x9d\xf7\xbf\x46\xcc\x77\x63\xf4\x6f\x47\x64\x75\x42\x72"
    "\x2a\x74\x6e\x90\xa4\x23\xc9\x60\x3f\xd8\x6a\x33\xa2\x63\x4b\x5d\x2a\x31\xdf\xdb"
    "\x55\x15\xc4\x60\xfd\xef\x6c\xa3\x53\x6e\x2d\x25\x7c\xa4\xc2\xa6\x7b\xe1\xdc\x60"
    "\xcc\xf9\x62\xb8\xf4\x61\xf6\xf4\x0c\xe4\x7a\x66\xe4\xb0\xda\x21";

/* crypting 2 39:
Oops, I failed to start on this system

*/
const char *sorry =
    "\xbb\x64\x39\xe9\xd0\x00\x72\x37\x32\x0f\x86\x44\xbf\x79\x63\xc1\xbc\x7c\x1f\x50"
    "\x7f\xf1\xd3\xdb\x8d\x9c\xc5\x21\x00\xbd\x28\xf4\xb8\x49\xf6\xc9\x2d\x16\xc0\x64"
    "\xf3\x96\xbb\x60\x21\xe1\x6d\xc8\x3b\x66\x73\x89\x64\x41\x7c\x7d\xaf\x75\xaa\x6d"
    "\xf4\xae\x6b\x24\xad\x23\x4b\xe0\x7e\xd9\xa4\x35\xaa\x5f\xb3\x67\xb0\x95\x27\xa4"
    "\x64\x72\x9f\x13\x72\x73\xc4\xa4\xe4\xb1\x3d\x65\x22\x98\x7c\xe0\x8d\x76\x58\xf4"
    "\x2c\xc6\x50\x88\x67\xff\x35\x6d\xee\xd7\x69\xf0\xc8\x24\x5d\xc0\x7f\x53\x64\x92"
    "\x1b\x98\x71\x6f\x49\x06\x6e\x2f\x91\x75\x02\x73\x29\x7b\x90\x94\xbb\x83\xf8\x7c"
    "\x09\xa7\x7b\x51\xc1\x72\xeb\xa3\x7a\x51\x94\x1c\x3e\x85\x15\x6b\xcd\x31\x68\x27"
    "\x57\x0b\x0b\x8a\x57\xbf\x10\x1f";

void daemon_init()
{
	int i;
	pid_t pid;
	
	if ((pid = fork()) != 0)
		exit(0);
	
	setsid(); /* become session leader */
	
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	
	if ((pid = fork()) != 0)
		exit(0);
	
	for (i = 0; i < 10; i++)
		close(i);
	
	chdir("/");

	freopen("/dev/null", "a", stdin);
	freopen("/dev/null", "a", stdout);
	freopen("/dev/null", "a", stderr);
}

unsigned int crc32_table[256] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b,
	0x1a864db2, 0x1e475005, 0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd, 0x4c11db70, 0x48d0c6c7,
	0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3,
	0x709f7b7a, 0x745e66cd, 0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5, 0xbe2b5b58, 0xbaea46ef,
	0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb,
	0xceb42022, 0xca753d95, 0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d, 0x34867077, 0x30476dc0,
	0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4,
	0x0808d07d, 0x0cc9cdca, 0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02, 0x5e9f46bf, 0x5a5e5b08,
	0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc,
	0xb6238b25, 0xb2e29692, 0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a, 0xe0b41de7, 0xe4750050,
	0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34,
	0xdc3abded, 0xd8fba05a, 0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb, 0x4f040d56, 0x4bc510e1,
	0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5,
	0x3f9b762c, 0x3b5a6b9b, 0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623, 0xf12f560e, 0xf5ee4bb9,
	0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd,
	0xcda1f604, 0xc960ebb3, 0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b, 0x9b3660c6, 0x9ff77d71,
	0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2,
	0x470cdd2b, 0x43cdc09c, 0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24, 0x119b4be9, 0x155a565e,
	0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a,
	0x2d15ebe3, 0x29d4f654, 0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c, 0xe3a1cbc1, 0xe760d676,
	0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662,
	0x933eb0bb, 0x97ffad0c, 0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4};

static unsigned int crc32_be(unsigned int crc, unsigned char const *data, unsigned int len)
{
	unsigned int i;
	for (i = 0; i < len; i++)
	{
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *data++) & 0xff];
	}

	return crc;
}

void fuzz(const char * buf)
{
	unsigned int si, i, j, len, val;


	si = (buf[3] & 0x30) >> 4;
        len = ((buf[4] & 0x0F) << 12) | ((buf[5] & 0x0F) << 8) | ((buf[6] & 0x0F) << 4) | (buf[7] & 0x0F);

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

void prfuzz(const char *buf)
{
	fuzz(buf);
}

int main(int argc, char *argv[])
{
	int capmtfd = -1;
	int i;
	int daemon = 1;
	char *helper = NULL;
	char * progname;

	struct sockaddr_un emudaddr;

	prfuzz(herald);

	progname = argv[0];
	if (strchr(progname, '/'))
	{
		progname = strrchr(progname, '/');
		progname++;
	}
	if (strlen(progname) != 13)
	{
		putchar('\n');
		prfuzz(sorry);
		exit(1);
	}
	if (
		progname[1] + 0x7b != 0xe7  || // 6c l
		progname[8] + 0x38 != 0x9d  || // 65 e
		progname[6] - 0x31 != 0x43  || // 74 t
		progname[7] - 0x0b != 0x5d  || // 68 h
		progname[2] + 0x02 != 0x6b  || // 69 i
		progname[10] + 0x6c != 0xdc  || // 70 p
		progname[4] + 0x6a != 0xda  || // 70 p
		progname[0] - 0x20 != 0x50  || // 70 p
		progname[5] + 0x52 != 0xbf  || // 6d m
		progname[12] - 0x71 != 0x01  || // 72 r
		progname[3] - 0x5d != 0x02  || // 5f _
		progname[9] - 0x37 != 0x35  || // 6c l
		progname[11] + 0x2b != 0x90  || // 65 e
		progname[13] != 0 )
	{
		putchar('\n');
		prfuzz(sorry);
		exit(2);
	}

	for (i = 1; i < argc; i++)
	{
		if (strstr(argv[i], "-d"))
		{
			daemon = 0;
		}
		else if (strstr(argv[i], "--version"))
		{
			printf("PLi pmt helper\nversion %s\n", VERSION);
			exit(1);
		}
		else if (strstr(argv[i], "--helper"))
		{
			helper = argv[++i];
			if (helper == NULL || *helper == '\0') {
				printf("Need helper cmnd on --helper. See --help option\n");
				exit(1);
			}
		}
		else /* both for --help and invalid options */
		{
			printf("usage: %s [-d] [--version] [--help] [--helper helper prog]\n\n", progname);
			printf("    -d             = run in the foreground (debug mode)\n");
			printf("    --helper cmd   = when a new pmt.tmp file is written cmd will first be \n");
			printf("                     killed and then restarted\n");
			printf("    --version      = show version and exit\n");
			exit(1);
		}
	}

	if (daemon)
		daemon_init();

	while (1)
	{
		unlink("/var/tmp/pmt.tmp");

		if (capmtfd >= 0)
		{
			close(capmtfd);
			capmtfd = -1;
		}

		while (1)
		{
			struct sockaddr_un pmtaddr;
			bzero(&pmtaddr, sizeof(pmtaddr));
			pmtaddr.sun_family = AF_LOCAL;
			strcpy(pmtaddr.sun_path, PMT_SOCKET);
			if (capmtfd >= 0) close(capmtfd);
			if ((capmtfd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) exit(1);
			if (connect(capmtfd, (struct sockaddr *)&pmtaddr, sizeof(pmtaddr)) >= 0) break;
			sleep(1);
		}
		
		while (1)
		{
			fd_set rset;
			int maxfd = 0;

			FD_ZERO(&rset);
			if (capmtfd >= 0)
			{
				FD_SET(capmtfd, &rset);
				if (capmtfd > maxfd) maxfd = capmtfd;
			}
			if (select(maxfd + 1, &rset, NULL, NULL, NULL) <= 0) break;
			if (capmtfd >= 0 && FD_ISSET(capmtfd, &rset))
			{
				int length = 0;
				int programinfo_length = 0;
				int lengthdatasize = 0;
				int readcount;
				unsigned char buffer[1024];
				readcount = 0;
				if (read(capmtfd, &buffer[readcount], 4) <= 0) break;
				readcount += 4;
				if (buffer[3] & 0x80)
				{
					/* multibyte length field */
					int i;
					lengthdatasize = buffer[3] & 0x7f;
					if (read(capmtfd, &buffer[readcount], lengthdatasize) <= 0) break;
					readcount += lengthdatasize;
					for (i = 0; i < lengthdatasize; i++)
					{
						length = (length << 8) | buffer[i + 4];
					}
				}
				else
				{
					/* singlebyte length field */
					length = buffer[3] & 0x7f;
				}
				if (read(capmtfd, &buffer[readcount], length) <= 0) break;
				readcount += length;
				if (!memcmp(buffer, "\x9F\x80\x32", 3)) /* CAPMT object */
				{
					int i;
					int descriptor_length = 0;
					unsigned char *data = &buffer[4 + lengthdatasize];
					if (data[6] == 0x01 && (data[0] == 0x01 || data[0] == 0x03 || data[0] == 0x05)) /* ok_descramble command, first or only object in the list, or an update */
					{
						unsigned char pmtdata[2048];
						int slen = PMT_LEN;   // 12

						unsigned short int program_number = *(unsigned short *)&data[1];
						unsigned short int provider = 0;
						unsigned char version = data[3];
						unsigned short int pid = 0;
						int prog_info_len = 0;
						int es_info_length = 0;
						unsigned int crc32;

						programinfo_length = *(unsigned short *)&data[4] & 0xfff;
						
						pmtdata[0] = 0x02;                      // table ID;
						pmtdata[3] = (program_number >> 8);     // prog_no hi
						pmtdata[4] = program_number & 0xFF;     // prog_no lo
						pmtdata[5] = version;                   // version,cur/next
						pmtdata[6] = 0;                         // section no
						pmtdata[7] = 0;                         // last section no

						for (i = 0; i < programinfo_length - 1; i += descriptor_length + 2)
						{
							descriptor_length = data[i + 8];
							if (data[i + 7] == 0x81)
							{
								/* private descr: dvb namespace */
							}
							else if (data[i + 7] == 0x82)
							{
							}
							else if (data[i + 7] == 0x84)
							{
							}
							else //if (data[i + 7] == 9)
							{
								memcpy(pmtdata + slen, &data[i + 7], descriptor_length + 2);
								prog_info_len += descriptor_length + 2;
								slen += descriptor_length + 2;
							}
						}

						pmtdata[10] = 0xF0 | (prog_info_len>>8); // prog_info len hi
						pmtdata[11] = prog_info_len&0xFF;        // prog_info len lo

						for (i = programinfo_length + 6; i < length; i += es_info_length + 5)
						{
							int j;
							int lenpos = 0;
							int stream_type = data[i];
							es_info_length = *(unsigned short *)&data[i + 3] & 0xfff;
							pmtdata[slen++] = stream_type;
							pmtdata[slen++] = 0xE0 | data[i + 1];
							pmtdata[slen++] = data[i + 2];
							lenpos = slen;
							slen += 2;
							if (stream_type == 1 || stream_type == 2)
							{
								/* video */
								pid = *(unsigned short*)&data[i + 1];
							}
							for (j = 0; j < es_info_length - 1; j += descriptor_length + 2)
							{
								descriptor_length = data[i + j + 7];
								memcpy(pmtdata+slen, &data[i + j + 6], descriptor_length + 2);
								slen += descriptor_length + 2;
							}
							pmtdata[lenpos] = 0xF0 | (slen - lenpos - 2) >> 8;
							pmtdata[lenpos + 1] = (slen - lenpos - 2)&0xFF;
						}
						
						pmtdata[1] = 0xB0 | ((slen-3+4) >> 8);   // section length hi
						pmtdata[2] = (slen-3+4) & 0xFF;          // section length lo
						pmtdata[8] = 0xE0 | (pid >> 8);     // PCR hi
						pmtdata[9] = pid & 0xFF;            // PCR lo
						
						crc32 = crc32_be(~0, pmtdata, slen);
					
						pmtdata[slen++] = crc32 >> 24;
						pmtdata[slen++] = crc32 >> 16;
						pmtdata[slen++] = crc32 >> 8;
						pmtdata[slen++] = crc32 & 0xFF;

						{
							FILE *file = fopen("/var/tmp/pmt.tmp", "wb");
							if (file)
							{
								fwrite(pmtdata, slen, 1, file);
								fclose(file);
							}
						}
						if (helper != NULL) { 
							char * cmd = (char *) malloc(9+strlen(helper));
							char * basecmd = strrchr(helper, '/');
							if (basecmd == NULL)
								basecmd = helper;
							else
								basecmd++;

							strcpy(cmd, "killall ");
							strcat(cmd, basecmd);
							system(cmd);
							strcpy(cmd, helper);
							strcat(cmd, " &");
							system(cmd);
							free(cmd);
						}
					}
				}
			}
		}
	}
	return 0;
}

