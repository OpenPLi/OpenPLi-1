/*******************************************************
*
*  Author: the_moon
*  Software: Solitair for D-Box
*  Created in 2003
*
********************************************************/

#include "Slot.h"
#include <time.h>
#include "stdlib.h"
#include <stdio.h>

CSlot::CSlot()
{
	this->cards_stored		= 0;
	this->cards_selected	= 0;
	this->cardstack			= NULL;
}

CSlot::~CSlot()
{
	if( cardstack )
	{
		free(cardstack);
		cardstack = NULL;
	}

}


/************************************************************
*
* Function : PushCard
*
* Description: Stored a card in a array
*
*
*************************************************************/
bool CSlot::PushCard( const CCard& _card, bool no_check )
{


	if( false == _card.IsValid() ) return false;



	if( false == no_check )
		if( false == this->CheckPushPrecondition( _card ) )
			return false;



	if( 0 == cards_stored )
	{

		cardstack = (CCard*)malloc( sizeof( CCard) );
	}
	else
	{

		cardstack = (CCard*)realloc( cardstack, sizeof( CCard ) * ( cards_stored + 1 ) );
	}

	cardstack[cards_stored++] = _card;

	return true;

}

/************************************************************
*
* Function : PopCard
*
* Description: removes a card from the array
*              if the slot is empty the return value a card
*              with illegal values of suit and value
*
*
*************************************************************/
CCard CSlot::PopCard()
{
	CCard tmp;



	if( 0 == cards_stored ) return CCard();


	if( cards_selected )
		this->cards_selected--;


	this->cards_stored--;


	tmp = this->cardstack[ cards_stored ];

	if( cards_stored )
	{

		cardstack = (CCard*)realloc( cardstack, sizeof( CCard ) * ( cards_stored ) );
	}
	else
	{

		free( cardstack );
		cardstack = NULL;
	}

	return tmp;
}


/************************************************************
*
* Function : Fill
*
* Description: fills a slot with a full deck
*
*
*************************************************************/
void CSlot::Fill()
{
	unsigned char suit = 0;
	unsigned char value = 0;

	unsigned char i;

	for( i = 0; i < 52; i++ )
	{
		if( 13 == value )
		{
			value = 0;
			suit ++;
		}

		this->PushCard( CCard( suit, value++ ), true );
	}
}

void CSlot::Shuffle()
{
	unsigned char i;


	srand((unsigned)time(NULL));

	for( i = 0; i < cards_stored; i++ )
	{
		unsigned char pair = rand()%cards_stored;

		CCard tmp		= cardstack[pair];
		cardstack[pair]	= cardstack[i];
		cardstack[i]	= tmp;
	}
}


/************************************************************
*
* Function : RemoveAll
*
* Description: Cleans up the slot, frees memory allocated
*
*
*************************************************************/
void CSlot::RemoveAll()
{




	this->cards_stored	= 0;
	if( cardstack )
	{

		free( cardstack );
		cardstack = NULL;
	}

	cards_selected = 0;

}

/************************************************************
*
* Function : PeekCard
*
* Description: Retrieves a card on top of the stack whithout
*              to remove it from the stack
*
*
*************************************************************/
CCard CSlot::PeekCard()
{
	if( 0 == cards_stored )
		return CCard();

	return this->cardstack[ cards_stored - 1 ];
}

/************************************************************
*
* Function : Frcrease selection
*
* Description: Decreases the count of selected cards
*
*
*************************************************************/
bool CSlot::DecreaseSelection()
{
	if( 0 == cards_selected )
		return false;

	cards_selected--;
	return true;
}

/************************************************************
*
* Function : GetSelectedCards pops all selected cards
*
* Description:
*
*
*************************************************************/
void CSlot::GetSelectedCards( CSlot *buffer )
{
	if( NULL == buffer || 0 == cards_stored ) return;

	buffer->RemoveAll();

	while( cards_stored && 0 != cards_selected )
	{
		CCard tmp = this->PopCard();
		buffer->PushCard( tmp );
	}
}

void CSlot::PeekAllCards( CSlot *buffer )
{

	if( NULL == buffer ) return;


	buffer->RemoveAll();

	int i = cards_stored-1;


	while( 0 <= i )
	{
		buffer->PushCard( cardstack[i--] );
	}
}
