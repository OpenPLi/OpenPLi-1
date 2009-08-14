#ifndef _KEYBOARD_H
#define _KEYBOARD_H

struct hbtn {
        int shift_l;
        int shift_r;
        int altgr;
        int mouse1;
        int mouse2;
        int mouse3;
        int pan;
        int action;
};
#ifdef HAVE_DREAMBOX_HARDWARE
#define MAXKEYS 19
static int fbvnc_keymap_dreambox[MAXKEYS*4] = {
	0x41, 0x00, 0x00,XK_Up       ,
	0x42, 0x00, 0x00,XK_Down     ,
	0x43, 0x00, 0x00,XK_Right    ,
	0x44, 0x00, 0x00,XK_Left     ,
	0x33, 0x7e, 0x00,XK_Delete   ,
	0x32, 0x7e, 0x00,XK_Insert   ,
	0x35, 0x7e, 0x00,XK_Page_Up  ,
	0x36, 0x7e, 0x00,XK_Page_Down,
	0x5b, 0x41, 0x00,XK_F1       ,
	0x5b, 0x42, 0x00,XK_F2       ,
	0x5b, 0x43, 0x00,XK_F3       ,
	0x5b, 0x44, 0x00,XK_F4       ,
	0x5b, 0x45, 0x00,XK_F5       ,
	0x31, 0x37, 0x7e,XK_F6       ,
	0x31, 0x38, 0x7e,XK_F7       ,
	0x31, 0x39, 0x7e,XK_F8       ,
	0x32, 0x30, 0x7e,XK_F9       ,
	0x32, 0x31, 0x7e,XK_F10      ,
	0x32, 0x33, 0x7e,XK_F11
};
#endif
#define N_SCANCODE 256
extern int *fbvnc_keymap;
extern struct hbtn hbtn;
extern bool img_saved;
extern bool fn_action;
extern int pan_toggle_count;
extern int rep_key;
extern char btn_state[N_SCANCODE];

extern void init_keyboard(void);
extern bool key_special_action(int key);
extern int /* keysym */ key_map(int hwkey);
extern void key_press(int hwkey);
extern void key_release(int hwkey);

#endif
