#include "fbClass.h"

fbClass::fbClass(variables *v, int x, int y, int bpp)
{
	int fd;
	vars = v;

	if ((fd = open("/dev/vc/0", O_RDWR)) < 0)
		perror ("/dev/vc/0");
	else if (ioctl(fd, KDSETMODE, KD_GRAPHICS) < 0)
		perror("KDSETMODE");
	close (fd);

	fbfd = open(FB_DEV, O_RDWR);
	if (!fbfd)
	{
		perror("open framebuffer");
		exit(1);
	}

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &old_finfo))
	{
		perror("FBIOGET_FSCREENINFO");
		exit(2);
	}


	if (ioctl(fbfd, FBIOGET_VSCREENINFO, &old_vinfo))
	{
		perror("FBIOGET_VSCREENINFO");
		exit(3);
	}

	setMode(x, y, bpp);

	cmap.start  = 0;
	cmap.len = 256;
	cmap.red = r;
	cmap.green = g;
	cmap.blue = b;
	cmap.transp = tr;

	transparent_color = 0;
	setPalette(0, 0, 0, 0, 0xff);

	for (int i = 0; i < 256; i++)
	{
		fade_down[i] = (int) (((float)i / (float)255) * (COLORFADE - 1));
	}
}

fbClass::~fbClass()
{
	close (fbfd);
}

void fbClass::test()
{
	FILE *fp = fopen("share/tuxbox/lcars/skin/skin.png", "rb");
	if (!fp)
	{
		perror("Open skin");
		goto noskin;
		return;
	}
noskin:
	printf("Skin file wasn't in path.\n");
}



void fbClass::runCommand(std::string command_string)
{
	std::string command;
	std::istringstream iss(command_string);
	std::getline(iss, command, ' ');
	int values_int[10];
	std::string value;

	if (command == "FILLBOX")
	{
		int i = 0;
		while (std::getline(iss, value, ' '))
		{
			values_int[i++] = atoi(value.c_str());
		}
		if (i >= 4)
		{
			fillBox(values_int[0], values_int[1], values_int[2], values_int[3], values_int[4]);
		}
	}
	else if (command == "SETFADE")
	{
		int i = 0;
		while (std::getline(iss, value, ' '))
		{
			values_int[i++] = atoi(value.c_str());
		}

		if (i >= 7)
		{
			setFade(values_int[0], values_int[1], values_int[2], values_int[3], values_int[4], values_int[5], values_int[6]);
		}
	}
	else if (command == "PUTTEXT")
	{
		for (int i = 0; i < 5; i++)
		{
			std::getline(iss, value, ' ');
			values_int[i] = atoi(value.c_str());
		}
		std::getline(iss, value, '\n');
		float size;
		sscanf(value.c_str(), "%f", &size);
		std::getline(iss, value, '\n');

		if (vars->isavailable(value))
			value = vars->getvalue(value);
		if (size != factor)
			setTextSize(size);

		putText(values_int[0], values_int[1], values_int[2], value, values_int[3], values_int[4]);
	}
	else if (command == "CLEARSCREEN")
	{
		clearScreen();
	}
	else if (command == "SETTEXTSIZE")
	{
		std::getline(iss, value, ' ');
		float val;
		sscanf(value.c_str(), "%f", &val);
		setTextSize(val);
	}
}

void fbClass::setMode(int x, int y, int bpp)
{
	int screensize;

	memcpy(&vinfo, &old_vinfo, sizeof(struct fb_var_screeninfo));
	memcpy(&finfo, &old_finfo, sizeof(struct fb_fix_screeninfo));

	vinfo.xres_virtual = vinfo.xres = x;
	vinfo.yres_virtual = vinfo.yres = y;
	vinfo.bits_per_pixel = bpp;
	if (ioctl(fbfd, FBIOPUT_VSCREENINFO, &vinfo))
	{
		perror("FBIOPUT_VSCREENINFO");
		exit(3);
	}

	if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo))
	{
		perror("FBIOGET_FSCREENINFO");
		exit(2);
	}

	screensize = finfo.smem_len;
	fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (!fbp)
	{
		perror("mmap");
		exit(4);
	}

	bytes_per_pixel = vinfo.bits_per_pixel / 8;
	for (int i = 0; i < (int)vinfo.xres; i++)
		x_calc[i] = (i + vinfo.xoffset) * bytes_per_pixel;
	for (int i = 0; i < (int)vinfo.yres; i++)
		y_calc[i] = (i + vinfo.yoffset) * finfo.line_length;
}

