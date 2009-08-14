#ifndef __GTX_H__
#define __GTX_H__

#undef CR0
#undef CR1

#define GTX_REG_BASE	0x08400000
#define GTX_REG_SIZE	0x00003000
#define GTX_MEM_BASE	0x08000000
#define GTX_MEM_SIZE	0x00200000
#define GTX_FB_OFFSET	0x0100000
#define GTX_INTERRUPT	SIU_IRQ1

#define AVIA_GT_GTX_IR_CLOCK	40500000

/* Graphics */
#define GTX_REG_GMR	0x000
#define GTX_REG_CLTA	0x004
#define GTX_REG_CLTD	0x006
#define GTX_REG_TCR	0x008
#define GTX_REG_CCR	0x00A
#define GTX_REG_GVSA	0x00C
#define GTX_REG_GVP	0x010
#define GTX_REG_GVS	0x014
#define GTX_REG_CSA	0x018
#define GTX_REG_CPOS	0x01C
#define GTX_REG_GFUNC	0x020

/* SAR */
#define GTX_REG_TRP	0x040
#define GTX_REG_TRW	0x044
#define GTX_REG_TRL	0x046
#define GTX_REG_RRP	0x048
#define GTX_REG_RRW	0x04C
#define GTX_REG_RRL	0x04E
#define GTX_REG_RVCA	0x050
#define GTX_REG_RVCD	0x052
#define GTX_REG_STXC	0x054
#define GTX_REG_SRXC	0x056

/* Smart Card */
#define GTX_REG_TMA	0x060
#define GTX_REG_RMA	0x064
#define GTX_REG_SCS	0x068
#define GTX_REG_TMS	0x06A
#define GTX_REG_RMS	0x06C
#define GTX_REG_SCC	0x06E
#define GTX_REG_CWI	0x070
#define GTX_REG_ETU	0x072
#define GTX_REG_GDT	0x074
#define GTX_REG_SCON	0x076

/* Interrupt */
#define GTX_REG_ISR0	0x080
#define GTX_REG_ISR1	0x082
#define GTX_REG_ISR2	0x084
#define GTX_REG_IMR0	0x086
#define GTX_REG_IMR1	0x088
#define GTX_REG_IMR2	0x08A
#define GTX_REG_IPR0	0x08C
#define GTX_REG_IPR1	0x08E
#define GTX_REG_IPR2	0x090
#define GTX_REG_ISR3	0x092
#define GTX_REG_IMR3	0x094
#define GTX_REG_IPR3	0x096
#define GTX_REG_ICC	0x09C
#define GTX_REG_DCC	0x09E

/* CRC */
#define GTX_REG_CRCC	0x0A0
#define GTX_REG_TCRC	0x0A4
#define GTX_REG_RCRC	0x0A8

/* Blitter */
#define GTX_REG_BDST	0x0B0
#define GTX_REG_BMR	0x0B4
#define GTX_REG_BDR	0x0B6
#define GTX_REG_BCLR	0x0B8
#define GTX_REG_BPW	0x0BC
#define GTX_REG_BPO	0x0BE

/* SPI */
#define GTX_REG_SPID	0x0D0
#define GTX_REG_SPIC	0x0D2

/* PCM Audio */
#define GTX_REG_PCMA	0x0E0
#define GTX_REG_PCMN	0x0E4
#define GTX_REG_PCMC	0x0E8
#define GTX_REG_PCMD	0x0EC

/* Video */
#define GTX_REG_VBR	0x0F0
#define GTX_REG_VCR	0x0F4
#define GTX_REG_VLC	0x0F6
#define GTX_REG_VLI1	0x0F8
#define GTX_REG_VHT	0x0FA
#define GTX_REG_VLT	0x0FC
#define GTX_REG_VLI2	0x0FE

/* Configuration and Control */
#define GTX_REG_RR0	0x100
#define GTX_REG_RR1	0x102
#define GTX_REG_CR0	0x104
#define GTX_REG_CR1	0x106
#define GTX_REG_C0CR	0x10C
#define GTX_REG_C1CR	0x10E

/* DAC */
#define GTX_REG_DPCR	0x110
#define GTX_REG_DPR	0x112

/* Framer */
#define GTX_REG_PCRPID	0x120
#define GTX_REG_PCR2	0x122
#define GTX_REG_PCR1	0x124
#define GTX_REG_PCR0	0x126
#define GTX_REG_LSTC2	0x128
#define GTX_REG_LSTC1	0x12A
#define GTX_REG_LSTC0	0x12C
#define GTX_REG_STCC2	0x12E
#define GTX_REG_STCC1	0x130
#define GTX_REG_STCC0	0x132
#define GTX_REG_FCR	0x134
#define GTX_REG_SYNCH	0x136
#define GTX_REG_PFIFO	0x138

/* IDC Interface */
#define GTX_REG_IDCCR	0x140
#define GTX_REG_IDCSR	0x142
#define GTX_REG_IDCSA	0x144
#define GTX_REG_IDCRD	0x146
#define GTX_REG_IDCMA	0x148
#define GTX_REG_IDCTD	0x14A
#define GTX_REG_IDCC	0x14C
#define GTX_REG_IDCFF	0x14E

/* Audio/Video Decoder Interface */
#define GTX_REG_AVI	0x150

/* RISC Engine */
#define GTX_REG_RISCPC	0x170
#define GTX_REG_RISCCON	0x178

