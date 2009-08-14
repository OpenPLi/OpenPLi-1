// Wastepile.h: interface for the CWastepile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __WASTEPILE_H__
#define __WASTEPILE_H__


#include "Slot.h"

class CWastepile : public CSlot
{
public:
	bool IncreaseSelection();
	void Draw( unsigned int _x, unsigned int _y, bool selected = false );
	CWastepile();
	virtual ~CWastepile();

private:
	bool CheckPushPrecondition( const CCard& _card );
};

#endif //__WASTEPILE_H__
