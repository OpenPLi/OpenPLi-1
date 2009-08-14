// Block.h: interface for the CBlock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __BLOCK_H__
#define __BLOCK_H__

#include "Slot.h"

class CBlock : public CSlot
{
public:
	bool IncreaseSelection();
	void Draw( unsigned int _x, unsigned int _y, bool selected = false );
	void Init();
	CBlock();
	virtual ~CBlock();

private:
	bool CheckPushPrecondition( const CCard& _card );
};

#endif // __BLOCK_H__