/* Queue Write Pointer */
#define GTX_REG_QWPnL	0x180
#define GTX_REG_QWPnH	0x182
#define GTX_REG_QWP0L	0x180
#define GTX_REG_QWP0H	0x182
#define GTX_REG_QWP1L	0x184
#define GTX_REG_QWP1H	0x186
#define GTX_REG_QWP2L	0x188
#define GTX_REG_QWP2H	0x18A
#define GTX_REG_QWP3L	0x18C
#define GTX_REG_QWP3H	0x18E
#define GTX_REG_QWP4L	0x190
#define GTX_REG_QWP4H	0x192
#define GTX_REG_QWP5L	0x194
#define GTX_REG_QWP5H	0x196
#define GTX_REG_QWP6L	0x198
#define GTX_REG_QWP6H	0x19A
#define GTX_REG_QWP7L	0x19C
#define GTX_REG_QWP7H	0x19E
#define GTX_REG_QWP8L	0x1A0
#define GTX_REG_QWP8H	0x1A2
#define GTX_REG_QWP9L	0x1A4
#define GTX_REG_QWP9H	0x1A6
#define GTX_REG_QWP10L	0x1A8
#define GTX_REG_QWP10H	0x1AA
#define GTX_REG_QWP11L	0x1AC
#define GTX_REG_QWP11H	0x1AE
#define GTX_REG_QWP12L	0x1B0
#define GTX_REG_QWP12H	0x1B2
#define GTX_REG_QWP13L	0x1B4
#define GTX_REG_QWP13H	0x1B6
#define GTX_REG_QWP14L	0x1B8
#define GTX_REG_QWP14H	0x1BA
#define GTX_REG_QWP15L	0x1BC
#define GTX_REG_QWP15H	0x1BE

/* Queue Interrupt */
#define GTX_REG_QIn	0x1C0
#define GTX_REG_QI0	0x1C0
#define GTX_REG_QI1	0x1C2
#define GTX_REG_QI2	0x1C4
#define GTX_REG_QI3	0x1C6
#define GTX_REG_QI4	0x1C8
#define GTX_REG_QI5	0x1CA
#define GTX_REG_QI6	0x1CC
#define GTX_REG_QI7	0x1CE
#define GTX_REG_QI8	0x1D0
#define GTX_REG_QI9	0x1D2
#define GTX_REG_QI10	0x1D4
#define GTX_REG_QI11	0x1D6
#define GTX_REG_QI12	0x1D8
#define GTX_REG_QI13	0x1DA
#define GTX_REG_QI14	0x1DC
#define GTX_REG_QI15	0x1DE

/* Audio Queue Manager */
#define GTX_REG_AQRPL	0x1E0
#define GTX_REG_AQRPH	0x1E2
#define GTX_REG_AQWPL	0x1E4
#define GTX_REG_AQWPH	0x1E6

/* Teletext Queue Manager */
#define GTX_REG_TQRPL	0x1E8
#define GTX_REG_TQRPH	0x1EA
#define GTX_REG_TQWPL	0x1EC
#define GTX_REG_TQWPH	0x1EE

/* Video Queue Manager */
#define GTX_REG_VQRPL	0x1F0
#define GTX_REG_VQRPH	0x1F2
#define GTX_REG_VQWPL	0x1F4
#define GTX_REG_VQWPH	0x1F6

/* Copy Engine */
#define GTX_REG_CBWn	0x200
#define GTX_REG_CBW0	0x200
#define GTX_REG_CBW1	0x202
#define GTX_REG_CBW2	0x204
#define GTX_REG_CBW3	0x206
#define GTX_REG_CBW4	0x208
#define GTX_REG_CBW5	0x20A
#define GTX_REG_CBW6	0x20C
#define GTX_REG_CBW7	0x20E
#define GTX_REG_CCSA	0x220
#define GTX_REG_CDA	0x224
#define GTX_REG_CCOM	0x228
#define GTX_REG_RWTC	0x22A
#define GTX_REG_CCOM2	0x22C
#define GTX_REG_CCOM3	0x22E

/* Video Plane Display */
#define GTX_REG_VPSA	0x240
#define GTX_REG_VPO	0x244
#define GTX_REG_VPP	0x248
#define GTX_REG_VPS	0x24C
#define GTX_REG_VPOE	0x250

/* Video Capture */
#define GTX_REG_VCSA	0x260
#define GTX_REG_VCSP	0x264
#define GTX_REG_VCS	0x268

/* Semaphore */
#define GTX_REG_SEM1	0x270
#define GTX_REG_SEM2	0x272

/* Teletext */
#define GTX_REG_PTS0	0x280
#define GTX_REG_PTS1	0x282
#define GTX_REG_PTSO	0x284
#define GTX_REG_TTCR	0x286
#define GTX_REG_TSR	0x288

/* Infrared */
#define GTX_REG_CWP	0x2A0
#define GTX_REG_CWPH	0x2A2
#define GTX_REG_MSPR	0x2A4
#define GTX_REG_MSPL	0x2A6
#define GTX_REG_RTC	0x2A8
#define GTX_REG_RTP	0x2AA
#define GTX_REG_RFR	0x2AC
#define GTX_REG_RPH	0x2AE
#define GTX_REG_IRQA	0x2B0
#define GTX_REG_IRRE	0x2B4
#define GTX_REG_IRTE	0x2B6
#define GTX_REG_IRRO	0x2B8
#define GTX_REG_IRTO	0x2BA

#define GTX_REG_RISC	0x1000

#define GTX_IRQ_REG_ISR0	0
#define GTX_IRQ_REG_ISR1	1
#define GTX_IRQ_REG_ISR2	2
#define GTX_IRQ_REG_ISR3	3

#define GTX_IRQ_PCR			AVIA_GT_IRQ(GTX_IRQ_REG_ISR0, 8)
#define GTX_IRQ_IR_TX		AVIA_GT_IRQ(GTX_IRQ_REG_ISR1, 8)
#define GTX_IRQ_IR_RX		AVIA_GT_IRQ(GTX_IRQ_REG_ISR1, 9)
#define GTX_IRQ_PCM_PF		AVIA_GT_IRQ(GTX_IRQ_REG_ISR1, 10)
#define GTX_IRQ_VL0			AVIA_GT_IRQ(GTX_IRQ_REG_ISR1, 11)
#define GTX_IRQ_PCM_AD		AVIA_GT_IRQ(GTX_IRQ_REG_ISR1, 12)
#define GTX_IRQ_VL1			AVIA_GT_IRQ(GTX_IRQ_REG_ISR1, 13)
#define GTX_IRQ_TT			AVIA_GT_IRQ(GTX_IRQ_REG_ISR1, 15)

