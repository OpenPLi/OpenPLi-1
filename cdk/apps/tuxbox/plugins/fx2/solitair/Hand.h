// Hand.h: interface for the CHand class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __HAND_H__
#define __HAND_H__

#include "Slot.h"

class CHand : public CSlot
{
public:
	bool IncreaseSelection();
	void Draw( unsigned int _x, unsigned int _y, bool selected =false );
	bool CheckPushPrecondition( const CCard& _card );
	CHand();
	virtual ~CHand();

	CSlot*	GetSource() { return taken_from;};
	void	SetSource( CSlot* _taken_from ) { this->taken_from = _taken_from; }


private:

	CSlot*	taken_from;

};

#endif // __HAND_H__
