// Table.cpp: implementation of the CTable class.
//
//////////////////////////////////////////////////////////////////////


#include "Table.h"

extern "C"
{
#include "rcinput.h"
#include "draw.h"
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pnm_file.h"

#include "Buffer.h"
#include "backbuffer.h"
#include <sys/time.h>

#include <config.h>

extern	unsigned short	actcode; //from rcinput
extern	unsigned short	realcode; //from rcinput
extern	int				doexit;

#define T_ROWS		2
#define T_COLS		9

#define D_LEFT		0
#define D_UP		1
#define D_RIGHT		2
#define D_DOWN		3


#define BLOCK		1
#define WASTEPILE	2

#define HAND		3

#define FOUNDATION1	4
#define FOUNDATION2	5
#define FOUNDATION3	6
#define FOUNDATION4	7

#define TABLEAU1	8
#define TABLEAU2	9
#define TABLEAU3	10
#define TABLEAU4	11
#define TABLEAU5	12
#define TABLEAU6	13
#define TABLEAU7	14

static int 	win_counter 	= 0;
int 		ShowCards 	= 3;

CTable::CTable()
{
	this->act_slot = BLOCK;
	memset( changed, 0xFF, sizeof( changed ) );

	Load();

	BBCreate();
}

CTable::~CTable()
{
	Save();
	BBFree();
}

void CTable::Init()
{


	unsigned char i = 0;

	win_counter = 0;

	BBSetBackground( BACK_C );

	this->act_slot = BLOCK;
	memset( changed, 0xFF, sizeof( changed ) );


	block.RemoveAll();
	block.Fill();

	for( i = 0; i < 7; i++ )
		block.Shuffle();

	for( i = 0; i < 7; i++ )
		tableau[i].RemoveAll();

	for( i = 0; i < 4; i++ )
		foundation[i].RemoveAll();

	hand.RemoveAll();

	wastepile.RemoveAll();

	unsigned char	filled;

	for( filled = 1; filled < 7; filled++ )
	{
		for( i = filled; i < 7 ; i++ )
		{
			CCard tmp = block.PopCard();
			tableau[i].PushCard( tmp, true );
		}
	}

	for( i = 0; i < 7 ; i++ )
	{
		CCard tmp = block.PopCard();
		tmp.Flip();
		tableau[i].PushCard( tmp, true );
	}

	act_slot = BLOCK;

	ChangeSelection( true );

}

void CTable::MoveCursor(unsigned char _direction)
{
	this->ChangeSelection( false );

	switch( act_slot )
	{
	case BLOCK :
			switch( _direction )
			{
			case D_LEFT: act_slot = FOUNDATION4; break;
			case D_RIGHT: act_slot = WASTEPILE; break;
			case D_UP: act_slot = TABLEAU1; break;
			case D_DOWN: act_slot = TABLEAU1; break;
			} break;
	case WASTEPILE :
			switch( _direction )
			{
			case D_LEFT: act_slot = BLOCK; break;
			case D_RIGHT: act_slot = FOUNDATION1; break;
			case D_UP: act_slot = TABLEAU1; break;
			case D_DOWN: act_slot = TABLEAU1; break;
			} break;
	case FOUNDATION1 :
			switch( _direction )
			{
			case D_LEFT: act_slot = WASTEPILE; break;
			case D_RIGHT: act_slot = FOUNDATION2; break;
			case D_UP: act_slot = TABLEAU4; break;
			case D_DOWN: act_slot = TABLEAU4; break;
			} break;
	case FOUNDATION2 :
			switch( _direction )
			{
			case D_LEFT: act_slot = FOUNDATION1; break;
			case D_RIGHT: act_slot = FOUNDATION3; break;
			case D_UP: act_slot = TABLEAU5; break;
			case D_DOWN: act_slot = TABLEAU5; break;
			} break;
	case FOUNDATION3 :
			switch( _direction )
			{
			case D_LEFT: act_slot = FOUNDATION2; break;
			case D_RIGHT: act_slot = FOUNDATION4; break;
			case D_UP: act_slot = TABLEAU6; break;
			case D_DOWN: act_slot = TABLEAU6; break;
			} break;
	case FOUNDATION4 :
			switch( _direction )
			{
			case D_LEFT: act_slot = FOUNDATION3; break;
			case D_RIGHT: act_slot = BLOCK; break;
			case D_UP: act_slot = TABLEAU7; break;
			case D_DOWN: act_slot = TABLEAU7; break;
			} break;
	case TABLEAU1 :
			switch( _direction )
			{
			case D_LEFT: act_slot = TABLEAU7; break;
			case D_RIGHT: act_slot = TABLEAU2; break;
			case D_UP: act_slot = WASTEPILE; break;
			case D_DOWN: act_slot = WASTEPILE; break;
			} break;
	case TABLEAU2 :
			switch( _direction )
			{
			case D_LEFT: act_slot = TABLEAU1; break;
			case D_RIGHT: act_slot = TABLEAU3; break;
			case D_UP: act_slot = WASTEPILE; break;
			case D_DOWN: act_slot = WASTEPILE; break;
			} break;
	case TABLEAU3 :
			switch( _direction )
			{
			case D_LEFT: act_slot = TABLEAU2; break;
			case D_RIGHT: act_slot = TABLEAU4; break;
			case D_UP: act_slot = FOUNDATION1; break;
			case D_DOWN: act_slot = FOUNDATION1; break;
			}  break;
	case TABLEAU4 :
			switch( _direction )
			{
			case D_LEFT: act_slot = TABLEAU3; break;
			case D_RIGHT: act_slot = TABLEAU5; break;
			case D_UP: act_slot = FOUNDATION1; break;
			case D_DOWN: act_slot = FOUNDATION1; break;
			}  break;
	case TABLEAU5 :
			switch( _direction )
			{
			case D_LEFT: act_slot = TABLEAU4; break;
			case D_RIGHT: act_slot = TABLEAU6; break;
			case D_UP: act_slot = FOUNDATION2; break;
			case D_DOWN: act_slot = FOUNDATION2; break;
			}  break;
	case TABLEAU6 :
			switch( _direction )
			{
			case D_LEFT: act_slot = TABLEAU5; break;
			case D_RIGHT: act_slot = TABLEAU7; break;
			case D_UP: act_slot = FOUNDATION3; break;
			case D_DOWN: act_slot = FOUNDATION3; break;
			}  break;
	case TABLEAU7 :
			switch( _direction )
			{
			case D_LEFT: act_slot = TABLEAU6; break;
			case D_RIGHT: act_slot = TABLEAU1; break;
			case D_UP: act_slot = FOUNDATION4; break;
			case D_DOWN: act_slot = FOUNDATION4; break;
			}  break;
	}


	this->ChangeSelection( true );
}

void CTable::Save()
{
	FILE* sav = fopen( GAMESDIR "/solitair.sav", "wb" );

	if( sav == NULL ) return;

	fwrite( &ShowCards, 1, sizeof( ShowCards ), sav );

	fwrite( &win_counter, 1, sizeof( win_counter ), sav );

	//Save BLOCK
	unsigned char CardsStored = block.GetCardsStored();

	fwrite( &CardsStored, 1, sizeof( CardsStored ), sav );

	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		CCard card = block.PopCard();
		fwrite( &card, 1, sizeof( card ), sav );
	}

	//save WASTEPILE
	CardsStored = wastepile.GetCardsStored();

	fwrite( &CardsStored, 1, sizeof( CardsStored ), sav );

	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		CCard card = wastepile.PopCard();
		fwrite( &card, 1, sizeof( card ), sav );
	}

	//Save HAND
	int HSource = 0;

	if( &block == hand.GetSource() )
	{
		HSource = BLOCK;
	}
	else
	if( &wastepile == hand.GetSource() )
	{
		HSource = WASTEPILE;
	}
	else
	if( &foundation[0] == hand.GetSource() )
	{
		HSource = FOUNDATION1;
	}
	else
	if( &foundation[1] == hand.GetSource() )
	{
		HSource = FOUNDATION2;
	}
	else
	if( &foundation[2] == hand.GetSource() )
	{
		HSource = FOUNDATION3;
	}
	else
	if( &foundation[3] == hand.GetSource() )
	{
		HSource = FOUNDATION4;
	}
	else
	if( &tableau[0] == hand.GetSource() )
	{
		HSource = TABLEAU1;
	}
	else
	if( &tableau[1] == hand.GetSource() )
	{
		HSource = TABLEAU2;
	}
	else
	if( &tableau[2] == hand.GetSource() )
	{
		HSource = TABLEAU3;
	}
	else
	if( &tableau[3] == hand.GetSource() )
	{
		HSource = TABLEAU4;
	}
	else
	if( &tableau[4] == hand.GetSource() )
	{
		HSource = TABLEAU5;
	}
	else
	if( &tableau[5] == hand.GetSource() )
	{
		HSource = TABLEAU6;
	}
	else
	if( &tableau[6] == hand.GetSource() )
	{
		HSource = TABLEAU7;
	}

	fwrite( &HSource, 1, sizeof( HSource ), sav );

	CardsStored = hand.GetCardsStored();

	fwrite( &CardsStored, 1, sizeof( CardsStored ), sav );

	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		CCard card = hand.PopCard();
		fwrite( &card, 1, sizeof( card ), sav );
	}

	//Save Tableaus
	for( int j = 0; j < 7; j++ )
	{
		CardsStored = tableau[j].GetCardsStored();

		fwrite( &CardsStored, 1, sizeof( CardsStored ), sav );

		for( unsigned char i = 0; i < CardsStored; i++ )
		{
			CCard card = tableau[j].PopCard();
			fwrite( &card, 1, sizeof( card ), sav );
		}
	}

	//Save Foundations
	for( int j = 0; j < 4; j++ )
	{
		CardsStored = foundation[j].GetCardsStored();

		fwrite( &CardsStored, 1, sizeof( CardsStored ), sav );

		for( unsigned char i = 0; i < CardsStored; i++ )
		{
			CCard card = foundation[j].PopCard();
			fwrite( &card, 1, sizeof( card ), sav );
		}
	}

	fclose( sav );

}

