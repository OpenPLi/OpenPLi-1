// Card.cpp: implementation of the CCard class.
//
//////////////////////////////////////////////////////////////////////

#include "Card.h"
#include "math.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
extern "C"
{
#include "draw.h"
}
#include "colors.h"
#include "pnm_file.h"

#include "backbuffer.h"


#define CARD_WIDTH		74
#define CARD_HEIGHT		110

#define CARD_SPACE		5
#define CARD_CORNER		15



char*			suits[4] = { "h", "d", "c", "s" };

const char*		terms[] =
{
	/* 0 */
	"small",
	"large",
	"face",

	/* 3 */
	"red",
	"black",

	/* 5 */
	"border",
	"box",
	"back",
	"ace",

	/* 9 */
	"a",
	"k",
	"q",
	"j",

	/* 13 */
	"t",
	"9",
	"8",
	"7",
	"6",

	/* 18 */
	"5",
	"4",
	"3",
	"2"
};

#define TRED	3
#define TBLACK	4

#define TLARGE	1
#define TFACE	2

#define TACE	9



struct card_element
{
	unsigned char	term;
	bool			suit;
	unsigned char	value; //if suit is false, add value
	int				dx;
	int				dy;
	bool			invert;
};


struct card_def
{
	unsigned char	num_of_elements;

	card_element	elements[10];
};

card_def	cards[13] =
{
	{ /* ace */
		1, 
		{ TLARGE, true, 9, 30,40, false }
	},
	{ /* king */
		2, 
		{
			{ TFACE, false, 10, 14,10, false },
			{ TFACE, false, 10, 58,85, true },
		}
	},
	{ /* queen */
		2,
		{
			{ TFACE, false, 10, 14,10, false },
			{ TFACE, false, 10, 58,85, true },
		}
	},
	{ /* jacke */
		2, 
		{
			{ TFACE, false, 10, 14,10, false },
			{ TFACE, false, 10, 58,85, true },
		}
	},
	{ /* 10 */
		10, 
		{
			{ TLARGE, true, 10, 14,10, false },
			{ TLARGE, true, 10, 44,10, false },
			{ TLARGE, true, 10, 29,19, false },
			{ TLARGE, true, 10, 14,30, false },
			{ TLARGE, true, 10, 44,30, false },
			{ TLARGE, true, 10, 28,64, true },
			{ TLARGE, true, 10, 58,64, true },
			{ TLARGE, true, 10, 43,75, true },
			{ TLARGE, true, 10, 28,84, true },
			{ TLARGE, true, 10, 58,84, true },
		}
	},
	{ /* 9 */
		9, 
		{
			{ TLARGE, true, 10, 14,10, false },
			{ TLARGE, true, 10, 44,10, false },
			{ TLARGE, true, 10, 14,30, false },
			{ TLARGE, true, 10, 44,30, false },
			{ TLARGE, true, 10, 29,41, false },
			{ TLARGE, true, 10, 28,64, true },
			{ TLARGE, true, 10, 58,64, true },
			{ TLARGE, true, 10, 28,84, true },
			{ TLARGE, true, 10, 58,84, true },
		}
	},
	{ /* 8 */
		8, 
		{
			{ TLARGE, true, 10, 14,10, false },
			{ TLARGE, true, 10, 44,10, false },
			{ TLARGE, true, 10, 14,30, false },
			{ TLARGE, true, 10, 44,30, false },
			{ TLARGE, true, 10, 28,64, true },
			{ TLARGE, true, 10, 58,64, true },
			{ TLARGE, true, 10, 28,84, true },
			{ TLARGE, true, 10, 58,84, true },
		}
	},
	{ /* 7 */
		7, 
		{
			{ TLARGE, true, 10, 14,10, false },
			{ TLARGE, true, 10, 44,10, false },
			{ TLARGE, true, 10, 29,25, false },
			{ TLARGE, true, 10, 14,40, false },
			{ TLARGE, true, 10, 44,40, false },
			{ TLARGE, true, 10, 28,84, true },
			{ TLARGE, true, 10, 58,84, true },
		}
	},
	{ /* 6 */
		6, 
		{
			{ TLARGE, true, 10, 14,10, false },
			{ TLARGE, true, 10, 44,10, false },
			{ TLARGE, true, 10, 14,40, false },
			{ TLARGE, true, 10, 44,40, false },
			{ TLARGE, true, 10, 28,84, true },
			{ TLARGE, true, 10, 58,84, true },
		}
	},
	{ /* 5 */
		5,
		{
			{ TLARGE, true, 10, 14,10, false },
			{ TLARGE, true, 10, 44,10, false },
			{ TLARGE, true, 10, 29,40, false },
			{ TLARGE, true, 10, 28,84, true },
			{ TLARGE, true, 10, 58,84, true },
		}
	},
	{ /* 4 */
		4,
		{
			{ TLARGE, true, 10, 14,10, false },
			{ TLARGE, true, 10, 44,10, false },
			{ TLARGE, true, 10, 28,84, true },
			{ TLARGE, true, 10, 58,84, true },
		}
	},
	{ /* 3 */
		3,
		{
			{ TLARGE, true, 10, 29,10, false },
			{ TLARGE, true, 10, 29,40, false },
			{ TLARGE, true, 10, 43,84, true },
		}
	},
	{ /* 2 */
		2,
		{
			{ TLARGE, true, 10, 29,10, false },
			{ TLARGE, true, 10, 43,84, true },
		}
	},
};



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCard::CCard()
{
	this->suit		= 4;
	this->value		= 14;
	this->face_down = true;
}

