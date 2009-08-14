#ifndef __ENX_H
#define __ENX_H

#define ENX_REG_BASE			0x08000000
#define ENX_REG_SIZE			0x00003400
#define ENX_MEM_BASE			0x09000000
#define ENX_MEM_SIZE			0x00200000

#define ENX_INTERRUPT			SIU_IRQ1

#define TDP_INSTR_RAM			0x2000
#define TDP_DATA_RAM			0x2800
#define CAM_RAM					0x3000

#define AVIA_GT_ENX_IR_CLOCK	AVIA_GT_HALFSYSCLK

#define ENX_REG_PFCR			0x0780			// Parallel FIFO Control Register
#define ENX_REG_PFQR			0x0782			// Parallel FIFO Quantity Register
#define ENX_REG_SDTCR			0x0784			// Stale Data Timer Count Register
#define ENX_REG_SDTPR			0x0786			// Stale Data Timer Period Register
#define ENX_REG_NER				0x0788			// Negotiation Enable Register
#define ENX_REG_NSR				0x078A			// Negotiation Status Register
#define ENX_REG_PPIER			0x078C			// Parallel Port Interrupt Enable Register
#define ENX_REG_PPISR			0x078E			// Parallel Port Interrupt Status Register
#define ENX_REG_SCR				0x0790			// Special Command Register
#define ENX_REG_TDR				0x0792			// Tagged Data Register
#define ENX_REG_SPR				0x0794			// Short Pulse Register
#define ENX_REG_HTVR			0x0796			// Host Time-out Value Register
#define ENX_REG_AVI_0			0x0940			// AVI Configuration Register 0
#define ENX_REG_AVI_1			0x0942			// AVI Configuration Register 1
#define ENX_REG_RSTR0			0x0000			// Reset Register 0
#define ENX_REG_CFGR0			0x0008			// Configuration Register 0
#define ENX_REG_CRR				0x000C			// Chip Revision Register
#define ENX_REG_BER				0x0020			// Bus Error Register
#define ENX_REG_RCSC			0x0080			// Register Chip Select Configuration
#define ENX_REG_SCSC			0x0084			// SDRAM Chip Select Configuration Register
#define ENX_REG_SR				0x01C0			// Semaphore Register
#define ENX_REG_UART_1_DIV		0x084A			// UART 1 Divide Register
#define ENX_REG_UART2_DIV		0x0856			// UART 2 Divide Register
#define ENX_REG_GCSRC1			0x05C0			// Graphics Copy Source Address 1 Register
#define ENX_REG_GCDST			0x05C4			// Graphics Copy Destination Address Register
#define ENX_REG_GCCMD			0x05C8			// Graphics Copy Command Register
#define ENX_REG_CPCSRC1			0x05D0			// Copy/CRC Engine Source Address 1 Register
#define ENX_REG_CPCDST			0x05D4			// Copy/CRC Engine Destination Address Register
#define ENX_REG_CPCCMD			0x05D8			// Copy/CRC Engine Command Register
#define ENX_REG_CPCTCAR			0x05DA			// Copy/CRC Engine Transparent Color or Alpha Register
#define ENX_REG_CPCCRCSRC2		0x05DC			// Graphics Copy CRC-32 Accumulator or Source 2 Address
#define ENX_REG_GCPWn			0x0640			// Graphics Copy Buffer Word n
#define ENX_REG_CPCBWn			0x0680			// Copy/CRC Engine Buffer Word n
#define ENX_REG_TMCR			0x0800			// Timer Master Control Register
#define ENX_REG_TPCV			0x0804			// Timer Prescale Count Value Register
#define ENX_REG_T1CR			0x0810			// Timer 1 Control Register
#define ENX_REG_T1RV			0x0814			// Timer 1 Reload Value Register
#define ENX_REG_T1CV			0x0818			// Timer 1 Count Value Register
#define ENX_REG_T2CR			0x0828			// Timer 2 Control Register
#define ENX_REG_T2RV			0x082C			// Timer 2 Reload Value Register
#define ENX_REG_T2CV			0x0830			// Timer 2 Count Value Register
#define ENX_REG_CCCNTR			0x0834			// Clock Counter Count Value Register
#define ENX_REG_CCCAPR			0x0838			// Clock Counter Capture Register
#define ENX_REG_WTCTL			0x083C			// Watchdog Timer Control Register
#define ENX_REG_WTCMD			0x0840			// Watchdog Timer Command Register
#define ENX_REG_WTVAL			0x0844			// Watchdog Timer Value Register
#define ENX_REG_PWM_TMCR		0x084C			// PWM Timer Master Control Register
#define ENX_REG_PWM_TPCV		0x0850			// PWM Timer Prescale Count Value Register
#define ENX_REG_PWM_T1SRV		0x0858			// PWM Timer 1 Second Reload Value Register
#define ENX_REG_PWM_T1CR		0x085C			// PWM Timer 1 Control Register
#define ENX_REG_PWM_T1FRV		0x0860			// PWM Timer 1 First Reload Value Register
#define ENX_REG_PWM_T1CV		0x0864			// PWM Timer 1 Count Value Register
#define ENX_REG_PWM_T2SRV		0x0870			// PWM Timer 2 Second Reload Value Register
#define ENX_REG_PWM_T2FRV		0x0878			// PWM Timer 2 First Reload Value Register
#define ENX_REG_PWM_T2CV		0x087C			// PWM Timer 2 Count Value Register
#define ENX_REG_DAC_PC			0x0BC0			// DAC Pulse Count Register
#define ENX_REG_DAC_CP			0x0BC2			// DAC Clock Prescale
#define ENX_REG_GPDIR0			0x0182			// GPIO Direction Register Bank 0
#define ENX_REG_GPDAT0			0x0184			// GPIO Data Register Bank 0
#define ENX_REG_BDST			0x0580			// Blitter Destination Register
#define ENX_REG_BMR				0x0584			// Blitter Mask Register
#define ENX_REG_BDR				0x0588			// Blitter Data Register
#define ENX_REG_BPW				0x058C			// Blitter Pixel Width Register
#define ENX_REG_BPO				0x058E			// Blitter Pixel Offset Register
#define ENX_REG_BMODE			0x0590			// Blitter Mode Register
#define ENX_REG_BIMR			0x0594			// Blitter Internal Mask Register
#define ENX_REG_BIDR			0x0598			// Blitter Internal Data Register
#define ENX_REG_BDRR			0x059C			// Blitter Data Register Restore
#define ENX_REG_BCLR01			0x05A0			// Blitter Color Register
#define ENX_REG_BCLR23			0x05A4			// Blitter Color Register
#define ENX_REG_GMR1			0x03C0			// Graphics Mode Plane 1
#define ENX_REG_GBLEV1			0x03C6			// Graphics Blend Level Plane 1
#define ENX_REG_TCR1			0x03C8			// Transparent Color Plane 1 Register
#define ENX_REG_GVSA1			0x03CC			// Graphics Viewport Start Address Plane 1 Register
#define ENX_REG_GVP1			0x03D0			// Graphics Viewport Position Plane 1 Register
#define ENX_REG_GVSZ1			0x03D4			// Graphics Viewport Size Plane 1 Register
#define ENX_REG_CLUTA			0x03DA			// Color Look-up Table Address Register
#define ENX_REG_CLUTD			0x03DC			// Color Look-up Table Data Register
#define ENX_REG_GMR2			0x03E0			// Graphics Mode Plane 2 Register
#define ENX_REG_GBLEV2			0x03E6			// Graphics Blend Level Plane 2 Register
#define ENX_REG_TCR2			0x03E8			// Transparent Color Plane 2 Register
#define ENX_REG_GVSA2			0x03EC			// Graphics Viewport Start Address Plane 2 Register
#define ENX_REG_GVP2			0x03F0			// Graphics Viewport Position Plane 2 Register
#define ENX_REG_GVSZ2			0x03F4			// Graphics Viewport Size Plane 2 Register
#define ENX_REG_CPOS			0x03F8			// Cursor Position Register
#define ENX_REG_CSA				0x03FC			// Cursor Start Address Register
#define ENX_REG_VPSA1			0x0540			// PIG1 Video Plane Start Address Register
#define ENX_REG_VPOFFS1			0x0548			// PIG1 Video Plane Offset Register
#define ENX_REG_VPSO1			0x054C			// PIG1 Video Plane Stacking Order Register
#define ENX_REG_VPSTR1			0x054E			// PIG1 Video Plane Stride Register
#define ENX_REG_VPP1			0x0550			// PIG1 Video Plane Position Register
#define ENX_REG_VPSZ1			0x0554			// PIG1 Video Plane Size Register
#define ENX_REG_VPSA2			0x0500			// PIG2 Video Plane Start Address Register
#define ENX_REG_VPOFFS2			0x0508			// PIG2 Video Plane Offset Register
#define ENX_REG_VPSO2			0x050C			// PIG2 Video Plane Stacking Order Register
#define ENX_REG_VPSTR2			0x050E			// PIG2 Video Plane Stride Register
#define ENX_REG_VPP2			0x0510			// PIG2 Video Plane Position Register
#define ENX_REG_VPSZ2			0x0514			// PIG2 Video Plane Size Register
#define ENX_REG_VCSA1			0x0480			// Video Capture Start Address Register
#define ENX_REG_VCSA2			0x0484			// Alternate Frame Video Capture Start Address Register
#define ENX_REG_VCOFFS			0x0488			// Video Capture Odd Field Offset Register
#define ENX_REG_VCSTR			0x048E			// Video Capture Stride Register
#define ENX_REG_VCP				0x0490			// Video Capture Start Position Register
#define ENX_REG_VCSZ			0x0494			// Video Capture Size Register
#define ENX_REG_VCYIE			0x0498			// Vid Cap Y Initial for Even Field
#define ENX_REG_VCYIO			0x049A			// Vid Cap Y Initial for Odd Field
#define ENX_REG_VCYSI			0x049C			// Vid Cap Y Skip Increment
#define ENX_REG_VCYCI			0x049E			// Vid Cap Y Capture Increment
#define ENX_REG_VCXI			0x04A2			// Vid Cap X Initial
#define ENX_REG_VCXSI			0x04A4			// Vid Cap X Skip Increment
#define ENX_REG_VCXCI			0x04A6			// Vid Cap Y Capture Increment
#define ENX_REG_VBR				0x04C0			// Video Background Register
#define ENX_REG_VCR				0x04C4			// Video Control Register
#define ENX_REG_VLC				0x04C6			// Video Line Count Register
#define ENX_REG_VLI1			0x04C8			// Video Line Interrupt 1 Register
#define ENX_REG_VHT				0x04CA			// Video Horizontal Total Register
#define ENX_REG_VLT				0x04CC			// Video Line Total Register
#define ENX_REG_VLI2			0x04CE			// Video Line Interrupt 2 Register
#define ENX_REG_BPOS			0x04D0			// Background Blend Region Position Register
#define ENX_REG_BSZ				0x04D4			// Background Blend Region Size Register
#define ENX_REG_BALP			0x04D8			// Background Blend Region Alpha Register
#define ENX_REG_VMCR			0x04DA			// Video Mixer Control Register
#define ENX_REG_G1CFR			0x04DC			// Graphics1 Chroma Filter Control Register
#define ENX_REG_G2CFR			0x04DE			// Graphics2 Chroma Filter Control Register
#define ENX_REG_VAS				0x04E0			// Video Active Start Register
#define ENX_REG_HSPC			0x08F8			// Hi-Speed Port Configuration Register
#define ENX_REG_IDCCR1			0x0980			// IDC Control Register 1
#define ENX_REG_IDCSR			0x0982			// IDC Status Register
#define ENX_REG_IDCSA			0x0984			// IDC Slave Address Register
#define ENX_REG_IDCRD			0x0986			// IDC Receive Data Register
#define ENX_REG_IDCMA			0x0988			// IDC Master Address Register
#define ENX_REG_IDCTD			0x098A			// IDC Transmit Data Register
#define ENX_REG_IDCC			0x098C			// IDC Clock Register
#define ENX_REG_IDCFF			0x098E			// IDC FIFO Fullness Register
#define ENX_REG_IDCCR2			0x0990			// IDC Control Register 2
#define ENX_REG_CWP				0x0A00			// IR Transmit Carrier Wave Period Register
#define ENX_REG_CWPH			0x0A02			// IR Transmit Carrier Wave Pulse High Register
#define ENX_REG_MSPR			0x0A04			// IR Modulated Signal Period Register
#define ENX_REG_MSPL			0x0A06			// IR Modulated Signal Pulse Low Register
#define ENX_REG_RTC				0x0A08			// IR Receive Tick Count Register
#define ENX_REG_RTP				0x0A0A			// IR Receive Tick Period Register
#define ENX_REG_RFR				0x0A0C			// IR Receive Filter Register
#define ENX_REG_RPH				0x0A0E			// IR Receive Pulse High Tick Count Register
#define ENX_REG_IRQA			0x0A10			// IR Queue Base Address Register
#define ENX_REG_IRRE			0x0A14			// IR Receive End Offset Register
#define ENX_REG_IRTE			0x0A16			// IR Transmit End Offset Register
#define ENX_REG_IRRO			0x0A18			// IR Receive Queue Offset Register
#define ENX_REG_IRTO			0x0A1A			// IR Transmit Queue Offset Register
#define ENX_REG_ISR0			0x0100			// Interrupt Status Register 0
#define ENX_REG_ISR1			0x0102			// Interrupt Status Register 1
#define ENX_REG_ISR2			0x0104			// Interrupt Status Register 2
#define ENX_REG_ISR3			0x0106			// Interrupt Status Register 2
#define ENX_REG_ISR4			0x0108			// Interrupt Status Register 2
#define ENX_REG_ISR5			0x010A			// Interrupt Status Register 5
#define ENX_REG_IMR0			0x0110			// Interrupt Mask Register 0
#define ENX_REG_IMR1			0x0112			// Interrupt Mask Register 1
#define ENX_REG_IMR2			0x0114			// Interrupt Mask Register 2
#define ENX_REG_IMR3			0x0116			// Interrupt Mask Register 3
#define ENX_REG_IMR4			0x0118			// Interrupt Mask Register 4
#define ENX_REG_IMR5			0x011A			// Interrupt Mask Register 5
#define ENX_REG_IPR1			0x0124			// Interrupt Priority Register 1 for SPARC Microprocessor
#define ENX_REG_IPR2			0x0128			// Interrupt Priority Register 2 for SPARC Microprocessor
#define ENX_REG_IDR				0x0130			// SPARC Microprocessor Global Interrupt Disable Register
#define ENX_REG_IPR4			0x0140			// Interrupt Priority Register 4 for External Processor
#define ENX_REG_IPR5			0x0144			// Interrupt Priority Register 5
#define ENX_REG_EHIDR			0x0148			// External Host Interrupt Disable Register
#define ENX_REG_PCMA			0x0900			// PCM Address Register
#define ENX_REG_PCMN			0x0904			// PCM Attenuation Register
#define ENX_REG_PCMC			0x0908			// PCM Control Register
#define ENX_REG_PCMS			0x090A			// PCM Number of Samples Register
#define ENX_REG_PCMD			0x090C			// PCM DMA Address Register
#define ENX_REG_MC				0x0380			// Memory Configuration Register
#define ENX_REG_SCTMA_1			0x0A40			// Smart Card Transmit Memory Address Register
#define ENX_REG_SCTMA_2			0x0A80			// Smart Card Transmit Memory Address Register
#define ENX_REG_SCRMA_1			0x0A44			// Smart Card Receive Memory Address Register
#define ENX_REG_SCRMA_2			0x0A84			// Smart Card Receive Memory Address Register
#define ENX_REG_SCS_1			0x0A48			// Smart Card Status Register
#define ENX_REG_SCS_2			0x0A88			// Smart Card Status Register
#define ENX_REG_SCTMS_1			0x0A4A			// Smart Card Transmit Message Size Register
#define ENX_REG_SCTMS_2			0x0A8A			// Smart Card Transmit Message Size Register
#define ENX_REG_SCRMS_1			0x0A4C			// Smart Card Receive Message Size Register
#define ENX_REG_SCRMS_2			0x0A8C			// Smart Card Receive Message Size Register
#define ENX_REG_SCCmnd_1		0x0A4E			// Smart Card Command Register
#define ENX_REG_SCCmnd_2		0x0A8E			// Smart Card Command Register
#define ENX_REG_SCCWI_1			0x0A50			// Smart Card Character Waiting Index Register
#define ENX_REG_SCCWI_2			0x0A90			// Smart Card Character Waiting Index Register
#define ENX_REG_SCETU_1			0x0A52			// Smart Card Elementary Time Unit Register
#define ENX_REG_SCETU_2			0x0A92			// Smart Card Elementary Time Unit Register
#define ENX_REG_SCGT_1			0x0A54			// Smart Card Guard Time Register
#define ENX_REG_SCGT_2			0x0A94			// Smart Card Guard Time Register
#define ENX_REG_SCCD_1			0x0A56			// Smart Card Clock Divide Register
#define ENX_REG_SCCD_2			0x0A96			// Smart Card Clock Divide Register
#define ENX_REG_SCRA_1			0x0A58			// Smart Card Reset Activate Register
#define ENX_REG_SCRA_2			0x0A98			// Smart Card Reset Activate Register
#define ENX_REG_SCIE_1			0x0A5A			// Smart Card Input Enable Register
#define ENX_REG_SCIE_2			0x0A9A			// Smart Card Input Enable Register
#define ENX_REG_RTPTS0			0x0B80			// Received Teletext PTS Register 0
#define ENX_REG_RTPTS1			0x0B82			// Received Teletext PTS Register 1
#define ENX_REG_UPTSOST			0x0B84			// Unsigned PTS Offset Register
#define ENX_REG_TCNTL			0x0B86			// Teletext Control Register
#define ENX_REG_TSTATUS			0x0B88			// Teletext Status Register
#define ENX_REG_FC				0x0B24			// Framer Control Register
#define ENX_REG_SYNC_HYST		0x0B26			// Sync Hysteresis Register
#define ENX_REG_FIFO_PDCT		0x0B28			// Packet FIFO Peak Detector
#define ENX_REG_BQ				0x0B2A			// Block Qualifier Register
#define ENX_REG_PCR_PID			0x0B00			// Program Clock Reference Packet Identifier Register
#define ENX_REG_TP_PCR_2		0x0B02			// Transport Packet PCR Register 2
#define ENX_REG_TP_PCR_1		0x0B04			// Transport Packet PCR Register 1
#define ENX_REG_TP_PCR_0		0x0B06			// Transport Packet PCR Register 0
#define ENX_REG_LC_STC_2		0x0B08			// Latched STC Register 2
#define ENX_REG_LC_STC_1		0x0B0A			// Latched STC Register 1
#define ENX_REG_LC_STC_0		0x0B0C			// Latched STC Register 0
#define ENX_REG_STC_COUNTER_2	0x0B0E			// STC Counter 2 Register
#define ENX_REG_STC_COUNTER_1	0x0B20			// STC Counter 1 Register
#define ENX_REG_STC_COUNTER_0	0x0B22			// STC Counter 0 Register
#define ENX_REG_EPC				0x0B60			// Transport Demux Processor PC Register
#define ENX_REG_EC				0x0B68			// Transport Demux Processor Control Register
#define ENX_REG_SPPCR1			0x3200			// Section Filter Configuration Register 1
#define ENX_REG_SPPCR2			0x3202			// Section Filter Configuration Register 2
#define ENX_REG_SPPCR3			0x3204			// Section Filter Configuration Register 3
#define ENX_REG_SPPCR4			0x3206			// Section Filter Configuration Register 4
#define ENX_REG_AQRPL			0x08E0			// Audio Queue Read Pointer, Lower Word Register
#define ENX_REG_AQRPH			0x08E2			// Audio Queue Read Pointer, Upper Word Register
#define ENX_REG_AQWPL			0x08E4			// Audio Queue Write Pointer, Lower Word Register
#define ENX_REG_AQWPH			0x08E6			// Audio Queue Write Pointer, Upper Word Register
#define ENX_REG_TQRPL			0x08E8			// Teletext Queue Read Pointer, Lower Word Register
#define ENX_REG_TQRPH			0x08EA			// Teletext Queue Read Pointer, Upper Word Register
#define ENX_REG_TQWPL			0x08EC			// Teletext Queue Write Pointer - Lower Word Register
#define ENX_REG_TQWPH			0x08EE			// Teletext Queue Write Pointer - Upper Word Register
#define ENX_REG_VQRPL			0x08F0			// Video Queue Read Pointer, Lower Word Register
#define ENX_REG_VQRPH			0x08F2			// Video Queue Read Pointer, Upper Word Register
#define ENX_REG_VQWPL			0x08F4			// Video Queue Write Pointer, Lower Word Register
#define ENX_REG_VQWPH			0x08F6			// Video Queue Write Pointer, Upper Word Register
#define ENX_REG_RBR_1			0x0700			// Receive Buffer Register
#define ENX_REG_RBR_2			0x0740			// Receive Buffer Register
#define ENX_REG_THR_1			0x0700			// Transmit Holding Register
#define ENX_REG_THR_2			0x0740			// Transmit Holding Register
#define ENX_REG_IER_1			0x0704			// Interrupt Enable Register
#define ENX_REG_IER_2			0x0744			// Interrupt Enable Register
#define ENX_REG_DLL_1			0x0700			// Divisor Latch- LSB Register
#define ENX_REG_DLL_2			0x0740			// Divisor Latch- LSB Register
#define ENX_REG_DLM_1			0x0704			// Divisor Latch-MSB Register
#define ENX_REG_DLM_2			0x0744			// Divisor Latch-MSB Register
#define ENX_REG_IIR_1			0x0708			// Interrupt Identification Register
#define ENX_REG_IIR_2			0x0748			// Interrupt Identification Register
#define ENX_REG_FCR_1			0x0708			// FIFO Control Register
#define ENX_REG_FCR_2			0x0748			// FIFO Control Register
#define ENX_REG_LCR_1			0x070C			// Line Control Register
#define ENX_REG_LCR_2			0x074C			// Line Control Register
#define ENX_REG_MCR_1			0x0710			// Modem Control Register
#define ENX_REG_MCR_2			0x0750			// Modem Control Register
#define ENX_REG_LSR_1			0x0714			// Line Status Register
#define ENX_REG_LSR_2			0x0754			// Line Status Register
#define ENX_REG_MSR_1			0x0718			// Modem Status Register
#define ENX_REG_MSR_2			0x0758			// Modem Status Register
#define ENX_REG_SPR_1			0x071C			// Scratch Pad Register
#define ENX_REG_SPR_2			0x075C			// Scratch Pad Register
#define ENX_REG_QWPnL			0x0880
#define ENX_REG_QWPnH			0x0882
#define ENX_REG_QnINT			0x08C0

