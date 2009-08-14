// Foundation.cpp: implementation of the CFoundation class.
//
//////////////////////////////////////////////////////////////////////


#include "Foundation.h"
extern "C"
{
#include "draw.h"
}

#include "backbuffer.h"


CFoundation::CFoundation()
{

}

CFoundation::~CFoundation()
{

}

bool CFoundation::CheckPushPrecondition( const CCard &_card )
{
	
	if( false == CSlot::CheckPushPrecondition( _card ) ) return false;
	
	if( _card.IsFaceDown() ) return false;
	
	if( 0 == this->GetCardsStored() && 0 == _card.GetValue() /* ace */ )
		return true;
	else
	{
		/* Get Card from top and put it back */
		CCard tmp = this->PeekCard();
	
		if	( 
				tmp.GetSuit() == _card.GetSuit() &&
				( 
					( tmp.GetValue() == 0 && -12 == tmp.CompareValue( _card ) ) ||
					( tmp.GetValue() != 0 && 1 == tmp.CompareValue( _card ) )
				)
			)
			return true;
		else
			return false;
	}
}

void CFoundation::Draw(unsigned int _x, unsigned int _y, bool selected )
{
//	FBFillRect( _x -2, _y, 80, 100, 15 );
	
	BBFillRect( _x -2, _y, 80, 100, BACK_C );

	if( cards_stored )
	{

		for( unsigned char i = 0; i < cards_stored; i++ )
		{
			cardstack[ i ].Draw( _x, _y + ( i * 2 ) );
		
		}
	}
	else
	{
		CCard::DrawPNM( "fill.pnm", _x, _y );
	}

	/* draw hand */

	if( selected )
	{
		CCard::DrawPNM( "pointer.ppm", _x+20, _y+10, false, 0, 0, true );
	}
}

bool CFoundation::IncreaseSelection()
{
	if( cards_stored && cards_selected == 0 )
	{
		cards_selected = 1;
		return true;
	}
	return false;
}
