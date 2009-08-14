
#ifdef __cplusplus
extern	"C" {
#endif

typedef unsigned char	UBYTE;

typedef struct {
	int		width;
	int		height;
	int		bytesperpixel;
	int		colors;
	int		linewidth;
	int		memory;
	int		flags;

} vga_modeinfo;

extern	vga_modeinfo	*vga_getmodeinfo( int mode );
extern	int				vga_setmode(int mode );
extern	void			*vga_getgraphmem( void );
#define vga_drawscanline(y,mem)
extern	int				vga_setlinearaddressing(void);
extern	void			vga_setpalette( int i, UBYTE red, UBYTE green, UBYTE blue );

extern	void			vga_drawlogo(void);
extern	void			vga_printscreen( char *fname );

#define	TEXT			1
#define G640x480x256	2

#define CAPABLE_LINEAR	1

#ifdef __cplusplus
};
#endif