#define ENX_IRQ_REG_ISR0		0
#define ENX_IRQ_REG_ISR1		1
#define ENX_IRQ_REG_ISR2		2
#define ENX_IRQ_REG_ISR3		3
#define ENX_IRQ_REG_ISR4		4
#define ENX_IRQ_REG_ISR5		5

#define ENX_IRQ_IR_TX			AVIA_GT_IRQ(ENX_IRQ_REG_ISR0, 1)
#define ENX_IRQ_IR_RX			AVIA_GT_IRQ(ENX_IRQ_REG_ISR0, 2)
#define ENX_IRQ_PCM_PF			AVIA_GT_IRQ(ENX_IRQ_REG_ISR0, 3)
#define ENX_IRQ_PCM_AD			AVIA_GT_IRQ(ENX_IRQ_REG_ISR0, 4)
#define ENX_IRQ_VL1				AVIA_GT_IRQ(ENX_IRQ_REG_ISR0, 5)
#define ENX_IRQ_VL2				AVIA_GT_IRQ(ENX_IRQ_REG_ISR0, 6)
#define ENX_IRQ_CAPTURE			AVIA_GT_IRQ(ENX_IRQ_REG_ISR0, 7)
#define ENX_IRQ_PCR				AVIA_GT_IRQ(ENX_IRQ_REG_ISR1, 5)
#define ENX_IRQ_TT				AVIA_GT_IRQ(ENX_IRQ_REG_ISR1, 8)

