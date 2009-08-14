
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <dbox/lcd-ks0713.h>

#include "bmp.h"
#include "raw.h"

#define swap32bits(i) (i>>24) | (i<<24) | ((i>>8) & 0x00f0) | ((i<<8) & 0x0f00)
#define swap16bits(i) (i>>8) | (i<<8)

int main(int argc, char **argv) {
	FILE *fbmp;

	struct bmp_header bh;
	struct bmp_color *colors;
	long int line_size, bmpline_size, image_size;
	unsigned char *image;

	lcd_raw_buffer raw;
	lcd_packed_buffer s;
	int lcd_fd, mode;
	
	//int y, x, ofs, bit1, bit2;

	if (argc < 2) {
		fprintf(stderr, "bmp_show (C) 2001 by Ge0rG\n");
		fprintf(stderr, "	%s file.bmp\n",
		    argv[0]);
		exit(1);
	}
	if ((fbmp = fopen(argv[1], "r"))==NULL) {
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

	#if 1
	bh.file_size = swap32bits(bh.file_size);
	bh.width = swap32bits(bh.width);
	bh.height = swap32bits(bh.height);
	bh.bit_count = swap16bits(bh.bit_count);
	#endif

	// 4 * 2^bit_count
	colors = malloc(4<<bh.bit_count);
	if (fread(colors, 1, 4<<bh.bit_count, fbmp)!=4<<bh.bit_count) {
		perror("fread(BMP_COLORS)");
		exit(5);
	}

	// image
	line_size = (bh.width*bh.bit_count / 8);
	bmpline_size = (line_size + 3) & ~3;
	image_size = bmpline_size*bh.height;

	image = malloc(image_size);
	if (fread(image, 1, image_size, fbmp)!=image_size) {
		perror("fread(BMP_IMAGE)");
		exit(6);
	}
	fclose(fbmp);
	
	if ((bh.width != 120) || (bh.height != 64))
		printf("WARNING: Not 120x64 pixels - result unpredictable.\n");
	if (bh.compression != 0)
		printf("WARNING: Image is compressed - result unpredictable.\n");

        /*printf("File size:      %li\n", bh.file_size);
        printf("Width*Height:   %li*%li\n", bh.width, bh.height);
        printf("Colors:         %i\n", 1<<bh.bit_count);
        printf("Line size:      %li\n", line_size);
        printf("Image size:     %li\n\n", image_size);*/

	bmp2raw(bh, image, raw);
	raw2packed(raw, s);

        if ((lcd_fd = open("/dev/dbox/lcd0",O_RDWR)) < 0) {
                perror("open(/dev/dbox/lcd0)");
                exit(1);
        }
        mode = LCD_MODE_BIN;
        if (ioctl(lcd_fd,LCD_IOCTL_ASC_MODE,&mode) < 0) {
                perror("init(LCD_MODE_BIN)");
                exit(1);
        }
	write(lcd_fd, &s, LCD_BUFFER_SIZE);
	close(lcd_fd);
	return 0;
}
