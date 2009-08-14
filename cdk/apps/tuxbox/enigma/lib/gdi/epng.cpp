#include <png.h>
#include <stdio.h>
#include <lib/gdi/epng.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>

gImage *loadPNG(const char *filename)
{
	__u8 header[8];
	FILE *fp;
	gImage *res=0;
	png_structp png_ptr = 0;
	png_infop info_ptr = 0;
	png_infop end_info = 0;
	
	if (!(fp = fopen(filename, "rb")))
	{
//		eDebug("[ePNG] %s not found\n", filename);
		return 0;
	}

	if (!fread(header, 8, 1, fp))
	{
		fclose(fp);
		return 0;
	}
	
	if (png_sig_cmp(header, 0, 8))
		goto pngerror;

	if (!(png_ptr=png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0)))
		goto pngerror;
	
	if (!(info_ptr=png_create_info_struct(png_ptr)))
		goto pngerror;
		
	if (!(end_info=png_create_info_struct(png_ptr)))
		goto pngerror;
		
	if (setjmp(png_ptr->jmpbuf))
		goto pngerror;
		
	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);
	// png_set_invert_alpha(png_ptr);	// has no effect on indexed images
	png_read_info(png_ptr, info_ptr);
	
	png_uint_32 width, height;
	int bit_depth;
	int color_type;
	int channels;
	
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
	//eDebug("%s: before %dx%dx%d png, %d", filename, (int)width, (int)height, (int)bit_depth, color_type);

	if (bit_depth == 16) // convert 16 bit images to 8 bit!
		png_set_strip_16(png_ptr);
	
	/*
	 * convert 1,2 and 4 bpp to 8bpp images that enigma can blit
	 * Expand G+tRNS to GA, RGB+tRNS to RGBA
	 */
	if ((color_type != PNG_COLOR_TYPE_PALETTE && png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) ||
	    (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8))
		png_set_expand (png_ptr);

	if (color_type == PNG_COLOR_TYPE_PALETTE && bit_depth < 8)
		png_set_packing (png_ptr);
	
	// gPixmaps use 4 bytes for 24 bit images
	if (color_type == PNG_COLOR_TYPE_RGB)
		png_set_add_alpha(png_ptr, 0, PNG_FILLER_BEFORE);

	if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		png_set_swap_alpha(png_ptr);
		png_set_invert_alpha(png_ptr);
	}

	// Update the info structures after the transformations take effect
	png_read_update_info (png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
	//eDebug("%s: after %dx%dx%d png, %d", filename, (int)width, (int)height, (int)bit_depth, color_type);

	channels = png_get_channels(png_ptr, info_ptr);

	res = new gImage(eSize(width, height), bit_depth * channels);

	png_bytep *rowptr=new png_bytep[height];
	for (unsigned int i=0; i<height; i++)
		rowptr[i]=((png_byte*)(res->data))+i*res->stride;
	// png_read_rows(png_ptr, rowptr, 0, height);
	png_read_image(png_ptr, rowptr);

	delete [] rowptr;

	if (color_type == PNG_COLOR_TYPE_PALETTE) 
	{
		// indexed pictures without a palette make no sense
		if (!png_get_valid(png_ptr, info_ptr, PNG_INFO_PLTE))
			goto pngerror;

		png_color *palette;
		int num_palette, num_trans;
		png_get_PLTE(png_ptr, info_ptr, &palette, &num_palette);
		if (num_palette) 
		{
			res->clut.data=new gRGB[num_palette];
			res->clut.colors=num_palette;
		}

		for (int i=0; i<num_palette; i++)
		{
			res->clut.data[i].a=0;
			res->clut.data[i].r=palette[i].red;
			res->clut.data[i].g=palette[i].green;
			res->clut.data[i].b=palette[i].blue;
		}
		if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		{
			png_byte *trans;
			png_color_16 *transparent_color;

			png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &transparent_color);
			//if (transparent_color)
			//	res->clut.data[transparent_color->index].a=255;
			if (trans)					
				for (int i=0; i<num_trans && i < num_palette; i++)
				res->clut.data[i].a=255-trans[i];
		}
	}

	png_destroy_read_struct(&png_ptr, &info_ptr,&end_info);
	fclose(fp);
	return res;

pngerror:
	eDebug("[ePNG] png structure failure in %s\n", filename);
	if (res)
		delete res;
	if (png_ptr)
		png_destroy_read_struct(&png_ptr, info_ptr ? &info_ptr : (png_infopp)NULL, end_info ? &end_info : (png_infopp)NULL);
	if (fp)
		fclose(fp);
	return 0;
}

int savePNG(const char *filename, gPixmap *pixmap)
{
	FILE *fp=fopen(filename, "wb");
	if (!fp)
		return -1;
	png_structp png_ptr=png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if (!png_ptr)
	{
		eDebug("[ePNG] write: couldn't allocate write struct");
		fclose(fp);
		unlink(filename);
		return -2;
	}
	png_infop info_ptr=png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		eDebug("[ePNG] write: couldn't allocate info struct");
		png_destroy_write_struct(&png_ptr, 0);
		fclose(fp);
		unlink(filename);
		return -3;
	}
	if (setjmp(png_ptr->jmpbuf))
	{
		eDebug("error :/");
		eDebug("[ePNG] write: couldn't set setjmp");
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		unlink(filename);
		return -4;
	}
	png_init_io(png_ptr, fp);
	png_set_filter(png_ptr, 0, PNG_FILTER_NONE|PNG_FILTER_SUB|PNG_FILTER_PAETH);
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	if (pixmap->bpp == 8)
	{
		png_set_IHDR(png_ptr, info_ptr, pixmap->x, pixmap->y, 8, 
			pixmap->clut.data ? PNG_COLOR_TYPE_PALETTE : PNG_COLOR_TYPE_GRAY, 
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		if (pixmap->clut.data)
		{
			png_color palette[pixmap->clut.colors];
			png_byte trans[pixmap->clut.colors];
			for (int i=0; i<pixmap->clut.colors; ++i)
			{
				palette[i].red=pixmap->clut.data[i].r;
				palette[i].green=pixmap->clut.data[i].g;
				palette[i].blue=pixmap->clut.data[i].b;
				trans[i]=255-pixmap->clut.data[i].a;
			}
			png_set_PLTE(png_ptr, info_ptr, palette, pixmap->clut.colors);
			png_set_tRNS(png_ptr, info_ptr, trans, pixmap->clut.colors, 0);
		}
		png_write_info(png_ptr, info_ptr);
		png_set_packing(png_ptr);
	}
	else if (pixmap->bpp == 32)
	{
		png_set_IHDR(png_ptr, info_ptr, pixmap->x, pixmap->y, 8, 
			PNG_COLOR_TYPE_RGB_ALPHA, 
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);

		/* swap location of alpha bytes from ARGB to RGBA */
		png_set_swap_alpha(png_ptr);
	}
	else
	{
		fclose(fp);
		eDebug("[ePNG] write: png file with %d bpp colordepth not suported yet. Update gdi/epng.cpp !", pixmap->bpp);
		return -5;
	}
	png_byte *row_pointers[pixmap->y];
	for (int i=0; i<pixmap->y; ++i)
	{
		row_pointers[i]=((png_byte*)pixmap->data)+i*pixmap->stride;
	}
	// eDebug("[ePNG] write: rows done. write image");
	png_write_image(png_ptr, row_pointers);
	// eDebug("[ePNG] write: write done. write end");
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fp);
	eDebug("[ePNG] write: %d bit png fine", pixmap->bpp);
	return 0;
}
