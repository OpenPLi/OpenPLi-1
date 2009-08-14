#include <stdio.h>
#include <board.h>
#include <draw.h>
#include <rcinput.h>
#include <colors.h>
#include <pics.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#include <config.h>

extern	int		doexit;
extern	unsigned short	actcode;

static int 	man_x = 1;
static int	man_y = 1;
static int 	moves = 0;
static int 	pushes = 0;
static int 	ox,oy;
int 		level = 0;
int 		max_level = 1;
static char* 	levelname[500];
static int	levelverz[500];
static int 	Rand = 0;
char		Zuege[1500];
static	unsigned char	board[22][18];
static int MAZEW;
static int MAZEH;

static int one (struct dirent *name)		// Filter für ladeVerzeichnis
{
  char* ext = ".xsb";
  if (strstr(name->d_name,ext)!=NULL) { return 1;}
	else { return 0;}
}

void ladeVerzeichnis (void)
{
	struct dirent **eps;
	int n;
//  char* Verz2;

//  char* Verzeichnis = getcwd (NULL, 0);
//  puts(Verzeichnis);


//	Verz2 = (char*) malloc(strlen(Verzeichnis)+2);
//	strcpy(Verz2,Verzeichnis);
//	strcat(Verz2,"/");
//	puts(Verz2);

//  n = scandir (Verzeichnis, &eps, one, alphasort);

	max_level = 0;
	n = scandir ( DATADIR "/sokoban/", &eps, one, alphasort);
	if (n >= 0)
		{
		int cnt;
		for (cnt = 0; cnt < n && cnt < 500; ++cnt)
			{
			levelname[cnt] = strdup(eps[cnt]->d_name);
			levelverz[cnt] = 1;
			}
		max_level = n;
		}

	n = scandir ( CONFIGDIR "/sokoban/", &eps, one, alphasort);
	if (n >= 0)
		{
		int cnt;
		for (cnt = 0; cnt < n && cnt < 500; ++cnt)
			{
			levelname[cnt+max_level] = strdup(eps[cnt]->d_name);
			levelverz[cnt+max_level] = 2;
			}
		max_level = max_level + n;
		}

	if (max_level == 0)
		{
		levelname[0]=strdup("Installation error");
		}

//	free(Verz2);
//	free(Verzeichnis);
}

void freeMem(void)
{
	int	i;
	for (i = 0; i < max_level; i++)
		free(levelname[i]);
}

void	DrawScore( void )
{
	char tmoves[ 64 ];
	char tpushes[ 64 ];		
	sprintf(tmoves,"%d",moves);

	FBDrawString( 100, 480, 30, "Moves", WHITE, 0 );
	FBFillRect( 100, 510, 30, 30, BLACK );
	FBDrawString( 100, 510, 30, tmoves, WHITE, 0 );

	sprintf(tpushes,"%d",pushes);

	FBDrawString( 160, 480, 30, "Pushes", WHITE, 0 );
	FBFillRect( 160, 510, 30, 30, BLACK );
	FBDrawString( 160, 510, 30, tpushes, WHITE, 0 );
}

char getField( int x, int y)
{
	return board[x][y];
}

void setField(int x, int y, char c)
{
	board[x][y] = c;
}

int win( void )
{
	int x;
	int y;

	for( y = 0; y < MAZEH; y++ )
	{
		for( x = 0; x < MAZEW; x++)
		{
		if (board[x][y] == '.' || board[x][y] == '+')	// sind noch Zielfelder frei ?
			{
			return 0;	// noch nicht gewonnen
			}
		}
	}
	return 1;	// gewonnen
}