#pragma pack(1)

typedef struct {

  unsigned char Reserved1: 3;
  unsigned char VRQ: 1;
  unsigned char ARQ: 1;
  unsigned char VEN: 1;
  unsigned char AEN: 1;
  unsigned char Reserved2: 1;
  unsigned char SER_MAX_RT: 2;
  unsigned char Reseved3: 2;
  unsigned char AUDIO_XFT_RATE: 4;

} sENX_REG_AVI_0;

typedef struct {

  unsigned char AlphaOut: 8;
  unsigned char AlphaIn: 8;

} sENX_REG_BALP;

typedef struct {

  unsigned char PCMA: 1;
  unsigned char PAR1284: 1;
  unsigned char UART2: 1;
  unsigned char Reserved1: 1;
  unsigned char SCEN: 1;
  unsigned char ORDR: 1;
  unsigned char LATE: 1;
  unsigned char DOE: 1;
  unsigned char BUS_TIMEOUT;
  unsigned char BRD_ID;
  unsigned char BIU_SEL: 2;
  unsigned char Reserved2: 1;
  unsigned char UPQ: 1;
  unsigned char TCP: 1;
  unsigned char Reserved3: 1;
  unsigned char ACP: 1;
  unsigned char VCP: 1;

} sENX_REG_CFGR0;

