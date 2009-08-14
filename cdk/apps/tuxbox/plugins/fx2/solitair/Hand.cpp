// Hand.cpp: implementation of the CHand class.
//
//////////////////////////////////////////////////////////////////////


#include "Hand.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern "C"
{
#include "draw.h"
}

#include "backbuffer.h"

CHand::CHand()
{

}

CHand::~CHand()
{

}

bool CHand::CheckPushPrecondition(const CCard &_card)
{
	return true;
}

void CHand::Draw(unsigned int _x, unsigned int _y, bool selected )
{
//	FBFillRect( _x , _y, 80, 300, 15 );

	BBFillRect( _x , _y, 80, 300, BACK_C );
	
	if( cards_stored )
	{

		for( unsigned char i = 0; i < cards_stored; i++ )
		{
			cardstack[ i ].Draw( _x, _y + ( i * 14 ) );
		}
	}
	else
	{
		CCard::DrawPNM( "fill.ppm", _x, _y );
	}

}

bool CHand::IncreaseSelection()
{
	if( cards_stored && cards_selected < cards_stored )
	{
		cards_selected++;
		return true;
	}

	return false;
}
