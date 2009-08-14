/*
** initial coding by fx2
*/

// a hugh amount of debug output on stdout
//#define SOLBOARD_DEBUG

// only 2 tuxes on the board
//#define SOLBOARD_BOARD_DEBUG

#include <stdio.h>

#include <draw.h>
#include <sys/time.h>
#include <rcinput.h>
#include <colors.h>
#include <pics.h>

//#define	STATUS_X		80
//#define STATUS_Y		50
#define LOGO_X			540
#define LOGO_Y			80

extern	double	sqrt( double in );

extern	int		doexit;

extern	unsigned short	actcode;

static	long	        score = 0;   // initial score
static	int		mouse_x = 7; // initial x coordinate
static	int		mouse_y = 7; // initial y coordinate
static int              tuxes = 44;  // initial tux count

static int selected_x = 0;           // x coord of current selection
static int selected_y = 0;           // y coord of current selection
static int selection = 0;            // if a cell is selected or not

static	struct timeval starttv;      // starting time of game

/**
 * Draws the score on the screen
 * Score depends on the time needed to end the game and the
 * remaining tuxes on the board
 */
void	DrawScore( void )
{
	char			tscore[ 64 ];
	struct timeval tv;
	gettimeofday(&tv,0);

	score = tv.tv_sec - starttv.tv_sec;

	if ( score > 35000 )
		score=35000;

	score = 35000 - score;
	score = score - tuxes *1000;
	if (score < 0) {
	  score = 0;
	}
	if (tuxes == 1) {
	  score = 44444;
	  FBDrawString( 190, 130, 64, "You did it!", WHITE, 0 );
	}
	sprintf(tscore,"%ld",score);
	FBDrawString( 190, 210, 64, "Score", WHITE, 0 );
	FBDrawString( 190, 290, 64, tscore, WHITE, 0 );
#ifdef SOLBOARD_DEBUG
	printf("draw score %ld\n",score);
#endif

}

/**
 * Highlights the currently active cell.
 */
static	void	DrawMouse( void )
{
//	FBDrawVLine( mouse_x * 32 + 15, mouse_y * 32 + 4, 24, GREEN );
//	FBDrawHLine( mouse_x * 32 + 4, mouse_y * 32 + 15, 24, GREEN );
	FBFillRect( mouse_x * 32 + 4, mouse_y * 32 + 15, 24, 2, GREEN );
	FBFillRect( mouse_x * 32 + 15, mouse_y * 32 + 4, 2, 24, GREEN );
}

/**
 * Draws a specific cell of the board. The look depends on the content
 * of the cell which is stored in maze from pics.h.
 *
 * Possible values of a field:
 * N   : nothing
 * n,b : empty board cell
 * x   : board cell with a tux
 * s   : board cell with a tux which is selected
 *
 * @param x x-coordinate of the field to draw
 * @param y y-coordinate of the field to draw
 * @return 0
 */
static	int	DrawField( int x, int y )
{
	unsigned char	*p = maze + MAZEW * y + x;

	switch ( *p )
	{
	case 'N' :
		break;
	case 'n' :
	case 'b' :
		FBCopyImage( x*32, y*32, 32, 32, dout );
		break;
	case 'x' :
		FBCopyImage( x*32, y*32, 32, 32, dtux );
		break;

	case 's':
		FBCopyImage( x*32, y*32, 32, 32, dselectedtux );
		break;
	  
	}
	return 0;
}

/**
 * Draws the complete board according to the values of the cells
 * and the fx logo
 *
 * Possible field values:
 * #    : inactive cell (not reachable with mouse)
 * ' '  : field which is not on the board
 * n,b,B: empty field
 * x    : cell with a tux
 * s    : selected cell with a tux 
 */
