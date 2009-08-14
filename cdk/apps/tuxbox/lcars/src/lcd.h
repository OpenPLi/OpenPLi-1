#ifndef LCD_H
#define LCD_H

#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <map>

#define LCD_DEV "/dev/dbox/lcd0"

class lcddisplay
{
	int fd;

	unsigned char *s;

	// Freetype-stuff
	FT_Library library;

	struct character
	{
		int size;

		int top;
		int left;
		int width;
		int rows;
		int pitch;
		int advancex;
		int bearingY;

		char bitmap[2000]; // very ugly, I know... :(
	};

	std::vector<FT_Face*> font_faces;
	typedef std::map<char, character> font_characters;
	typedef std::map<int, font_characters*> font_cache; // <size, <char-index, character>>
	std::vector<font_cache*> fonts;

	int current_textsize;
public:
	int width, height;

	lcddisplay();
	~lcddisplay();

	void clearDirect();
	void setDirectPixel(int x, int y, int val);

	void clear();
	void setPixel(int x, int y);
	void writeToLCD();
	void readFromLCD();
	void writeToFile(std::string filename);
	void loadFromFile(std::string filename);
	void loadFrom8bitFile(std::string filename);

	int loadFont(std::string filename);
	void setTextSize(int size);

	void putText(int x, int y, int font, std::string text);
	void draw_bitmap(character *ch, int x, int y);

};

#endif