void CTable::Load()
{
	int SavShowCards = 0;

	FILE* sav = fopen( GAMESDIR "/solitair.sav", "rb" );

	if( sav == NULL ) return;

	fread( &SavShowCards, 1, sizeof( SavShowCards ), sav );

	if( ( SavShowCards != 3 ) && ( SavShowCards != 1 ) )
	{
		return;
	}

	ShowCards = SavShowCards;

	fread( &win_counter, 1, sizeof( win_counter ), sav );

	CCard card;
	unsigned char CardsStored;
	CBuffer buffer;


//Restore BLOCK

	block.RemoveAll();

	fread( &CardsStored, 1, sizeof( CardsStored ), sav );

	buffer.RemoveAll();
	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		fread( &card, 1, sizeof( card ), sav );
		buffer.PushCard( card, true );
	}

	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		block.PushCard( buffer.PopCard(), true );
	}

//Restore Wastepile
	fread( &CardsStored, 1, sizeof( CardsStored ), sav );

	buffer.RemoveAll();
	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		fread( &card, 1, sizeof( card ), sav );
		buffer.PushCard( card, true );
	}

	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		wastepile.PushCard( buffer.PopCard(), true );
	}
//Restore HAND
	int HSource;
	fread( &HSource, 1, sizeof( HSource ), sav );

	switch( HSource )
	{
		case 0:
			hand.SetSource(NULL);
			break;
		case BLOCK:
			hand.SetSource(&block);
			break;
		case WASTEPILE:
			hand.SetSource(&wastepile);
			break;
		case FOUNDATION1:
			hand.SetSource(&foundation[0]);
			break;
		case FOUNDATION2:
			hand.SetSource(&foundation[1]);
			break;
		case FOUNDATION3:
			hand.SetSource(&foundation[2]);
			break;
		case FOUNDATION4:
			hand.SetSource(&foundation[3]);
			break;
		case TABLEAU1:
			hand.SetSource(&tableau[0]);
			break;
		case TABLEAU2:
			hand.SetSource(&tableau[1]);
			break;
		case TABLEAU3:
			hand.SetSource(&tableau[2]);
			break;
		case TABLEAU4:
			hand.SetSource(&tableau[3]);
			break;
		case TABLEAU5:
			hand.SetSource(&tableau[4]);
			break;
		case TABLEAU6:
			hand.SetSource(&tableau[5]);
			break;
		case TABLEAU7:
			hand.SetSource(&tableau[6]);
			break;
	}


	fread( &CardsStored, 1, sizeof( CardsStored ), sav );

	buffer.RemoveAll();
	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		fread( &card, 1, sizeof( card ), sav );
		buffer.PushCard( card, true );
	}

	for( unsigned char i = 0; i < CardsStored; i++ )
	{
		hand.PushCard( buffer.PopCard(), true );
	}

