// Buffer.cpp: implementation of the CBuffer class.
//
//////////////////////////////////////////////////////////////////////

#include "Buffer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBuffer::CBuffer()
{

}

CBuffer::~CBuffer()
{

}

bool CBuffer::CheckPushPrecondition(const CCard &_card)
{
	return true;
}

void CBuffer::Draw(unsigned int _x, unsigned int _y, bool selected )
{
	/* empty */
}

bool CBuffer::IncreaseSelection()
{
return false;
}
