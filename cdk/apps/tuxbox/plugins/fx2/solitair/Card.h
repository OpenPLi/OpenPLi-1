// Card.h: interface for the CCard class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __CARD_H__
#define __CARD_H__

#include "pnm_file.h"

#define BACK_C			(Convert_24_8( 0, 100, 70 ))

class CCard
{
public:
	void Draw(unsigned int _x, unsigned int _y, bool draw_selected = false, unsigned char _width = 0, unsigned char _height = 0  );
	bool IsValid() const;
	CCard();
	CCard( unsigned char _suit, unsigned char _value, bool _face_down = true )
	{
		this->SetValue( _value );
		this->SetSuit( _suit );
		this->face_down = _face_down;
	}

	virtual ~CCard();

	void SetSuit( unsigned char _suit ) { suit = _suit; }
	char GetSuit( ) const { return suit; }

	void SetValue( unsigned char _value ) { value = _value; }
	unsigned char GetValue( ) const { return value; }

	int	CompareValue( const CCard& _card );

private:

	unsigned char	suit;
	unsigned char	value;
	bool		face_down;

public:
	bool IsFaceDown() const;
	bool Flip();
	static int DrawPNM(
			const char* _file_name,
			int _x, int _y,
			bool invert = false, /* flip vertical */
			unsigned char _width = 0, unsigned char _height = 0,
			bool pbp = false /* pixel by pixel */   );

};

#endif // __CARD_H__