//Restore Tableaus
	for( int j = 0; j < 7; j++ )
	{
		fread( &CardsStored, 1, sizeof( CardsStored ), sav );

		buffer.RemoveAll();
		for( unsigned char i = 0; i < CardsStored; i++ )
		{
			fread( &card, 1, sizeof( card ), sav );
			buffer.PushCard( card, true );
		}

		for( unsigned char i = 0; i < CardsStored; i++ )
		{
			tableau[j].PushCard( buffer.PopCard(), true );
		}
	}
//Restore Foundations
	for( int j = 0; j < 4; j++ )
	{
		fread( &CardsStored, 1, sizeof( CardsStored ), sav );

		buffer.RemoveAll();
		for( unsigned char i = 0; i < CardsStored; i++ )
		{
			fread( &card, 1, sizeof( card ), sav );
			buffer.PushCard( card, true );
		}

		for( unsigned char i = 0; i < CardsStored; i++ )
		{
			foundation[j].PushCard( buffer.PopCard(), true );
		}
	}


	fclose( sav );

}


int CTable::Run()
{

	BBSetBackground( BACK_C );

	FBCopyImage( 0, 0, WIDTH, HEIGHT, BBGetData() );

	FBDrawString( 300, 255, 42, "Loading...", 250, 0 );

	//Outer loop for initialization
	while( 0 == doexit )
	{

//		this->Init();

		this->Display();

		//Inner loop for game controlling
		while( 0 == doexit )
		{
			//Handle keys
			this->HandleKeysPressed();

			if( 0 != doexit ) break;

			//Display changes
		}
	}

	return 0;
}