void	DrawBoard(void)
{
	int				x;
	int				y;
	unsigned char	*p = maze;

	for( y = 0; y < MAZEH; y++ )
	{
		for( x = 0; x < MAZEW; x++, p++ )
		{
			switch ( *p )
			{
			case '#' :
				FBFillRect( x*32, y*32, 32, 32, STEELBLUE );
				break;
			case ' ' :
				FBFillRect( x*32, y*32, 32, 32, BLACK );
				break;
			case 'n' :
				FBCopyImage( x*32, y*32, 32, 32, dout );
				break;
			case 'b' :
			        FBCopyImage( x*32, y*32, 32, 32, dout );
			        break;
			case 'B' :
			        FBCopyImage( x*32, y*32, 32, 32, dout );
				break;
			case 'x' :
			        FBCopyImage ( x*32, y*32, 32, 32, dtux );
			        break;
			case 's' :
			        FBCopyImage( x*32, y*32, 32, 32, dselectedtux );
			        break;
			}
		}
	}
	// draw white frame around board
	FBDrawRect( 3*32-3, 3*32-3, 9*32+5, 9*32+5, WHITE );
	FBDrawRect( 3*32-4, 3*32-4, 9*32+7, 9*32+7, WHITE );

	FBDrawString( LOGO_X-90, LOGO_Y, 32, "powered by", WHITE, 0 );
	FBDrawFx2Logo( LOGO_X, LOGO_Y );
	FBDrawString( LOGO_X-90, LOGO_Y+70, 32, 
		      "brought to you by", WHITE, 0 );
	FBDrawString( LOGO_X-80, LOGO_Y+110, 32, 
		      "ChakaZulu", WHITE, 0 );
	gettimeofday(&starttv,0);

	DrawMouse();
}

/**
 * Initializes the board with tuxes etc. 
 */
void	BoardInitialize( void )
{
	int				x;
	int				y;
	unsigned char	*p = maze;

	for( y = 0; y < MAZEH; y++ )
	{
		for( x = 0; x < MAZEW; x++, p++ )
		{
#ifdef SOLBOARD_BOARD_DEBUG
	// only the cells on the inner field can have
		  // one of the following values
		  if ((*p == 'x') || (*p == 's') || 
		      (*p == 'n')) {
		    // we are on the inner field enclosed by #s
		    if ((x == 7) && ((y == 7)|| (y == 8))) {
		      //  only put 2 tuxes on the field
		      *p = 'x';
		    } else {
		      *p = 'n';
		    }
		  }
		}
	}
#else
		  // only the cells on the inner field can have
		  // one of the following values
		  if ((*p == 'x') || (*p == 's') || 
		      (*p == 'n')) {
		    // we are on the inner field enclosed by #s
		    if ((x == 7) && (y == 7)) {
		      // do not put a tux at the center
		      *p = 'n';
		    } else {
		      *p = 'x';
		    }
		  }
		}
	}
#endif
	actcode=0xee;
	score=0;
	mouse_x=7;
	mouse_y=7;
	tuxes=44;

}

/**
 * Returns the absolute value of an int
 * should be the 34812485493 implementation ;-)
 * @param i the value to return the absolute value of
 * @return the absolute value of i
 */
int abs(int i) {
  return (i > 0 ? i : -i);
}

/**
 * Returns if the field is on the board or not
 *
 * @param x x coordinate of the field
 * @param y y coordinate of the field
 * @return 1 if the field is on the board, 0 otherwise
 */
int isFieldOnBoard(int x, int y) {
  //areas:
  //  1
  // 234
  //  5

  // areas 1,3,5
  if ( ((y > 2) && (y < 12)) && ((x > 5) && (x < 9)) ) {
#ifdef SOLBOARD_DEBUG
    printf("field %d,%d is on board\n",x,y);
#endif
    return 1;
  }
  // fields 2,3,4
  if ( ((y > 5) && (y < 9)) && ((x > 2) && (x < 12)) ) {
#ifdef SOLBOARD_DEBUG
    printf("field %d,%d is on board\n",x,y);
#endif
    return 1;
  }
#ifdef SOLBOARD_DEBUG
  printf("fieln %d,%d is *not* on board\n",x,y); 
#endif
  return 0;
  
}