typedef struct {

  unsigned char W: 1;
  unsigned char C: 1;
  unsigned char P: 1;
  unsigned char N: 1;
  unsigned char T: 1;
  unsigned char D: 1;
  unsigned char Cmd: 2;
  unsigned char Reserved1: 2;
  unsigned char Len: 6;

} sENX_REG_CPCCMD;

/*typedef struct {

  union {

    struct {

      unsigned int CRC;

    } CRC;

    struct {

      unsigned char Reserved1: 8;
      unsigned int Addr: 24;

    } Composite;

  };

} sENX_REG_CPCCRCSRC2;*/

typedef struct {

  unsigned char Reserved1: 8;
  unsigned int Addr: 24;

} sENX_REG_CPCDST;

typedef struct {

  unsigned char Reserved1: 8;
  unsigned int Addr: 24;

} sENX_REG_CPCSRC1;

typedef struct {

  unsigned char ARCH_ID: 8;
  unsigned char API_VERSION: 8;
  unsigned char VERSION: 8;
  unsigned char REVISION: 8;

} sENX_REG_CRR;

typedef struct {

  unsigned char Reserved1: 4;
  unsigned short CarrierWavePeriod: 12;

} sENX_REG_CWP;

typedef struct {

  unsigned char Reserved1: 5;
  unsigned short WavePulseHigh: 11;

} sENX_REG_CWPH;