void CTable::ShowHelp()
{

	const int Hx = 100;
	const int Hy = 100;
	const int Hwidth = 520;
	const int Hheight = 376;

	FBFillRect( Hx+8, Hy+8, Hwidth, Hheight, Convert_24_8( 0,0,0) );
	FBFillRect( Hx, Hy, Hwidth, Hheight, 19 );

	FBDrawHLine( Hx + 5, Hy + 55, 400, Convert_24_8( 255,255,255) );
	FBDrawHLine( Hx + 5, Hy + 56, 400, Convert_24_8( 255,255,255) );

	FBDrawString( Hx + 5, Hy + 5, 48, "Solitair for D-BOX", Convert_24_8( 255,255,255), 0 );

	FBDrawString( Hx + 5, Hy + 60, 32, "TheMoon, 2003", Convert_24_8( 255,255,255), 0 );

	FBDrawString( Hx + 5, Hy + 102, 32, "Key mappings:", Convert_24_8( 255,255,255), 0 );

	FBDrawString( Hx + 5, Hy + 135, 32, "(Red)", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 105, Hy + 135, 32, "- Restart game", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 5, Hy + 168, 32, "(Blue)", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 105, Hy + 168, 32, "- Wizard", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 5, Hy + 201, 32, "(?)", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 105, Hy + 201, 32, "- Help screen", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 5, Hy + 234, 32, "(Home)", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 105, Hy + 234, 32, "- Exit", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 5, Hy + 267, 32, "(1 - 7)", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 105, Hy + 267, 32, "- Tableau", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 5, Hy + 300, 32, "(-) (+)", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 105, Hy + 300, 32, "- Change selection", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 5, Hy + 333, 32, "DBOX", Convert_24_8( 255,255,255), 0 );
	FBDrawString( Hx + 105, Hy + 333, 32, "- Setup menu", Convert_24_8( 255,255,255), 0 );



}