void fbClass::fillBox(int x1, int y1, int x2, int y2, int color)
{
	if (color == -1)
		color = transparent_color;
	else
		color = fades[color][0];
	int size = bytes_per_pixel * (x2 - x1);
	if (vinfo.bits_per_pixel == 8)
	{
		for (int y = y1; y < y2; y++)
		{
			memset(fbp + x_calc[x1] + y_calc[y], color, size);
		}
	}
	else
	{
		for (int x = x1; x < x2; x++)
		{
			*((unsigned short int*)(fbp + x_calc[x] + y_calc[y1])) = color;
		}

		for (int y = y1 + 1; y < y2; y++)
		{
			memcpy((fbp + x_calc[x1] + y_calc[y]), fbp + x_calc[x1] + y_calc[y1], size);
		}
	}
}

void fbClass::setPalette(int color, int red, int green, int blue, int transp)
{
	r[color] = red << 8;
	g[color] = green << 8;
	b[color] = blue << 8;
	tr[color] = transp << 8;

	ioctl(fbfd, FBIOPUTCMAP, &cmap);
}

void fbClass::setFade(int color, int r_start, int g_start, int b_start, int r_stop, int g_stop, int b_stop)
{
	for (int i = 0; i < COLORFADE; i++)
	{
		float percent = (i * (255 / COLORFADE)) / (float) 255;

		int r = r_start + (int) ((r_stop - r_start) * percent);
		int g = g_start + (int) ((g_stop - g_start) * percent);
		int b = b_start + (int) ((b_stop - b_start) * percent);

		int act_color = color * COLORFADE + i;
		setPalette(act_color, r, g, b, 0);
		fades[color][i] = act_color;
	}
}

void fbClass::clearScreen()
{
	fillBox(0, 0, vinfo.xres, vinfo.yres, -1);
}

void fbClass::fillScreen(int color)
{
	fillBox(0, 0, vinfo.xres, vinfo.yres, color);
}

void fbClass::setPixel(int x, int y, int color)
{
	//x *= 2;
	if (vinfo.bits_per_pixel == 8)
		memset((unsigned short int*)(fbp + x_calc[x] + y_calc[y]), color, 1);
	else
		*((unsigned short int*)(fbp + x_calc[x] + y_calc[y])) = color;
}

unsigned short int fbClass::getPixel(int x, int y)
{
	return *((unsigned short int*) (fbp + x_calc[x] + y_calc[y]));
}

int fbClass::getMaxX()
{
	return vinfo.xres;
}

int fbClass::getMaxY()
{
	return vinfo.yres;
}

int fbClass::getWidth(char c)
{
	int width = 0, error;
	struct font_cache font;
	bool found = false;
	if (cache.count(c) != 0)
	{
		std::pair<It, It> ip = cache.equal_range(c);
		for (It it = ip.first; it != ip.second; ++it)
		{
			if ((*it).second.size == ((int) (40 * factor)))
			{
				found = true;
				width = (*it).second.width;
			}
		}
	}

	if (!found)
	{
		error = FT_Load_Char(face, c, FT_LOAD_RENDER);
		if (error)
		{
			perror("FT_Load_Char");
		}
		font.left = slot->bitmap_left;
		font.top = slot->bitmap_top;
		width = font.width = (&slot->bitmap)->width;
		font.size = (int) (40 * factor);
		font.rows = (&slot->bitmap)->rows;
		font.pitch = (&slot->bitmap)->pitch;
		font.advancex = slot->advance.x >> 6;
		int position = 0;
		for (int y_pos = 0; y_pos < font.rows; y_pos++)
		{
			int y_position = y_pos * font.pitch;
			for (int x_pos = 0; x_pos < font.pitch; x_pos++)
			{
				font.bitmap[position++] = (&slot->bitmap)->buffer[x_pos+y_position];
			}
		}
		cache.insert(std::pair<char const, struct font_cache>(c, font));
	}

	return width;
}


