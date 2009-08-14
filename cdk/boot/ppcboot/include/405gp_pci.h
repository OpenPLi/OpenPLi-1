#ifndef _405GP_PCI_H
#define _405GP_PCI_H

typedef struct pciHeaderDevice
    {
    short	vendorId;	/* vendor ID */
    short	deviceId;	/* device ID */
    short	command;	/* command register */
    short	status;		/* status register */
    char	revisionId;	/* revision ID */
    char	classCode;	/* class code */
    char	subClass;	/* sub class code */
    char	progIf;		/* programming interface */
    char	cacheLine;	/* cache line */
    char	latency;	/* latency time */
    char	headerType;	/* header type */
    char	bist;		/* BIST */
    int		base0;		/* base address 0 */
    int		base1;		/* base address 1 */
    int		base2;		/* base address 2 */
    int		base3;		/* base address 3 */
    int		base4;		/* base address 4 */
    int		base5;		/* base address 5 */
    int		cis;		/* cardBus CIS pointer */
    short	subVendorId;	/* sub system vendor ID */
    short	subSystemId;	/* sub system ID */
    int		romBase;	/* expansion ROM base address */
    int		reserved0;	/* reserved */
    int		reserved1;	/* reserved */
    char	intLine;	/* interrupt line */
    char	intPin;		/* interrupt pin */
    char	minGrant;	/* min Grant */
    char	maxLatency;	/* max Latency */
    } PCI_HEADER_DEVICE;

typedef struct pciHeaderBridge
    {
    short	vendorId;	/* vendor ID */
    short	deviceId;	/* device ID */
    short	command;	/* command register */
    short	status;		/* status register */
    char	revisionId;	/* revision ID */
    char	classCode;	/* class code */
    char	subClass;	/* sub class code */
    char	progIf;		/* programming interface */
    char	cacheLine;	/* cache line */
    char	latency;	/* latency time */
    char	headerType;	/* header type */
    char	bist;		/* BIST */
    int		base0;		/* base address 0 */
    int		base1;		/* base address 1 */
    char	priBus;		/* primary bus number */
    char	secBus;		/* secondary bus number */
    char	subBus;		/* subordinate bus number */
    char	secLatency;	/* secondary latency timer */
    char	ioBase;		/* IO base */
    char	ioLimit;	/* IO limit */
    short	secStatus;	/* secondary status */
    short	memBase;	/* memory base */
    short	memLimit;	/* memory limit */
    short	preBase;	/* prefetchable memory base */
    short	preLimit;	/* prefetchable memory limit */
    int		preBaseUpper;	/* prefetchable memory base upper 32 bits */
    int		preLimitUpper;	/* prefetchable memory base upper 32 bits */
    short	ioBaseUpper;	/* IO base upper 16 bits */
    short	ioLimitUpper;	/* IO limit upper 16 bits */
    int		reserved;	/* reserved */
    int		romBase;	/* expansion ROM base address */
    char	intLine;	/* interrupt line */
    char	intPin;		/* interrupt pin */
    short	control;	/* bridge control */
    } PCI_HEADER_BRIDGE;

/*----------------------------------------------------------------------------+
| 405GP PCI core memory map defines.
+----------------------------------------------------------------------------*/
#define MIN_PCI_MEMADDR1    0x80000000
#define MIN_PCI_MEMADDR2    0x00000000
#define MIN_PLB_PCI_IOADDR  0xE8000000  /* PLB side of PCI I/O address space */
#define MIN_PCI_PCI_IOADDR  0x00000000  /* PCI side of PCI I/O address space */
/*#define MAX_PCI_DEVICES     5*/ /* test-only */
#define MAX_PCI_DEVICES     32

/*----------------------------------------------------------------------------+
| Defines for the 405GP PCI Config address and data registers followed by
| defines for the standard PCI device configuration header.
+----------------------------------------------------------------------------*/
#define PCICFGADR       0xEEC00000
#define PCICFGDATA      0xEEC00004

#define PCIVENDORID     0x00
#define PCIDEVICEID     0x02
#define PCICMD          0x04
  #define SERR_EN       0x0100
  #define BM_EN         0x0004
  #define MEM_EN        0x0002
  #define IO_EN         0x0001
#define PCISTATUS       0x06
  #define CAPABLE_66MHZ 0x0020
#define PCIREVID        0x08
#define PCICLASSPI      0x09
#define PCICLASSCODE    0x0A
#define PCICACHELINE    0x0C
#define PCILATENCYTIM   0x0D
#define PCIHEADERTYPE   0x0E
#define PCIBIST         0x0F
#define PCIBASEADDR0    0x10
#define PCIBASEADDR1    0x14
#define PCIBASEADDR2    0x18
#define PCIINTLINE      0x3C
#define PCIINTPIN       0x3D
#define PCIMINGRANT     0x3E
#define PCIMAXLATENCY   0x3F

#define PCIBUSNUM       0x40        /* 405GP specific parameters */
#define PCISUBBUSNUM    0x41
#define PCIDISCOUNT     0x42
#define PCIBRDGOPT1     0x4A
#define PCIBRDGOPT2     0x60

/*----------------------------------------------------------------------------+
| Defines for 405GP PCI Master local configuration regs.
+----------------------------------------------------------------------------*/
#define PMM0LA          0xEF400000
#define PMM0MA          0xEF400004
#define PMM0PCILA       0xEF400008
#define PMM0PCIHA       0xEF40000C
#define PMM1LA          0xEF400010
#define PMM1MA          0xEF400014
#define PMM1PCILA       0xEF400018
#define PMM1PCIHA       0xEF40001C
#define PMM2LA          0xEF400020
#define PMM2MA          0xEF400024
#define PMM2PCILA       0xEF400028
#define PMM2PCIHA       0xEF40002C

