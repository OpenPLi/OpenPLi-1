#ifndef __FB_H__
#define __FB_H__

typedef struct {

	uint32_t sx;	/* screen-relative */
	uint32_t sy;
	uint32_t width;
	uint32_t height;
	uint32_t dx;
	uint32_t dy;
	
} fb_copyarea;

#define AVIA_GT_GV_SET_BLEV	0	/* blend level */
#define AVIA_GT_GV_SET_POS	1	/* position of graphics frame */
#define AVIA_GT_GV_HIDE		2	/* hide framebuffer */
#define AVIA_GT_GV_SHOW		3	/* show framebuffer */
#define AVIA_GT_GV_COPYAREA	4	/* copy area */

#endif /* __FB_H__ */
