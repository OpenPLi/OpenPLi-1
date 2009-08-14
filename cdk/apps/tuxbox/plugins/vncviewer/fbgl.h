#ifndef _FBGL_H
#define _FBGL_H

#include "list.h"

#ifndef __cplusplus
typedef int bool;
#endif
typedef unsigned short Pixel;

#define MAX_SCALE 4

struct viewport {
	int v_xsize;
	int v_ysize;
	int v_x0;
	int v_y0;
	int v_scale;
	int v_bpp;
	Pixel *v_buf;
};

typedef struct {
	/* physical display */
	int framebuf_fds;
	int smem_len;
	void* p_framebuf;
	int p_xsize;
	int p_ysize;
	int pv_xsize;
	int pv_ysize;
	int p_xoff;
	int p_yoff;
	Pixel *p_buf;

	/* viewport */
	int v_xsize;
	int v_ysize;
	int v_x0;
	int v_y0;
	int v_scale;
	int v_bpp;
	Pixel *v_buf;

	/* overlays */
	List *overlays;
	int hide_overlays; /* ref count */

	/* input devices */
	int mouse_x;
	int mouse_y;
	bool ts_pressed;

	int ts_fd;
	int kb_fd;

	/* user-defined input + output */
	int num_read_fds;
	int *read_fd;

	int num_write_fds;
	int *write_fd;
} fbvnc_framebuffer_t;

/* for efficiency reasons, don't access the contents of the global
 * structure directly. (Check the compiler's assembly
 * output - the compiler needs to reload the elements inside of loops,
 * because it cannot know if the elements might be modified by other
 * functions in the meantime.)
 */
extern fbvnc_framebuffer_t global_framebuffer;

/* The following macro defines auto variables with copies of the
 * global_framebufferstructure. The compiler is smart enough not to
 * load variables that aren't actually used. They are "const" to
 * guard against changing them accidentally - changes must be
 * made to the real global_framebuffer vars.
 */
#define IMPORT_FRAMEBUFFER_VARS						\
	void *const p_framebuf = global_framebuffer.p_framebuf;		\
	const int p_xsize = global_framebuffer.p_xsize;			\
	const int p_ysize = global_framebuffer.p_ysize;			\
	const int pv_xsize = global_framebuffer.pv_xsize;			\
	const int pv_ysize = global_framebuffer.pv_ysize;			\
	const int p_xoff = global_framebuffer.p_xoff;			\
	const int p_yoff = global_framebuffer.p_yoff;			\
	Pixel *const p_buf = global_framebuffer.p_buf;			\
	const int v_xsize = global_framebuffer.v_xsize;			\
	const int v_ysize = global_framebuffer.v_ysize;			\
	const int v_x0 = global_framebuffer.v_x0;			\
	const int v_y0 = global_framebuffer.v_y0;			\
	const int v_scale = global_framebuffer.v_scale;			\
	const int v_bpp = global_framebuffer.v_bpp;			\
	Pixel *const v_buf = global_framebuffer.v_buf;			\
	List *const overlays = global_framebuffer.overlays;		\
	const bool hide_overlays = global_framebuffer.hide_overlays;	\
	const int mouse_x = global_framebuffer.mouse_x;			\
	const int mouse_y = global_framebuffer.mouse_y;			\
	const bool ts_pressed = global_framebuffer.ts_pressed;		\
	const int ts_fd = global_framebuffer.ts_fd;			\
	const int kb_fd = global_framebuffer.kb_fd;			\
	const int num_read_fds = global_framebuffer.num_read_fds;	\
	const int *read_fd = global_framebuffer.read_fd;		\
	const int num_write_fds = global_framebuffer.num_write_fds;	\
	int *const write_fd = global_framebuffer.write_fd;		\
/* Suppress 'unused variable' warnings */				\
	(void)p_framebuf;						\
	(void)p_xsize;							\
	(void)p_ysize;							\
	(void)pv_xsize;							\
	(void)pv_ysize;							\
	(void)p_xoff;							\
	(void)p_yoff;							\
	(void)p_buf;							\
	(void)v_xsize;							\
	(void)v_ysize;							\
	(void)v_x0;							\
	(void)v_y0;							\
	(void)v_scale;							\
	(void)v_bpp;							\
	(void)v_buf;							\
	(void)overlays;							\
	(void)hide_overlays;						\
	(void)mouse_x;							\
	(void)mouse_y;							\
	(void)ts_pressed;						\
	(void)ts_fd;							\
	(void)kb_fd;							\
	(void)num_read_fds;						\
	(void)read_fd;							\
	(void)num_write_fds;						\
	(void)write_fd;							\
/* FRAMEBUFFER_VARS end */
	

enum fbvnc_event {
	FBVNC_EVENT_NULL = 0, /* false alarm */
	FBVNC_EVENT_TS_DOWN = 0x0001,
	FBVNC_EVENT_TS_MOVE = 0x0002,
	FBVNC_EVENT_TS_UP = 0x0004,

	FBVNC_EVENT_BTN_DOWN = 0x0010,
	FBVNC_EVENT_BTN_UP = 0x0020,

	FBVNC_EVENT_DATA_READABLE = 0x0100,
	FBVNC_EVENT_DATA_WRITABLE = 0x0200,

	FBVNC_EVENT_QUIT          = 0x0400,

	FBVNC_EVENT_TIMEOUT = 0x1000,
	FBVNC_EVENT_SEND_UPDATE_REQUEST = 0x2000,
	FBVNC_EVENT_KEYREPEAT = 0x4000,
	FBVNC_EVENT_TICK_SECOND = 0x8000,
	FBVNC_EVENT_ZOOM_IN        = 0x10000,
	FBVNC_EVENT_ZOOM_OUT       = 0x20000,
	FBVNC_EVENT_DCLICK         = 0x40000
};

typedef struct {
	int evtype;
	int x, y;
	int dx, dy;
	int key;
	int fd;
} fbvnc_event_t;

extern List *sched;

extern void schedule_add(List *s, int ms_delta, int event);

extern void gl_setpalettecolor(int i, int r, int g, int b);
extern void gl_copybox(int x1, int y1, int w, int h, int x2, int y2);
extern void gl_putbox(int x1, int y1, int w, int h, CARD8 *buf);
extern void gl_fillbox(int x, int y, int w, int h, int col);

#define EXIT_OK 0
#define EXIT_ERROR 1
#define EXIT_SYSERROR 2

extern void redraw_virt(int xv, int yv, int wv, int hv);
extern void redraw_phys(int xp, int yp, int wp, int hp);
extern void redraw_phys_all(void);
extern void vp_pan_virt(int x0, int y0);
extern void vp_pan(int pdx, int pdy);
extern void grid_pan(int dx, int dy);
extern void flip_orientation(void);
extern void cleanup_and_exit(char *msg, int ret);
extern void draw_border(int xp, int yp, int wp, int hp, int oxp, int oyp, int owp, int ohp);

extern int img_read(struct viewport *vport, FILE *f);

#endif
