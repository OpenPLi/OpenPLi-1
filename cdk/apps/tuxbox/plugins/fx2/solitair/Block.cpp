// Block.cpp: implementation of the CBlock class.
//
//////////////////////////////////////////////////////////////////////

#include "Block.h"
extern "C"
{
#include "draw.h"
}

#include "backbuffer.h"
#include "Card.h"

extern int ShowCards;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBlock::CBlock()
{
	this->Init();
}

CBlock::~CBlock()
{

}

bool CBlock::CheckPushPrecondition(const CCard &_card)
{
	return true;
}

void CBlock::Init()
{
	RemoveAll();
	this->Fill();
	this->Shuffle();
}

void CBlock::Draw(unsigned int _x, unsigned int _y, bool selected )
{
	BBFillRect( _x - 15, _y-15, 90, 120, BACK_C );

	for( unsigned char i = 0; this->GetCardsStored() && i <= this->GetCardsStored() / 4; i++ )
	{
		cardstack[0].Draw( _x - (i*2), _y - (i*2));
	}

	if( !cards_stored )
	{
		CCard::DrawPNM( "fill.pnm", _x, _y );
	}

	/* draw hand */
	if( selected )
	{
		CCard::DrawPNM( "pointer.ppm", _x+20, _y+10, false, 0, 0, true );
	}

}

bool CBlock::IncreaseSelection()
{
	if( cards_stored && cards_selected < ShowCards )
	{
		cards_selected++;
		return true;
	}
	return false;
}
