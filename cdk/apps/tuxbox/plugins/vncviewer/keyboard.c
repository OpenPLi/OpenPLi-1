#include <X11/keysym.h>

#include "vncviewer.h"
#include "fbgl.h"
#include "overlay.h"
#include "keyboard.h"
#include <linux/input.h>

int *fbvnc_keymap;
struct hbtn hbtn;
static int caps_lock=0;
/*
shift_l, shift_r, altgr,
mouse1, mouse2, mouse3,
pan, action
*/                                                                

#ifdef HAVE_DBOX_HARDWARE
static int keys_sent_map[N_SCANCODE];

static int fbvnc_keymap_dbox[N_SCANCODE*4] = {
	0, 0, 0, 0,				/* 0 */
	XK_Escape, 0, 0, 0,
	XK_1, XK_exclam, 0, 0,
	XK_2, XK_quotedbl, XK_twosuperior, 0,
	XK_3, XK_section, XK_threesuperior, 0,
	XK_4, XK_dollar, 0, 0,
	XK_5, XK_percent, 0, 0,
	XK_6, XK_ampersand, 0, 0,
	XK_7, XK_slash, XK_braceleft, 0,
	XK_8, XK_parenleft, XK_bracketleft, 0,
	XK_9, XK_parenright, XK_bracketright, 0,		/* 10 */
	XK_0, XK_equal, XK_braceright, 0,
	XK_ssharp, XK_question, XK_backslash, 0,
	XK_apostrophe, XK_grave, 0, 0,
	XK_BackSpace, 0, 0, 0,
	XK_Tab, 0, 0, 0,
	XK_q, XK_Q, XK_at, 0,
	XK_w, XK_W, 0, 0,
	XK_e, XK_E, 0, 0,
	XK_r, XK_R, 0, 0,
	XK_t, XK_T, 0, 0,			/* 20 */
	XK_z, XK_Z, 0, 0,
	XK_u, XK_U, 0, 0,
	XK_i, XK_I, 0, 0,
	XK_o, XK_O, 0, 0,
	XK_p, XK_P, 0, 0,
	XK_udiaeresis, XK_Udiaeresis, 0, 0,
	XK_plus, XK_asterisk, XK_asciitilde, 0,
	XK_Return, 0, 0, 0,
	XK_Control_L, 0, 0, 0,
	XK_a, XK_A, 0, 0,	/* 30 */
	XK_s, XK_S, 0, 0,
	XK_d, XK_D, 0, 0,
	XK_f, XK_F, 0, 0,
	XK_g, XK_G, 0, 0,
	XK_h, XK_H, 0, 0,
	XK_j, XK_J, 0, 0,
	XK_k, XK_K, 0, 0,
	XK_l, XK_L, 0, 0,
	XK_odiaeresis, XK_Odiaeresis, 0, 0,
	XK_adiaeresis, XK_Adiaeresis, 0, 0,	/* 40 */
	XK_asciicircum, XK_degree, 0, 0,
	XK_Shift_L, 0, 0, 0,
	XK_numbersign, XK_apostrophe, 0, 0,
	XK_y, XK_Y, 0, 0,
	XK_x, XK_X, 0, 0,
	XK_c, XK_C, 0, 0,
	XK_v, XK_V, 0, 0,
	XK_b, XK_B, 0, 0,
	XK_n, XK_N, 0, 0,
	XK_m, XK_M, XK_mu, 0,			/* 50 */
	XK_comma, XK_semicolon, 0, 0,
	XK_period, XK_colon, 0, 0,
	XK_minus, XK_underscore, 0, 0,
	XK_Shift_R, 0, 0, 0,
	XK_KP_Multiply, 0, 0, 0,
	XK_Alt_L, 0, 0, 0,
	XK_space, 0, 0, 0,
	XK_Caps_Lock , 0, 0, 0,
	XK_F1, 0, 0, 0,
	XK_F2, 0, 0, 0,			/* 60 */
	XK_F3, 0, 0, 0,
	XK_F4, 0, 0, 0,
	XK_F5, 0, 0, 0,
	XK_F6, 0, 0, 0,
	XK_F7, 0, 0, 0,
	XK_F8, 0, 0, 0,
	XK_F9, 0, 0, 0,
	XK_F10, 0, 0, 0,
	XK_Num_Lock, 0, 0, 0,
	XK_Scroll_Lock, 0, 0, 0,	/* 70 */
	XK_KP_7, 0, 0, 0,
	XK_KP_8, 0, 0, 0,
	XK_KP_9, 0, 0, 0,
	XK_KP_Subtract, 0, 0, 0,
	XK_KP_4, 0, 0, 0,
	XK_KP_5, 0, 0, 0,
	XK_KP_6, 0, 0, 0,
	XK_KP_Add, 0, 0, 0,
	XK_KP_1, 0, 0, 0,
	XK_KP_2, 0, 0, 0,		/* 80 */
	XK_KP_3, 0, 0, 0,
	XK_KP_0, 0, 0, 0,
	XK_KP_Separator, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	XK_less, XK_greater, XK_bar, 0,
	XK_F11, 0, 0, 0,
	XK_F12, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,				/* 90 */
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	XK_KP_Enter, 0, 0, 0,
	XK_Control_R, 0, 0, 0,
	XK_KP_Divide, 0, 0, 0,
	XK_Print, 0, 0, 0,
	0 /* XK_Alt_R */, 0, 0, 0,		/* 100 */
	XK_Break, 0, 0, 0,
	XK_Home, 0, 0, 0,
	XK_Up, 0, 0, 0,
	XK_Prior, 0, 0, 0,
	XK_Left, 0, 0, 0,
	XK_Right, 0, 0, 0,
	XK_End, 0, 0, 0,
	XK_Down, 0, 0, 0,
	XK_Next, 0, 0, 0,
	XK_Insert, 0, 0, 0,		/* 110 */
	XK_Delete, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0,
	XK_Pause, 0, 0, 0,
};
#endif	