bool CTable::Setup()
{
	const int Hx = 100;
	const int Hy = 100;
	const int Hwidth = 520;
	const int Hheight = 376;

	bool SomeChanges = true;
	bool FirstRun = true;

	int loiShowCards = ShowCards;

	while( 1 )
	{
		timeval	tv;

		tv.tv_sec = 0;
		tv.tv_usec = 60000;
		select( 0, 0, 0, 0, &tv );		/* 60ms pause */

		RcGetActCode();

		if( false == FirstRun && realcode == 0xee ) continue;

		switch( actcode )
		{
		case RC_OK :
			ShowCards = loiShowCards;
			return true;
			break;
		case RC_RIGHT :
		case RC_LEFT :
			if( 3 == loiShowCards )
				loiShowCards = 1;
			else
				loiShowCards = 3;

			SomeChanges = true;
			break;
		case RC_SETUP :
			if( FirstRun ) break;
			RcGetActCode();
			return false;
			break;
		}

		if( SomeChanges )
		{
			unsigned char FC = Convert_24_8( 0,0,128);
			unsigned char BC = Convert_24_8( 255,255,0);

			if( FirstRun )
			{
				FBFillRect( Hx+8, Hy+8, Hwidth, Hheight, Convert_24_8( 0,0,0) );
				FBFillRect( Hx, Hy, Hwidth, Hheight, 19 );
			}

			FBDrawHLine( Hx + 5, Hy + 55, 400, Convert_24_8( 255,255,255) );
			FBDrawHLine( Hx + 5, Hy + 56, 400, Convert_24_8( 255,255,255) );

			FBDrawString( Hx + 5, Hy + 5, 48, "Setup", Convert_24_8( 255,255,255), 0 );

			FBDrawString( Hx + 5, Hy + 102, 32, "Open cards:", Convert_24_8( 255,255,255), 0 );


			FBFillRect( Hx + 150, Hy + 102, 100, 32, loiShowCards==3?BC:19 );
			FBFillRect( Hx + 260, Hy + 102, 100, 32, loiShowCards==1?BC:19 );

			FBDrawString( Hx + 160, Hy + 102, 32, "3 Cards",
				loiShowCards==3?FC:Convert_24_8( 255,255,255), 0 );
			FBDrawString( Hx + 270, Hy + 102, 32, "1 Card",
				loiShowCards==1?FC:Convert_24_8( 255,255,255), 0 );

			FBDrawString( Hx + 5, Hy + 333, 24, "(OK) - Accept changes, (DBOX) - Cancel", Convert_24_8( 255,255,255), 0 );

		}

		SomeChanges = false;
		FirstRun = false;
	}
	return false;
}

