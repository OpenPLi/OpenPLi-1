/*
 * $Id: ucodex.c,v 1.5 2002/09/18 07:34:54 obi Exp $
 *
 * extract avia firmware from srec and binary files
 *
 * (C) 2002 by Andreas Oberritter <obi@tuxbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ucodex.h"

#define	VERBOSE	1
#define	WRITE	1

const struct ucode_type_s types[] = {
	{
		"avia500", "\x43\x33\x55\x58\x02",
		{
			{"avia500v083",  92918, "\x96\x5d\x83\x4c\x7b\xb7\x43\xdf\x68\x41\xf4\xfd\xa5\xb4\xe7\x90"},
			{"avia500v090",  92894, "\x10\x8f\xa2\xfa\x0e\xe8\x44\x51\xea\x7e\xa2\xdb\x9a\xda\xa4\x4b"},
			{"avia500v093", 101318, "\xfe\xce\x1d\x33\x24\xe0\x91\x7b\x92\x1d\x81\x44\x90\xd8\xa8\x24"},
			{"avia500v110", 101374, "\x73\x73\xf3\x93\x42\x63\xb3\xc3\xea\x1d\x0f\x50\x0f\x00\x44\xa5"},
			{"", 0, ""}
		}
	},{
		"avia600", "\x43\x33\x55\x58\x01",
		{
			{"avia600vb016", 138906, "\x6f\xd4\x53\x84\xd7\x05\x28\x9a\x2f\xc8\xf2\x12\xb2\x85\x5f\x5c"},
			{"avia600vb017", 126846, "\xda\x49\x21\x46\xba\x7e\x17\x78\x83\xfe\xad\xaa\x0c\xf8\x9a\xa5"},
			{"avia600vb018", 126878, "\xc3\x1d\xc5\x70\xcf\x94\x1a\xfb\x6f\xc4\x81\x3f\x56\x1a\xa3\x78"},
			{"avia600vb022", 128214, "\x6a\x74\x8f\xb2\x80\x00\x73\x8c\xaf\xeb\x9e\x27\x44\x3a\xc6\x23"},
			{"avia600vb028", 128174, "\xf8\x57\x7c\x6a\x70\x56\x59\x0c\xa5\x84\x75\x20\xd8\x13\x39\xfc"},
			{"", 0, ""}
		}
	},{
		"dmx", "\xB0\x09\xA8",
		{
			{"ucode_0000", 1024, "\x53\xc5\xbc\x40\x81\xdf\xad\xab\x99\x35\xad\x25\x6e\x4d\x62\x39"},
			{"ucode_0001", 1024, "\x93\x6b\x13\xb9\x01\x8b\x25\x41\x03\x6a\x7f\x20\xe7\x66\x48\xff"},
			{"ucode_000C", 2048, "\x0f\x61\x0f\x3e\x0f\x1f\x2b\x56\xfc\x2b\xb4\x00\x77\x00\x83\x6b"},
			{"ucode_0010", 2048, "\xf0\x41\x8c\x35\x05\x47\xe9\x8f\x2c\x5d\x7f\x4d\xf0\x0f\x92\x61"},
			{"ucode_0013", 2048, "\x66\x62\x7c\x5d\xdf\x26\x9a\x1f\x3a\x9f\x9f\x3c\x22\xfb\xd4\x1b"},
			{"ucode_0014", 2048, "\x65\x82\xa8\x9e\x7e\x13\xe4\x10\xc3\x66\xe4\x7b\x4e\xf9\xd3\x8e"},
			{"ucode_001A", 2048, "\xe8\xfb\x83\x44\x66\xd3\x4c\xb7\x5b\xb0\xe6\x4e\xf0\x8b\x55\x44"},
			{"ucode_B102", 2048, "\x36\xa0\x5f\x39\xca\xa0\xfb\x37\x5d\xf8\x5c\x99\xe6\x6e\x40\x6d"},
			{"ucode_B107", 2048, "\xd4\xc1\x2d\xf0\xd4\xce\x8b\xa9\xeb\x85\x85\x09\xd8\x32\xdf\x65"},
			{"ucode_B121", 2048, "\x4e\x08\x08\x73\x12\x6f\x15\x6a\x0b\x48\x9a\xf1\x76\x52\x06\x20"},
			{"", 0, ""}
		}
	}
#if 0
	,{
		"cam-alpha", "\xC8\xA5",
		{
			{"cam_01_01_001D",      131072, "\xff\x6f\xaf\xbd\x2a\xa1\xf2\x9a\xfe\x23\x2a\x72\xfa\xc5\x78\x70"},
			{"cam_01_01_001E",      131072, "\x7b\x9b\x72\x78\x66\x23\xe3\x75\x03\x35\xc7\x9a\xf0\x44\xe7\x18"},
			{"cam_01_01_002E",      131072, "\xbe\x7f\x1b\xeb\x1b\xb4\x37\xb7\xf7\xc9\x9f\x8e\x5a\x96\x88\x82"},
			{"cam_01_01_003E",      131072, "\xa8\x68\x9d\x88\xe0\xd2\xdf\x12\xa1\x57\x32\xc9\x8f\x86\x5b\x11"},
			{"cam_01_01_004D",      131072, "\xc4\x2d\x67\x53\x79\x4d\xd9\x51\x46\xea\xc3\x1f\x2a\x65\xb5\x16"},
			{"cam_01_01_004E",      131072, "\x43\x36\xe7\xd3\xfe\xd4\x3c\x9e\x06\x32\x10\xbc\xdc\x95\xd2\x3e"},
			{"cam_01_01_004F",      131072, "\xc7\x34\x20\x7d\xde\xa7\xb8\xce\xaf\xa2\x50\x5f\x13\x60\xf3\xbf"},
			{"cam_01_01_005D",      131072, "\xbe\x4b\x0f\x38\x55\x7c\x41\x6c\xe0\x4e\x7f\xa3\xfa\x63\x4f\x95"},
			{"cam_01_01_005E",      131072, "\x99\x7b\x1f\x85\x8f\x1e\xfe\xe5\x25\xe6\x84\x25\x58\xed\xbe\x3c"},
			{"cam_01_01_005F",      131072, "\xa5\x98\x48\x25\xff\x55\x4e\xa5\x30\xef\xc4\x73\x3f\xfd\x74\x73"},
			{"cam_01_02_002D",      131072, "\x19\x05\x39\x06\x36\xe7\x0c\x96\x65\x74\xa3\x29\x8a\x1b\x89\xc3"},
			{"cam_01_02_002E",      131072, "\x7f\x56\xe6\x93\xa9\x16\xb3\x9a\x6e\x27\x34\xdc\x9b\x5a\xab\x7a"},
			{"cam_01_02_002F",      131072, "\x70\x4c\xb8\xd9\x96\x5b\xab\xbd\xc7\xd4\xe7\xca\xe6\xe5\x58\x4e"},
			{"cam_01_02_105D",      131072, "\xf2\x7e\xda\x69\x8c\x20\x2d\x17\xaf\x7b\x9b\xab\x83\x97\x3e\x8c"},
			{"cam_01_02_105E",      131072, "\xba\xc1\x97\x0b\x0e\x86\x5c\x00\x01\x5b\x3d\x78\xa2\x09\xb5\xbf"},
			{"cam_01_02_105F",      131072, "\xc5\x10\x74\xb2\xed\xf6\xc1\x4e\x0b\xb9\x9e\xe3\xed\x8b\x9c\x47"},
			{"cam_NOKIA_PRODTEST2", 131072, "\x16\xc5\xe1\xeb\xa0\xcf\xe6\x3f\x2b\xbc\x64\x8e\x16\x44\xc8\x83"},
			{"cam_ProdTestImage_2", 131072, "\xff\x44\x1c\xe2\xf3\xfe\xab\x81\x5d\x36\x0e\x30\x32\xd2\x38\x88"},
			{"cam_STREAMHACK",      131072, "\x42\x0b\xaf\x44\x7b\xfd\x52\x9a\x79\x4b\xb3\x6e\xf8\x0f\x16\x52"},
			{"", 0, ""}
		}
	}
#endif
};

unsigned int char2hex (const unsigned char * src, unsigned char * dest, const unsigned int size) {

	unsigned int count = 0;
	unsigned char tmp[3];

	tmp[2] = 0x00;

	while (count != size) {
		memcpy(tmp, src, 2);
		*dest = strtoul(tmp, NULL, 16);
		src += 2;
		dest++;
		count++;
	}

	return count;
}

int md5cmp (const unsigned char * buf, const unsigned int size, const unsigned char * md5sum) {

	unsigned char md[MD5_DIGEST_LENGTH];

	MD5(buf, size, md);

	if (memcmp(md5sum, md, MD5_DIGEST_LENGTH))
		return EXIT_FAILURE;

	if (VERBOSE) {
		unsigned char i;
		for (i = 0; i < MD5_DIGEST_LENGTH; i++)
			fprintf(stdout, "%02x", md5sum[i]);
	}

	return EXIT_SUCCESS;
}

int writebuf (const unsigned char * buf, const unsigned int size, const unsigned char * filename) {

	if (VERBOSE)
		fprintf(stdout, "\t%s\n", filename);

	if (WRITE) {
		FILE * file = fopen(filename, "w");

		if (file == NULL) {
			perror(filename);
			return EXIT_FAILURE;
		}

		fwrite(buf, size, 1, file);

		if (fclose(file) < 0) {
			perror("fclose");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int scan_file (const char * filename, off_t file_size) {

	FILE * file;
	int probe;

	unsigned int i, j;

	unsigned char * buf = NULL;
	unsigned char * psrc = NULL;
	unsigned char * pdest = NULL;

	/* open file */
	file = fopen(filename, "r");

	if (file == NULL) {
		perror(filename);
		return EXIT_FAILURE;
	}

	/* check file type */
	probe = fgetc(file);

	if (ungetc(probe, file) != probe) {
		fprintf(stderr, "ungetc failed\n");
		fclose(file);
		return EXIT_FAILURE;
	}

	buf = (unsigned char *) malloc(file_size);

	/* Motorola S-Record; binary data in text format */
	if (probe == 'S') {
		unsigned char linebuf[515];
		unsigned char addrlen;

		pdest = buf;

		while (fgets(linebuf, sizeof(linebuf), file) != NULL) {

			switch ((linebuf[0] << 8) | linebuf[1]) {
			case 0x5331:
				addrlen = 4;
				break;
			case 0x5332:
				addrlen = 6;
				break;
			case 0x5333:
				addrlen = 8;
				break;
			default:
				continue;
			}

			linebuf[4] = 0;
			pdest += char2hex(linebuf + 4 + addrlen, pdest, strtoul(linebuf + 2, NULL, 16) - (addrlen / 2) - 1);
		}

		file_size = pdest - buf;
	}

	/* binary data */
	else {
		if ((fread(buf, file_size, 1, file) != 1) || (ferror(file) != 0)) {
			fprintf(stderr, "fread failed: %d\n", ferror(file));
			free(buf);
			fclose(file);
			return EXIT_FAILURE;
		}
	}

	if (fclose(file) < 0) {
		perror("fclose");
		free(buf);
		return EXIT_FAILURE;
	}

	/* search for ucodes in buffer */
	for (psrc = buf; psrc < buf + file_size; psrc++)
		for (i = 0; i < sizeof(types) / sizeof(struct ucode_type_s); i++)
			if ((psrc - buf + strlen(types[i].magic) <= file_size) &&
				(!memcmp(psrc, types[i].magic, strlen(types[i].magic)))) {
				for (j = 0; types[i].ucodes[j].size != 0; j++)
					if ((psrc - buf + types[i].ucodes[j].size <= file_size) &&
						((0) || (!md5cmp(psrc, types[i].ucodes[j].size, types[i].ucodes[j].md5sum)))) {
						writebuf(psrc, types[i].ucodes[j].size, types[i].ucodes[j].name);
						psrc += types[i].ucodes[j].size - 1;
						break;
					}
				if (types[i].ucodes[j].size == 0)
					fprintf(stdout, "%s: unknown %s ucode. please report.\n", filename, types[i].name);
			}

	free(buf);
	return EXIT_SUCCESS;
}