/**
 * Returns if it is allowed to jump with a tux on sel_x,sel_y to
 * x,y. As the rules say this is only possible if the movement is
 * either horizontal or vertical and if there is exactly one field
 * with a tux between sel_x,sel_y and x,y and if x,y is empty. 
 *
 * @precondition a tux is on field sel_x, sel_y
 * @param x the x coordinate of the target field
 * @param y the y coordinate of the target field
 * @param sel_x the x coordinate of the source field
 * @param sel_y the y coordinate of the source field
 * @param r a pointer to the field between x,y and sel_x, sel_y
 * @return 1 if the jump is valid, 0 otherwise
 */
int isValidJumpPtr(int x, int y, int sel_x, int sel_y, unsigned char *r) {

  unsigned char *p = maze + MAZEW*y + x;

  if (isFieldOnBoard(x, y)) {
    if ( ( ((x == sel_x) && (abs(y - sel_y) == 2)) ||
	  ((y == sel_y) && (abs(x - sel_x) == 2)) ) &&
	 (*r == 'x')  && (*p == 'n') ) {
#ifdef SOLBOARD_DEBUG
          printf("jump from %d,%d to %d,%d is valid\n",sel_x,sel_y,x,y);
#endif
      return 1;
    } else { 
#ifdef SOLBOARD_DEBUG
      printf("jump from %d,%d to %d,%d *not* valid\n",sel_x,sel_y,x,y);
#endif
      return 0;
    }
  } else {
#ifdef SOLBOARD_DEBUG
      printf("jump from %d,%d to %d,%d *not* valid\n",sel_x,sel_y,x,y);
#endif
    return 0;
  }
}

/**
 * Returns if it is allowed to jump with a tux on sel_x,sel_y to
 * x,y. As the rules say this is only possible if the movement is
 * either horizontal or vertical and if there is exactly one field
 * with a tux between sel_x,sel_y and x,y and if x,y is empty. 
 *
 * @precondition a tux is on field sel_x, sel_y
 * @param x the x coordinate of the target field
 * @param y the y coordinate of the target field
 * @param sel_x the x coordinate of the source field
 * @param sel_y the y coordinate of the source field
 * @param mid_x the x coordinate of the field between source and target
 * @param mid_y the y coordinate of the field between source and target
 * @return 1 if the jump is valid, 0 otherwise
 */
int isValidJump(int x, int y, int sel_x, int sel_y, int mid_x, int mid_y) {

  unsigned char *r = maze + MAZEW * mid_y + mid_x;
  return isValidJumpPtr(x,y,sel_x,sel_y,r);
}

/**
 * Checks if the game is over (there is no move possible).
 *
 * @return 0 if there is at least one move to do, 1 otherwise
 */
int isGameOver() {
  int result = 1;
  int x,y;
  if (tuxes == 1) {
    return 1;
  } else {
    unsigned char *p = maze;
    // for each tux on the field check if a jump
    // in north,east,south,west direction is possible    
#ifdef SOLBOARD_DEBUG
    printf("begin gamover\n");
#endif
    for (y=0;y<MAZEH;y++) {
      for (x=0;x<MAZEW;x++,p++) {
	if ((*p == 'x') || (*p == 's')) { // a tux /a selected tux
#ifdef SOLBOARD_DEBUG
	  printf("checking %d,%d for possible move",x,y);
#endif
	  if (isValidJump(x,y-2,x,y,x,y-1) || // north
	      isValidJump(x+2,y,x,y,x+1,y) || // east
	      isValidJump(x,y+2,x,y,x,y+1) || // south
	      isValidJump(x-2,y,x,y,x-1,y)) { // west
#ifdef SOLBOARD_DEBUG
	    printf("end game over: move possible: %d,%d\n",x,y);
#endif
	    result = 0;
	    goto break_outer; // leave both loops
	  }
	}
      }
    }
#ifdef SOLBOARD_DEBUG
    printf("no more moves possible\n");
#endif
  break_outer:;
  }
  return result;
}