#pragma pack(1)

/* Graphics */
typedef struct {

	unsigned char GMD: 2;
	unsigned char L: 1;
	unsigned char F: 1;
	unsigned char C: 1;
	unsigned char I: 1;
	unsigned char CFT: 2;
	unsigned char BLEV1: 4;
	unsigned char BLEV0: 4;
	unsigned char Reserved1: 5;
	unsigned short STRIDE: 10;
	unsigned char Reserved2: 1;

} sGTX_REG_GMR;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char Addr: 8;

} sGTX_REG_CLTA;

typedef struct {

	unsigned char T: 1;
	unsigned char R: 5;
	unsigned char G: 5;
	unsigned char B: 5;

} sGTX_REG_CLTD;

typedef struct {

	unsigned char E: 1;
	unsigned char R: 5;
	unsigned char G: 5;
	unsigned char B: 5;

} sGTX_REG_TCR;

typedef struct {

	unsigned char Reserved1: 1;
	unsigned char R: 5;
	unsigned char G: 5;
	unsigned char B: 5;

} sGTX_REG_CCR;

typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Addr: 21;
	unsigned char Reserved2: 1;

} sGTX_REG_GVSA;

typedef struct {

	unsigned char SPP: 5;
	unsigned char Reserved1: 1;
	unsigned short XPOS: 10;
	unsigned char Reserved2: 6;
	unsigned short YPOS: 10;

} sGTX_REG_GVP;

typedef struct {

	unsigned char IPS: 5;
	unsigned char Reserved1: 1;
	unsigned short XSZ: 10;
	unsigned char Reserved2: 6;
	unsigned short YSZ: 10;

} sGTX_REG_GVS;

typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Addr: 21;
	unsigned char Reserved2: 1;

} sGTX_REG_CSA;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned short XPOS: 10;
	unsigned char Reserved2: 6;
	unsigned short YPOS: 10;

} sGTX_REG_CPOS;

typedef struct {

	unsigned short Reserved1: 11;
	unsigned char D: 1;
	unsigned char Bank: 4;

} sGTX_REG_GFUNC;



/* SAR */
typedef struct {

	unsigned short Reserved1: 10;
	unsigned short Base: 12;
	unsigned char Index: 8;
	unsigned char Reserved2: 2;

} sGTX_REG_TRP;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned char Limit: 8;
	unsigned char Reserved2: 1;
	unsigned char D: 1;

} sGTX_REG_TRW;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned char Limit: 8;
	unsigned char Reserved2: 1;
	unsigned char D: 1;

} sGTX_REG_TRL;

typedef struct {

	unsigned short Reserved1: 10;
	unsigned short Base: 10;
	unsigned short Index: 10;
	unsigned char Reserved2: 2;

} sGTX_REG_RRP;

typedef struct {

	unsigned char Reserved1: 4;
	unsigned short Limit: 10;
	unsigned char Reserved2: 1;
	unsigned char D: 1;

} sGTX_REG_RRW;

typedef struct {

	unsigned char Reserved1: 4;
	unsigned short Limit: 10;
	unsigned char Reserved2: 1;
	unsigned char D: 1;

} sGTX_REG_RRL;

typedef struct {

	unsigned short Reserved1: 12;
	unsigned char Addr: 4;

} sGTX_REG_RVCA;

typedef struct {

	unsigned short Entry: 16;

} sGTX_REG_RVCD;

typedef struct {

	unsigned short Reserved1: 14;
	unsigned char TXC: 2;

} sGTX_REG_STXC;

typedef struct {

	unsigned short Reserved1: 14;
	unsigned char RXC: 2;

} sGTX_REG_SRXC;



/* Smart Card */
typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Transmit_Data_DRAM_Address: 21;
	unsigned char Reserved2: 1;

} sGTX_REG_TMA;

typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Receive_Data_DRAM_Address: 21;
	unsigned char Reserved2: 1;

} sGTX_REG_RMA;

typedef struct {

	unsigned short SmartCardStatus: 16;

} sGTX_REG_SCS;

typedef struct {

	unsigned char Reserved1: 7;
	unsigned short MessageSize: 9;

} sGTX_REG_TMS;

typedef struct {

	unsigned char Reserved1: 7;
	unsigned short MessageSize: 9;

} sGTX_REG_RMS;

typedef struct {

	unsigned char SMR: 1;
	unsigned char RD: 1;
	unsigned char RL: 1;
	unsigned char RST: 1;
	unsigned char COE: 1;
	unsigned char CD: 1;
	unsigned char VCC: 1;
	unsigned char VPL: 1;
	unsigned char VPE: 1;
	unsigned char CRC: 1;
	unsigned char IE: 1;
	unsigned char PT: 1;
	unsigned char TE: 1;
	unsigned char RE: 1;
	unsigned char PTS: 1;
	unsigned char SAR: 1;

} sGTX_REG_SCC;

typedef struct {

	unsigned short Reserved1: 12;
	unsigned char CWI: 4;

} sGTX_REG_CWI;

typedef struct {

	unsigned char Reserved1: 1;
	unsigned char CP: 1;
	unsigned char C: 1;
	unsigned char EP: 1;
	unsigned short ETU: 12;

} sGTX_REG_ETU;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char GuardTime: 8;

} sGTX_REG_GDT;

typedef struct {

	unsigned short Reserved1: 14;
	unsigned char COD: 1;
	unsigned char SCD: 1;

} sGTX_REG_SCON;