typedef struct {

  unsigned char Reserved1: 6;
  unsigned short PC: 10;

} sENX_REG_EPC;

typedef struct {

  unsigned char FE: 1;
  unsigned char FH: 1;
  unsigned char TEI: 1;
  unsigned char DM: 1;
  unsigned char CDP: 1;
  unsigned char CCP: 1;
  unsigned char DO: 1;
  unsigned char FD: 1;
  unsigned char Sync_Byte: 8;

} sENX_REG_FC;

typedef struct {

  unsigned char BLEV11;
  unsigned char BLEV10;

} sENX_REG_GBLEV1;

typedef struct {

  unsigned short Reserved1: 14;
  unsigned char CFT: 2;

} sENX_REG_G1CFR;

#define sENX_REG_G2CFR sENX_REG_G1CFR

typedef struct {

  unsigned char L: 1;
  unsigned char F: 1;
  unsigned char I: 1;
  unsigned char P: 1;
  unsigned char S: 2;
  unsigned char C: 1;
  unsigned char B: 1;
  unsigned char GMD: 4;
  unsigned char BANK: 4;
  unsigned short STRIDE: 14;
  unsigned char Reserved1: 2;

} sENX_REG_GMR1;

typedef struct {

  unsigned char SPP: 5;
  unsigned char Reserved1: 1;
  unsigned short XPOS: 10;
  unsigned char Reserved2: 6;
  unsigned short YPOS: 10;

} sENX_REG_GVP1;

