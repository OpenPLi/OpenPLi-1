// Foundation.h: interface for the CFoundation class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __FOUNDATION_H__
#define __FOUNDATION_H__

#include "Slot.h"

class CFoundation : public CSlot
{
public:
	bool IncreaseSelection();
	void Draw( unsigned _x, unsigned int _y, bool selected = false );
	bool CheckPushPrecondition( const CCard& _card );
	CFoundation();
	virtual ~CFoundation();

};

#endif // __FOUNDATION_H__