static struct hbtn
hbtn_dbox = {
	KEY_LEFTSHIFT, KEY_RIGHTSHIFT, KEY_RIGHTALT,
	230, 231, 232,
	233, 234,
};

void
init_keyboard(void)
{
#ifdef HAVE_DBOX_HARDWARE
		fbvnc_keymap = fbvnc_keymap_dbox;
#endif
		hbtn = hbtn_dbox;
	memset(btn_state,0 , N_SCANCODE);
}

int rep_key = 0;

char btn_state[N_SCANCODE];
int fn_action = 0;
int pan_toggle_count = 0;

static bool
repeating_key(int key)
{
	if (key == XK_Shift_L
	 || key == XK_Shift_R
	 || key == XK_Control_L 
	 || key == XK_Control_R
	 || key == XK_Alt_L 
	 || key == XK_Alt_R
	 || key == XK_Scroll_Lock 
	 || key == XK_Num_Lock
	 || key == XK_Caps_Lock
	) {
	 	return 0;
	} else {
	 	return 1;
	}
}

bool
key_special_action(int key)
{
	dprintf("fn_action=%d, btn_state[hbtn.action]=%d\n",
			fn_action, btn_state[hbtn.action]);
	if (! (fn_action || btn_state[hbtn.action])) return 0;

	if (key==XK_bracketleft || key==XK_Next) {
		ev_zoom_out(0, 0);
	} else if (key==XK_bracketright || key==XK_Prior) {
		ev_zoom_in(0, 0);
	} else if (key==XK_Escape) {
		system("zsuspend");
	} else if (key==XK_Return) {
		toggle_light();
	} else if (key==XK_f) {
		extern void refresh_framerate(void);
		refresh_framerate();
	} else if (key==XK_Down) {
		grid_pan(0, 1);
	} else if (key==XK_Up) {
		grid_pan(0, -1);
	} else if (key==XK_Left) {
		grid_pan(-1, 0);
	} else if (key==XK_Right) {
		grid_pan(1, 0);
	} else if (key==XK_Q) {
		cleanup_and_exit("Bye.", EXIT_OK);
	} else if (key==XK_z && img_saved) {
		show_pnm_image();
	} else if (key==XK_Caps_Lock) {
		caps_lock = !caps_lock;
	}
	return 1;
}

int  /* keysym */
key_map(int hwkey)
{
#ifdef HAVE_DREAMBOX_HARDWARE
	fn_action = 0;
	return hwkey;
#else
	int pos;
	int key;

	if (hwkey < 0 || hwkey >= N_SCANCODE) return 0;
	pos=hwkey*4;
	if (!fbvnc_keymap[pos]) return 0;
	
	if (btn_state[hbtn.altgr]) {
		if (fbvnc_keymap[pos+2]) {
			pos+=2;
		} else {
			fn_action = 1;
		}
	}
	if (btn_state[hbtn.shift_l] || btn_state[hbtn.shift_r] || caps_lock) {
		if (fbvnc_keymap[pos+1]) pos++;
	}
	key = fbvnc_keymap[pos];

	return key;
#endif
}

void
key_press(int hwkey) {
	int key;

	fn_action = 0;
	key = key_map(hwkey);
	if (!key) return;

	if (key_special_action(key)) return;

	if (repeating_key(key)) {
		rep_key = key;
		dprintf("Adding Keyrepeat\n");
		schedule_add(sched, kbdDelay, FBVNC_EVENT_KEYREPEAT);
	}

#ifdef HAVE_DBOX_HARDWARE
	keys_sent_map[hwkey] = key;
#endif
	dprintf("key_press: hwkey=%d, keysym=%d\n", hwkey, key);
	SendKeyEvent(key, 1);
}

void
key_release(int hwkey)
{
	int key;

	if (fn_action) return;
#ifdef HAVE_DBOX_HARDWARE
	if (hwkey < 0 || hwkey >= N_SCANCODE) return;

	key = keys_sent_map[hwkey];
#else
	key = hwkey;
#endif
	dprintf("key_release: hwkey=%d, keysym=%d\n", hwkey, key);
	SendKeyEvent(key, 0);

	pan_toggle_count = 0;
}

