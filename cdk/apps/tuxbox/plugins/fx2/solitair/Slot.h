// Slot.h: interface for the CSlot class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __SLOT_H__
#define __SLOT_H__


#include "Card.h"

class CSlot
{
public:
	CSlot();
	virtual ~CSlot();

public:
	virtual void Draw( unsigned int _x, unsigned int _y, bool selected ) = 0;

	void			GetSelectedCards( CSlot* buffer );
	void			PeekAllCards( CSlot *buffer );
	virtual bool		DecreaseSelection() ;
	virtual bool		IncreaseSelection()= 0;

	CCard			PeekCard();
	void			RemoveAll();
		void			Shuffle();
	void			Fill();
	CCard			PopCard();
	bool			PushCard( const CCard& _card, bool no_check = false );

	unsigned char		GetCardsStored() { return this->cards_stored; }


protected:

	virtual bool	CheckPushPrecondition( const CCard& _card )
	{ return _card.IsValid(); };

	unsigned char	cards_stored;
	unsigned char	cards_selected;
	CCard*		cardstack;

};

#endif // __SLOT_H__
