#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "raw.h"

int main(int argc, char **argv) {
	FILE *fbmp, *fraw;

	struct bmp_header bh;
	struct bmp_color *colors;
	long int line_size, bmpline_size, image_size;
	unsigned char *image;

	struct raw_header rh;
	lcd_raw4bit_buffer raw4bit_out;
	lcd_raw_buffer raw;
	
	//int y, x, ofs, bit1, bit2;

	if (argc < 2) {
		fprintf(stderr, "bmp2raw (C) 2001 by Ge0rG\n");
		fprintf(stderr, "	%s in.bmp [out.raw [transparency]] \n",
		    argv[0]);
		fprintf(stderr, "\n[WARNING: Hack] This app writes a ppcboot logo to ./bmp2raw.bll\n");
		exit(1);
	}
	if ((fbmp = fopen(argv[1], "r"))==NULL) {
		perror("fopen(BMP_FILE)");
		exit(2);
	}
	// FSCKING gcc setzt einfach dummy-bytes zum Padding ein!!!!!!
	if (fread(&bh, 1, sizeof(bh), fbmp)!=sizeof(bh)) {
		perror("fread(BMP_HEADER)");
		exit(3);
	}
	if ((bh._B!='B')||(bh._M!='M')) {
		fprintf(stderr, "Bad Magic (not a BMP file).\n");
		exit(4);
	}

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
	
	printf("bmp2raw (C) 2001 by Ge0rG\n\n");
	printf("File size:	%li\n", bh.file_size);
	printf("Width*Height:	%li*%li\n", bh.width, bh.height);
	printf("Colors:		%i\n", 1<<bh.bit_count);
	printf("Line size:	%li\n", line_size);
	printf("Image size:	%li\n\n", image_size);
	/*for (x=0; x< (1<<bh.bit_count); x++) {
		printf("  col[%02i] =	%02x.%02x.%02x\n",
		    x, colors[x].r, colors[x].g, colors[x].b);
	}*/

	if ((bh.width != 120) || (bh.height != 64))
		printf("WARNING: Not 120x64 pixels - result unpredictable.\n");
	if (bh.compression != 0)
		printf("WARNING: Image is compressed - result unpredictable.\n");
	if (argc < 3) {
		printf("No output filename given. Stopping now.\n");
		return 0;
	}

	/*switch (bh.bit_count) {
	case 1:
		for (y=0; y<bh.height; y++) {
			for (x=0; x < (bh.width>>1); x++) {
				// 0xf0*pixel1 + 0x0f*pixel2
				ofs = (bh.height-1-y)*bmpline_size + (x>>3);
				bit1 = (image[ofs] >> (x&7)) & 1;
				bit2 = (image[ofs] >> ((x+1)&7)) & 1;
				raw4bit_out[y][x] = bit1*0x0F + bit2*0xF0;
			}
		}
	case 4:
		for (y=0; y<bh.height; y++) {
			memcpy(&raw4bit_out[y],
			    &image[(bh.height-1-y)*bmpline_size],
			    line_size);
		}
		break;
	default:
		printf("WARNING: unknown color depth - result unpredictable.\n");
	}*/

	bmp2raw(bh, image, raw);
	/* boot logo erzeugen */
	printf("Writing bmp2raw.bll (BootLogo for Lcd)\n");
	fraw = fopen("bmp2raw.bll", "w");
	fwrite(&raw, 1, sizeof(raw), fraw);
	fclose(fraw);

	raw2raw4bit(raw, raw4bit_out);

	rh.width=120;
	rh.height=64;

	if (argc >= 4) {
		rh.trans = atoi(argv[3]);
	} else
		rh.trans = 0;

	printf("Writing %s (icon for lcdd)\n", argv[2]);
	fraw = fopen(argv[2], "w");
	fwrite(&rh, 1, sizeof(rh), fraw);
	fwrite(&raw4bit_out, 1, sizeof(raw4bit_out), fraw);
	fclose(fraw);
	return 0;
}
