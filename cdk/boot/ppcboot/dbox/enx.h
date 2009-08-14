#ifndef __ENX_H
#define __ENX_H


#define ENX_REG_BASE	0x08000000
#define ENX_REG_SIZE	0x00003400
#define ENX_MEM_BASE	0x09000000
#define ENX_MEM_SIZE	0x00200000


#define ENX_FB_OFFSET	0x00100000


#define ENX_INTERRUPT	SIU_IRQ1


#define TDP_INSTR_RAM	0x2000
#define TDP_DATA_RAM	0x2800
#define CAM_RAM		0x3000


#define PFCR		0x0780			// Parallel FIFO Control Register
#define PFQR		0x0782			// Parallel FIFO Quantity Register
#define SDTCR		0x0784			// Stale Data Timer Count Register
#define SDTPR		0x0786			// Stale Data Timer Period Register
#define NER		0x0788			// Negotiation Enable Register
#define NSR		0x078A			// Negotiation Status Register
#define PPIER		0x078C			// Parallel Port Interrupt Enable Register
#define PPISR		0x078E			// Parallel Port Interrupt Status Register
#define SCR		0x0790			// Special Command Register
#define TDR		0x0792			// Tagged Data Register
#define SPR		0x0794			// Short Pulse Register
#define HTVR		0x0796			// Host Time-out Value Register
#define AVI_0		0x0940			// AVI Configuration Register 0
#define AVI_1		0x0942			// AVI Configuration Register 1
#define RSTR0		0x0000			// Reset Register 0
#define CFGR0		0x0008			// Configuration Register 0
#define CRR		0x000C			// Chip Revision Register
#define BER		0x0020			// Bus Error Register
#define RCSC		0x0080			// Register Chip Select Configuration
#define SCSC		0x0084			// SDRAM Chip Select Configuration Register
#define SR		0x01C0			// Semaphore Register
#define UART_1_DIV	0x084A			// UART 1 Divide Register
#define UART2_DIV	0x0856			// UART 2 Divide Register
#define GCSRC1		0x05C0			// Graphics Copy Source Address 1 Register
#define GCDST		0x05C4			// Graphics Copy Destination Address Register
#define GCCMD		0x05C8			// Graphics Copy Command Register
#define CPCSRC1		0x05D0			// Copy/CRC Engine Source Address 1 Register
#define CPCDST		0x05D4			// Copy/CRC Engine Destination Address Register
#define CPCCMD		0x05D8			// Copy/CRC Engine Command Register
#define GCPWn		0x0640			// Graphics Copy Buffer Word n
#define CPCBWn		0x0680			// Copy/CRC Engine Buffer Word n
#define TMCR		0x0800			// Timer Master Control Register
#define TPCV		0x0804			// Timer Prescale Count Value Register
#define T1CR		0x0810			// Timer 1 Control Register
#define T1RV		0x0814			// Timer 1 Reload Value Register
#define T1CV		0x0818			// Timer 1 Count Value Register
#define T2CR		0x0828			// Timer 2 Control Register
#define T2RV		0x082C			// Timer 2 Reload Value Register
#define T2CV		0x0830			// Timer 2 Count Value Register
#define CCCNTR		0x0834			// Clock Counter Count Value Register
#define CCCAPR		0x0838			// Clock Counter Capture Register
#define WTCTL		0x083C			// Watchdog Timer Control Register
#define WTCMD		0x0840			// Watchdog Timer Command Register
#define WTVAL		0x0844			// Watchdog Timer Value Register
#define PWM_TMCR	0x084C			// PWM Timer Master Control Register
#define PWM_TPCV	0x0850			// PWM Timer Prescale Count Value Register
#define PWM_T1SRV	0x0858			// PWM Timer 1 Second Reload Value Register
#define PWM_T1CR	0x085C			// PWM Timer 1 Control Register
#define PWM_T1FRV	0x0860			// PWM Timer 1 First Reload Value Register
#define PWM_T1CV	0x0864			// PWM Timer 1 Count Value Register
#define PWM_T2SRV	0x0870			// PWM Timer 2 Second Reload Value Register
#define PWM_T2FRV	0x0878			// PWM Timer 2 First Reload Value Register
#define PWM_T2CV	0x087C			// PWM Timer 2 Count Value Register
#define DAC_PC		0x0BC0			// DAC Pulse Count Register
#define DAC_CP		0x0BC2			// DAC Clock Prescale
#define GPDIR0		0x0182			// GPIO Direction Register Bank 0
#define GPDAT0		0x0184			// GPIO Data Register Bank 0
#define BDST		0x0580			// Blitter Destination Register
#define BMR		0x0584			// Blitter Mask Register
#define BDR		0x0588			// Blitter Data Register
#define BPW		0x058C			// Blitter Pixel Width Register
#define BPO		0x058E			// Blitter Pixel Offset Register
#define BMODE		0x0590			// Blitter Mode Register
#define BIMR		0x0594			// Blitter Internal Mask Register
#define BIDR		0x0598			// Blitter Internal Data Register
#define BDRR		0x059C			// Blitter Data Register Restore
#define BCLR01		0x05A0			// Blitter Color Register
#define BCLR23		0x05A4			// Blitter Color Register
#define GMR1		0x03C0			// Graphics Mode Plane 1
#define GBLEV1		0x03C6			// Graphics Blend Level Plane 1
#define TCR1		0x03C8			// Transparent Color Plane 1 Register
#define GVSA1		0x03CC			// Graphics Viewport Start Address Plane 1 Register
#define GVP1		0x03D0			// Graphics Viewport Position Plane 1 Register
#define GVSZ1		0x03D4			// Graphics Viewport Size Plane 1 Register
#define CLUTA		0x03DA			// Color Look-up Table Address Register
#define CLUTD		0x03DC			// Color Look-up Table Data Register
#define GMR2		0x03E0			// Graphics Mode Plane 2 Register
#define GBLEV2		0x03E6			// Graphics Blend Level Plane 2 Register
#define TCR2		0x03E8			// Transparent Color Plane 2 Register
#define GVSA2		0x03EC			// Graphics Viewport Start Address Plane 2 Register
#define GVP2		0x03F0			// Graphics Viewport Position Plane 2 Register
#define GVSZ2		0x03F4			// Graphics Viewport Size Plane 2 Register
#define CPOS		0x03F8			// Cursor Position Register
#define CSA		0x03FC			// Cursor Start Address Register
#define P1VPSA		0x0540			// PIG1 Video Plane Start Address Register
#define P1VPOFFS		0x0548			// PIG1 Video Plane Offset Register
#define P1VPSO		0x054C			// PIG1 Video Plane Stacking Order Register
#define P1VPSTR		0x054E			// PIG1 Video Plane Stride Register
#define P1VPP		0x0550			// PIG1 Video Plane Position Register
#define P1VPSZ		0x0554			// PIG1 Video Plane Size Register
#define P2VPSA		0x0500			// PIG2 Video Plane Start Address Register
#define P2VPOFFS		0x0508			// PIG2 Video Plane Offset Register
#define P2VPSO		0x050C			// PIG2 Video Plane Stacking Order Register
#define P2VPSTR		0x050E			// PIG2 Video Plane Stride Register
#define P2VPP		0x0510			// PIG2 Video Plane Position Register
#define P2VPSZ		0x0514			// PIG2 Video Plane Size Register
#define VCSA1		0x0480			// Video Capture Start Address Register
#define VCSA2		0x0484			// Alternate Frame Video Capture Start Address Register
#define VCOFFS		0x0488			// Video Capture Odd Field Offset Register
#define VCSTR		0x048E			// Video Capture Stride Register
#define VCP		0x0490			// Video Capture Start Position Register
#define VCSZ		0x0494			// Video Capture Size Register
#define VCYIE		0x0498			// Vid Cap Y Initial for Even Field
#define VCYIO		0x049A			// Vid Cap Y Initial for Odd Field
#define VCYSI		0x049C			// Vid Cap Y Skip Increment
#define VCYCI		0x049E			// Vid Cap Y Capture Increment
#define VCXI		0x04A2			// Vid Cap X Initial
#define VCXSI		0x04A4			// Vid Cap X Skip Increment
#define VCXCI		0x04A6			// Vid Cap Y Capture Increment
#define VBR		0x04C0			// Video Background Register
#define VCR		0x04C4			// Video Control Register
#define VLC		0x04C6			// Video Line Count Register
#define VLI1		0x04C8			// Video Line Interrupt 1 Register
#define VHT		0x04CA			// Video Horizontal Total Register
#define VLT		0x04CC			// Video Line Total Register
#define VLI2		0x04CE			// Video Line Interrupt 2 Register
#define BPOS		0x04D0			// Background Blend Region Position Register
#define BSZ		0x04D4			// Background Blend Region Size Register
#define BALP		0x04D8			// Background Blend Region Alpha Register
#define VMCR		0x04DA			// Video Mixer Control Register
#define G1CFR		0x04DC			// Graphics1 Chroma Filter Control Register
#define G2CFR		0x04DE			// Graphics2 Chroma Filter Control Register
#define VAS		0x04E0			// Video Active Start Register
#define HSPC		0x08F8			// Hi-Speed Port Configuration Register
#define IDCCR1		0x0980			// IDC Control Register 1
#define IDCSR		0x0982			// IDC Status Register
#define IDCSA		0x0984			// IDC Slave Address Register
#define IDCRD		0x0986			// IDC Receive Data Register
#define IDCMA		0x0988			// IDC Master Address Register
#define IDCTD		0x098A			// IDC Transmit Data Register
#define IDCC		0x098C			// IDC Clock Register
#define IDCFF		0x098E			// IDC FIFO Fullness Register
#define IDCCR2		0x0990			// IDC Control Register 2
#define CWP		0x0A00			// IR Transmit Carrier Wave Period Register
#define CWPH		0x0A02			// IR Transmit Carrier Wave Pulse High Register
#define MSPR		0x0A04			// IR Modulated Signal Period Register
#define MSPL		0x0A06			// IR Modulated Signal Pulse Low Register
#define RTC		0x0A08			// IR Receive Tick Count Register
#define RTP		0x0A0A			// IR Receive Tick Period Register
#define RFR		0x0A0C			// IR Receive Filter Register
#define RPH		0x0A0E			// IR Receive Pulse High Tick Count Register
#define IRQA		0x0A10			// IR Queue Base Address Register
#define IRRE		0x0A14			// IR Receive End Offset Register
#define IRTE		0x0A16			// IR Transmit End Offset Register
#define IRRO		0x0A18			// IR Receive Queue Offset Register
#define IRTO		0x0A1A			// IR Transmit Queue Offset Register
#define ISR0		0x0100			// Interrupt Status Register 0
#define ISR1		0x0102			// Interrupt Status Register 1
#define ISR2		0x0104			// Interrupt Status Register 2
#define ISR3		0x0106			// Interrupt Status Register 2
#define ISR4		0x0108			// Interrupt Status Register 2
#define ISR5		0x010A			// Interrupt Status Register 5
#define IMR0		0x0110			// Interrupt Mask Register 0
#define IMR1		0x0112			// Interrupt Mask Register 1
#define IMR2		0x0114			// Interrupt Mask Register 2
#define IMR3		0x0116			// Interrupt Mask Register 3
#define IMR4		0x0118			// Interrupt Mask Register 4
#define IMR5		0x011A			// Interrupt Mask Register 5
#define IPR1		0x0124			// Interrupt Priority Register 1 for SPARC Microprocessor
#define IPR2		0x0128			// Interrupt Priority Register 2 for SPARC Microprocessor
#define IDR		0x0130			// SPARC Microprocessor Global Interrupt Disable Register
#define IPR4		0x0140			// Interrupt Priority Register 4 for External Processor
#define IPR5		0x0144			// Interrupt Priority Register 5
#define EHIDR		0x0148			// External Host Interrupt Disable Register
#define PCMA		0x0900			// PCM Address Register
#define PCMN		0x0904			// PCM Attenuation Register
#define PCMC		0x0908			// PCM Control Register
#define PCMS		0x090A			// PCM Number of Samples Register
#define PCMD		0x090C			// PCM DMA Address Register
#define MC		0x0380			// Memory Configuration Register
#define SCTMA_1		0x0A40			// Smart Card Transmit Memory Address Register
#define SCTMA_2		0x0A80			// Smart Card Transmit Memory Address Register
#define SCRMA_1		0x0A44			// Smart Card Receive Memory Address Register
#define SCRMA_2		0x0A84			// Smart Card Receive Memory Address Register
#define SCS_1		0x0A48			// Smart Card Status Register
#define SCS_2		0x0A88			// Smart Card Status Register
#define SCTMS_1		0x0A4A			// Smart Card Transmit Message Size Register
#define SCTMS_2		0x0A8A			// Smart Card Transmit Message Size Register
#define SCRMS_1		0x0A4C			// Smart Card Receive Message Size Register
#define SCRMS_2		0x0A8C			// Smart Card Receive Message Size Register
#define SCCmnd_1	0x0A4E			// Smart Card Command Register
#define SCCmnd_2	0x0A8E			// Smart Card Command Register
#define SCCWI_1		0x0A50			// Smart Card Character Waiting Index Register
#define SCCWI_2		0x0A90			// Smart Card Character Waiting Index Register
#define SCETU_1		0x0A52			// Smart Card Elementary Time Unit Register
#define SCETU_2		0x0A92			// Smart Card Elementary Time Unit Register
#define SCGT_1		0x0A54			// Smart Card Guard Time Register
#define SCGT_2		0x0A94			// Smart Card Guard Time Register
#define SCCD_1		0x0A56			// Smart Card Clock Divide Register
#define SCCD_2		0x0A96			// Smart Card Clock Divide Register
#define SCRA_1		0x0A58			// Smart Card Reset Activate Register
#define SCRA_2		0x0A98			// Smart Card Reset Activate Register
#define SCIE_1		0x0A5A			// Smart Card Input Enable Register
#define SCIE_2		0x0A9A			// Smart Card Input Enable Register
#define RTPTS0		0x0B80			// Received Teletext PTS Register 0
#define RTPTS1		0x0B82			// Received Teletext PTS Register 1
#define UPTSOST		0x0B84			// Unsigned PTS Offset Register
#define TCNTL		0x0B86			// Teletext Control Register
#define TSTATUS		0x0B88			// Teletext Status Register
#define FC		0x0B24			// Framer Control Register
#define SYNC_HYST	0x0B26			// Sync Hysteresis Register
#define FIFO_PDCT	0x0B28			// Packet FIFO Peak Detector
#define BQ		0x0B2A			// Block Qualifier Register
#define PCR_PID		0x0B00			// Program Clock Reference Packet Identifier Register
#define TP_PCR_2	0x0B02			// Transport Packet PCR Register 2
#define TP_PCR_1	0x0B04			// Transport Packet PCR Register 1
#define TP_PCR_0	0x0B06			// Transport Packet PCR Register 0
#define LC_STC_2	0x0B08			// Latched STC Register 2
#define LC_STC_1	0x0B0A			// Latched STC Register 1
#define LC_STC_0	0x0B0C			// Latched STC Register 0
#define STC_COUNTER_2	0x0B0E			// STC Counter 2 Register
#define STC_COUNTER_1	0x0B20			// STC Counter 1 Register
#define STC_COUNTER_0	0x0B22			// STC Counter 0 Register
#define EPC		0x0B60			// Transport Demux Processor PC Register
#define EC		0x0B68			// Transport Demux Processor Control Register
#define SPPCR1		0x3200			// Section Filter Configuration Register 1
#define SPPCR2		0x3202			// Section Filter Configuration Register 2
#define SPPCR3		0x3204			// Section Filter Configuration Register 3
#define SPPCR4		0x3206			// Section Filter Configuration Register 4
#define AQRPL		0x08E0			// Audio Queue Read Pointer, Lower Word Register
#define AQRPH		0x08E2			// Audio Queue Read Pointer, Upper Word Register
#define AQWPL		0x08E4			// Audio Queue Write Pointer, Lower Word Register
#define AQWPH		0x08E6			// Audio Queue Write Pointer, Upper Word Register
#define TQRPL		0x08E8			// Teletext Queue Read Pointer, Lower Word Register
#define TQRPH		0x08EA			// Teletext Queue Read Pointer, Upper Word Register
#define TQWPL		0x08EC			// Teletext Queue Write Pointer - Lower Word Register
#define TQWPH		0x08EE			// Teletext Queue Write Pointer - Upper Word Register
#define VQRPL		0x08F0			// Video Queue Read Pointer, Lower Word Register
#define VQRPH		0x08F2			// Video Queue Read Pointer, Upper Word Register
#define VQWPL		0x08F4			// Video Queue Write Pointer, Lower Word Register
#define VQWPH		0x08F6			// Video Queue Write Pointer, Upper Word Register
#define RBR_1		0x0700			// Receive Buffer Register
#define RBR_2		0x0740			// Receive Buffer Register
#define THR_1		0x0700			// Transmit Holding Register
#define THR_2		0x0740			// Transmit Holding Register
#define IER_1		0x0704			// Interrupt Enable Register
#define IER_2		0x0744			// Interrupt Enable Register
#define DLL_1		0x0700			// Divisor Latch- LSB Register
#define DLL_2		0x0740			// Divisor Latch- LSB Register
#define DLM_1		0x0704			// Divisor Latch-MSB Register
#define DLM_2		0x0744			// Divisor Latch-MSB Register
#define IIR_1		0x0708			// Interrupt Identification Register
#define IIR_2		0x0748			// Interrupt Identification Register
#define FCR_1		0x0708			// FIFO Control Register
#define FCR_2		0x0748			// FIFO Control Register
#define LCR_1		0x070C			// Line Control Register
#define LCR_2		0x074C			// Line Control Register
#define MCR_1		0x0710			// Modem Control Register
#define MCR_2		0x0750			// Modem Control Register
#define LSR_1		0x0714			// Line Status Register
#define LSR_2		0x0754			// Line Status Register
#define MSR_1		0x0718			// Modem Status Register
#define MSR_2		0x0758			// Modem Status Register
#define SPR_1		0x071C			// Scratch Pad Register
#define SPR_2		0x075C			// Scratch Pad Register
#define QWPnL           0x0880
#define QWPnH           0x0882