void CTable::HandleKeysPressed()
{
	static bool	HelpOnScreen = false;
	static bool	MenuOnScreen = false;

	timeval	tv;

	tv.tv_sec = 0;
	tv.tv_usec = 60000;
	select( 0, 0, 0, 0, &tv );		/* 60ms pause */

	RcGetActCode();

	if( realcode == 0xee ) return;

	if( true == HelpOnScreen )
	{
		FBCopyImage( 0, 0, WIDTH, HEIGHT, BBGetData() );
		HelpOnScreen = false;
		return;
	}

	switch( actcode )
	{
		case RC_0 :

			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ BLOCK ] = true;
			act_slot = BLOCK;
			ChangeSelection( );


			break;
		case RC_1 :

			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ TABLEAU1 ] = true;
			act_slot = TABLEAU1;
			ChangeSelection( true, false );

			break;
		case RC_2 :
			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ TABLEAU2 ] = true;
			act_slot = TABLEAU2;
			ChangeSelection( true, false );

			break;
		case RC_3 :
			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ TABLEAU3 ] = true;
			act_slot = TABLEAU3;
			ChangeSelection( true, false );

			break;
		case RC_4 :
			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ TABLEAU4 ] = true;
			act_slot = TABLEAU4;
			ChangeSelection( true, false );

			break;
		case RC_5 :
			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ TABLEAU5 ] = true;
			act_slot = TABLEAU5;
			ChangeSelection( true, false );

			break;
		case RC_6 :
			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ TABLEAU6 ] = true;
			act_slot = TABLEAU6;
			ChangeSelection( true, false );

			break;
		case RC_7 :
			ChangeSelection( false );
			changed[ act_slot ] = true;
			changed[ TABLEAU7 ] = true;
			act_slot = TABLEAU7;
			ChangeSelection( true, false );

			break;
		case RC_8 :

			break;
		case RC_9 :

			break;
		case RC_RIGHT :
			this->MoveCursor( 2 );
			break;
		case RC_LEFT :
			this->MoveCursor( 0 );
			break;
		case RC_UP :
			this->MoveCursor( 1 );
			break;
		case RC_DOWN :
			this->MoveCursor( 3 );
			break;
		case RC_OK :
			this->DoAction();
			break;
		case RC_SPKR :
			break;

		case RC_STANDBY :

			break;
		case RC_GREEN :

			break;
		case RC_YELLOW :
//			FBPrintScreen();
			break;
		case RC_RED	 :
			this->Init();
			break;
		case RC_BLUE :
			Wizard();
			break;
		case RC_PLUS :
			changed[ act_slot ] = true;
			ChangeSelection( true, false );
			break;
		case RC_MINUS :
			changed[ act_slot ] = true;
			ChangeSelection( false, false );
			break;
		case RC_HELP :

			this->ShowHelp();
			HelpOnScreen = true;

			break;
		case RC_SETUP :
			if( this->Setup() )
				this->Init();
			FBCopyImage( 0, 0, WIDTH, HEIGHT, BBGetData() );
			break;
		case RC_HOME :


			break;
		case RC_PAGE_DOWN :

			break;
		case RC_PAGE_UP :

			break;

		default:
//
			return;

			break;
	}

this->Display();

actcode = 0;

}


#define TOP	50
#define LEFT	50

void CTable::Display()
{

	bool some_changes = false;

	if( changed[ BLOCK ] )
	{
//
		some_changes = true;
		block.Draw( LEFT + 15, TOP, BLOCK == act_slot );
	}

	if( changed[ WASTEPILE ] )
	{
		wastepile.Draw( LEFT + 95, TOP, WASTEPILE == act_slot );
		some_changes = true;
	}

	int i;

	for( i = 0; i < 4; i++ )
	{
		if( changed[ FOUNDATION1 + i ] )
		{
			foundation[i].Draw( LEFT+(4+i)*80, TOP, (FOUNDATION1 + i) == act_slot );
			some_changes = true;
		}
	}

	if( changed[ HAND ] )
	{
		hand.Draw( LEFT, TOP + 150 );
		some_changes = true;
	}

	for( i = 0; i < 7; i++ )
	{
		if( changed[ TABLEAU1 + i ] )
		{
			tableau[i].Draw( LEFT+(1+i)*80, TOP + 150, (TABLEAU1 + i) == act_slot );
			some_changes = true;
		}
	}

	memset( changed, 0, sizeof( changed ) );

	if( some_changes )
		FBCopyImage( 0, 0, WIDTH, HEIGHT, BBGetData() );
}

