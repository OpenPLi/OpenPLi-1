#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

#include "ani.h"
#include "bmp.h"

int main(int argc, char **argv) {
	FILE *fbmp, *fani;

	struct bmp_header bh;
	//struct bmp_color *colors;
	long int line_size, bmpline_size, image_size;
	unsigned char *image;

	struct ani_header ah;
	lcd_raw_buffer raw;
	lcd_packed_buffer packed;
	int i;
	size_t rs;
	//int y, x, ofs, bit1, bit2;

	if (argc < 4) {
		fprintf(stderr, "bmp2ani (C) 2001 by Ge0rG\n");
		fprintf(stderr, "	%s delay out.ani in0.bmp [in1.bmp ...]\n",
		    argv[0]);
		exit(1);
	}
	printf("bmp2ani (C) 2001 by Ge0rG\n\n");

	memcpy(ah.magic, "LCDA", 4);
	ah.format = htons(0);
	ah.width = htons(120);
	ah.height = htons(64);
	ah.count = htons(argc-3);
	ah.delay = htonl(atol(argv[1]));

	printf("[%s] Creating ani (120x64, %i pics, %li µs delay):\n", argv[2],
	    ntohs(ah.count), ntohl(ah.delay));

	fani = fopen(argv[2], "w");
	fwrite(&ah, 1, sizeof(ah), fani);

	for (i = 3; i<argc; i++) {
		printf("[%s] Adding %s...\n", argv[2], argv[i]);

		if ((fbmp = fopen(argv[i], "r"))==NULL) {
			perror("fopen(BMP_FILE)");
			exit(2);
		}
		if (fread(&bh, 1, sizeof(bh), fbmp)!=sizeof(bh)) {
			perror("fread(BMP_HEADER)");
			exit(3);
		}
		if ((bh._B!='B')||(bh._M!='M')) {
			fprintf(stderr, "Bad Magic (not a BMP file).\n");
			exit(4);
		}

#define get32(h) (((unsigned char*)h)[0] + ((unsigned char*)h)[1]*256 + ((unsigned char*)h)[2]*256*256 + ((unsigned char*)h)[3]*256*256*256)
#define get16(h) (((unsigned char*)h)[0] + ((unsigned char*)h)[1]*256)
        bh.file_size = get32(&bh.file_size);     // file size
        bh.reserved = get32(&bh.reserved);      // = 0
        bh.data_offset = get32(&bh.data_offset);   // start of raw data
        bh.header_size = get32(&bh.header_size);   // = 40
        bh.width = get32(&bh.width);         // = 120
        bh.height = get32(&bh.height);        // = 64
        bh.planes = get16(&bh.planes);       // = 1
        bh.bit_count = get16(&bh.bit_count);    // 1..24
        bh.compression = get32(&bh.compression);   // = 0
        bh.image_size = get32(&bh.image_size);    // 120*64*bitcount/8
        bh.x_ppm = get32(&bh.x_ppm);         // X pixels/meter
        bh.y_ppm = get32(&bh.y_ppm);         // Y pixels/meter
        bh.colors_used = get32(&bh.colors_used);   //
        bh.colors_vip = get32(&bh.colors_vip);    // important colors (all=0)

#if 0
        printf("HEADER_SIZE = %d\n", sizeof(bh));     // file size
        printf("file_size   = %ld\n", bh.file_size);     // file size
        printf("reserved    = %ld\n", bh.reserved);      // = 0
        printf("data_offset = %ld\n", bh.data_offset);   // start of raw data
        printf("header_size = %ld\n", bh.header_size);   // = 40
        printf("width       = %ld\n", bh.width);         // = 120
        printf("height      = %ld\n", bh.height);        // = 64
        printf("planes      = %d\n",  bh.planes);       // = 1
        printf("bit_count   = %d\n",  bh.bit_count);    // 1..24
        printf("compression = %ld\n", bh.compression);   // = 0
        printf("image_size  = %ld\n", bh.image_size);    // 120*64*bitcount/8
        printf("x_ppm       = %ld\n", bh.x_ppm);         // X pixels/meter
        printf("y_ppm       = %ld\n", bh.y_ppm);         // Y pixels/meter
        printf("colors_used = %ld\n", bh.colors_used);   //
        printf("colors_vip  = %ld\n", bh.colors_vip);    // important colors (all=0)
#endif

		// 4 * 2^bit_count
		//fseek(fbmp, 4<<bh.bit_count, SEEK_CUR);
		fseek(fbmp, bh.data_offset, SEEK_SET);

		// image
		line_size = bh.width * bh.bit_count / 8;
		bmpline_size = (line_size + 3) & ~3;
		image_size = bmpline_size*bh.height;

#if 0
printf("line_size=%d, bmpline_size=%d, image_size=%d\n", line_size, bmpline_size, image_size);
printf("bit_count=%d, bmpline_size=%d, image_size=%d\n", bh.bit_count, bmpline_size, image_size);
#endif

		image = malloc(image_size);
		if ((rs = fread(image, 1, image_size, fbmp)) != image_size) {
			if (rs < 0)
				perror("fread(BMP_IMAGE)");
			else
				fprintf(stderr, "Only read %ld of %ld expecteed bytes. Quitting\n", rs, image_size);
			exit(6);
		}
		fclose(fbmp);
		
		if (bh.width != 120 || bh.height != 64) {
			printf("BMP not 120x64 - aborting\n");
			exit(7);
		}
		if (bh.compression != 0)
			printf("WARNING: Image is compressed - result unpredictable.\n");

		bmp2raw(bh, image, raw);
		free(image);

		raw2packed(raw, packed);

		fwrite(&packed, 1, sizeof(packed), fani);
	}

	fclose(fani);
	printf("[%s] ready.\n", argv[2]);
	return 0;
}