/* Interrupt */
typedef struct {

	unsigned char Reserved1: 6;
	unsigned char IDC: 1;
	unsigned char PCR: 1;
	unsigned char DROP: 1;
	unsigned char LOCK: 1;
	unsigned char WT: 1;
	unsigned char NR: 1;
	unsigned char PO: 1;
	unsigned char MC: 1;
	unsigned char CD: 1;
	unsigned char PE: 1;

} sGTX_REG_ISR0;

typedef struct {

	unsigned char TT: 1;
	unsigned char CD: 1;
	unsigned char VL1: 1;
	unsigned char AD: 1;
	unsigned char VL0: 1;
	unsigned char PF: 1;
	unsigned char IR: 1;
	unsigned char IT: 1;
	unsigned char RL: 1;
	unsigned char RW: 1;
	unsigned char RE: 1;
	unsigned char RC: 1;
	unsigned char TL: 1;
	unsigned char TW: 1;
	unsigned char TE: 1;
	unsigned char TC: 1;

} sGTX_REG_ISR1;

typedef struct {

	unsigned char Q0: 1;
	unsigned char Q1: 1;
	unsigned char Q2: 1;
	unsigned char Q3: 1;
	unsigned char Q4: 1;
	unsigned char Q5: 1;
	unsigned char Q6: 1;
	unsigned char Q7: 1;
	unsigned char Q8: 1;
	unsigned char Q9: 1;
	unsigned char Q10: 1;
	unsigned char Q11: 1;
	unsigned char Q12: 1;
	unsigned char Q13: 1;
	unsigned char Q14: 1;
	unsigned char Q15: 1;

} sGTX_REG_ISR2;

typedef struct {

	unsigned char Q16: 1;
	unsigned char Q17: 1;
	unsigned char Q18: 1;
	unsigned char Q19: 1;
	unsigned char Q20: 1;
	unsigned char Q21: 1;
	unsigned char Q22: 1;
	unsigned char Q23: 1;
	unsigned char Q24: 1;
	unsigned char Q25: 1;
	unsigned char Q26: 1;
	unsigned char Q27: 1;
	unsigned char Q28: 1;
	unsigned char Q29: 1;
	unsigned char Q30: 1;
	unsigned char Q31: 1;

} sGTX_REG_ISR3;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned char IDC: 1;
	unsigned char PCR: 1;
	unsigned char DROP: 1;
	unsigned char LOCK: 1;
	unsigned char WT: 1;
	unsigned char NR: 1;
	unsigned char PO: 1;
	unsigned char MC: 1;
	unsigned char CD: 1;
	unsigned char PE: 1;

} sGTX_REG_IMR0;

typedef struct {

	unsigned char TT: 1;
	unsigned char CD: 1;
	unsigned char VL1: 1;
	unsigned char AD: 1;
	unsigned char VL0: 1;
	unsigned char PF: 1;
	unsigned char IR: 1;
	unsigned char IT: 1;
	unsigned char RL: 1;
	unsigned char RW: 1;
	unsigned char RE: 1;
	unsigned char RC: 1;
	unsigned char TL: 1;
	unsigned char TW: 1;
	unsigned char TE: 1;
	unsigned char TC: 1;

} sGTX_REG_IMR1;

typedef struct {

	unsigned char Q0: 1;
	unsigned char Q1: 1;
	unsigned char Q2: 1;
	unsigned char Q3: 1;
	unsigned char Q4: 1;
	unsigned char Q5: 1;
	unsigned char Q6: 1;
	unsigned char Q7: 1;
	unsigned char Q8: 1;
	unsigned char Q9: 1;
	unsigned char Q10: 1;
	unsigned char Q11: 1;
	unsigned char Q12: 1;
	unsigned char Q13: 1;
	unsigned char Q14: 1;
	unsigned char Q15: 1;

} sGTX_REG_IMR2;

typedef struct {

	unsigned char Q16: 1;
	unsigned char Q17: 1;
	unsigned char Q18: 1;
	unsigned char Q19: 1;
	unsigned char Q20: 1;
	unsigned char Q21: 1;
	unsigned char Q22: 1;
	unsigned char Q23: 1;
	unsigned char Q24: 1;
	unsigned char Q25: 1;
	unsigned char Q26: 1;
	unsigned char Q27: 1;
	unsigned char Q28: 1;
	unsigned char Q29: 1;
	unsigned char Q30: 1;
	unsigned char Q31: 1;

} sGTX_REG_IMR3;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned char IDC: 1;
	unsigned char PCR: 1;
	unsigned char DROP: 1;
	unsigned char LOCK: 1;
	unsigned char WT: 1;
	unsigned char NR: 1;
	unsigned char PO: 1;
	unsigned char MC: 1;
	unsigned char CD: 1;
	unsigned char PE: 1;

} sGTX_REG_IPR0;

typedef struct {

	unsigned char TT: 1;
	unsigned char CD: 1;
	unsigned char VL1: 1;
	unsigned char AD: 1;
	unsigned char VL0: 1;
	unsigned char PF: 1;
	unsigned char IR: 1;
	unsigned char IT: 1;
	unsigned char RL: 1;
	unsigned char RW: 1;
	unsigned char RE: 1;
	unsigned char RC: 1;
	unsigned char TL: 1;
	unsigned char TW: 1;
	unsigned char TE: 1;
	unsigned char TC: 1;

} sGTX_REG_IPR1;

typedef struct {

	unsigned char Q0: 1;
	unsigned char Q1: 1;
	unsigned char Q2: 1;
	unsigned char Q3: 1;
	unsigned char Q4: 1;
	unsigned char Q5: 1;
	unsigned char Q6: 1;
	unsigned char Q7: 1;
	unsigned char Q8: 1;
	unsigned char Q9: 1;
	unsigned char Q10: 1;
	unsigned char Q11: 1;
	unsigned char Q12: 1;
	unsigned char Q13: 1;
	unsigned char Q14: 1;
	unsigned char Q15: 1;

} sGTX_REG_IPR2;