void	DrawField( int x, int y )
{
	switch ( board[x][y] )
	{
			case '#' :									// Wand
				FBCopyImage( x*32 + ox, y*28 + oy, 32, 28, wall );
				break;
			case ' ' :									// leeres Feld
				FBCopyImage( x*32 + ox, y*28 + oy, 32, 28, empty );
				break;
			case '@' :									// Pinguin
				FBCopyImage( man_x*32 + ox, man_y*28 + oy, 32, 28, man );
				break;
			case '+' :									// Pinguin auf Zielfeld
				FBCopyImage( man_x*32 + ox, man_y*28 + oy, 32, 28, manongoal );
				break;
			case '$' :									// Kiste
				FBCopyImage( x*32 + ox, y*28 + oy, 32, 28, box );
				break;
			case '*' :									// Kiste auf Zielfeld
				FBCopyImage( x*32 + ox, y*28 + oy, 32, 28, boxongoal );
				break;
			case '.' :									// Zielfeld
				FBCopyImage( x*32 + ox, y*28 + oy, 32, 28, goal );
				break;
			case 'a' :									// äußeres Feld (nicht xsb-konform)
				if (Rand == 1)
					{
					FBCopyImage( x*32 + ox, y*28 + oy, 32, 28, test );
					}
				break;
	}
}

void	DrawBoard()
{
	int x;
	int y;

	for( y = 0; y < MAZEH; y++ )
	{
		for( x = 0; x < MAZEW; x++)
		{
		DrawField( x, y);
		}
	}
}

void	BoardInitialize()
{
	char* verz = DATADIR "/sokoban/";
	char* verz2 = CONFIGDIR "/sokoban/";
	FILE *Datei=0;
	int x,y,z;
	int i;
	char* d;

	if (levelverz[level]==1)
		{
		d = malloc (strlen(verz)+strlen(levelname[level])+1);
		strcpy (d,verz);
		strcat (d,levelname[level]);
		}
	else
		{
		d = malloc (strlen(verz2)+strlen(levelname[level])+1);
		strcpy (d,verz2);
		strcat (d,levelname[level]);
		}

	FBFillRect( 0, 0, 720, 576, BLACK );

	Datei = fopen(d, "r");

	free(d);

	if ( !Datei )
	{	// installation error
		doexit=4;
		FBDrawString( 200, 510, 30, "Installation Error. Cant find Level", WHITE, 0 );
		return;
	}

	MAZEW = 0;
	MAZEH = 0;

	for (y=0; y<18; y++)
		{
		for (x=0; x<22; x++)
			{
			setField(x, y, 'a');
			}
		}

	x=0; y=0;
	while(!feof(Datei))
		{
		z = fgetc(Datei);

		if (z!='#' && z!='.' && z!='$' && z!='@' && z!='+' && z!='*' && z!=' ' && z!=0x0d && z!=0x0a)
			{
			MAZEH=y;
			break;
			}
		else if (z == 0x0d)
			{
			z=fgetc(Datei);
			for (i=x; board[i][y] != '#' && i > 3; i--) {x=i;}
			if (MAZEW < x) {MAZEW = x;}
			y++;
			x=0;
			MAZEH=y;
			}
		else if (z == 0x0a)
			{
			for (i=x; board[i][y] != '#' && i > 3; i--) {x=i;}
			if (MAZEW < x) {MAZEW = x;}
			y++;
			x=0;
			MAZEH=y;
			}
		else
			{
			if (y<18 && x<22)
				{
				setField(x, y, z);
				if ((z=='@') || (z=='+'))
					{
					man_x = x;
					man_y = y;
					}
				}
			x++;
			}
		}

	fclose(Datei);

	ox = (720 - (MAZEW*32))/2;
	oy = (576 - (MAZEH*28))/2;

	for (y=0; y<18; y++)
		{
		for (x=0; x<22; x++)
			{
			if (getField(x, y) != '#') {setField (x, y, 'a');}
				else break;
			}
		for (x=21; x>=0; x--)
			{
			if (getField(x, y) != '#') {setField (x, y, 'a');}
				else break;
			}
		}

	for (x=0; x<22; x++)
		{
		for (y=0; y<18; y++)
			{
			if (getField(x, y) != '#') {setField (x, y, 'a');}
				else break;
			}
		for (y=17; y>=0; y--)
			{
			if (getField(x, y) != '#') {setField (x, y, 'a');}
				else break;
			}
		}

	moves = 0;
	pushes = 0;

	DrawBoard();
	DrawScore();
	FBDrawString( 300, 510, 30, levelname[level], WHITE, 0 );
	
	for (i=0; i<=1500;i++)
		{
		Zuege[i]=' ';
		}
}