typedef struct {

  unsigned char F_Reserved1: 3;
  unsigned char F_VRQ: 1;
  unsigned char F_ARQ: 1;
  unsigned char F_VEN: 1;
  unsigned char F_AEN: 1;
  unsigned char F_Reserved2: 1;
  unsigned char F_SER_MAX_RT: 2;
  unsigned char F_Reseved3: 2;
  unsigned char F_AUDIO_XFT_RATE: 4;

} sAVI_0;

typedef struct {

  unsigned char F_PCMA: 1;
  unsigned char F_1284: 1;
  unsigned char F_UART2: 1;
  unsigned char F_Reserved1: 1;
  unsigned char F_SCEN: 1;
  unsigned char F_ORDR: 1;
  unsigned char F_LATE: 1;
  unsigned char F_DOE: 1;
  unsigned char F_BUS_TIMEOUT;
  unsigned char F_BRD_ID;
  unsigned char F_BIU_SEL: 2;
  unsigned char F_Reserved2: 1;
  unsigned char F_UPQ: 1;
  unsigned char F_TCP: 1;
  unsigned char F_Reserved3: 1;
  unsigned char F_ACP: 1;
  unsigned char F_VCP: 1;

} sCFGR0;

typedef struct {

  unsigned char F_ARCH_ID;
  unsigned char F_API_VERSION;
  unsigned char F_VERSION;
  unsigned char F_REVISION;
  
} sCRR;

typedef struct {

  unsigned short F_Reserved1: 6;
  unsigned short F_PC: 10;
  
} sEPC;


extern unsigned char* enx_get_mem_addr(void);
extern unsigned char* enx_get_reg_addr(void);
extern void enx_free_irq(int reg, int bit);
extern int enx_allocate_irq(int reg, int bit, void (*isr)(int, int));

#define enx_reg_w(register) ((unsigned int)(*((unsigned int*)(enx_get_reg_addr() + register))))
#define enx_reg_h(register) ((unsigned short)(*((unsigned short*)(enx_get_reg_addr() + register))))
#define enx_reg_s(register) ((s##register *)(&enx_reg_w(register)))
#define enx_reg_o(offset) (enx_get_reg_addr() + offset)

#endif
