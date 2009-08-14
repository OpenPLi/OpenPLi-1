
typedef struct _Sprite
{
	unsigned char	*ori_data;
	unsigned char	*flip_data;
	unsigned char	*data;
	int				width;
	int				height;
	int				oldx;
	int				oldy;
	int				x;
	int				y;
	int				sz;
	int				countdown;
	int				counter1;
	int				counter2;
	struct _Sprite	*next;
	struct _Sprite	*pre;
	unsigned long	type;
	char			picid;
	char			anilocked;
	char			backlocked;
	char			dir;
	char			ani;
	char			maxani;
	char			partikel;
} Sprite;

extern	int		SpriteCollide( Sprite *s, int x, int y );
extern	Sprite	*CreateSprite( int picid, int ani, int x, int y );
extern	void	MirrorSprite( Sprite *s );
extern	void	SpriteNextPic( Sprite *s );
extern	void	SpriteSelPic( Sprite *s, int ani );
extern	void	DrawSprite( Sprite *s );
extern	void	FreeSprites( void );
extern	void	SpritesGetBackground( void );
extern	void	SpriteGetBackground( Sprite *s );
extern	void	DrawSprites( void );
extern	void	SpriteChangePic( Sprite *s, int picid );

#define	TYP_CLIMB		(1L<<0)		// 0001
#define	TYP_FALL		(1L<<1)		// 0002
#define	TYP_EXPLODE		(1L<<2)		// 0004
#define TYP_UTILS		0x7
#define	TYP_STOPPER		(1L<<3)		// 0008
#define TYP_BUILDER		(1L<<4)		// 0010
#define	TYP_DIGHORI		(1L<<5)		// 0020
#define	TYP_DIGDIAG		(1L<<6)		// 0040
#define	TYP_DIGDOWN		(1L<<7)		// 0080
#define TYP_WALKER		(1L<<8)		// 0100
#define TYP_CLIMBER		(1L<<9)		// 0200
#define TYP_FALLER		(1L<<10)	// 0400
#define TYP_WORK		0x7f8
#define TYP_FALLEN		(1L<<11)	// 0800
#define TYP_ATHOME		(1L<<12)	// 1000
