#ifndef __info_h
#define __info_h

/*

							gtxID
				mID	feID	fpID	enxID	hwREV	fpREV
		sagem sat	3 	00	52	3	21	0.23
		sagem kabel	3	00	52	3	41	0.23
		nokia kabel 	1	7a	5a	0b	X5	0.81
																					X5							avia 500
		nokia sat	1	dd	5a	0b	09	0.81
		philips sat	2	00	52	3	01	0.30
*/

#define DBOX_FE_CABLE		0
#define DBOX_FE_SAT		1

#define DBOX_DEMOD_VES1820	0
#define DBOX_DEMOD_VES1893	1
#define DBOX_DEMOD_AT76C651	2
#define DBOX_DEMOD_VES1993	3
#define DBOX_DEMOD_TDA8044H	4

#define DBOX_MID_NOKIA		1
#define DBOX_MID_PHILIPS	2
#define DBOX_MID_SAGEM		3

struct dbox_info_struct
{
	int mID, feID, fpID, enxID, gtxID, hwREV, fpREV, avia;
	unsigned char dsID[8];
	
	int fe, demod;
};

#ifdef __KERNEL__
extern int dbox_get_info(struct dbox_info_struct *info);
extern int dbox_get_info_ptr(struct dbox_info_struct **info);
#endif

#endif