int process_path (const char * directory, const char * filename) {

	struct stat filestat;

	char path[strlen(directory) + strlen(filename) + 2];

	if ((!strcmp(filename, ".")) || (!strcmp(filename, "..")))
		return EXIT_FAILURE;

	if (!strcmp(directory, ""))
		strcpy(path, filename);
	else
		sprintf(path, "%s/%s", directory, filename);

	/* check file type */
	if (stat(path, &filestat) < 0) {
		perror(path);
		return EXIT_FAILURE;
	}

	if (S_ISREG(filestat.st_mode))
		return scan_file(path, filestat.st_size);

	if (S_ISDIR(filestat.st_mode)) {

		struct dirent * entry;
		DIR * dir = opendir(path);

		if (dir == NULL) {
			fprintf(stderr, "%s: opendir failed\n", path);
			return EXIT_FAILURE;
		}

		while ((entry = readdir(dir)) != NULL)
			process_path(path, entry->d_name);

		if (closedir(dir) < 0) {
			fprintf(stderr, "%s: closedir failed\n", path);
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
	}

	return EXIT_FAILURE;
}

int main (int argc, char ** argv) {

	int i;

	if (argc < 2) {
		fprintf(stderr, "usage: %s <file or directory> ...\n", argv[0]);
		return EXIT_FAILURE;
	}

	for (i = 1; i < argc; i++)
		process_path("", argv[i]);

	return EXIT_SUCCESS;
}

