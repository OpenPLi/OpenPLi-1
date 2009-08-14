/*
** initial coding by fx2
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <rcinput.h>
#include <colors.h>
#include <sprite.h>

extern	int		doexit;

extern	unsigned short	realcode;
extern	unsigned short	actcode;

extern	unsigned char	*GetPic( int idx,int *maxani,int *width, int *height );
extern	unsigned char *GetMirrorPic( char picid );
extern	void	dblCopyImage( int x1, int y1, int dx,int dy,unsigned char *src);

static	Sprite	*root=0;
static	Sprite	*last=0;
extern	int		main_x;

void	DrawSprite( Sprite *s )
{
	dblCopyImage( s->x-main_x, s->y, s->width, s->height,s->data+s->ani*s->sz);
}

int SpriteCollide( Sprite *s, int x, int y )
{
	return ((x <  s->x+s->width) &&
			(x >= s->x) &&
			(y <  s->y+s->height) &&
			(y >= s->y ));
}

Sprite *CreateSprite( int picid, int ani, int x, int y )
{
	Sprite *s;
	int		ma;

	s=malloc(sizeof(Sprite));
	memset(s,0,sizeof(Sprite));
	s->countdown = 5;
	s->counter1 = 0;
	s->counter2 = 0;
	s->type = TYP_WALKER;
	s->x = x;
	s->y = y;
	s->oldx = x;
	s->oldy = y;
	s->picid = (char)picid;
	s->ori_data= GetPic( picid, &ma, &s->width, &s->height );
	s->data = s->ori_data;
	s->maxani = (char)ma;
	s->sz=s->width*s->height;
	if ( ani <= s->maxani )
		s->ani = ani;

	s->next=0;
	s->pre=last;

	if ( s->pre )
		s->pre->next = s;

	last=s;

	if ( !root )
		root=s;

	return s;
}

void	DrawSprites( void )
{
	Sprite	*s;

	for( s=root; s; s=s->next )
		DrawSprite(s);
}

void	SpriteNextPic( Sprite *s )
{
	if ( s->anilocked )
		return;
	s->ani++;
	if ( s->ani > s->maxani )
		s->ani=0;
}

void	SpriteSelPic( Sprite *s, int ani )
{
	s->ani = ani;
	if ( s->ani > s->maxani )
		s->ani=0;
}

void	MirrorSprite( Sprite *s )
{
	if ( !s->flip_data )
		s->flip_data=GetMirrorPic( s->picid );
	if ( s->data == s->ori_data )
		s->data = s->flip_data;
	else
		s->data = s->ori_data;
}

void	SpriteGetBackground( Sprite *s )
{
	s->oldx=s->x;
	s->oldy=s->y;
}

void	SpritesGetBackground( void )
{
	Sprite	*s;

	for( s=root; s; s=s->next )
		SpriteGetBackground(s);
}

void	FreeSprites( void )
{
	Sprite	*s;
	Sprite	*n;

	for( s=root; s; s=n )
	{
		n=s->next;
		free(s);
	}
	root=0;
	last=0;
}

void	SpriteChangePic( Sprite *s, int picid )
{
	int		ma;

	s->picid = (char)picid;
	s->ori_data= GetPic( picid, &ma, &s->width, &s->height );
	s->data = s->ori_data;
	s->flip_data=0;
	s->maxani = (char)ma;
	s->sz=s->width*s->height;
	if ( s->ani > s->maxani )
		s->ani = 0;
}
