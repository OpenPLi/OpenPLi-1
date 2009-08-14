// Tableau.cpp: implementation of the CTableau class.
//
//////////////////////////////////////////////////////////////////////


#include "Tableau.h"
#include "Buffer.h"
extern "C"
{
#include "draw.h"
}

#include "backbuffer.h"

CTableau::CTableau()
{

}

CTableau::~CTableau()
{

}

bool CTableau::CheckPushPrecondition(const CCard &_card)
{

	if( false == CSlot::CheckPushPrecondition( _card ) ) return false;


	/* 0 and 1 - red suit */
	/* 2 and 3 - black suit */

	/*
		0 - ace
		1 - king
		2 - queen
		3 - jack
		4 - 10
		5 - 9
		6 - 8
		7 - 7
		8 - 6
	*/

	//Get a card from the top
	if( this->cards_stored == 0 )
	{
		if( _card.IsFaceDown() )
			return true;

		if( 1 == _card.GetValue() ) /* king */
			return true;
		else
			return false;
	}
	else
	{
		CCard tmp = this->PeekCard();

		if
			(	false == _card.IsFaceDown() &&
				-1 == tmp.CompareValue( _card ) &&
				(
					( tmp.GetSuit() < 2 && _card.GetSuit() > 1 ) ||
					( tmp.GetSuit() > 1 && _card.GetSuit() < 2 )
				)
			)
			return true;
		else
			return false;
	}
}

void CTableau::OpenCard()
{
	if( this->cardstack[ cards_stored-1 ].IsFaceDown() )
		this->cardstack[ cards_stored-1 ].Flip();
}

void CTableau::Draw( unsigned int _x, unsigned int _y, bool selected )
{

	unsigned char i = 0;
	unsigned char closed = 0;

	BBFillRect( _x -2, _y, 80, 350, BACK_C );

	for( i = 0; i < cards_stored; i++ )
	{
		cardstack[i].Draw( _x , _y + (5 * closed ) + (20 * (i - closed )), false, 0,cardstack[i].IsFaceDown()?5:0  );
		if( cardstack[i].IsFaceDown() ) closed ++;
	}

	i--;

	if( cards_stored && cards_stored == closed-- )
	{
		cardstack[i].Draw( _x , _y + (5 * closed ) + (20 * (i - closed )), false, 0,0  );
	}

	if( !cards_stored )
	{
		CCard::DrawPNM( "fill.pnm", _x, _y );
	}

	/* draw hand */

	if( selected )
	{

		closed = 0;
		for( i = 0; cards_stored && i < cards_stored - cards_selected + 1 ; i++ )
		{
			if( cardstack[i].IsFaceDown() ) closed ++;
		}

		if( cards_stored )
			CCard::DrawPNM( "pointer.ppm",
				_x + 20,
				_y + ( 5 * closed ) + ( 20 * ( i - 1 - closed ) ),
				false, 0, 0, true );
		else
			CCard::DrawPNM( "pointer.ppm", _x + 20, _y + 2, false, 0, 0, true );


	}
//	else
//		CCard::DrawPNM( "pointer.ppm", _x + 20, _y + 2, false, 0, 0, true );

}

bool CTableau::IncreaseSelection()
{
	if( cards_stored == 0 || cards_selected == cards_stored ) return false;

	if( false == cardstack[ cards_stored - cards_selected - 1].IsFaceDown() )
	{
		cards_selected++;
		return true;
	}

	return false;

}

bool CTableau::DecreaseSelection()
{
	if( cards_stored == 0 )
	{
		cards_selected = 0;
		return false;
	}

	if( cardstack[ cards_stored - 1 ].IsFaceDown() )
	{
		cards_selected = 0;
		return false;
	}

	if( 1 == cards_selected )
		return false;

	cards_selected--;
	return true;
}


//End of file


