#include <config.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H
#include FT_CACHE_SMALL_BITMAPS_H

typedef struct fbvnc_overlay {
	/* current coordinates */
	int x, y;
	
	/* height and width */
	int w, h;

	/* default coordinates in landscape mode */
	/* when held in landscape mode, origin is top left, x is horizontal */
	int x0l, y0l;

	/* default coordinates in portrait mode */
	/* when held in portrait mode, origin is top left, x is horizontal */
	int x0p, y0p;
	
	Pixel *pixels;
	void *data;

	/* callback returns 1 if click taken, 0 if transparent */
	int (*callback)(fbvnc_event_t *ev, struct fbvnc_overlay *);
	int events_wanted;

	bool visible;
} fbvnc_overlay_t;

typedef int fbvnc_ev_callback_t(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);

extern void overlay_destructor(void *p);
extern fbvnc_overlay_t *add_overlay(int x0l, int y0l, int x0p, int y0p,
	int h, int w, Pixel *icon,
        int events_wanted, fbvnc_ev_callback_t func, void *data);
extern bool remove_overlay(fbvnc_overlay_t *ov);

extern void overlays_init(void);
extern void draw_overlay(fbvnc_overlay_t *ov);
extern void draw_pixmap(Pixel *pix, int x, int y, int w, int h);
extern void redraw_all_overlays();
extern void redraw_overlays(int xp, int yp, int wp, int hp);
extern bool overlay_event(fbvnc_event_t *ev, fbvnc_overlay_t *ov,
	bool check_hit);
extern fbvnc_overlay_t *check_overlays(fbvnc_event_t *ev);
extern void vp_hide_overlays(void);
extern void vp_restore_overlays(void);

extern void set_mouse_state(bool multi);
extern void set_light(bool on);
extern void toggle_light(void);
extern void toggle_keyboard(void);
extern void show_pnm_image(void);

extern int mouse_button;
extern int mouse_multibutton_mode;

/* overlay event handlers */
extern int ev_quit(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);
extern int ev_zoom_out(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);
extern int ev_zoom_in(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);
extern int ev_keybd(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);
extern int ev_kbd_sel(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);
extern int ev_mouse(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);
extern int ev_calibrate(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);
extern int ev_battery(fbvnc_event_t *ev, fbvnc_overlay_t *this_overlay);

extern int selectServer(char* szServerNr, int rc_fd);
extern void MessageBox(const char* szMsg, int rc_fd);
extern void cleanupFT();