//automaticly select all possible cards or deselect all cards
void CTable::ChangeSelection(bool select, bool select_all )
{

	CSlot* sel_slot = NULL;

	changed[ act_slot ] = true;

	switch( act_slot )
	{
	case BLOCK :		sel_slot = &block;		break;
	case WASTEPILE :	sel_slot = &wastepile;		break;
	case FOUNDATION1 :
	case FOUNDATION2 :
	case FOUNDATION3 :
	case FOUNDATION4 :  sel_slot = &foundation[act_slot - FOUNDATION1];	break;
	case TABLEAU1 :
	case TABLEAU2 :
	case TABLEAU3 :
	case TABLEAU4 :
	case TABLEAU5 :
	case TABLEAU6 :
	case TABLEAU7 :		sel_slot = &tableau[act_slot - TABLEAU1]; break;
	}

	if( sel_slot )
	{
		if( select_all )
		{
			if( select )
				while( sel_slot->IncreaseSelection() );
			else
				while( sel_slot->DecreaseSelection() );
		}
		else
		{
			if( select )
				sel_slot->IncreaseSelection();
			else
				sel_slot->DecreaseSelection();
		}
	}
}

void CTable::DoAction()
{

	CBuffer buffer;
	CCard	tmp;

	CSlot* fnd = NULL;
	CSlot* tab = NULL;

	switch( act_slot )
	{
	case BLOCK :
		/* move 3 cards to wastepile */

		//check IncreaseSelection and DecreaseSelection
		//regarding increaseSelection (all) or (+1)

		if( 0 != hand.GetCardsStored() ) return;

		buffer.RemoveAll();

		if( block.GetCardsStored() != 0 )
		{
			tmp = block.PopCard();
			tmp.Flip();
			wastepile.PushCard( tmp );

			if( 3 == ShowCards )
			{
				tmp = block.PopCard();
				tmp.Flip();
				wastepile.PushCard( tmp );

				tmp = block.PopCard();
				tmp.Flip();
				wastepile.PushCard( tmp );
			}

			act_slot = WASTEPILE;

		}
		else
		{
			if( wastepile.GetCardsStored() )
			{
				tmp = wastepile.PopCard();

				while( tmp.IsValid() )
				{
					tmp.Flip();
					block.PushCard( tmp, true );
					tmp = wastepile.PopCard();
				}

			}
		}
		changed[ WASTEPILE ] = true;
		changed[ BLOCK ] = true;

		ChangeSelection( );

		return;

	case WASTEPILE :
		/* put one card to hand */
		/* or put back from the hand */

		if( hand.GetCardsStored() )
		{
			if( hand.GetSource() == &wastepile )
			{
				tmp = hand.PopCard();
				wastepile.PushCard( tmp, true );

				hand.SetSource( NULL );
			}

		}
		else
		{
			tmp = wastepile.PopCard();
			if( false == tmp.IsValid() ) break;

			if( false == hand.PushCard( tmp ) )
			{
				wastepile.PushCard( tmp, true );
			}

			hand.SetSource( &wastepile );
		}

		changed[ WASTEPILE ] = true;
		changed[ HAND ] = true;

		return;

	case FOUNDATION1 :
	case FOUNDATION2 :
	case FOUNDATION3 :
	case FOUNDATION4 :  fnd = &foundation[act_slot - FOUNDATION1];	break;
	case TABLEAU1 :
	case TABLEAU2 :
	case TABLEAU3 :
	case TABLEAU4 :
	case TABLEAU5 :
	case TABLEAU6 :
	case TABLEAU7 :		tab = &tableau[act_slot - TABLEAU1]; break;
	}

	if( fnd )
	{
		if( 0 == hand.GetCardsStored() )
		{

			tmp = fnd->PopCard();

			if( false == tmp.IsValid() ) return;

			hand.PushCard( tmp );

			hand.SetSource( fnd );
			changed[ act_slot ] = true;
			changed[ HAND ] = true;

			CheckWin( false );

			ChangeSelection( );

			return;
		}
		else
		{
			if( fnd == hand.GetSource() )
			{
				tmp = hand.PopCard();

				fnd->PushCard( tmp, true );

				changed[ act_slot ] = true;
				changed[ HAND ] = true;

				CheckWin( true );

				ChangeSelection( );

				return;
			}
			else
			{
				tmp = hand.PeekCard();


				if( false == fnd->PushCard( tmp ) ) return;

				hand.RemoveAll();

				changed[HAND] = true;
				changed[act_slot] = true;

				CheckWin( true );


				ChangeSelection();

				return;
			}
		}

	}

	if( tab )
	{
		if( 0 == hand.GetCardsStored() )
		{
			tmp = tab->PeekCard();

			if( tmp.IsValid() && tmp.IsFaceDown() )
			{
				tmp = tab->PopCard();
				tmp.Flip();
				tab->PushCard( tmp, true );
				changed[ act_slot ] = true;
				ChangeSelection();
				return;
			}

			tab->GetSelectedCards( &buffer );

			tmp = buffer.PopCard();

			while( tmp.IsValid() )
			{
				hand.PushCard( tmp );
				tmp = buffer.PopCard();
			}

			hand.SetSource( tab );
			changed[ act_slot ] = true;
			changed[ HAND ] = true;

			ChangeSelection( );

			return;
		}
		else
		{

			if( tab == hand.GetSource() )
			{


				hand.PeekAllCards( &buffer );

				hand.RemoveAll();


				tmp = buffer.PopCard();

				while( tmp.IsValid() )
				{

					tab->PushCard( tmp, true );

					tmp = buffer.PopCard();
				}
				changed[ act_slot ] = true;
				changed[ HAND ] = true;


				ChangeSelection( );


				return;
			}
			else
			{


				hand.PeekAllCards( &buffer );


				tmp = buffer.PopCard();


				if( false == tab->PushCard( tmp ) ) return;


				tmp = buffer.PopCard();

				while( tmp.IsValid() )
				{

					tab->PushCard( tmp );

					tmp = buffer.PopCard();
				}


				hand.RemoveAll();

				changed[HAND] = true;
				changed[act_slot] = true;

				ChangeSelection();

				return;
			}
		}
	}

}