/**
 * Handles the selection of a cell.
 * Selects a cell, unselects it or moves a tux
 * from a selected cell to the current one.
 * Actions depend on the rules for this game ;-)
 *
 * @param x the x-coordinate of the field to select
 * @param y the y-coordinate of the field to select
 */
static void SelectField( int x, int y )
{
  // the field to select
  unsigned char	*p = maze + MAZEW * y + x;

  // the currently selected field
  unsigned char   *s = maze + MAZEW * selected_y 
    + selected_x;

  // the field with a tux which will be removed
  unsigned char   *r;
  
  // the coordinates of the field above
  int remove_y = (selected_y + y)/2;
  int remove_x = (selected_x + x)/2;

	switch( *p )
	{
	case 'p' :		// is pressed
		break;
	case 'n' :		// empty field
	  if (selection == 1) {
	    // unmark old selected field 
	    // remove selected and in between tux
	    // add tux at x,y
	    r = maze + MAZEW * remove_y + remove_x;
	    if (isValidJumpPtr(x,y,selected_x,selected_y, r)) {
	      *p = 'x';
	      *s = 'n';
	      *r = 'n';
	      DrawField(selected_x, selected_y); // now empty field
	      DrawField(x,y); // now tux without selection
	      DrawField(remove_x,remove_y); // now empty field
	      selection = 0;
	      tuxes--;
	      if (isGameOver() == 1) {
		doexit = 2;
	      }
	      DrawMouse();
	    }
	  }
	  break;
	case 'x' : // a tux without selection, select it
	  if (selection == 1) {
	    // unmark old selected field
	    *s = 'x';
	    DrawField(selected_x, selected_y);
	  }
	  *p = 's';
	  selected_x = x;
	  selected_y = y;
	  selection = 1;
	  DrawField(selected_x, selected_y);
	  DrawMouse();
	  break;
	case 's' : // a selected tux, deselect it
	  selection = 0;
	  selected_x = 0;
	  selected_y = 0;
	  *p = 'x';
	  DrawField(x,y);
	  DrawMouse();
	  break;
	}
}

/**
 * Moves the mouse on the board.
 * Checks if the move is valid and then moves the mouse.
 */
void	MoveMouse( void )
{
static	int	locked = 0;

	if ( locked )
	{
		locked--;
		actcode=0xee;
		return;
	}
	switch( actcode )
	{
	case RC_RIGHT :
	  if (isFieldOnBoard(mouse_x +1,mouse_y) == 1)
	    {
	      DrawField( mouse_x, mouse_y );
	      mouse_x++;
	      DrawMouse();
	      locked=1;
	    }
	  break;
	case RC_LEFT :
	  if (isFieldOnBoard(mouse_x-1,mouse_y) == 1)
	    {
	      DrawField( mouse_x, mouse_y );
	      mouse_x--;
	      DrawMouse();
	      locked=1;
	    }
	  break;
	case RC_DOWN :
	  if (isFieldOnBoard(mouse_x,mouse_y+1) == 1)
	    {
	      DrawField( mouse_x, mouse_y );
	      mouse_y++;
	      DrawMouse();
	      locked=1;
	    }
	  break;
	case RC_UP :
	  if (isFieldOnBoard(mouse_x,mouse_y-1) == 1)
	    {
	      DrawField( mouse_x, mouse_y );
	      mouse_y--;
	      DrawMouse();
	      locked=1;
	    }
	  break;
	case RC_OK :
	  SelectField( mouse_x, mouse_y );
	  locked=1;
	  break;
	case RC_RED : // main loop draws score and resets field
	  doexit = 2;
	  locked=1;
	  break;
	}
}