typedef struct {

	unsigned char Q16: 1;
	unsigned char Q17: 1;
	unsigned char Q18: 1;
	unsigned char Q19: 1;
	unsigned char Q20: 1;
	unsigned char Q21: 1;
	unsigned char Q22: 1;
	unsigned char Q23: 1;
	unsigned char Q24: 1;
	unsigned char Q25: 1;
	unsigned char Q26: 1;
	unsigned char Q27: 1;
	unsigned char Q28: 1;
	unsigned char Q29: 1;
	unsigned char Q30: 1;
	unsigned char Q31: 1;

} sGTX_REG_IPR3;



/* CRC */
typedef struct {

	unsigned char CMD: 2;
	unsigned char Reserved1: 2;
	unsigned char LEN: 3;
	unsigned char L: 1;
	unsigned char Reserved2: 2;
	unsigned int Addr: 21;
	unsigned char F: 1;

} sGTX_REG_CRCC;

typedef struct {

	unsigned int CRC: 32;

} sGTX_REG_TCRC;

typedef struct {

	unsigned int CRC: 32;

} sGTX_REG_RCRC;



/* Blitter */
typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Addr: 18;
	unsigned char Count: 4;

} sGTX_REG_BDST;

typedef struct {

	unsigned short M: 16;

} sGTX_REG_BMR;

typedef struct {

	unsigned short D: 16;

} sGTX_REG_BDR;

typedef struct {

	unsigned char COLOR0: 8;
	unsigned char COLOR1: 8;
	unsigned char COLOR2: 8;
	unsigned char COLOR3: 8;

} sGTX_REG_BCLR;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char Width: 8;

} sGTX_REG_BPW;

typedef struct {

	unsigned char M: 1;
	unsigned char F: 1;
	unsigned char T: 1;
	unsigned char K: 1;
	unsigned char Reserved1: 8;
	unsigned char Offs: 4;

} sGTX_REG_BPO;



/* Infrared */
typedef struct {

	unsigned char Reserved1: 5;
	unsigned short CarrierWavePeriod: 11;

} sGTX_REG_CWP;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned short WavePulseHigh: 10;

} sGTX_REG_CWPH;

typedef struct {

	unsigned char Reserved1: 4;
	unsigned char P: 1;
	unsigned char E: 1;
	unsigned short MSP: 10;

} sGTX_REG_MSPR;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char PulseLowLen: 8;

} sGTX_REG_MSPL;

typedef struct {

	unsigned char Reserved1: 7;
	unsigned char S: 1;
	unsigned char RTC: 8;

} sGTX_REG_RTC;

typedef struct {

	unsigned char Reserved1: 3;
	unsigned short TickPeriod: 13;

} sGTX_REG_RTP;



/* SPI */
typedef struct {

	unsigned char DataMSB: 8;
	unsigned char DataLSB: 8;

} sGTX_REG_SPID;

typedef struct {

	unsigned char Reserved1: 2;
	unsigned char M: 1;
	unsigned char Speed: 5;
	unsigned char D: 1;
	unsigned char Reserved2: 4;
	unsigned char L: 1;
	unsigned char R: 1;
	unsigned char C: 1;

} sGTX_REG_SPIC;



/* PCM Audio */
typedef struct {

	unsigned short NSAMP: 10;
	unsigned int Addr: 21;
	unsigned char W: 1;

} sGTX_REG_PCMA;

typedef struct {

	unsigned char PCMAL: 7;
	unsigned char Reserved1: 1;
	unsigned char PCMAR: 7;
	unsigned char Reserved2: 1;
	unsigned char MPEGAL: 7;
	unsigned char Reserved3: 1;
	unsigned char MPEGAR: 7;
	unsigned char Reserved4: 1;

} sGTX_REG_PCMN;

typedef struct {

	unsigned char R: 2;
	unsigned char W: 1;
	unsigned char C: 1;
	unsigned char S: 1;
	unsigned char T: 1;
	unsigned char V: 1;
	unsigned char P: 1;
	unsigned char M: 1;
	unsigned char I: 1;
	unsigned char ADV: 2;
	unsigned char ACD: 2;
	unsigned char BCD: 2;

} sGTX_REG_PCMC;

typedef struct {

	unsigned short Reserved1: 10;
	unsigned int B: 21;
	unsigned char Reserved2: 1;

} sGTX_REG_PCMD;



/* Video */
typedef struct {

	unsigned char Reserved1: 7;
	unsigned char E: 1;
	unsigned char Y: 8;
	unsigned char Cr: 8;
	unsigned char Cb: 8;

} sGTX_REG_VBR;

typedef struct {

	unsigned char S: 1;
	unsigned char P: 1;
	unsigned char C: 1;
	unsigned char A: 1;
	unsigned char HP: 2;
	unsigned char FP: 2;
	unsigned char E: 1;
	unsigned char D: 1;
	unsigned char N: 1;
	unsigned char DELAY: 5;

} sGTX_REG_VCR;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned short LINE: 9;
	unsigned char F: 1;

} sGTX_REG_VLC;

typedef struct {

	unsigned char E: 1;
	unsigned char Reserved1: 5;
	unsigned short LINE: 9;
	unsigned char F: 1;

} sGTX_REG_VLI1;

typedef struct {

	unsigned char SD: 2;
	unsigned char Reserved1: 4;
	unsigned short Width: 10;

} sGTX_REG_VHT;

typedef struct {

	unsigned char VBI: 5;
	unsigned char Reserved1: 1;
	unsigned short Lines: 10;

} sGTX_REG_VLT;

typedef struct {

	unsigned char E: 1;
	unsigned char Reserved1: 5;
	unsigned short LINE: 9;
	unsigned char F: 1;

} sGTX_REG_VLI2;



