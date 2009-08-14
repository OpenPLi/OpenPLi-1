#ifndef __FP_H
#define __FP_H

#define FP_IOCTL_GETID          0
#define FP_IOCTL_POWEROFF       1
#define FP_IOCTL_REBOOT         2
#define FP_IOCTL_LCD_DIMM		3
#define FP_IOCTL_LED			4
#define FP_IOCTL_GET_WAKEUP_TIMER		5
#define FP_IOCTL_SET_WAKEUP_TIMER		6
#define FP_IOCTL_GET_VCR		7
#define FP_IOCTL_GET_REGISTER		8
#define FP_IOCTL_IS_WAKEUP		9

#define RC_IOCTL_BCODES 	0	

#define P_HOR           0
#define P_VERT          1

#define T_UNKNOWN       0
#define T_QAM           1
#define T_QPSK          2

#ifdef __KERNEL__

#define FP_MAJOR        60
#define FP_MINOR        0
#define RC_MINOR        1

int fp_set_tuner_dword(int type, u32 tw);
int fp_set_sec(int power,int tone);
int fp_do_reset(int type);
int fp_cam_reset(void);
int fp_send_diseqc(int style, u8 *cmd,unsigned int len);
int fp_sec_status(void);
int fp_sagem_set_SECpower(int power,int tone);

#endif
#endif
