// Wastepile.cpp: implementation of the CWastepile class.
//
//////////////////////////////////////////////////////////////////////


#include "Wastepile.h"
extern "C"
{
#include "draw.h"
}

#include "backbuffer.h"

extern int ShowCards;

CWastepile::CWastepile()
{

}

CWastepile::~CWastepile()
{

}

bool CWastepile::CheckPushPrecondition(const CCard &_card)
{
	if( false == CSlot::CheckPushPrecondition( _card ) ) return false;

	if( _card.IsFaceDown() ) return false;

	return true;
}

void CWastepile::Draw(unsigned int _x, unsigned int _y, bool selected )
{
	unsigned char i = 0;

	BBFillRect( _x - 1, _y-1, 105, 100, BACK_C );

	if( this->cards_stored )
	{
		for( ; i < cards_stored ; i++ )
		{
			unsigned char modificator = ( (i%ShowCards)?(i%ShowCards):0 );
			cardstack[i].Draw( _x + ( 15 * modificator ), _y );
		}
	}
	else
	{
		CCard::DrawPNM( "fill.pnm", _x, _y );
	}

		/* draw hand */

	if( selected )
	{
		if( 0 == this->cards_stored )
			CCard::DrawPNM( "pointer.ppm", _x + 20, _y+10, false, 0, 0, true );
		else
			CCard::DrawPNM( "pointer.ppm", _x + ( 15 * ( ((i-1)%ShowCards)?((i-1)%ShowCards):0 ) ) + 20, _y+10, false, 0, 0, true );
	}
}

bool CWastepile::IncreaseSelection()
{
	if( cards_stored && 0 == cards_selected )
	{
		cards_selected = 1;
		return true;
	}
	else
		return false;
}


