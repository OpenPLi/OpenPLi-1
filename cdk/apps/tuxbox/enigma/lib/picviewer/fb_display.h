#ifndef __pictureviewer_fbdisplay__
#define __pictureviewer_fbdisplay__

///// YUV framebuffer stuff
#define BPP_RED   8
#define BPP_GREEN 8
#define BPP_BLUE  8

#define NR_RED   (1<<(BPP_RED))
#define NR_GREEN (1<<(BPP_GREEN))
#define NR_BLUE  (1<<(BPP_BLUE))

#define lut_r_y (lut + 0 * (NR_RED + NR_GREEN + NR_BLUE) + 0)
#define lut_g_y (lut + 0 * (NR_RED + NR_GREEN + NR_BLUE) + NR_RED)
#define lut_b_y (lut + 0 * (NR_RED + NR_GREEN + NR_BLUE) + NR_RED + NR_GREEN)
#define lut_r_u (lut + 1 * (NR_RED + NR_GREEN + NR_BLUE) + 0)
#define lut_g_u (lut + 1 * (NR_RED + NR_GREEN + NR_BLUE) + NR_RED)
#define lut_b_u (lut + 1 * (NR_RED + NR_GREEN + NR_BLUE) + NR_RED + NR_GREEN)
#define lut_r_v (lut + 2 * (NR_RED + NR_GREEN + NR_BLUE) + 0)
#define lut_g_v (lut + 2 * (NR_RED + NR_GREEN + NR_BLUE) + NR_RED)
#define lut_b_v (lut + 2 * (NR_RED + NR_GREEN + NR_BLUE) + NR_RED + NR_GREEN)
extern unsigned short lut[3 * (NR_RED + NR_GREEN + NR_BLUE) ];

inline int Y(int R, int G, int B)
{
	return 66 * R + 129 * G + 25 * B;
}

inline int U(int R, int G, int B)
{
	return -38 * R - 74 * G + 112 * B;
}

inline int V(int R, int G, int B)
{
	return 112 * R - 94 * G - 18 * B;
}
void * convertRGB2FB(unsigned char *rgbbuff, unsigned long count, int bpp, int *cpp);
extern void fb_display(unsigned char *rgbbuff, int x_size, int y_size, int x_pan, int y_pan, int x_offs, int y_offs, int winx, int winy);
extern void getCurrentRes(int *x, int *y);
extern void clearFB(int bpp, int cpp, int trans);
#endif