void Startbildschirm()
{
	int i;
	char tlevel[ 64 ];

//	int x,y;
//	for (y=0; y<576; y=y+56)
//		{
//		for (x=0; x<720; x=x+64)
//			{
//			FB2CopyImage(  x, y, 32, 28, man, 2);
//			}
//		}

static int bx[] = {
64,32,0,0,0,32,64,64,64,32,0,128,160,192,192,192,192,192,160,128,128,128,
128,256,256,256,256,256,288,320,320,320,320,384,416,448,448,448,448,448,416,
384,384,384,384,512,544,576,224,224,224,224,224,256,288,288,256,288,288,256,
352,352,352,352,352,384,416,416,416,416,416,384,480,480,480,480,480,512,512,
544,544,576,576,576,576,576 };

static int by[] = {
0,0,0,28,56,56,56,84,112,112,112,0,0,0,28,56,84,112,112,112,84,56,28,0,28,56,
84,112,56,0,28,84,112,0,0,0,28,56,84,112,112,112,84,56,28,56,56,56,312,284,256,
228,200,200,200,228,256,284,312,312,312,284,256,228,200,200,200,228,256,284,312,
256,200,228,256,284,312,228,256,256,284,200,228,256,284,312 };

	for (i=0;i<86;i++)
		{
		FBCopyImage(bx[i]+56,by[i]+118, 32, 28, wall );
		}

	FBFillRect( 55, 365, 200 , 40, BLACK );
	sprintf(tlevel,"%d",max_level);
	FBDrawString( 75,370, 30, tlevel, WHITE, BLACK );
	FBDrawString( 120,370, 30, "Level gefunden", WHITE, BLACK);

#ifdef USEX     
	FBFlushGrafic(); 
#endif

	while( actcode != RC_OK )
		{
			if (i>87) { i=0;} else {i++;}
			if (i<86) {FBCopyImage( bx[i]+56,by[i]+118, 32, 28, box );}
			if (i>=1 && i-1<=85) { FBCopyImage( bx[i-1]+56,by[i-1]+118, 32, 28, man );}
			if (i>=2 && i-2<=85) { FBCopyImage( bx[i-2]+56,by[i-2]+118, 32, 28, wall );}

#ifdef USEX
			FBFlushGrafic(); 
#endif
			RcGetActCode( );
			sleep(1);
		}
	FBFillRect(   0,  0, 720 , 576, BLACK );
}

void	DrawGameOver()
{
	DrawScore();
	FBDrawString( 300, 210, 64, "Level geschaft!", RED, 0 );
}

