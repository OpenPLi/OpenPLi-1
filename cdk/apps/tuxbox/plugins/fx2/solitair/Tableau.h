// Tableau.h: interface for the CTableau class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __TABLEAU_H__
#define __TABLEAU_H__

#include "Slot.h"

class CTableau : public CSlot
{
public:
	bool DecreaseSelection() ;
	bool IncreaseSelection();
	void Draw( unsigned int _x, unsigned int _y, bool selected = false );
	void OpenCard();
	bool CheckPushPrecondition( const CCard& _card );
	CTableau();
	virtual ~CTableau();

};

#endif // __TABLEAU_H__