void fbClass::loadFonts(std::string standardfont, std::string vtfont)
{
	int error;

	error = FT_Init_FreeType( &library );
	if (error)
	{
		perror("FT_Init_FreeType");
		return;
	}

	error = FT_New_Face( library,
	                     standardfont.c_str(),
	                     0,
	                     &face );
	if ( error == FT_Err_Unknown_File_Format )
	{
		perror("FT_New_Face");
		return;
	}
	else if ( error )
	{
		perror("FT_New_Face");
		return;
	}

	slot = face->glyph;

	error = FT_New_Face( library,
	                     vtfont.c_str(),
	                     0,
	                     &face_vt );
	if ( error == FT_Err_Unknown_File_Format )
	{
		perror("FT_New_Face");
		return;
	}
	else if ( error )
	{
		perror("FT_New_Face");
		return;
	}

	slot_vt = face_vt->glyph;

	setTextSize(0.5);
}

void fbClass::setTextSize(float setfactor)
{
	int error;


	factor = setfactor;

	error = FT_Set_Pixel_Sizes(
	            face,
	            0,
	            (int) (40 * factor) );


	if (ycorrector.count(setfactor) < 1)
	{
		FT_Load_Char(face, 'A', FT_LOAD_RENDER);
		ycorrector.insert(std::pair<float, int> (setfactor, slot->bitmap_top));
	}
	if (error)
	{
		perror("FT_Set_Pixel_Sizes");
	}
}

void fbClass::draw_bitmap(font_cache font, int x, int y, int color)
{
	for (int y_pos = 0; y_pos < font.rows; y_pos++)
	{
		int y_position = y_pos * font.pitch;
		int y_calc_temp = y_calc[y + y_pos];
		for (int x_pos = 0; x_pos < font.pitch; x_pos++)
		{
			long int location;

			location = x_calc[x + x_pos] + y_calc_temp;
			setPixel(x + x_pos, y + y_pos, fades[color][fade_down[font.bitmap[x_pos+y_position]]]);
		}
	}
}

void fbClass::putText(int xpos, int ypos, int color, std::string text, int max_size, int alignment)
{
	char c_text[500];
	strcpy(c_text, text.c_str());
	putText(xpos, ypos, color, c_text, max_size, alignment);

}

void fbClass::putText(int xpos, int ypos, int color, int i, int max_size, int alignment)
{
	char c_text[500];
	sprintf(c_text, "%d", i);
	putText(xpos, ypos, color, c_text, max_size,alignment);

}

void fbClass::putText(int xpos, int ypos, int color, char text[500], int max_size, int alignment)
{
	int error;

	int pen_x, pen_y, n;

	pen_x = xpos;
	pen_y = ypos;
	int endx = xpos + max_size;
	bool found;

	for (int i = 0; i < (int)strlen(text); i++ )
	{
		if (alignment == 1)
			n = strlen(text) - i - 1;
		else
			n = i;
		if (text[n] == 135 || text[n] == 134 || text[n] == 5)
			continue;
		struct font_cache font;
		found = false;
		if (cache.count(text[n]) != 0)
		{
			std::pair<It, It> ip = cache.equal_range(text[n]);
			for (It it = ip.first; it != ip.second; ++it)
			{
				if ((*it).second.size == ((int) (40 * factor)))
				{
					found = true;
					font = (*it).second;
				}
			}
		}


		if (!found)
		{
			error = FT_Load_Char(face, text[n], FT_LOAD_RENDER);
			if (error)
			{
				perror("FT_Load_Char");
				continue;
			}
			font.left = slot->bitmap_left;
			font.top = slot->bitmap_top;
			font.width = (&slot->bitmap)->width;
			font.size = (int) (40 * factor);
			font.rows = (&slot->bitmap)->rows;
			font.pitch = (&slot->bitmap)->pitch;
			font.advancex = slot->advance.x >> 6;
			font.bearingY = slot->metrics.horiBearingY;
			int position = 0;
			for (int y_pos = 0; y_pos < font.rows; y_pos++)
			{
				int y_position = y_pos * font.pitch;
				for (int x_pos = 0; x_pos < font.pitch; x_pos++)
				{
					font.bitmap[position++] = (&slot->bitmap)->buffer[x_pos+y_position];
				}
			}
			cache.insert(std::pair<char const, struct font_cache>(text[n], font));
		}

		if (pen_x + font.left + font.width > endx && max_size > -1)
			break;

		if (alignment == 0)
		{
			draw_bitmap( font, pen_x + font.left, pen_y - font.top + (*ycorrector.find(factor)).second, color );
			pen_x += font.advancex;
		}
		else if (alignment == 1)
		{
			draw_bitmap( font, pen_x - font.advancex + font.left, pen_y - font.top + (*ycorrector.find(factor)).second, color );
			pen_x -= font.advancex + 2;
		}
	}
}