/* takes top card from the slot and movies it (if possible) to the foundation */
void CTable::Wizard()
{

	CBuffer buffer;
	CCard	tmp;

	CSlot* slot = NULL;

	if( hand.GetCardsStored() ) return;


	unsigned char prev_slot = act_slot;

	switch( act_slot )
	{
	case BLOCK : this->DoAction(); break;
	case WASTEPILE : slot = &wastepile;	break;
	case FOUNDATION1 :  slot = &foundation[0]; break;
	case FOUNDATION2 :  slot = &foundation[1]; break;
	case FOUNDATION3 :  slot = &foundation[2]; break;
	case FOUNDATION4 :  slot = &foundation[3]; break;
	case TABLEAU1 :	slot = &tableau[0]; break;
	case TABLEAU2 :	slot = &tableau[1]; break;
	case TABLEAU3 :	slot = &tableau[2]; break;
	case TABLEAU4 :	slot = &tableau[3]; break;
	case TABLEAU5 :	slot = &tableau[4]; break;
	case TABLEAU6 :	slot = &tableau[5]; break;
	case TABLEAU7 :	slot = &tableau[6]; break;
	}

	if( slot )
	{
		tmp = slot->PeekCard();

		if( false == tmp.IsValid() ) return;

		for( unsigned char i = 0; i < 4; i++ )
		{
			if( foundation[i].PushCard( tmp ) )
			{
				changed[ FOUNDATION1 + i ] = true;
				changed[ prev_slot ] = true;

				slot->PopCard();

				CheckWin( true );

				return;
			}
		}
	}
}


void CTable::CheckWin( bool _add )
{

	if( _add )
		win_counter++;
	else
		win_counter--;



	if( win_counter == 52 )
	{
		this->Init();
	}
}