typedef struct {

  unsigned char Reserved1: 8;
  unsigned int Addr: 22;
  unsigned char Reserved2: 2;

} sENX_REG_GVSA1;

typedef struct {

  unsigned char IPP: 5;
  unsigned char Reserved1: 1;
  unsigned short XSZ: 10;
  unsigned char Reserved2: 6;
  unsigned short YSZ: 10;

} sENX_REG_GVSZ1;

typedef struct {

  unsigned char E: 1;
  unsigned char L: 1;
  unsigned char Reserved1: 7;
  unsigned char Offset: 7;

} sENX_REG_IRRE;

typedef struct {

  unsigned short Reserved1: 9;
  unsigned char Offset: 7;

} sENX_REG_IRRO;

typedef struct {

  unsigned char E: 1;
  unsigned char C: 1;
  unsigned char Reserved1: 7;
  unsigned char Offset: 7;

} sENX_REG_IRTE;

typedef struct {

  unsigned short Reserved1: 9;
  unsigned char Offset: 7;

} sENX_REG_IRTO;

typedef struct {

  unsigned char Reserved1: 8;
  unsigned short Addr: 15;
  unsigned short Reserved2: 9;

} sENX_REG_IRQA;

typedef struct {

  unsigned char Latched_STC_Base: 1;
  unsigned char Reserved1: 6;
  unsigned short Latched_STC_Extension: 9;

} sENX_REG_LC_STC_0;

typedef struct {

  unsigned short Latched_STC_Base: 16;

} sENX_REG_LC_STC_1;

#define sENX_REG_LC_STC_2 sENX_REG_LC_STC_1

typedef struct {

  unsigned char Reserved1: 8;
  unsigned int Addr: 23;
  unsigned char W: 1;

} sENX_REG_PCMA;

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
  unsigned char Reserved: 2;
  unsigned char ACD: 2;
  unsigned char BCD: 2;

} sENX_REG_PCMC;

typedef struct {

  unsigned char PCMAL: 7;
  unsigned char Reserved1: 1;
  unsigned char PCMAR: 7;
  unsigned char Reserved2: 1;
  unsigned char MPEGAL: 7;
  unsigned char Reserved3: 1;
  unsigned char MPEGAR: 7;
  unsigned char Reserved4: 1;

} sENX_REG_PCMN;

typedef struct {

  unsigned char Reserved1: 4;
  unsigned short NSAMP: 12;

} sENX_REG_PCMS;

typedef struct {

	u8 Reserved1: 2;
	u8 E: 1;
	u16 PID: 13;

} sENX_REG_PCR_PID;

typedef struct {

  unsigned char Reserved1: 6;
  unsigned char Q_Size: 4;
  unsigned char Queue_n_Write_Pointer: 6;

//} sENX_REG_QWPnH __attribute__ ((packed));
} sENX_REG_QWPnH;