/* Configuration and Control */
typedef struct {

	unsigned char PIG: 1;
	unsigned char VCAP: 1;
	unsigned char VID: 1;
	unsigned char ACLK: 1;
	unsigned char COPY: 1;
	unsigned char DRAM: 1;
	unsigned char PCM: 1;
	unsigned char SPI: 1;
	unsigned char IR: 1;
	unsigned char BLIT: 1;
	unsigned char CRC: 1;
	unsigned char INT: 1;
	unsigned char SCD: 1;
	unsigned char SRX: 1;
	unsigned char STX: 1;
	unsigned char GV: 1;

} sGTX_REG_RR0;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char TTX: 1;
	unsigned char DAC: 1;
	unsigned char RISC: 1;
	unsigned char FRMR: 1;
	unsigned char CHAN: 1;
	unsigned char AVD: 1;
	unsigned char IDC: 1;
	unsigned char DESC: 1;

} sGTX_REG_RR1;

typedef struct {

	unsigned char REV_ID: 4;
	unsigned char GOF: 1;
	unsigned char SOF: 1;
	unsigned char POF: 1;
	unsigned char WBD: 1;
	unsigned char DD1: 1;
	unsigned char DD0: 1;
	unsigned char DOD: 1;
	unsigned char SPI: 1;
	unsigned char _16M: 1;
	unsigned char RFD: 1;
	unsigned char MAP: 1;
	unsigned char RES: 1;

} sGTX_REG_CR0;

typedef struct {

	unsigned char BRD_ID: 8;
	unsigned char Reserved1: 3;
	unsigned char UPQ: 1;
	unsigned char TCP: 1;
	unsigned char FH: 1;
	unsigned char ACP: 1;
	unsigned char VCP: 1;

} sGTX_REG_CR1;

typedef struct {

	unsigned short Address: 12;
	unsigned char Size: 4;

} sGTX_REG_C0CR;

typedef struct {

	unsigned short Address: 12;
	unsigned char Size: 4;

} sGTX_REG_C1CR;



/* DAC */
typedef struct {

	unsigned short DAC_Pulse_Count: 16;
	unsigned short Reserved1: 11;
	unsigned char Prescale: 5;

} sGTX_REG_DPCR;



/* Framer */
typedef struct {

	unsigned char Reserved1: 2;
	unsigned char E: 1;
	unsigned short PID: 13;

} sGTX_REG_PCRPID;

typedef struct {

	unsigned short PCR_Base: 16;

} sGTX_REG_PCR2;

typedef struct {

	unsigned short PCR_Base: 16;

} sGTX_REG_PCR1;

typedef struct {

	unsigned char PCR_Base: 1;
	unsigned char Reserved1: 6;
	unsigned short PCR_Extension: 9;

} sGTX_REG_PCR0;

typedef struct {

	unsigned short Latched_STC_Base: 16;

} sGTX_REG_LSTC2;

typedef struct {

	unsigned short Latched_STC_Base: 16;

} sGTX_REG_LSTC1;

typedef struct {

	unsigned char Latched_STC_Base: 1;
	unsigned char Reserved1: 6;
	unsigned short Latched_STC_Extension: 9;

} sGTX_REG_LSTC0;

typedef struct {

	unsigned short STC_Count: 16;

} sGTX_REG_STCC2;

typedef struct {

	unsigned short STC_Count: 16;

} sGTX_REG_STCC1;

typedef struct {

	unsigned char STC_Count: 1;
	unsigned char Reserved1: 6;
	unsigned short STC_Extension: 9;

} sGTX_REG_STCC0;

typedef struct {

	unsigned char FE: 1;
	unsigned char FH: 1;
	unsigned char TEI: 1;
	unsigned char DM: 1;
	unsigned char CDP: 1;
	unsigned char CCP: 1;
	unsigned char DO: 1;
	unsigned char FD: 1;
	unsigned char SyncByte: 8;

} sGTX_REG_FCR;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char SyncDrop: 3;
	unsigned char SyncLock: 5;

} sGTX_REG_SYNCH;

typedef struct {

	unsigned short Reserved1: 9;
	unsigned char FIFO_Depth: 7;

} sGTX_REG_PFIFO;



/* IDC Interface */
typedef struct {

	unsigned char Reserved1: 2;
	unsigned char SN: 1;
	unsigned char VD: 1;
	unsigned char Byte2Rd: 4;
	unsigned char MM: 1;
	unsigned char LB: 1;
	unsigned char FR: 1;
	unsigned char FT: 1;
	unsigned char IE: 1;
	unsigned char RS: 1;
	unsigned char ME: 1;
	unsigned char SE: 1;

} sGTX_REG_IDCCR;

typedef struct {

	unsigned char LA: 4;
	unsigned char SN: 1;
	unsigned char TxE: 1;
	unsigned char SD: 1;
	unsigned char RS: 1;
	unsigned char SAd: 1;
	unsigned char MI: 1;
	unsigned char SI: 1;
	unsigned char NAK: 1;
	unsigned char AK: 1;
	unsigned char RxR: 1;
	unsigned char Gen: 1;
	unsigned char CSD: 1;

} sGTX_REG_IDCSR;

typedef struct {

	unsigned short Reserved1: 9;
	unsigned char SAddr: 7;

} sGTX_REG_IDCSA;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char RxData: 8;

} sGTX_REG_IDCRD;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char MAddr: 7;
	unsigned char RW: 1;

} sGTX_REG_IDCMA;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char TxData: 8;

} sGTX_REG_IDCTD;

typedef struct {

	unsigned short Reserved1: 9;
	unsigned char SCL: 7;

} sGTX_REG_IDCC;

typedef struct {

	unsigned char Reserved1: 4;
	unsigned char Byte2Rd: 4;
	unsigned char RxCNT: 4;
	unsigned char TxCNT: 4;

} sGTX_REG_IDCFF;