CCard::~CCard()
{

}

/*
	returns zerro if values are equal
	returns negative for this.value less than parameter.value,
	returns positive for this.value greater than parameter.value

	return value is the difference between values
*/

int	CCard::CompareValue( const CCard& _card )
{
	return this->value - _card.value;
}


bool CCard::IsValid() const
{
	if( 3 >= this->suit  )
		if( 12>= this->value )
			return true;

	return false;
}



void CCard::Draw( unsigned int _x, unsigned int _y, bool draw_selected, unsigned char _width, unsigned char _height )
{
	char filename[255];

	if( false == this->IsValid() ) return;

	//Draw border
	DrawPNM( "border.pnm", _x,_y, false, _width, _height );

	if( true == face_down )
	{
		DrawPNM( "back.pnm", _x+4,_y+4, false, _width, _height );
		return;
	}

/* Draw Corners */

	strcpy( filename, suit<2?terms[TRED]:terms[TBLACK] );
	strcat( filename, "-" );
	strcat( filename, terms[ 9 + this->value ] );
	strcat( filename, ".pnm" );
	DrawPNM( filename, _x + 3, _y + 4, false );
	DrawPNM( filename, _x + 73 - 3, _y + 97- 4, true );

	strcpy( filename, "small-" );
	strcat( filename, suits[ suit ] );
	strcat( filename, ".pnm" );
	DrawPNM( filename, _x + 3, _y + 19, false );
	DrawPNM( filename, _x + 73 - 3, _y + 97- 19, true );


/* Draw middle part */
#define CARD_DEF cards[this->value].elements[i]

	unsigned char i = 0;

	for( i = 0; i < cards[this->value].num_of_elements; i++ )
	{
		strcpy( filename, terms[CARD_DEF.term] );
		strcat( filename, "-" );
		if( false == CARD_DEF.suit )
		{
			strcat( filename, terms[ 9 + this->value ] );
		}

		strcat( filename, suits[ suit ] );

		strcat( filename, ".pnm" );

		DrawPNM( filename, _x + CARD_DEF.dx,_y + CARD_DEF.dy, CARD_DEF.invert );
	}

}

int CCard::DrawPNM(
			const char* _file_name,
			int _x, int _y,
			bool invert, /* flip vertical */
			unsigned char _width, unsigned char _height,
			bool pbp /* pixel by pixel */   )
{
	IMAGE* 	ci = read_image( _file_name );

	if( ci == NULL ) return 0;

	int 		current_row = 0;
	int		drow = ci->width;
	int		mod = invert?-1:1;
	unsigned char	transparent = Convert_24_8( 0x20, 0xFF, 0xFF );

	for( int i = 0; i < ci->height; i++, current_row += drow )
	{
		if( pbp )
		{
			for( int ipx = 0; ipx < ci->width; ipx++ )
			{
				if( transparent != ci->raw_data[current_row + ipx] )
					BBPutPixel( ci->raw_data[current_row + ipx], _x + ipx * mod, _y + i * mod );
			}

		}
		else
		{
			if( !invert )
			{
				memcpy( &BBGetData()[(_y + i ) * WIDTH + _x], ci->raw_data + current_row, ci->width );
			}
			else
			{
				for( int ipx = 0; ipx < ci->width; ipx++ )
				{
					BBPutPixel( ci->raw_data[current_row + ipx], _x + ipx * mod, _y + i * mod );
				}
			}
		}
	}

	return 0;
}

bool CCard::Flip()
{
	return this->face_down = !this->face_down;
}

bool CCard::IsFaceDown() const
{
	return this->face_down;
}