typedef struct {

  unsigned short Queue_n_Write_Pointer: 16;

} sENX_REG_QWPnL __attribute__ ((packed));

typedef struct {

  unsigned char Reserved1: 7;
  unsigned char P: 1;
  unsigned char Filt_H: 4;
  unsigned char Filt_L: 4;

} sENX_REG_RFR;

typedef struct {

  unsigned char Reserved1: 8;
  unsigned char RTCH: 8;

} sENX_REG_RPH;

typedef struct {

  unsigned char FRCH: 1;
  unsigned char SC2: 1;
  unsigned char SC1: 1;
  unsigned char PCM: 1;
  unsigned char AVI: 1;
  unsigned char IR: 1;
  unsigned char Reserved1: 2;
  unsigned char FRMR: 1;
  unsigned char TDMP: 1;
  unsigned char TTX: 1;
  unsigned char DAC: 1;
  unsigned char Reserved2: 1;
  unsigned char IDC: 1;
  unsigned char PIG2: 1;
  unsigned char Reserved3: 1;
  unsigned char SPRC: 1;
  unsigned char Reserved4: 1;
  unsigned char QUE: 1;
  unsigned char SDCT: 1;
  unsigned char GFIX: 1;
  unsigned char VIDC: 1;
  unsigned char VDEO: 1;
  unsigned char Reserved5: 1;
  unsigned char PIG1: 1;
  unsigned char BLIT: 1;
  unsigned char COPY: 1;
  unsigned char TIMR: 1;
  unsigned char UART2: 1;
  unsigned char UART1: 1;
  unsigned char PAR1284: 1;
  unsigned char PCMA: 1;

} sENX_REG_RSTR0;

typedef struct {

  unsigned char Reserved1: 7;
  unsigned char S: 1;
  unsigned char RTC: 8;

} sENX_REG_RTC;

typedef struct {

  unsigned char Reserved1: 3;
  unsigned short TickPeriod: 13;

} sENX_REG_RTP;

typedef struct {

  unsigned char STC_Count: 1;
  unsigned char Reserved1: 6;
  unsigned short STC_Extension: 9;

} sENX_REG_STC_COUNTER_0;

typedef struct {

  unsigned short STC_Count: 16;

} sENX_REG_STC_COUNTER_1;

#define sENX_REG_STC_COUNTER_2 sENX_REG_STC_COUNTER_1

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

} sENX_REG_TCNTL;

typedef struct {

  unsigned char Reserved1: 7;
  unsigned char E: 1;
  unsigned char Red: 8;
  unsigned char Green: 8;
  unsigned char Blue: 8;

} sENX_REG_TCR1;

#define sENX_REG_TCR2 sENX_REG_TCR1

typedef struct {

  unsigned char PCR_Base: 1;
  unsigned char Reserved1: 6;
  unsigned short PCR_Extension: 9;

} sENX_REG_TP_PCR_0;

typedef struct {

  unsigned short PCR_Base: 16;

} sENX_REG_TP_PCR_1;

#define sENX_REG_TP_PCR_2 sENX_REG_TP_PCR_1

typedef struct {

  unsigned char PTS: 1;
  unsigned char R: 1;
  unsigned char E: 1;
  unsigned char Reserved1: 8;
  unsigned char State: 5;

} sENX_REG_TSTATUS;

typedef struct {

  unsigned char Reserved1: 7;
  unsigned char E: 1;
  unsigned char Y: 8;
  unsigned char Cr: 8;
  unsigned char Cb: 8;

} sENX_REG_VBR;

typedef struct {

  unsigned short Reserved1: 11;
  unsigned int Offset: 19;
  unsigned char Reserved2: 2;

} sENX_REG_VCOFFS;

typedef struct {

  unsigned char Reserved1: 6;
  unsigned short HPOS: 9;
  unsigned char U: 1;
  unsigned char Reserved2: 2;
  unsigned char OVOFFS: 4;
  unsigned short EVPOS: 9;
  unsigned char Reserved3: 1;

} sENX_REG_VCP;

typedef struct {

  unsigned char S: 1;
  unsigned char P: 1;
  unsigned char C: 1;
  unsigned char Reserved1: 1;
  unsigned char HP: 2;
  unsigned char FP: 2;
  unsigned char E: 1;
  unsigned char D: 1;
  unsigned char Reserved2: 6;

} sENX_REG_VCR;

typedef struct {

  unsigned char Reserved1: 8;
  unsigned int Addr: 22;
  unsigned char Reserved2: 1;
  unsigned char E: 1;

} sENX_REG_VCSA1;

typedef struct {

  unsigned char Reserved1: 8;
  unsigned int Addr: 22;
  unsigned char Reserved2: 2;

} sENX_REG_VCSA2;

typedef struct {

  unsigned char Reserved1: 3;
  unsigned short STRIDE: 11;
  unsigned char Reserved2: 1;
  unsigned char B: 1;

} sENX_REG_VCSTR;

typedef struct {

  unsigned char HDEC: 4;
  unsigned char Reserved1: 2;
  unsigned short HSIZE: 9;
  unsigned char F: 1;
  unsigned char VDEC: 4;
  unsigned char Reserved2: 2;
  unsigned short VSIZE: 9;
  unsigned char B: 1;

} sENX_REG_VCSZ;