/* Audio/Video Decoder Interface */
typedef struct {

	unsigned char HEN: 1;
	unsigned char HAA: 1;
	unsigned char HWT: 1;
	unsigned char VRQ: 1;
	unsigned char ARQ: 1;
	unsigned char VEN: 1;
	unsigned char AEN: 1;
	unsigned char INV: 1;
	unsigned char SerRT_Max: 2;
	unsigned char ParRT_Max: 2;
	unsigned char Audio_Xfer_Rate: 4;
	unsigned short Reserved1: 11;
	unsigned char H: 1;
	unsigned char AS: 1;
	unsigned char AP: 1;
	unsigned char VS: 1;
	unsigned char VP: 1;

} sGTX_REG_AVI;



/* RISC Engine */
typedef struct {

	unsigned char Reserved1: 7;
	unsigned short PC: 9;

} sGTX_REG_RISCPC;

typedef struct {

	unsigned short Reserved1: 14;
	unsigned char SP: 1;
	unsigned char SS: 1;

} sGTX_REG_RISCCON;



/* Queue Write Pointer */
typedef struct {

	unsigned char Reserved1: 6;
	unsigned char Q_Size: 4;
	unsigned char Upper_WD_n: 6;

} sGTX_REG_QWPnH;

typedef struct {

	unsigned short Queue_n_Write_Pointer: 16;

} sGTX_REG_QWPnL;

typedef struct {

	unsigned char QIM: 1;
	unsigned char InterruptMatchingAddress: 5;
	unsigned short Reserved1: 10;

} sGTX_REG_QIR;



/* Audio Queue Manager */
typedef struct {

	unsigned short AudioQueueReadPointer: 16;

} sGTX_REG_AQRPL;

typedef struct {

	unsigned char H: 1;
	unsigned short Reserved1: 9;
	unsigned char AudioQueueReadPointer: 6;

} sGTX_REG_AQRPH;

typedef struct {

	unsigned short AudioQueueWritePointer: 16;

} sGTX_REG_AQWPL;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned char Q_Size: 4;
	unsigned char AudioQueueWritePointer: 6;

} sGTX_REG_AQWPH;



/* Teletext Queue Manager */
typedef struct {

	unsigned short TeletextQueueReadPointer: 16;

} sGTX_REG_TQRPL;

typedef struct {

	unsigned char H: 1;
	unsigned short Reserved1: 9;
	unsigned char TeletextQueueReadPointer: 6;

} sGTX_REG_TQRPH;

typedef struct {

	unsigned short TeletextQueueWritePointer: 16;

} sGTX_REG_TQWPL;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned char Q_Size: 4;
	unsigned char TeletextQueueWritePointer: 6;

} sGTX_REG_TQWPH;



/* Video Queue Manager */
typedef struct {

	unsigned short VideoQueueReadPointer: 16;

} sGTX_REG_VQRPL;

typedef struct {

	unsigned char H: 1;
	unsigned short Reserved1: 9;
	unsigned char VideoQueueReadPointer: 6;

} sGTX_REG_VQRPH;

typedef struct {

	unsigned short VideoOueueWritePointer: 16;

} sGTX_REG_VQWPL;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned char Q_Size: 4;
	unsigned char VideoQueueWritePointer: 6;

} sGTX_REG_VQWPH;



/* Queue Interrupt */
typedef struct {

	unsigned char M: 1;
	unsigned char BLOCK: 5;
	unsigned short Reserved1: 10;

} sGTX_REG_QIn;



/* Copy Engine */
typedef struct {

	unsigned short Word_n: 16;

} sGTX_REG_CBWn;

typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Addr: 22;

} sGTX_REG_CCSA;

typedef struct {

	 unsigned short Reserved1: 10;
	 unsigned int Addr: 22;

} sGTX_REG_CDA;

typedef struct {

	unsigned char Reserved1: 3;
	unsigned char NS: 1;
	unsigned char RWT: 1;
	unsigned char DADD: 1;
	unsigned char WE: 1;
	unsigned char RE: 1;
	unsigned char Reserved2: 4;
	unsigned char LEN: 4;

} sGTX_REG_CCOM;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char T_Color: 8;

} sGTX_REG_RWTC;

typedef struct {

	unsigned char Reserved1: 2;
	unsigned char R: 1;
	unsigned char NS: 1;
	unsigned char RWT: 1;
	unsigned char DADD: 1;
	unsigned char WE: 1;
	unsigned char RE: 1;
	unsigned char Reserved2: 4;
	unsigned char LEN: 4;

} sGTX_REG_CCOM2;

typedef struct {

	unsigned char Reserved1: 2;
	unsigned char R: 1;
	unsigned char NS: 1;
	unsigned char RWT: 1;
	unsigned char DADD: 1;
	unsigned char WE: 1;
	unsigned char RE: 1;
	unsigned char Reserved2: 4;
	unsigned char LEN: 4;

} sGTX_REG_CCOM3;



/* Video Plane Display */
typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Addr: 21;
	unsigned char E: 1;

} sGTX_REG_VPSA;

typedef struct {

	unsigned short OFFSET;
	unsigned char Reserved1: 4;
	unsigned short STRIDE: 11;
	unsigned char B: 1;

} sGTX_REG_VPO;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned short HPOS: 9;
	unsigned char Reserved2: 7;
	unsigned short VPOS: 9;
	unsigned char F: 1;

} sGTX_REG_VPP;

typedef struct {

	unsigned char Reserved1: 6;
	unsigned short WIDTH: 9;
	unsigned char S: 1;
	unsigned char Reserved2: 6;
	unsigned short HEIGHT: 9;
	unsigned char P: 1;

} sGTX_REG_VPS;

typedef struct {

	unsigned short Reserved1: 14;
	unsigned char EXT: 2;

} sGTX_REG_VPOE;



/* Video Capture */
typedef struct {

	unsigned short Reserved1: 10;
	unsigned int Addr: 21;
	unsigned char E: 1;

} sGTX_REG_VCSA;

