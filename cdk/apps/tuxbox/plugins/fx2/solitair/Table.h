// Table.h: interface for the CTable class.
//
//////////////////////////////////////////////////////////////////////

#ifndef __TABLE_H__
#define __TABLE_H__

#include "Block.h"
#include "Hand.h"
#include "Tableau.h"
#include "Foundation.h"
#include "Wastepile.h"


class CTable
{
public:
	int Run();
	CTable();
	virtual ~CTable();

private:

	void ShowHelp();
	void CheckWin( bool _add );
	void Wizard();
	void DoAction();
	void MoveCursor( unsigned char _direction );
	void Init();
	void Display();
	bool Setup();
	void Save();
	void Load();



	unsigned char act_slot;
	void HandleKeysPressed();

	CHand			hand;

	CBlock			block;

	CWastepile		wastepile;

	CFoundation		foundation[4];

	CTableau		tableau[7];

	bool			changed[15];

protected:
	void ChangeSelection( bool select = true, bool select_all = true );
};

#endif // __TABLE_H__