typedef struct {

  unsigned char Reserved1: 6;
  unsigned short LINE: 9;
  unsigned char F: 1;

} sENX_REG_VLC;

typedef struct {

  unsigned char E: 1;
  unsigned char Reserved1: 5;
  unsigned short LINE: 9;
  unsigned char F: 1;

} sENX_REG_VLI1;

#define sENX_REG_VLI2 sENX_REG_VLI1

typedef struct {

  unsigned short Reserved1: 14;
  unsigned char FFM: 2;

} sENX_REG_VMCR;

typedef struct {

  unsigned short Reserved1: 11;
  unsigned int OFFSET: 19;
  unsigned char Reserved2: 2;

} sENX_REG_VPOFFS1;

#define sENX_REG_VPOFFS2 sENX_REG_VPOFFS1

typedef struct {

  unsigned char Reserved1: 6;
  unsigned short HPOS: 9;
  unsigned char U: 1;
  unsigned char Reserved2: 6;
  unsigned short VPOS: 9;
  unsigned char F: 1;

} sENX_REG_VPP1;

typedef struct {

  unsigned char Reserved1: 8;
  unsigned int Addr: 22;
  unsigned char Reserved: 1;
  unsigned char E: 1;

} sENX_REG_VPSA1;

#define sENX_REG_VPSA2 sENX_REG_VPSA1

typedef struct {

  unsigned short Reserved1: 14;
  unsigned char SO: 2;

} sENX_REG_VPSO1;

#define sENX_REG_VPSO2 sENX_REG_VPSO1

typedef struct {

  unsigned char Reserved1: 6;
  unsigned short WIDTH: 9;
  unsigned char S: 1;
  unsigned char Reserved2: 6;
  unsigned short HEIGHT: 9;
  unsigned char P: 1;

} sENX_REG_VPSZ1;

typedef struct {

  unsigned short Video_Queue_Read_Pointer: 16;

} sENX_REG_VQRPL;

typedef struct {

  unsigned short Reserved1: 10;
  unsigned char Video_Queue_Read_Pointer: 6;

} sENX_REG_VQRPH;

typedef struct {

  unsigned short Video_Queue_Write_Pointer: 16;

} sENX_REG_VQWPL;

typedef struct {

  unsigned char Reserved1: 6;
  unsigned char Q_Size: 4;
  unsigned char Video_Queue_Write_Pointer: 6;

} sENX_REG_VQWPH;

#pragma pack()

extern void avia_gt_enx_clear_irq(unsigned char irq_reg, unsigned char irq_bit);
extern unsigned short avia_gt_enx_get_irq_mask(unsigned char irq_reg);
extern unsigned short avia_gt_enx_get_irq_status(unsigned char irq_reg);
extern void avia_gt_enx_mask_irq(unsigned char irq_reg, unsigned char irq_bit);
extern void avia_gt_enx_unmask_irq(unsigned char irq_reg, unsigned char irq_bit);
extern void avia_gt_enx_init(void);
extern void avia_gt_enx_exit(void);

#define enx_reg_16(register) ((unsigned short)(*((unsigned short*)(gt_info->reg_addr + ENX_REG_ ## register))))
#define enx_reg_16n(offset) ((unsigned short)(*((unsigned short*)(gt_info->reg_addr + offset))))
#define enx_reg_32(register) ((unsigned int)(*((unsigned int*)(gt_info->reg_addr + ENX_REG_ ## register))))
#define enx_reg_32o(register, offset) ((unsigned int)(*((unsigned int*)(gt_info->reg_addr + ENX_REG_ ## register + offset))))
#define enx_reg_32n(offset) ((unsigned int)(*((unsigned int*)(gt_info->reg_addr + offset))))
#define enx_reg_o(offset) (gt_info->reg_addr + offset)
#define enx_reg_s(register) ((sENX_REG_##register *)(&enx_reg_32(register)))
#define enx_reg_sn(register, offset) ((sENX_REG_##register *)(enx_reg_o(offset)))
#define enx_reg_so(register, offset) ((sENX_REG_##register *)(&enx_reg_32o(register, offset)))
#define enx_reg_32s(register) ((sENX_REG_##register *)(&enx_reg_32(register)))
#define enx_reg_16s(register) ((sENX_REG_##register *)(&enx_reg_16(register)))

#define enx_reg_set_32s(register, field, value) { u32 tmp_reg_val = enx_reg_32(register); ((sENX_REG_##register *)&tmp_reg_val)->field = value; enx_reg_32(register) = tmp_reg_val; }
#define enx_reg_set_16s(register, field, value) { u16 tmp_reg_val = enx_reg_16(register); ((sENX_REG_##register *)&tmp_reg_val)->field = value; enx_reg_16(register) = tmp_reg_val; }
#define enx_reg_set(register, field, value) do { if (sizeof(sENX_REG_##register) == 4) enx_reg_set_32s(register, field, value) else if (sizeof(sENX_REG_##register ) == 2) enx_reg_set_16s(register, field, value) else printk("ERROR: struct size is %d\n", sizeof(sENX_REG_##register)); } while(0)
#define enx_reg_set_bit(register, bit) enx_reg_set(register, bit, 1)
#define enx_reg_clear_bit(register, bit) enx_reg_set(register, bit, 0)

#endif
