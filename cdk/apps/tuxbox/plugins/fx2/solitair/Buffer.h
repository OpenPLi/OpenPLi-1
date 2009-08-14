// Buffer.h: interface for the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include "Slot.h"

class CBuffer : public CSlot
{
public:
	bool IncreaseSelection();
	void Draw( unsigned int _x, unsigned int _y, bool selected  );
	bool CheckPushPrecondition( const CCard& _card );
	CBuffer();
	virtual ~CBuffer();

};

#endif // __BUFFER_H__