typedef struct {

	unsigned char V: 1;
	unsigned char Reserved1: 5;
	unsigned short HPOS: 9;
	unsigned char Reserved2: 3;
	unsigned char OVOFFS: 4;
	unsigned short EVPOS: 9;
	unsigned char Reserved3: 1;

} sGTX_REG_VCSP;

typedef struct {

	unsigned short HDEC: 4;
	unsigned char Reserved1: 2;
	unsigned short HSIZE: 9;
	unsigned char F: 1;
	unsigned char VDEC: 4;
	unsigned char Reserved2: 2;
	unsigned short VSIZE: 9;
	unsigned char B: 1;

} sGTX_REG_VCS;



/* Semaphore */
typedef struct {

	unsigned char Reserved1: 8;
	unsigned char PID: 7;
	unsigned char A: 1;

} sGTX_REG_SEM1;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char PID: 7;
	unsigned char A: 1;

} sGTX_REG_SEM2;



/* Teletext */
typedef struct {

	unsigned short PTS0: 16;

} sGTX_REG_PTS0;

typedef struct {

	unsigned short PTS1: 16;

} sGTX_REG_PTS1;

typedef struct {

	unsigned short PTS_OFFSET: 16;

} sGTX_REG_PTSO;

typedef struct {

	unsigned char Reserved1: 1;
	unsigned char PE: 1;
	unsigned char Reserved2: 1;
	unsigned char RP: 1;
	unsigned char FP: 1;
	unsigned char Reserved3: 1;
	unsigned char GO: 1;
	unsigned char IE: 1;
	unsigned char Data_ID: 8;

} sGTX_REG_TTCR;

typedef struct {

	unsigned char P: 1;
	unsigned char R: 1;
	unsigned char E: 1;
	unsigned char Reserved1: 8;
	unsigned char State: 5;

} sGTX_REG_TSR;



/* Infrared */
typedef struct {

	unsigned char Reserved1: 7;
	unsigned char P: 1;
	unsigned char Filt_H: 4;
	unsigned char Filt_L: 4;

} sGTX_REG_RFR;

typedef struct {

	unsigned char Reserved1: 8;
	unsigned char RTCH: 8;

} sGTX_REG_RPH;

typedef struct {

	unsigned short Reserved1: 10;
	unsigned short Address: 13;
	unsigned short Reserved2: 9;

} sGTX_REG_IRQA;

typedef struct {

	unsigned char E: 1;
	unsigned char L: 1;
	unsigned char Reserved1: 7;
	unsigned short Offset: 7;

} sGTX_REG_IRRE;

typedef struct {

	unsigned char E: 1;
	unsigned char C: 1;
	unsigned char Reserved1: 7;
	unsigned char Offset: 7;

} sGTX_REG_IRTE;

typedef struct {

	unsigned short Reserved1: 9;
	unsigned char Offset: 7;

} sGTX_REG_IRRO;

typedef struct {

	unsigned short Reserved1: 9;
	unsigned char Offset: 7;

} sGTX_REG_IRTO;

#pragma pack()

extern void avia_gt_gtx_clear_irq(unsigned char irq_reg, unsigned char irq_bit);
extern unsigned short avia_gt_gtx_get_irq_mask(unsigned char irq_reg);
extern unsigned short avia_gt_gtx_get_irq_status(unsigned char irq_reg);
extern void avia_gt_gtx_mask_irq(unsigned char irq_reg, unsigned char irq_bit);
extern void avia_gt_gtx_unmask_irq(unsigned char irq_reg, unsigned char irq_bit);
extern void avia_gt_gtx_init(void);
extern void avia_gt_gtx_exit(void);

#define gtx_reg_16(register) ((unsigned short)(*((unsigned short*)(gt_info->reg_addr + GTX_REG_ ## register))))
#define gtx_reg_16n(offset) ((unsigned short)(*((unsigned short*)(gt_info->reg_addr + offset))))
#define gtx_reg_32(register) ((unsigned int)(*((unsigned int*)(gt_info->reg_addr + GTX_REG_ ## register))))
#define gtx_reg_32n(offset) ((unsigned int)(*((unsigned int*)(gt_info->reg_addr + offset))))
#define gtx_reg_o(offset) (gt_info->reg_addr + offset)
#define gtx_reg_s(register) ((sGTX_REG_##register *)(&gtx_reg_32(register)))
#define gtx_reg_sn(register, offset) ((sGTX_REG_##register *)(gtx_reg_o(offset)))
#define gtx_reg_so(register, offset) ((sGTX_REG_##register *)(&gtx_reg_32(register + (offset))))
#define gtx_reg_32s(register) ((sGTX_REG_##register *)(&gtx_reg_32(register)))
#define gtx_reg_16s(register) ((sGTX_REG_##register *)(&gtx_reg_16(register)))

#define gtx_reg_set_32s(register, field, value) { u32 tmp_reg_val = gtx_reg_32(register); ((sGTX_REG_##register *)&tmp_reg_val)->field = value; gtx_reg_32(register) = tmp_reg_val; }
#define gtx_reg_set_16s(register, field, value) { u16 tmp_reg_val = gtx_reg_16(register); ((sGTX_REG_##register *)&tmp_reg_val)->field = value; gtx_reg_16(register) = tmp_reg_val; }
#define gtx_reg_set(register, field, value) do { if (sizeof(sGTX_REG_##register) == 4) gtx_reg_set_32s(register, field, value) else if (sizeof(sGTX_REG_##register ) == 2) gtx_reg_set_16s(register, field, value) else printk("ERROR: struct size is %d\n", sizeof(sGTX_REG_##register)); } while(0)
#define gtx_reg_set_bit(register, bit) gtx_reg_set(register, bit, 1)
#define gtx_reg_clear_bit(register, bit) gtx_reg_set(register, bit, 0)

#endif /* __GTX_H__ */