/*----------------------------------------------------------------------------+
| Defines for 405GP PCI Target local configuration regs.
+----------------------------------------------------------------------------*/
#define PTM1MS          0xEF400030
#define PTM1LA          0xEF400034
#define PTM2MS          0xEF400038
#define PTM2LA          0xEF40003C

/*-----------------------------------------------------------------------------+
|       PCI-PCI bridge header
+-----------------------------------------------------------------------------*/
#define PCIPCI_PRIMARYBUS       0x18
#define PCIPCI_SECONDARYBUS     0x19
#define PCIPCI_SUBORDINATEBUS   0x1A
#define PCIPCI_SECONDARYLATENCY 0x1B
#define PCIPCI_IOBASE           0x1C
#define PCIPCI_IOLIMIT          0x1D
#define PCIPCI_SECONDARYSTATUS  0x1E
#define PCIPCI_MEMBASE          0x20
#define PCIPCI_MEMLIMIT         0x22
#define PCIPCI_PREFETCHMEMBASE  0x24
#define PCIPCI_PREFETCHMEMLIMIT 0x26
#define PCIPCI_IOBASEUPPER16    0x30
#define PCIPCI_IOLIMITUPPER16   0x32

#define PCIDEVID_405GP 	0x0

/* Standard device configuration register offsets */
/* Note that only modulo-4 addresses are written to the address register */

#define	PCI_CFG_VENDOR_ID	0x00
#define	PCI_CFG_DEVICE_ID	0x02
#define	PCI_CFG_COMMAND		0x04
#define	PCI_CFG_STATUS		0x06
#define	PCI_CFG_REVISION	0x08
#define	PCI_CFG_PROGRAMMING_IF	0x09
#define	PCI_CFG_SUBCLASS	0x0a
#define	PCI_CFG_CLASS		0x0b
#define	PCI_CFG_CACHE_LINE_SIZE	0x0c
#define	PCI_CFG_LATENCY_TIMER	0x0d
#define	PCI_CFG_HEADER_TYPE	0x0e
#define	PCI_CFG_BIST		0x0f
#define	PCI_CFG_BASE_ADDRESS_0	0x10
#define	PCI_CFG_BASE_ADDRESS_1	0x14
#define	PCI_CFG_BASE_ADDRESS_2	0x18
#define	PCI_CFG_BASE_ADDRESS_3	0x1c
#define	PCI_CFG_BASE_ADDRESS_4	0x20
#define	PCI_CFG_BASE_ADDRESS_5	0x24
#define	PCI_CFG_CIS		0x28
#define	PCI_CFG_SUB_VENDER_ID	0x2c
#define	PCI_CFG_SUB_SYSTEM_ID	0x2e
#define	PCI_CFG_EXPANSION_ROM	0x30
#define	PCI_CFG_RESERVED_0	0x34
#define	PCI_CFG_RESERVED_1	0x38
#define	PCI_CFG_DEV_INT_LINE	0x3c
#define	PCI_CFG_DEV_INT_PIN	0x3d
#define	PCI_CFG_MIN_GRANT	0x3e
#define	PCI_CFG_MAX_LATENCY	0x3f
#define PCI_CFG_SPECIAL_USE     0x41
#define PCI_CFG_MODE            0x43


/* PCI-to-PCI bridge configuration register offsets */
/* Note that only modulo-4 addresses are written to the address register */

#define	PCI_CFG_PRIMARY_BUS	0x18
#define	PCI_CFG_SECONDARY_BUS	0x19
#define	PCI_CFG_SUBORDINATE_BUS	0x1a
#define	PCI_CFG_SEC_LATENCY	0x1b
#define	PCI_CFG_IO_BASE		0x1c
#define	PCI_CFG_IO_LIMIT	0x1d
#define	PCI_CFG_SEC_STATUS	0x1e
#define	PCI_CFG_MEM_BASE	0x20
#define	PCI_CFG_MEM_LIMIT	0x22
#define	PCI_CFG_PRE_MEM_BASE	0x24
#define	PCI_CFG_PRE_MEM_LIMIT	0x26
#define	PCI_CFG_PRE_MEM_BASE_U	0x28
#define	PCI_CFG_PRE_MEM_LIMIT_U	0x2c
#define	PCI_CFG_IO_BASE_U	0x30
#define	PCI_CFG_IO_LIMIT_U	0x32
#define	PCI_CFG_ROM_BASE	0x38
#define	PCI_CFG_BRG_INT_LINE	0x3c
#define	PCI_CFG_BRG_INT_PIN	0x3d
#define	PCI_CFG_BRIDGE_CONTROL	0x3e



unsigned int    PCI_Read_CFG_Reg(int BusDevFunc, int Reg, int Width);
int  		PCI_Write_CFG_Reg(int BusDevFunc, int Reg, unsigned int Value, int Width);
void            PCI_Scan(int BusNum);
void            PCI_Config_Device(int BusDevFunc, int NumBaseAddr);
void            PCI_Config_VGA_Device(int BusDevFunc, int NumBaseAddr);
void            PCI_Config_Bridge(int BusDevFunc);
void            PCI_Dump_Device(int BusDevFunc);
int             PCI_Find_Device(unsigned short VendorID, unsigned short DeviceID);
void            pciHeaderShow(int BusDevFunc);
void            pciDheaderPrint(PCI_HEADER_DEVICE * pD);
void            pciBheaderPrint(PCI_HEADER_BRIDGE * pB);

#endif