void	MoveMouse()
{
static	int	locked = 0;
int dx=0;
int dy=0;
char zug=' ';

	if ( locked )
	{
		locked--;
		actcode=0xee;
		return;
	}

	switch( actcode )
	{
	case RC_RIGHT :		// Pinguin nach rechts
		if ( man_x+1 < MAZEW )
		{
		dx = 1;
		dy = 0;
		zug = 'r';
		locked=1;
		}
		break;

	case RC_LEFT :		// Pinguin nach links
		if ( man_x > 1 )
		{
		dx = -1;
		dy = 0;
		zug = 'l';
		locked=1;
		}
		break;

	case RC_DOWN :		// Pinguin nach unten
		if ( man_y+1 < MAZEH )
		{
		dx = 0;
		dy = 1;
		zug = 'd';
		locked=1;
		}
		break;

	case RC_UP :		// Pinguin nach oben
		if ( man_y > 1 )
		{
		dx = 0;
		dy = -1;
		zug = 'u';
		locked=1;
		}
		break;

	case RC_MINUS :		// letzte Züge rückgängig machen
		if (moves>0)
			{
			if (getField(man_x,man_y)=='@')
				{
				setField(man_x,man_y,' ');
				}
			else
				{
				setField(man_x,man_y,'.');
				}

			if (Zuege[moves]=='r' || Zuege[moves]=='R')
				{
				man_x--;
				dx=1;
				}
			else if (Zuege[moves]=='l' || Zuege[moves]=='L')
				{
				man_x++;
				dx=-1;
				}
			else if (Zuege[moves]=='u' || Zuege[moves]=='U')
				{
				man_y++;
				dy=-1;
				}
			else if (Zuege[moves]=='d' || Zuege[moves]=='D')
				{
				man_y--;
				dy=1;
				}

			if (getField(man_x,man_y)==' ')
				{
				setField(man_x,man_y,'@');
				}
			else
				{
				setField(man_x,man_y,'+');
				}

			if (isupper(Zuege[moves]))
				{
				if (getField(man_x+2*dx,man_y+2*dy)=='$')
					{
					setField(man_x+2*dx,man_y+2*dy,' ');
					}
				else
					{
					setField(man_x+2*dx,man_y+2*dy,'.');
					}

				if (getField(man_x+dx,man_y+dy)==' ')
					{
					setField(man_x+dx,man_y+dy,'$');
					}
				else
					{
					setField(man_x+dx,man_y+dy,'*');
					}
				pushes--;
				}
			moves--;
			dx=0;
			dy=0;
			DrawBoard();
			DrawScore();
		}
		locked=1;
		break;

	case RC_PLUS :		// Rückgängig rückgängig machen
		if (Zuege[moves+1]!=' ')
			{
			if (Zuege[moves+1]=='r' || Zuege[moves+1]=='R')
				{
				dx=1;
				}
			else if (Zuege[moves+1]=='l' || Zuege[moves+1]=='L')
				{
				dx=-1;
				}
			else if (Zuege[moves+1]=='u' || Zuege[moves+1]=='U')
				{
				dy=-1;
				}
			else if (Zuege[moves+1]=='d' || Zuege[moves+1]=='D')
				{
				dy=1;
				}
			}
		locked=1;
		break;

	case RC_RED :		// vorheriges Level - bei angefangenem Level nachfragen, ob wirklich sicher
		if (moves!=0)
			{
			FBFillRect( 160, 70, 400, 436, B );
			FBDrawString( 160,75, 30, "Das Level ist noch nicht beendet!", WHITE, 0 );
			FBDrawString( 160,110, 30, "ROT   abbruch", WHITE, 0 );
			FBDrawString( 160,140, 30, "OK    naechstes Level", WHITE, 0 );
#ifdef USEX
			FBFlushGrafic();
#endif

			actcode=0xee;
			while( actcode != RC_OK && actcode != RC_RED )
				{
					RcGetActCode();
				}
			}
		if (actcode == RC_OK || moves==0)
			{
			if (level > 0)
				{
				level--;
				BoardInitialize();
				}
			else
				{
				level = max_level-1;
				BoardInitialize();
				}
			}
		else
			{
			FBFillRect( 0, 0, 720, 576, BLACK );
			DrawBoard();
			DrawScore();
			}

		locked=1;
		break;

	case RC_GREEN :		// naechstes Level - bei angefangenem Level nachfragen, ob wirklich sicher
		if (moves!=0)
			{
			FBFillRect( 160, 70, 400, 436, B );
			FBDrawString( 160,75, 30, "Das Level ist noch nicht beendet!", WHITE, 0 );
			FBDrawString( 160,110, 30, "ROT   abbruch", WHITE, 0 );
			FBDrawString( 160,140, 30, "OK    vorheriges Level", WHITE, 0 );
#ifdef USEX
			FBFlushGrafic();
#endif

			actcode=0xee;
			while( actcode != RC_OK && actcode != RC_RED )
				{
					RcGetActCode();
				}
			}
		if (actcode == RC_OK || moves==0)
			{
			if (level+1 < max_level)
				{
				level++;
				BoardInitialize();
				}
			else
				{
				level = 0;
				BoardInitialize();
				}
			}
		else
			{
			FBFillRect( 0, 0, 720, 576, BLACK );
			DrawBoard();
			DrawScore();
			}

		locked=1;
		break;

	case RC_YELLOW :	// Randfelder ('a') aus-/einblenden
		if (Rand == 0)
			{
			Rand = 1;
			}
		else
			{
			Rand = 0;
			}
		locked=1;
		FBFillRect( 0, 0, 720, 576, BLACK );
		DrawBoard();
		DrawScore();
		FBDrawString( 300, 510, 30, levelname[level], WHITE, 0 );
		break;

	case RC_BLUE :		// Level von vorne beginnen - bei angefangenem Level nachfragen, ob wirklich sicher
		if (moves!=0)
			{
			FBFillRect( 160, 70, 400, 436, B );
			FBDrawString( 160,75, 30, "Das Level ist noch nicht beendet!", WHITE, 0 );
			FBDrawString( 160,110, 30, "ROT   abbruch", WHITE, 0 );
			FBDrawString( 160,140, 30, "OK    vorheriges Level", WHITE, 0 );
#ifdef USEX
			FBFlushGrafic();
#endif
			actcode=0xee;
			while( actcode != RC_OK && actcode != RC_RED )
				{
					RcGetActCode();
				}
			}
		if (actcode == RC_OK || moves==0)
			{
			BoardInitialize();
			}
		else
			{
			FBFillRect( 0, 0, 720, 576, BLACK );
			DrawBoard();
			DrawScore();
			}
		locked=1;
		break;

	case RC_HELP :		// Hilfe anzeigen
		locked=1;
		FBFillRect( 160, 70, 400, 436, B );
		FBDrawString( 160, 70, 30, "ROT   vorheriges Level", WHITE, 0 );
		FBDrawString( 160,100, 30, "GRUEN naechstes Level", WHITE, 0 );
		FBDrawString( 160,130, 30, "GELB  Rand ein/aus", WHITE, 0 );
		FBDrawString( 160,160, 30, "BLAU  Level neu starten", WHITE, 0 );
		FBDrawString( 160,190, 30, "MINUS Zug zurueck", WHITE, 0 );
		FBDrawString( 160,220, 30, "PLUS  Zug vor", WHITE, 0 );
		FBDrawString( 160,260, 30, "HOME  Spiel beenden", WHITE, 0 );
		FBDrawString( 160,290, 30, "STUMM Spiel aus-/einblenden", WHITE, 0 );
		FBDrawString( 160,330, 30, "HILFE diese Hilfe", WHITE, 0 );
		FBDrawString( 160,370, 30, "OK    weiter", WHITE, 0 );
#ifdef USEX
		FBFlushGrafic();
#endif

		actcode=0xee;
		while( actcode != RC_OK )
			{
				RcGetActCode( );
			}
		FBFillRect( 0, 0, 720, 576, BLACK );
		DrawBoard();
		DrawScore();
		FBDrawString( 300, 510, 30, levelname[level], WHITE, 0 );
		break;
	}

	if (dx != 0 || dy != 0)		// Soll Pinguin verschoben werden?
	{
	if (getField(man_x+dx, man_y+dy) == ' ')	// Zug auf leeres Feld - keine Kiste vorhanden
		{
		if (getField(man_x,man_y)=='@') {setField(man_x,man_y,' ');}
			else {setField(man_x,man_y,'.');}
		DrawField(man_x, man_y);

		man_x += dx; dx = 0;
		man_y += dy; dy = 0;
		setField(man_x,man_y,'@');

		DrawField(man_x, man_y);

		moves++;
		DrawScore();
		}
	else if (getField(man_x+dx, man_y+dy) == '.')	// Zug auf Zielfeld - keine Kiste vorhanden
		{
		if (getField(man_x,man_y)=='@') {setField(man_x,man_y,' ');}
			else {setField(man_x,man_y,'.');}
		DrawField(man_x, man_y);

		man_x += dx; dx = 0;
		man_y += dy; dy = 0;
		setField(man_x,man_y,'+');

		DrawField(man_x, man_y);

		moves++;
		DrawScore();
		}
	else if (getField(man_x+dx, man_y+dy) == '$' && getField(man_x+(2*dx), man_y+(2*dy)) == ' ')	// Zug auf Feld mit Kiste - Kiste auf leeres Feld
		{
		setField(man_x+(2*dx), man_y+(2*dy),'$');
		setField(man_x+dx, man_y+dy,' ');
		DrawField(man_x+(2*dx), man_y+(2*dy));

		if (getField(man_x,man_y)=='@') {setField(man_x,man_y,' ');}
			else {setField(man_x,man_y,'.');}
		DrawField(man_x, man_y);

		man_x += dx; dx = 0;
		man_y += dy; dy = 0;
		setField(man_x,man_y,'@');

		DrawField(man_x, man_y);

		moves++;
		pushes++;
		zug = toupper (zug);
		DrawScore();
		}
	else if (getField(man_x+dx, man_y+dy) == '$' && getField(man_x+(2*dx), man_y+(2*dy)) == '.')	// Zug auf Feld mit Kiste - Kiste auf Zielfeld
		{
		setField(man_x+(2*dx), man_y+(2*dy),'*');
		setField(man_x+dx, man_y+dy,' ');
		DrawField(man_x+(2*dx), man_y+(2*dy));

		if (getField(man_x,man_y)=='@') {setField(man_x,man_y,' ');}
			else {setField(man_x,man_y,'.');}
		DrawField(man_x, man_y);

		man_x += dx; dx = 0;
		man_y += dy; dy = 0;
		setField(man_x,man_y,'@');

		DrawField(man_x, man_y);

		moves++;
		pushes++;
		zug = toupper (zug);
		DrawScore();
		}
	else if (getField(man_x+dx, man_y+dy) == '*' && getField(man_x+(2*dx), man_y+(2*dy)) == ' ')	// Zug auf Zielfeld mit Kiste - Kiste auf leeres Feld
		{
		setField(man_x+(2*dx), man_y+(2*dy),'$');
		setField(man_x+dx, man_y+dy,'.');
		DrawField(man_x+(2*dx), man_y+(2*dy));

		if (getField(man_x,man_y)=='@') {setField(man_x,man_y,' ');}
			else {setField(man_x,man_y,'.');}
		DrawField(man_x, man_y);

		man_x += dx; dx = 0;
		man_y += dy; dy = 0;
		setField(man_x,man_y,'+');

		DrawField(man_x, man_y);

		moves++;
		pushes++;
		zug = toupper (zug);
		DrawScore();
		}
	else if (getField(man_x+dx, man_y+dy) == '*' && getField(man_x+(2*dx), man_y+(2*dy)) == '.')	// Zug auf Zielfeld mit Kiste - Kiste auf Zielfeld
		{
		setField(man_x+(2*dx), man_y+(2*dy),'*');
		setField(man_x+dx, man_y+dy,'.');
		DrawField(man_x+(2*dx), man_y+(2*dy));

		if (getField(man_x,man_y)=='@') {setField(man_x,man_y,' ');}
			else {setField(man_x,man_y,'.');}
		DrawField(man_x, man_y);

		man_x += dx; dx = 0;
		man_y += dy; dy = 0;
		setField(man_x,man_y,'+');

		DrawField(man_x, man_y);

		moves++;
		pushes++;
		zug = toupper (zug);
		DrawScore();
		}
	else
		{
		zug = ' ';
		}
	}

	if (zug !=' ')
		{
		Zuege[moves]=zug;
		}

	if (win() == 1)		// durch letzten Zug gewonnen?
		{
		if (level+1 < max_level)
			{
			level++;
			}
		else
			{
			level = 0;
			}

		doexit=1;
		}
#ifdef USEX
	FBFlushGrafic();
#endif
	return;
}

