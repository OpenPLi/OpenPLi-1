#include "lcd.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <iostream>

#include <dbox/lcd-ks0713.h>

lcddisplay::lcddisplay()
{
	fd = open(LCD_DEV, O_RDWR);
	if (fd < 0)
	{
		std::cerr << "[lcd.cpp] Couldn't open LCD" << std::endl;
		exit(1);
	}
	int i = LCD_MODE_BIN;
	if (ioctl(fd, LCD_IOCTL_ASC_MODE,&i) < 0)
	{
		std::cerr << "[lcd.cpp] Couldn't switch to binary drawing mode" << std::endl;
		exit(1);
	}
	
	width = LCD_COLS;
	height = LCD_ROWS * 8;
	s = (unsigned char*)malloc(LCD_BUFFER_SIZE);

	int error = FT_Init_FreeType( &library );
	if (error)
	{
		std::cerr << "[lcd.cpp] Error initializing Freetype-library" << std::endl;
		exit(1);
	}

	clear();
	writeToLCD();

	std::cout << "[lcd.cpp] Initialized LCARS-LCD - Size " << width << "x" << height << std::endl;
}

lcddisplay::~lcddisplay()
{
	close(fd);
}

void lcddisplay::clearDirect()
{
	if (ioctl(fd, LCD_IOCTL_CLEAR) < 0)
	{
		std::cerr << "[lcd.cpp] Couldn't clear LCD" << std::endl;
	}
}

void lcddisplay::setDirectPixel(int x, int y, int val)
{
	lcd_pixel pixel;
	pixel.x = x;
	pixel.y = y;
	pixel.v = val;
	if (ioctl(fd, LCD_IOCTL_SET_PIXEL, pixel) < 0)
	{
		std::cerr << "[lcd.cpp] Couldn't set Pixel on  LCD" << std::endl;
	}
	
}

void lcddisplay::clear()
{
	memset(s, 0, LCD_BUFFER_SIZE);
}

void lcddisplay::setPixel(int x, int y)
{
	std::cout << (y % 8) << std::endl;
}

void lcddisplay::writeToLCD()
{
	::write(fd, s, LCD_BUFFER_SIZE);
}

void lcddisplay::readFromLCD()
{
	::read(fd, s, LCD_BUFFER_SIZE);
	for (int i = 0; i < LCD_BUFFER_SIZE; i++)
		if (s[i] != 0)
			std::cout << i << " - " << ((int)s[i]) << std::endl;
}

void lcddisplay::	writeToFile(std::string filename)
{
	int f = open(filename.c_str(), O_WRONLY | O_CREAT);
	::write(f, (void*)s, LCD_BUFFER_SIZE);
	close(f);
}

void lcddisplay::loadFromFile(std::string filename)
{
	int f = open(filename.c_str(), O_RDONLY);
	::read(f, (void*)s, LCD_BUFFER_SIZE);
	close(f);
	
}

void lcddisplay::loadFrom8bitFile(std::string filename)
{
	int f = open(filename.c_str(), O_RDONLY);
	unsigned char tmp_line[LCD_COLS];
	for (int i = 0; i < LCD_ROWS; i++)
	{
		for (int line = 1; line < 256; line*=2)
		{
			::read(f, (void*)tmp_line, LCD_COLS);
			for (int j = 0; j < LCD_COLS; j++)
			{
				s[i * LCD_COLS + j] |= ((tmp_line[j] >= 128) ? line : 0);
			}
		}
	}
	close(f);
	
}

int lcddisplay::loadFont(std::string filename)
{
	FT_Face *face = new FT_Face;
	
	int error = FT_New_Face(library, filename.c_str(), 0, face);
	if (error == FT_Err_Unknown_File_Format)
	{
		std::cerr << "[lcd.cpp] Error in font fileformat opening " << filename << std::endl;
	}
	else if (error)
	{
		std::cerr << "[lcd.cpp] Error opening " << filename << std::endl;
	}

	font_faces.push_back(face);
	font_cache *tmp_font_cache = new font_cache;
	fonts.push_back(tmp_font_cache);

	return (font_faces.size() - 1);
}

void lcddisplay::setTextSize(int size)
{
	current_textsize = size;
	//	error = FT_Set_Pixel_Sizes(face, 0, current_textsize);
}

void lcddisplay::putText(int x, int y, int font, std::string text)
{
	font_cache *curr_font_cache = fonts[font];
	int pen_x = x;
	int pen_y = y;
	for (unsigned int i = 0; i < text.size(); i++)
	{
		FT_GlyphSlot *slot = new FT_GlyphSlot;
		struct character ch;
		
		bool found = false;
		font_characters *curr_font_characters;
		if (curr_font_cache->size() != 0 && curr_font_cache->count(current_textsize) != 0) // font of this size available
		{
			curr_font_characters = (*curr_font_cache)[current_textsize];
			if (curr_font_characters->size() != 0 && curr_font_characters->count(text[i]) != 0) // character available
			{
				found = true;
				ch = (*curr_font_characters)[text[i]];
			}
		}
		else // font of this size not available
		{
			font_characters *tmp_font_characters = new font_characters;
			(*curr_font_cache).insert(std::pair<int, font_characters*>(current_textsize, tmp_font_characters));
			curr_font_characters = tmp_font_characters;
		}

		if (!found) // we don't have the character so we have to cache it first
		{
			FT_Face face = (*font_faces[font]);
			int error = FT_Set_Pixel_Sizes(face, 0, current_textsize); // Set the font size
			if (error)
			{
				std::cerr << "[lcd.cpp] Couldn't set font size" << std::endl;
				return;
			}
			error = FT_Load_Char(face, text[i], FT_LOAD_RENDER);//| FT_LOAD_MONOCHROME ); // render character
			if (error)
			{
				std::cerr << "[lcd.cpp] Couldn't render character " << text[i] << std::endl;
				continue;  // ignore errors
			}
			*slot = face->glyph;
			ch.left = (*slot)->bitmap_left;
			ch.top = (*slot)->bitmap_top;
			ch.width = (&(*slot)->bitmap)->width;
			ch.size = current_textsize;
			ch.rows = (&(*slot)->bitmap)->rows;
			ch.pitch = (&(*slot)->bitmap)->pitch;
			ch.advancex = (*slot)->advance.x >> 6;
			ch.bearingY = (*slot)->metrics.horiBearingY;
			int position = 0;
			for (int y_pos = 0; y_pos < ch.rows; y_pos++)
			{
				int y_position = y_pos * ch.pitch;
				for (int x_pos = 0; x_pos < ch.pitch; x_pos++)
				{
					ch.bitmap[position++] = (&(*slot)->bitmap)->buffer[x_pos+y_position];
				}
			}

			curr_font_characters->insert(std::pair<char, character>(text[i], ch));
		}
		
		draw_bitmap(&ch, pen_x + ch.left, pen_y - ch.top );

		pen_x += ch.advancex; 
	}
}

void lcddisplay::draw_bitmap(character *ch, int x, int y)
{
	for (int y_pos = 0; y_pos < ch->rows; y_pos++)
	{
		int y_position = y_pos * ch->pitch;
		for (int x_pos = 0; x_pos < ch->pitch; x_pos++)
		{
			if (ch->bitmap[x_pos+y_position] >= 128 && (x + x_pos) < width)
				setDirectPixel(x + x_pos, y + y_pos, 1);
		}
	}
}
