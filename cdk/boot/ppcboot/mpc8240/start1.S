Analyzing: d:\ppcboot\mpc8240\start1.o
Header idenfication: 0x7f, ELF, 32 bit MSB v1
    type:           1, machine:         20
    version: 00000001, entry:     00000000, phoff:  00000000
    shoff:   00003410, flags:     00000000, ehsize:       52, phentsize:  0
    phnum:          0, shentsize:       40, shnum:        10, shstrndx:  7


==> .shstrtab
    [  1]	.symtab
    [  9]	.strtab
    [ 17]	.shstrtab
    [ 27]	.text
    [ 33]	.rela.text
    [ 44]	.data
    [ 50]	.bss
    [ 55]	.got2
    [ 61]	.rela.got2

==> .text [1]: Section Header
    type:    00000001, flags:  00000006
    address: 00000000, offset: 00000034, size: 00003374
    link:    00000000, info:   00000000, alignment: 1, entry size: 0


==> .rela.text [2]: Section Header
    type:    00000004, flags:  00000000
    address: 00000000, offset: 0000416c, size: 000001a4
    link:    00000008, info:   00000001, alignment: 4, entry size: 12


==> .data [3]: Section Header
    type:    00000001, flags:  00000003
    address: 00000000, offset: 000033a8, size: 00000000
    link:    00000000, info:   00000000, alignment: 1, entry size: 0


==> .bss [4]: Section Header
    type:    00000008, flags:  00000003
    address: 00000000, offset: 000033a8, size: 00000000
    link:    00000000, info:   00000000, alignment: 1, entry size: 0


==> .got2 [5]: Section Header
    type:    00000001, flags:  00000003
    address: 00000000, offset: 000033a8, size: 00000020
    link:    00000000, info:   00000000, alignment: 1, entry size: 0


==> .rela.got2 [6]: Section Header
    type:    00000004, flags:  00000000
    address: 00000000, offset: 00004310, size: 00000060
    link:    00000008, info:   00000005, alignment: 4, entry size: 12


==> .shstrtab [7]: Section Header
    type:    00000003, flags:  00000000
    address: 00000000, offset: 000033c8, size: 00000048
    link:    00000000, info:   00000000, alignment: 1, entry size: 0


==> .symtab [8]: Section Header
    type:    00000002, flags:  00000000
    address: 00000000, offset: 000035a0, size: 00000680
    link:    00000009, info:   0000004a, alignment: 4, entry size: 16


==> .strtab [9]: Section Header
    type:    00000003, flags:  00000000
    address: 00000000, offset: 00003c20, size: 00000549
    link:    00000000, info:   00000000, alignment: 1, entry size: 0


==> .strtab
    [  1]	start.S
    [  9]	/cygdrive/d/ppcboot/include/asm/mmu.h
    [ 47]	/cygdrive/d/ppcboot/include/linux/config.h
    [ 90]	/cygdrive/d/ppcboot/include/asm/cache.h
    [ 130]	/cygdrive/d/ppcboot/include/asm/processor.h
    [ 174]	/cygdrive/d/ppcboot/include/asm/types.h
    [ 214]	/cygdrive/d/ppcboot/include/asm/ptrace.h
    [ 255]	/cygdrive/d/ppcboot/include/ppc_defs.h
    [ 294]	/cygdrive/d/ppcboot/include/ppc_asm.tmpl
    [ 335]	/cygdrive/d/ppcboot/include/version.h
    [ 373]	/cygdrive/d/ppcboot/include/mpc8240.h
    [ 411]	/cygdrive/d/ppcboot/include/config.h
    [ 448]	/cygdrive/d/ppcboot/include/config_Sandpoint8240.h
    [ 499]	boot_cold
    [ 509]	boot_warm
    [ 519]	in_flash
    [ 528]	MachineCheck
    [ 541]	int_return
    [ 552]	DataStorage
    [ 564]	InstStorage
    [ 576]	ExtInterrupt
    [ 589]	Alignment
    [ 599]	ProgramCheck
    [ 612]	FPUnavailable
    [ 626]	Decrementer
    [ 638]	Trap_0a
    [ 646]	Trap_0b
    [ 654]	SystemCall
    [ 665]	SingleStep
    [ 676]	Trap_0e
    [ 684]	Trap_0f
    [ 692]	InstructionTransMiss
    [ 713]	DataLoadTransMiss
    [ 731]	DataStoreTransMiss
    [ 750]	InstructionBreakpoint
    [ 772]	SysManageInt
    [ 785]	Reserved5
    [ 795]	Reserved6
    [ 805]	Reserved7
    [ 815]	Reserved8
    [ 825]	Reserved9
    [ 835]	ReservedA
    [ 845]	ReservedB
    [ 855]	ReservedC
    [ 865]	ReservedD
    [ 875]	ReservedE
    [ 885]	ReservedF
    [ 895]	RunModeTrace
    [ 908]	NotMax
    [ 915]	clear_bss
    [ 925]	trap_reloc
    [ 936]	_GOT2_TABLE_
    [ 949]	_FIXUP_TABLE_
    [ 963]	_start
    [ 970]	_start_of_vectors
    [ 988]	_end_of_vectors
    [ 1004]	transfer_to_handler
    [ 1024]	_end
    [ 1029]	version_string
    [ 1044]	_start_warm
    [ 1056]	cpu_init_f
    [ 1067]	board_init_f
    [ 1080]	MachineCheckException
    [ 1102]	UnknownException
    [ 1119]	external_interrupt
    [ 1138]	AlignmentException
    [ 1157]	ProgramCheckException
    [ 1179]	timer_interrupt
    [ 1195]	icache_enable
    [ 1209]	icache_disable
    [ 1224]	icache_status
    [ 1238]	dcache_enable
    [ 1252]	dcache_disable
    [ 1267]	dcache_status
    [ 1281]	dc_read
    [ 1289]	get_pvr
    [ 1297]	udelay
    [ 1304]	relocate_code
    [ 1318]	board_init_r
    [ 1331]	get_endaddr
    [ 1343]	trap_init

==> .symtab
    entry  spndx   value      size   bind   type  other    shndx         name
    [   0]     0   00000000      0   LOC    NULL      0    UND           
    [   1]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [   2]     9   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/mmu.h
    [   3]    47   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/linux/config.h
    [   4]     9   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/mmu.h
    [   5]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [   6]    90   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/cache.h
    [   7]   130   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/processor.h
    [   8]   174   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/types.h
    [   9]   130   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/processor.h
    [  10]   214   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/ptrace.h
    [  11]    47   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/linux/config.h
    [  12]   214   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/ptrace.h
    [  13]   130   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/processor.h
    [  14]    47   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/linux/config.h
    [  15]   130   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/processor.h
    [  16]    90   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/cache.h
    [  17]    47   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/linux/config.h
    [  18]    90   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/asm/cache.h
    [  19]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [  20]   255   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/ppc_defs.h
    [  21]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [  22]   294   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/ppc_asm.tmpl
    [  23]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [  24]   335   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/version.h
    [  25]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [  26]   373   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/mpc8240.h
    [  27]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [  28]   411   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/config.h
    [  29]   448   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/config_Sandpoint8240.h
    [  30]   411   00000000      0   LOC    FILE      0    ABS           /cygdrive/d/ppcboot/include/config.h
    [  31]     1   00000000      0   LOC    FILE      0    ABS           start.S
    [  32]     0   00000000      0   LOC    SECT      0    .text         .text
    [  33]     0   00000000      0   LOC    SECT      0    .data         .data
    [  34]     0   00000000      0   LOC    SECT      0    .bss          .bss
    [  35]     0   00000000      0   LOC    SECT      0    .got2         .got2
    [  36]   499   00000118      0   LOC    NULL      0    .text         boot_cold
    [  37]   509   00000118      0   LOC    NULL      0    .text         boot_warm
    [  38]   519   00000148      0   LOC    NULL      0    .text         in_flash
    [  39]   528   00000200      0   LOC    NULL      0    .text         MachineCheck
    [  40]   541   0000309c      0   LOC    NULL      0    .text         int_return
    [  41]   552   00000300      0   LOC    NULL      0    .text         DataStorage
    [  42]   564   00000400      0   LOC    NULL      0    .text         InstStorage
    [  43]   576   00000500      0   LOC    NULL      0    .text         ExtInterrupt
    [  44]   589   00000600      0   LOC    NULL      0    .text         Alignment
    [  45]   599   00000700      0   LOC    NULL      0    .text         ProgramCheck
    [  46]   612   00000800      0   LOC    NULL      0    .text         FPUnavailable
    [  47]   626   00000900      0   LOC    NULL      0    .text         Decrementer
    [  48]   638   00000a00      0   LOC    NULL      0    .text         Trap_0a
    [  49]   646   00000b00      0   LOC    NULL      0    .text         Trap_0b
    [  50]   654   00000c00      0   LOC    NULL      0    .text         SystemCall
    [  51]   665   00000d00      0   LOC    NULL      0    .text         SingleStep
    [  52]   676   00000e00      0   LOC    NULL      0    .text         Trap_0e
    [  53]   684   00000f00      0   LOC    NULL      0    .text         Trap_0f
    [  54]   692   00001000      0   LOC    NULL      0    .text         InstructionTransMiss
    [  55]   713   00001100      0   LOC    NULL      0    .text         DataLoadTransMiss
    [  56]   731   00001200      0   LOC    NULL      0    .text         DataStoreTransMiss
    [  57]   750   00001300      0   LOC    NULL      0    .text         InstructionBreakpoint
    [  58]   772   00001400      0   LOC    NULL      0    .text         SysManageInt
    [  59]   785   00001500      0   LOC    NULL      0    .text         Reserved5
    [  60]   795   00001600      0   LOC    NULL      0    .text         Reserved6
    [  61]   805   00001700      0   LOC    NULL      0    .text         Reserved7
    [  62]   815   00001800      0   LOC    NULL      0    .text         Reserved8
    [  63]   825   00001900      0   LOC    NULL      0    .text         Reserved9
    [  64]   835   00001a00      0   LOC    NULL      0    .text         ReservedA
    [  65]   845   00001b00      0   LOC    NULL      0    .text         ReservedB
    [  66]   855   00001c00      0   LOC    NULL      0    .text         ReservedC
    [  67]   865   00001d00      0   LOC    NULL      0    .text         ReservedD
    [  68]   875   00001e00      0   LOC    NULL      0    .text         ReservedE
    [  69]   885   00001f00      0   LOC    NULL      0    .text         ReservedF
    [  70]   895   00002000      0   LOC    NULL      0    .text         RunModeTrace
    [  71]   908   000031f8      0   LOC    NULL      0    .text         NotMax
    [  72]   915   000032a4      0   LOC    NULL      0    .text         clear_bss
    [  73]   925   00003354      0   LOC    NULL      0    .text         trap_reloc
    [  74]   936   00000000      0   GLOB   NULL      0    UND           _GOT2_TABLE_
    [  75]   949   00000000      0   GLOB   NULL      0    UND           _FIXUP_TABLE_
    [  76]   963   00000100      0   GLOB   NULL      0    .text         _start
    [  77]   970   0000016c      0   GLOB   NULL      0    .text         _start_of_vectors
    [  78]   988   00002090      0   GLOB   NULL      0    .text         _end_of_vectors
    [  79]   1004   00003000      0   GLOB   NULL      0    .text         transfer_to_handler
    [  80]   1024   00000000      0   GLOB   NULL      0    UND           _end
    [  81]   1029   00000004      0   GLOB   NULL      0    .text         version_string
    [  82]   1044   00000110      0   GLOB   NULL      0    .text         _start_warm
    [  83]   1056   00000000      0   GLOB   NULL      0    UND           cpu_init_f
    [  84]   1067   00000000      0   GLOB   NULL      0    UND           board_init_f
    [  85]   1080   00000000      0   GLOB   NULL      0    UND           MachineCheckException
    [  86]   1102   00000000      0   GLOB   NULL      0    UND           UnknownException
    [  87]   1119   00000000      0   GLOB   NULL      0    UND           external_interrupt
    [  88]   1138   00000000      0   GLOB   NULL      0    UND           AlignmentException
    [  89]   1157   00000000      0   GLOB   NULL      0    UND           ProgramCheckException
    [  90]   1179   00000000      0   GLOB   NULL      0    UND           timer_interrupt
    [  91]   1195   0000317c      0   GLOB   NULL      0    .text         icache_enable
    [  92]   1209   000031a8      0   GLOB   NULL      0    .text         icache_disable
    [  93]   1224   000031cc      0   GLOB   NULL      0    .text         icache_status
    [  94]   1238   000031dc      0   GLOB   NULL      0    .text         dcache_enable
    [  95]   1252   0000321c      0   GLOB   NULL      0    .text         dcache_disable
    [  96]   1267   00003240      0   GLOB   NULL      0    .text         dcache_status
    [  97]   1281   00003250      0   GLOB   NULL      0    .text         dc_read
    [  98]   1289   00003254      0   GLOB   NULL      0    .text         get_pvr
    [  99]   1297   0000325c      0   GLOB   NULL      0    .text         udelay
    [ 100]   1304   000032a4      0   GLOB   NULL      0    .text         relocate_code
    [ 101]   1318   00000000      0   GLOB   NULL      0    UND           board_init_r
    [ 102]   1331   000032d4      0   GLOB   NULL      0    .text         get_endaddr
    [ 103]   1343   000032dc      0   GLOB   NULL      0    .text         trap_init

==> .rela.text
    entry  offset   addend   type name(symbol id)
    [   0] 00000160 00000000   10 cpu_init_f(83)
    [   1] 00000168 00000000   10 board_init_f(84)
    [   2] 00000288 00000288   26 MachineCheckException(85)
    [   3] 00000388 00000388   26 UnknownException(86)
    [   4] 00000488 00000488   26 UnknownException(86)
    [   5] 00000588 00000588   26 external_interrupt(87)
    [   6] 0000069c 0000069c   26 AlignmentException(88)
    [   7] 0000078c 0000078c   26 ProgramCheckException(89)
    [   8] 00000888 00000888   26 UnknownException(86)
    [   9] 00000988 00000988   26 timer_interrupt(90)
    [  10] 00000a88 00000a88   26 UnknownException(86)
    [  11] 00000b88 00000b88   26 UnknownException(86)
    [  12] 00000c88 00000c88   26 UnknownException(86)
    [  13] 00000d88 00000d88   26 UnknownException(86)
    [  14] 00000e88 00000e88   26 UnknownException(86)
    [  15] 00000f88 00000f88   26 UnknownException(86)
    [  16] 00001088 00001088   26 UnknownException(86)
    [  17] 00001188 00001188   26 UnknownException(86)
    [  18] 00001288 00001288   26 UnknownException(86)
    [  19] 00001388 00001388   26 UnknownException(86)
    [  20] 00001488 00001488   26 UnknownException(86)
    [  21] 00001588 00001588   26 UnknownException(86)
    [  22] 00001688 00001688   26 UnknownException(86)
    [  23] 00001788 00001788   26 UnknownException(86)
    [  24] 00001888 00001888   26 UnknownException(86)
    [  25] 00001988 00001988   26 UnknownException(86)
    [  26] 00001a88 00001a88   26 UnknownException(86)
    [  27] 00001b88 00001b88   26 UnknownException(86)
    [  28] 00001c88 00001c88   26 UnknownException(86)
    [  29] 00001d88 00001d88   26 UnknownException(86)
    [  30] 00001e88 00001e88   26 UnknownException(86)
    [  31] 00001f88 00001f88   26 UnknownException(86)
    [  32] 00002088 00002088   26 UnknownException(86)
    [  33] 000032d0 00000000   10 board_init_r(101)
    [  34] 00003370 0000b21c   26 .got2(35)

==> .text
00000000: 27051956  dc.l     0x27051956              ; Invalid opcode ''..V'

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="version_string"  Size=0
00000004: 70706362  andi.    r16,r3,0x6362
00000008: 6F6F7420  xoris    r15,r27,0x7420
0000000C: 302E352E  addic    rsp,r14,13614
00000010: 3020284F  addic    rsp,r0,10319
00000014: 63742031  ori      r20,r27,0x2031
00000018: 31203230  addic    r9,r0,12848
0000001C: 3030202D  addic    rsp,r16,8237
00000020: 2031363A  subfic   rsp,r17,13882
00000024: 35353A34  addic.   r9,r21,14900
00000028: 38290000  addi     rsp,r9,0

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="_start"  Size=0
00000100: 3AA00001  li       r21,1
00000104: 48000014  b        *+20                    ; 0x00000118

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="_start_warm"  Size=0
00000110: 3AA00002  li       r21,2
00000114: 48000004  b        *+4                     ; 0x00000118

Hunk:	Kind=HUNK_LOCAL_CODE     Name="boot_warm"  Size=0

Hunk:	Kind=HUNK_LOCAL_CODE     Name="boot_cold"  Size=0
00000118: 38601002  li       r3,4098
0000011C: 7C600124  mtmsr    r3
00000120: 7C7B03A6  mtsrr1   r3
00000124: 7C7422A6  mfspr    r3,ECR
00000128: 3C000000  lis      r0,0
0000012C: 7C6000A6  mfmsr    r3
00000130: 3C80FFFF  lis      r4,-1
00000134: 6084FFCF  ori      r4,r4,0xffcf
00000138: 7C632038  and      r3,r3,r4
0000013C: 7C600124  mtmsr    r3
00000140: 4C00012C  isync
00000144: 7C0004AC  sync

Hunk:	Kind=HUNK_LOCAL_CODE     Name="in_flash"  Size=0
00000148: 3C20007F  lis      rsp,127
0000014C: 6021D000  ori      rsp,rsp,0xd000
00000150: 48000005  bl       *+4                     ; 0x00000154
00000154: 7DC802A6  mflr     r14
00000158: 800E321C  lwz      r0,12828(r14)
0000015C: 7DC07214  add      r14,r0,r14
00000160: 48000001  bl       cpu_init_f
00000164: 7EA3AB78  mr       r3,r21
00000168: 48000001  bl       board_init_f

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="_start_of_vectors"  Size=0

Hunk:	Kind=HUNK_LOCAL_CODE     Name="MachineCheck"  Size=0
00000200: 7E9043A6  mtsprg0  r20
00000204: 7EB143A6  mtsprg1  r21
00000208: 7E800026  mfcr     r20
0000020C: 7C350B78  mr       r21,rsp
00000210: 3AB5FF00  subi     r21,r21,256
00000214: 929500A8  stw      r20,168(r21)
00000218: 92D50068  stw      r22,104(r21)
0000021C: 92F5006C  stw      r23,108(r21)
00000220: 7E9042A6  mfsprg0  r20
00000224: 92950060  stw      r20,96(r21)
00000228: 7ED142A6  mfsprg1  r22
0000022C: 92D50064  stw      r22,100(r21)
00000230: 7E8802A6  mflr     r20
00000234: 929500A0  stw      r20,160(r21)
00000238: 7EC902A6  mfctr    r22
0000023C: 92D5009C  stw      r22,156(r21)
00000240: 7E8102A6  mfxer    r20
00000244: 929500A4  stw      r20,164(r21)
00000248: 7EDA02A6  mfsrr0   r22
0000024C: 7EFB02A6  mfsrr1   r23
00000250: 90150010  stw      r0,16(r21)
00000254: 90350014  stw      rsp,20(r21)
00000258: 90550018  stw      r2,24(r21)
0000025C: 90350000  stw      rsp,0(r21)
00000260: 7EA1AB78  mr       rsp,r21
00000264: 9075001C  stw      r3,28(r21)
00000268: 90950020  stw      r4,32(r21)
0000026C: 90B50024  stw      r5,36(r21)
00000270: 90D50028  stw      r6,40(r21)
00000274: 806E8014  lwz      r3,-32748(r14)
00000278: 7C6803A6  mtlr     r3
0000027C: 38610010  addi     r3,rsp,16
00000280: 3A801002  li       r20,4098
00000284: 4E800021  blrl
00000288: 00000288  dc.l     0x00000288              ; Invalid opcode '....'
0000028C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="DataStorage"  Size=0
00000300: 7E9043A6  mtsprg0  r20
00000304: 7EB143A6  mtsprg1  r21
00000308: 7E800026  mfcr     r20
0000030C: 7C350B78  mr       r21,rsp
00000310: 3AB5FF00  subi     r21,r21,256
00000314: 929500A8  stw      r20,168(r21)
00000318: 92D50068  stw      r22,104(r21)
0000031C: 92F5006C  stw      r23,108(r21)
00000320: 7E9042A6  mfsprg0  r20
00000324: 92950060  stw      r20,96(r21)
00000328: 7ED142A6  mfsprg1  r22
0000032C: 92D50064  stw      r22,100(r21)
00000330: 7E8802A6  mflr     r20
00000334: 929500A0  stw      r20,160(r21)
00000338: 7EC902A6  mfctr    r22
0000033C: 92D5009C  stw      r22,156(r21)
00000340: 7E8102A6  mfxer    r20
00000344: 929500A4  stw      r20,164(r21)
00000348: 7EDA02A6  mfsrr0   r22
0000034C: 7EFB02A6  mfsrr1   r23
00000350: 90150010  stw      r0,16(r21)
00000354: 90350014  stw      rsp,20(r21)
00000358: 90550018  stw      r2,24(r21)
0000035C: 90350000  stw      rsp,0(r21)
00000360: 7EA1AB78  mr       rsp,r21
00000364: 9075001C  stw      r3,28(r21)
00000368: 90950020  stw      r4,32(r21)
0000036C: 90B50024  stw      r5,36(r21)
00000370: 90D50028  stw      r6,40(r21)
00000374: 806E8014  lwz      r3,-32748(r14)
00000378: 7C6803A6  mtlr     r3
0000037C: 38610010  addi     r3,rsp,16
00000380: 3A801002  li       r20,4098
00000384: 4E800021  blrl
00000388: 00000388  dc.l     0x00000388              ; Invalid opcode '....'
0000038C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="InstStorage"  Size=0
00000400: 7E9043A6  mtsprg0  r20
00000404: 7EB143A6  mtsprg1  r21
00000408: 7E800026  mfcr     r20
0000040C: 7C350B78  mr       r21,rsp
00000410: 3AB5FF00  subi     r21,r21,256
00000414: 929500A8  stw      r20,168(r21)
00000418: 92D50068  stw      r22,104(r21)
0000041C: 92F5006C  stw      r23,108(r21)
00000420: 7E9042A6  mfsprg0  r20
00000424: 92950060  stw      r20,96(r21)
00000428: 7ED142A6  mfsprg1  r22
0000042C: 92D50064  stw      r22,100(r21)
00000430: 7E8802A6  mflr     r20
00000434: 929500A0  stw      r20,160(r21)
00000438: 7EC902A6  mfctr    r22
0000043C: 92D5009C  stw      r22,156(r21)
00000440: 7E8102A6  mfxer    r20
00000444: 929500A4  stw      r20,164(r21)
00000448: 7EDA02A6  mfsrr0   r22
0000044C: 7EFB02A6  mfsrr1   r23
00000450: 90150010  stw      r0,16(r21)
00000454: 90350014  stw      rsp,20(r21)
00000458: 90550018  stw      r2,24(r21)
0000045C: 90350000  stw      rsp,0(r21)
00000460: 7EA1AB78  mr       rsp,r21
00000464: 9075001C  stw      r3,28(r21)
00000468: 90950020  stw      r4,32(r21)
0000046C: 90B50024  stw      r5,36(r21)
00000470: 90D50028  stw      r6,40(r21)
00000474: 806E8014  lwz      r3,-32748(r14)
00000478: 7C6803A6  mtlr     r3
0000047C: 38610010  addi     r3,rsp,16
00000480: 3A801002  li       r20,4098
00000484: 4E800021  blrl
00000488: 00000488  dc.l     0x00000488              ; Invalid opcode '....'
0000048C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ExtInterrupt"  Size=0
00000500: 7E9043A6  mtsprg0  r20
00000504: 7EB143A6  mtsprg1  r21
00000508: 7E800026  mfcr     r20
0000050C: 7C350B78  mr       r21,rsp
00000510: 3AB5FF00  subi     r21,r21,256
00000514: 929500A8  stw      r20,168(r21)
00000518: 92D50068  stw      r22,104(r21)
0000051C: 92F5006C  stw      r23,108(r21)
00000520: 7E9042A6  mfsprg0  r20
00000524: 92950060  stw      r20,96(r21)
00000528: 7ED142A6  mfsprg1  r22
0000052C: 92D50064  stw      r22,100(r21)
00000530: 7E8802A6  mflr     r20
00000534: 929500A0  stw      r20,160(r21)
00000538: 7EC902A6  mfctr    r22
0000053C: 92D5009C  stw      r22,156(r21)
00000540: 7E8102A6  mfxer    r20
00000544: 929500A4  stw      r20,164(r21)
00000548: 7EDA02A6  mfsrr0   r22
0000054C: 7EFB02A6  mfsrr1   r23
00000550: 90150010  stw      r0,16(r21)
00000554: 90350014  stw      rsp,20(r21)
00000558: 90550018  stw      r2,24(r21)
0000055C: 90350000  stw      rsp,0(r21)
00000560: 7EA1AB78  mr       rsp,r21
00000564: 9075001C  stw      r3,28(r21)
00000568: 90950020  stw      r4,32(r21)
0000056C: 90B50024  stw      r5,36(r21)
00000570: 90D50028  stw      r6,40(r21)
00000574: 806E8014  lwz      r3,-32748(r14)
00000578: 7C6803A6  mtlr     r3
0000057C: 38610010  addi     r3,rsp,16
00000580: 3A801002  li       r20,4098
00000584: 4E800021  blrl
00000588: 00000588  dc.l     0x00000588              ; Invalid opcode '....'
0000058C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Alignment"  Size=0
00000600: 7E9043A6  mtsprg0  r20
00000604: 7EB143A6  mtsprg1  r21
00000608: 7E800026  mfcr     r20
0000060C: 7C350B78  mr       r21,rsp
00000610: 3AB5FF00  subi     r21,r21,256
00000614: 929500A8  stw      r20,168(r21)
00000618: 92D50068  stw      r22,104(r21)
0000061C: 92F5006C  stw      r23,108(r21)
00000620: 7E9042A6  mfsprg0  r20
00000624: 92950060  stw      r20,96(r21)
00000628: 7ED142A6  mfsprg1  r22
0000062C: 92D50064  stw      r22,100(r21)
00000630: 7E8802A6  mflr     r20
00000634: 929500A0  stw      r20,160(r21)
00000638: 7EC902A6  mfctr    r22
0000063C: 92D5009C  stw      r22,156(r21)
00000640: 7E8102A6  mfxer    r20
00000644: 929500A4  stw      r20,164(r21)
00000648: 7EDA02A6  mfsrr0   r22
0000064C: 7EFB02A6  mfsrr1   r23
00000650: 90150010  stw      r0,16(r21)
00000654: 90350014  stw      rsp,20(r21)
00000658: 90550018  stw      r2,24(r21)
0000065C: 90350000  stw      rsp,0(r21)
00000660: 7EA1AB78  mr       rsp,r21
00000664: 9075001C  stw      r3,28(r21)
00000668: 90950020  stw      r4,32(r21)
0000066C: 90B50024  stw      r5,36(r21)
00000670: 90D50028  stw      r6,40(r21)
00000674: 7C9302A6  mfspr    r4,DAR
00000678: 909500B4  stw      r4,180(r21)
0000067C: 7CB202A6  mfspr    r5,DSISR
00000680: 90B500B8  stw      r5,184(r21)
00000684: 38610010  addi     r3,rsp,16
00000688: 3A801002  li       r20,4098
0000068C: 52F40420  rlwimi   r20,r23,0,16,16
00000690: 80CE8014  lwz      r6,-32748(r14)
00000694: 7CC803A6  mtlr     r6
00000698: 4E800021  blrl
0000069C: 0000069C  dc.l     0x0000069c              ; Invalid opcode '....'
000006A0: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ProgramCheck"  Size=0
00000700: 7E9043A6  mtsprg0  r20
00000704: 7EB143A6  mtsprg1  r21
00000708: 7E800026  mfcr     r20
0000070C: 7C350B78  mr       r21,rsp
00000710: 3AB5FF00  subi     r21,r21,256
00000714: 929500A8  stw      r20,168(r21)
00000718: 92D50068  stw      r22,104(r21)
0000071C: 92F5006C  stw      r23,108(r21)
00000720: 7E9042A6  mfsprg0  r20
00000724: 92950060  stw      r20,96(r21)
00000728: 7ED142A6  mfsprg1  r22
0000072C: 92D50064  stw      r22,100(r21)
00000730: 7E8802A6  mflr     r20
00000734: 929500A0  stw      r20,160(r21)
00000738: 7EC902A6  mfctr    r22
0000073C: 92D5009C  stw      r22,156(r21)
00000740: 7E8102A6  mfxer    r20
00000744: 929500A4  stw      r20,164(r21)
00000748: 7EDA02A6  mfsrr0   r22
0000074C: 7EFB02A6  mfsrr1   r23
00000750: 90150010  stw      r0,16(r21)
00000754: 90350014  stw      rsp,20(r21)
00000758: 90550018  stw      r2,24(r21)
0000075C: 90350000  stw      rsp,0(r21)
00000760: 7EA1AB78  mr       rsp,r21
00000764: 9075001C  stw      r3,28(r21)
00000768: 90950020  stw      r4,32(r21)
0000076C: 90B50024  stw      r5,36(r21)
00000770: 90D50028  stw      r6,40(r21)
00000774: 38610010  addi     r3,rsp,16
00000778: 3A801002  li       r20,4098
0000077C: 52F40420  rlwimi   r20,r23,0,16,16
00000780: 80CE8014  lwz      r6,-32748(r14)
00000784: 7CC803A6  mtlr     r6
00000788: 4E800021  blrl
0000078C: 0000078C  dc.l     0x0000078c              ; Invalid opcode '....'
00000790: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="FPUnavailable"  Size=0
00000800: 7E9043A6  mtsprg0  r20
00000804: 7EB143A6  mtsprg1  r21
00000808: 7E800026  mfcr     r20
0000080C: 7C350B78  mr       r21,rsp
00000810: 3AB5FF00  subi     r21,r21,256
00000814: 929500A8  stw      r20,168(r21)
00000818: 92D50068  stw      r22,104(r21)
0000081C: 92F5006C  stw      r23,108(r21)
00000820: 7E9042A6  mfsprg0  r20
00000824: 92950060  stw      r20,96(r21)
00000828: 7ED142A6  mfsprg1  r22
0000082C: 92D50064  stw      r22,100(r21)
00000830: 7E8802A6  mflr     r20
00000834: 929500A0  stw      r20,160(r21)
00000838: 7EC902A6  mfctr    r22
0000083C: 92D5009C  stw      r22,156(r21)
00000840: 7E8102A6  mfxer    r20
00000844: 929500A4  stw      r20,164(r21)
00000848: 7EDA02A6  mfsrr0   r22
0000084C: 7EFB02A6  mfsrr1   r23
00000850: 90150010  stw      r0,16(r21)
00000854: 90350014  stw      rsp,20(r21)
00000858: 90550018  stw      r2,24(r21)
0000085C: 90350000  stw      rsp,0(r21)
00000860: 7EA1AB78  mr       rsp,r21
00000864: 9075001C  stw      r3,28(r21)
00000868: 90950020  stw      r4,32(r21)
0000086C: 90B50024  stw      r5,36(r21)
00000870: 90D50028  stw      r6,40(r21)
00000874: 806E8014  lwz      r3,-32748(r14)
00000878: 7C6803A6  mtlr     r3
0000087C: 38610010  addi     r3,rsp,16
00000880: 3A801002  li       r20,4098
00000884: 4E800021  blrl
00000888: 00000888  dc.l     0x00000888              ; Invalid opcode '....'
0000088C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Decrementer"  Size=0
00000900: 7E9043A6  mtsprg0  r20
00000904: 7EB143A6  mtsprg1  r21
00000908: 7E800026  mfcr     r20
0000090C: 7C350B78  mr       r21,rsp
00000910: 3AB5FF00  subi     r21,r21,256
00000914: 929500A8  stw      r20,168(r21)
00000918: 92D50068  stw      r22,104(r21)
0000091C: 92F5006C  stw      r23,108(r21)
00000920: 7E9042A6  mfsprg0  r20
00000924: 92950060  stw      r20,96(r21)
00000928: 7ED142A6  mfsprg1  r22
0000092C: 92D50064  stw      r22,100(r21)
00000930: 7E8802A6  mflr     r20
00000934: 929500A0  stw      r20,160(r21)
00000938: 7EC902A6  mfctr    r22
0000093C: 92D5009C  stw      r22,156(r21)
00000940: 7E8102A6  mfxer    r20
00000944: 929500A4  stw      r20,164(r21)
00000948: 7EDA02A6  mfsrr0   r22
0000094C: 7EFB02A6  mfsrr1   r23
00000950: 90150010  stw      r0,16(r21)
00000954: 90350014  stw      rsp,20(r21)
00000958: 90550018  stw      r2,24(r21)
0000095C: 90350000  stw      rsp,0(r21)
00000960: 7EA1AB78  mr       rsp,r21
00000964: 9075001C  stw      r3,28(r21)
00000968: 90950020  stw      r4,32(r21)
0000096C: 90B50024  stw      r5,36(r21)
00000970: 90D50028  stw      r6,40(r21)
00000974: 806E8014  lwz      r3,-32748(r14)
00000978: 7C6803A6  mtlr     r3
0000097C: 38610010  addi     r3,rsp,16
00000980: 3A801002  li       r20,4098
00000984: 4E800021  blrl
00000988: 00000988  dc.l     0x00000988              ; Invalid opcode '....'
0000098C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Trap_0a"  Size=0
00000A00: 7E9043A6  mtsprg0  r20
00000A04: 7EB143A6  mtsprg1  r21
00000A08: 7E800026  mfcr     r20
00000A0C: 7C350B78  mr       r21,rsp
00000A10: 3AB5FF00  subi     r21,r21,256
00000A14: 929500A8  stw      r20,168(r21)
00000A18: 92D50068  stw      r22,104(r21)
00000A1C: 92F5006C  stw      r23,108(r21)
00000A20: 7E9042A6  mfsprg0  r20
00000A24: 92950060  stw      r20,96(r21)
00000A28: 7ED142A6  mfsprg1  r22
00000A2C: 92D50064  stw      r22,100(r21)
00000A30: 7E8802A6  mflr     r20
00000A34: 929500A0  stw      r20,160(r21)
00000A38: 7EC902A6  mfctr    r22
00000A3C: 92D5009C  stw      r22,156(r21)
00000A40: 7E8102A6  mfxer    r20
00000A44: 929500A4  stw      r20,164(r21)
00000A48: 7EDA02A6  mfsrr0   r22
00000A4C: 7EFB02A6  mfsrr1   r23
00000A50: 90150010  stw      r0,16(r21)
00000A54: 90350014  stw      rsp,20(r21)
00000A58: 90550018  stw      r2,24(r21)
00000A5C: 90350000  stw      rsp,0(r21)
00000A60: 7EA1AB78  mr       rsp,r21
00000A64: 9075001C  stw      r3,28(r21)
00000A68: 90950020  stw      r4,32(r21)
00000A6C: 90B50024  stw      r5,36(r21)
00000A70: 90D50028  stw      r6,40(r21)
00000A74: 806E8014  lwz      r3,-32748(r14)
00000A78: 7C6803A6  mtlr     r3
00000A7C: 38610010  addi     r3,rsp,16
00000A80: 3A801002  li       r20,4098
00000A84: 4E800021  blrl
00000A88: 00000A88  dc.l     0x00000a88              ; Invalid opcode '....'
00000A8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Trap_0b"  Size=0
00000B00: 7E9043A6  mtsprg0  r20
00000B04: 7EB143A6  mtsprg1  r21
00000B08: 7E800026  mfcr     r20
00000B0C: 7C350B78  mr       r21,rsp
00000B10: 3AB5FF00  subi     r21,r21,256
00000B14: 929500A8  stw      r20,168(r21)
00000B18: 92D50068  stw      r22,104(r21)
00000B1C: 92F5006C  stw      r23,108(r21)
00000B20: 7E9042A6  mfsprg0  r20
00000B24: 92950060  stw      r20,96(r21)
00000B28: 7ED142A6  mfsprg1  r22
00000B2C: 92D50064  stw      r22,100(r21)
00000B30: 7E8802A6  mflr     r20
00000B34: 929500A0  stw      r20,160(r21)
00000B38: 7EC902A6  mfctr    r22
00000B3C: 92D5009C  stw      r22,156(r21)
00000B40: 7E8102A6  mfxer    r20
00000B44: 929500A4  stw      r20,164(r21)
00000B48: 7EDA02A6  mfsrr0   r22
00000B4C: 7EFB02A6  mfsrr1   r23
00000B50: 90150010  stw      r0,16(r21)
00000B54: 90350014  stw      rsp,20(r21)
00000B58: 90550018  stw      r2,24(r21)
00000B5C: 90350000  stw      rsp,0(r21)
00000B60: 7EA1AB78  mr       rsp,r21
00000B64: 9075001C  stw      r3,28(r21)
00000B68: 90950020  stw      r4,32(r21)
00000B6C: 90B50024  stw      r5,36(r21)
00000B70: 90D50028  stw      r6,40(r21)
00000B74: 806E8014  lwz      r3,-32748(r14)
00000B78: 7C6803A6  mtlr     r3
00000B7C: 38610010  addi     r3,rsp,16
00000B80: 3A801002  li       r20,4098
00000B84: 4E800021  blrl
00000B88: 00000B88  dc.l     0x00000b88              ; Invalid opcode '....'
00000B8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="SystemCall"  Size=0
00000C00: 7E9043A6  mtsprg0  r20
00000C04: 7EB143A6  mtsprg1  r21
00000C08: 7E800026  mfcr     r20
00000C0C: 7C350B78  mr       r21,rsp
00000C10: 3AB5FF00  subi     r21,r21,256
00000C14: 929500A8  stw      r20,168(r21)
00000C18: 92D50068  stw      r22,104(r21)
00000C1C: 92F5006C  stw      r23,108(r21)
00000C20: 7E9042A6  mfsprg0  r20
00000C24: 92950060  stw      r20,96(r21)
00000C28: 7ED142A6  mfsprg1  r22
00000C2C: 92D50064  stw      r22,100(r21)
00000C30: 7E8802A6  mflr     r20
00000C34: 929500A0  stw      r20,160(r21)
00000C38: 7EC902A6  mfctr    r22
00000C3C: 92D5009C  stw      r22,156(r21)
00000C40: 7E8102A6  mfxer    r20
00000C44: 929500A4  stw      r20,164(r21)
00000C48: 7EDA02A6  mfsrr0   r22
00000C4C: 7EFB02A6  mfsrr1   r23
00000C50: 90150010  stw      r0,16(r21)
00000C54: 90350014  stw      rsp,20(r21)
00000C58: 90550018  stw      r2,24(r21)
00000C5C: 90350000  stw      rsp,0(r21)
00000C60: 7EA1AB78  mr       rsp,r21
00000C64: 9075001C  stw      r3,28(r21)
00000C68: 90950020  stw      r4,32(r21)
00000C6C: 90B50024  stw      r5,36(r21)
00000C70: 90D50028  stw      r6,40(r21)
00000C74: 806E8014  lwz      r3,-32748(r14)
00000C78: 7C6803A6  mtlr     r3
00000C7C: 38610010  addi     r3,rsp,16
00000C80: 3A801002  li       r20,4098
00000C84: 4E800021  blrl
00000C88: 00000C88  dc.l     0x00000c88              ; Invalid opcode '....'
00000C8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="SingleStep"  Size=0
00000D00: 7E9043A6  mtsprg0  r20
00000D04: 7EB143A6  mtsprg1  r21
00000D08: 7E800026  mfcr     r20
00000D0C: 7C350B78  mr       r21,rsp
00000D10: 3AB5FF00  subi     r21,r21,256
00000D14: 929500A8  stw      r20,168(r21)
00000D18: 92D50068  stw      r22,104(r21)
00000D1C: 92F5006C  stw      r23,108(r21)
00000D20: 7E9042A6  mfsprg0  r20
00000D24: 92950060  stw      r20,96(r21)
00000D28: 7ED142A6  mfsprg1  r22
00000D2C: 92D50064  stw      r22,100(r21)
00000D30: 7E8802A6  mflr     r20
00000D34: 929500A0  stw      r20,160(r21)
00000D38: 7EC902A6  mfctr    r22
00000D3C: 92D5009C  stw      r22,156(r21)
00000D40: 7E8102A6  mfxer    r20
00000D44: 929500A4  stw      r20,164(r21)
00000D48: 7EDA02A6  mfsrr0   r22
00000D4C: 7EFB02A6  mfsrr1   r23
00000D50: 90150010  stw      r0,16(r21)
00000D54: 90350014  stw      rsp,20(r21)
00000D58: 90550018  stw      r2,24(r21)
00000D5C: 90350000  stw      rsp,0(r21)
00000D60: 7EA1AB78  mr       rsp,r21
00000D64: 9075001C  stw      r3,28(r21)
00000D68: 90950020  stw      r4,32(r21)
00000D6C: 90B50024  stw      r5,36(r21)
00000D70: 90D50028  stw      r6,40(r21)
00000D74: 806E8014  lwz      r3,-32748(r14)
00000D78: 7C6803A6  mtlr     r3
00000D7C: 38610010  addi     r3,rsp,16
00000D80: 3A801002  li       r20,4098
00000D84: 4E800021  blrl
00000D88: 00000D88  dc.l     0x00000d88              ; Invalid opcode '....'
00000D8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Trap_0e"  Size=0
00000E00: 7E9043A6  mtsprg0  r20
00000E04: 7EB143A6  mtsprg1  r21
00000E08: 7E800026  mfcr     r20
00000E0C: 7C350B78  mr       r21,rsp
00000E10: 3AB5FF00  subi     r21,r21,256
00000E14: 929500A8  stw      r20,168(r21)
00000E18: 92D50068  stw      r22,104(r21)
00000E1C: 92F5006C  stw      r23,108(r21)
00000E20: 7E9042A6  mfsprg0  r20
00000E24: 92950060  stw      r20,96(r21)
00000E28: 7ED142A6  mfsprg1  r22
00000E2C: 92D50064  stw      r22,100(r21)
00000E30: 7E8802A6  mflr     r20
00000E34: 929500A0  stw      r20,160(r21)
00000E38: 7EC902A6  mfctr    r22
00000E3C: 92D5009C  stw      r22,156(r21)
00000E40: 7E8102A6  mfxer    r20
00000E44: 929500A4  stw      r20,164(r21)
00000E48: 7EDA02A6  mfsrr0   r22
00000E4C: 7EFB02A6  mfsrr1   r23
00000E50: 90150010  stw      r0,16(r21)
00000E54: 90350014  stw      rsp,20(r21)
00000E58: 90550018  stw      r2,24(r21)
00000E5C: 90350000  stw      rsp,0(r21)
00000E60: 7EA1AB78  mr       rsp,r21
00000E64: 9075001C  stw      r3,28(r21)
00000E68: 90950020  stw      r4,32(r21)
00000E6C: 90B50024  stw      r5,36(r21)
00000E70: 90D50028  stw      r6,40(r21)
00000E74: 806E8014  lwz      r3,-32748(r14)
00000E78: 7C6803A6  mtlr     r3
00000E7C: 38610010  addi     r3,rsp,16
00000E80: 3A801002  li       r20,4098
00000E84: 4E800021  blrl
00000E88: 00000E88  dc.l     0x00000e88              ; Invalid opcode '....'
00000E8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Trap_0f"  Size=0
00000F00: 7E9043A6  mtsprg0  r20
00000F04: 7EB143A6  mtsprg1  r21
00000F08: 7E800026  mfcr     r20
00000F0C: 7C350B78  mr       r21,rsp
00000F10: 3AB5FF00  subi     r21,r21,256
00000F14: 929500A8  stw      r20,168(r21)
00000F18: 92D50068  stw      r22,104(r21)
00000F1C: 92F5006C  stw      r23,108(r21)
00000F20: 7E9042A6  mfsprg0  r20
00000F24: 92950060  stw      r20,96(r21)
00000F28: 7ED142A6  mfsprg1  r22
00000F2C: 92D50064  stw      r22,100(r21)
00000F30: 7E8802A6  mflr     r20
00000F34: 929500A0  stw      r20,160(r21)
00000F38: 7EC902A6  mfctr    r22
00000F3C: 92D5009C  stw      r22,156(r21)
00000F40: 7E8102A6  mfxer    r20
00000F44: 929500A4  stw      r20,164(r21)
00000F48: 7EDA02A6  mfsrr0   r22
00000F4C: 7EFB02A6  mfsrr1   r23
00000F50: 90150010  stw      r0,16(r21)
00000F54: 90350014  stw      rsp,20(r21)
00000F58: 90550018  stw      r2,24(r21)
00000F5C: 90350000  stw      rsp,0(r21)
00000F60: 7EA1AB78  mr       rsp,r21
00000F64: 9075001C  stw      r3,28(r21)
00000F68: 90950020  stw      r4,32(r21)
00000F6C: 90B50024  stw      r5,36(r21)
00000F70: 90D50028  stw      r6,40(r21)
00000F74: 806E8014  lwz      r3,-32748(r14)
00000F78: 7C6803A6  mtlr     r3
00000F7C: 38610010  addi     r3,rsp,16
00000F80: 3A801002  li       r20,4098
00000F84: 4E800021  blrl
00000F88: 00000F88  dc.l     0x00000f88              ; Invalid opcode '....'
00000F8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="InstructionTransMiss"  Size=0
00001000: 7E9043A6  mtsprg0  r20
00001004: 7EB143A6  mtsprg1  r21
00001008: 7E800026  mfcr     r20
0000100C: 7C350B78  mr       r21,rsp
00001010: 3AB5FF00  subi     r21,r21,256
00001014: 929500A8  stw      r20,168(r21)
00001018: 92D50068  stw      r22,104(r21)
0000101C: 92F5006C  stw      r23,108(r21)
00001020: 7E9042A6  mfsprg0  r20
00001024: 92950060  stw      r20,96(r21)
00001028: 7ED142A6  mfsprg1  r22
0000102C: 92D50064  stw      r22,100(r21)
00001030: 7E8802A6  mflr     r20
00001034: 929500A0  stw      r20,160(r21)
00001038: 7EC902A6  mfctr    r22
0000103C: 92D5009C  stw      r22,156(r21)
00001040: 7E8102A6  mfxer    r20
00001044: 929500A4  stw      r20,164(r21)
00001048: 7EDA02A6  mfsrr0   r22
0000104C: 7EFB02A6  mfsrr1   r23
00001050: 90150010  stw      r0,16(r21)
00001054: 90350014  stw      rsp,20(r21)
00001058: 90550018  stw      r2,24(r21)
0000105C: 90350000  stw      rsp,0(r21)
00001060: 7EA1AB78  mr       rsp,r21
00001064: 9075001C  stw      r3,28(r21)
00001068: 90950020  stw      r4,32(r21)
0000106C: 90B50024  stw      r5,36(r21)
00001070: 90D50028  stw      r6,40(r21)
00001074: 806E8014  lwz      r3,-32748(r14)
00001078: 7C6803A6  mtlr     r3
0000107C: 38610010  addi     r3,rsp,16
00001080: 3A801002  li       r20,4098
00001084: 4E800021  blrl
00001088: 00001088  dc.l     0x00001088              ; Invalid opcode '....'
0000108C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="DataLoadTransMiss"  Size=0
00001100: 7E9043A6  mtsprg0  r20
00001104: 7EB143A6  mtsprg1  r21
00001108: 7E800026  mfcr     r20
0000110C: 7C350B78  mr       r21,rsp
00001110: 3AB5FF00  subi     r21,r21,256
00001114: 929500A8  stw      r20,168(r21)
00001118: 92D50068  stw      r22,104(r21)
0000111C: 92F5006C  stw      r23,108(r21)
00001120: 7E9042A6  mfsprg0  r20
00001124: 92950060  stw      r20,96(r21)
00001128: 7ED142A6  mfsprg1  r22
0000112C: 92D50064  stw      r22,100(r21)
00001130: 7E8802A6  mflr     r20
00001134: 929500A0  stw      r20,160(r21)
00001138: 7EC902A6  mfctr    r22
0000113C: 92D5009C  stw      r22,156(r21)
00001140: 7E8102A6  mfxer    r20
00001144: 929500A4  stw      r20,164(r21)
00001148: 7EDA02A6  mfsrr0   r22
0000114C: 7EFB02A6  mfsrr1   r23
00001150: 90150010  stw      r0,16(r21)
00001154: 90350014  stw      rsp,20(r21)
00001158: 90550018  stw      r2,24(r21)
0000115C: 90350000  stw      rsp,0(r21)
00001160: 7EA1AB78  mr       rsp,r21
00001164: 9075001C  stw      r3,28(r21)
00001168: 90950020  stw      r4,32(r21)
0000116C: 90B50024  stw      r5,36(r21)
00001170: 90D50028  stw      r6,40(r21)
00001174: 806E8014  lwz      r3,-32748(r14)
00001178: 7C6803A6  mtlr     r3
0000117C: 38610010  addi     r3,rsp,16
00001180: 3A801002  li       r20,4098
00001184: 4E800021  blrl
00001188: 00001188  dc.l     0x00001188              ; Invalid opcode '....'
0000118C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="DataStoreTransMiss"  Size=0
00001200: 7E9043A6  mtsprg0  r20
00001204: 7EB143A6  mtsprg1  r21
00001208: 7E800026  mfcr     r20
0000120C: 7C350B78  mr       r21,rsp
00001210: 3AB5FF00  subi     r21,r21,256
00001214: 929500A8  stw      r20,168(r21)
00001218: 92D50068  stw      r22,104(r21)
0000121C: 92F5006C  stw      r23,108(r21)
00001220: 7E9042A6  mfsprg0  r20
00001224: 92950060  stw      r20,96(r21)
00001228: 7ED142A6  mfsprg1  r22
0000122C: 92D50064  stw      r22,100(r21)
00001230: 7E8802A6  mflr     r20
00001234: 929500A0  stw      r20,160(r21)
00001238: 7EC902A6  mfctr    r22
0000123C: 92D5009C  stw      r22,156(r21)
00001240: 7E8102A6  mfxer    r20
00001244: 929500A4  stw      r20,164(r21)
00001248: 7EDA02A6  mfsrr0   r22
0000124C: 7EFB02A6  mfsrr1   r23
00001250: 90150010  stw      r0,16(r21)
00001254: 90350014  stw      rsp,20(r21)
00001258: 90550018  stw      r2,24(r21)
0000125C: 90350000  stw      rsp,0(r21)
00001260: 7EA1AB78  mr       rsp,r21
00001264: 9075001C  stw      r3,28(r21)
00001268: 90950020  stw      r4,32(r21)
0000126C: 90B50024  stw      r5,36(r21)
00001270: 90D50028  stw      r6,40(r21)
00001274: 806E8014  lwz      r3,-32748(r14)
00001278: 7C6803A6  mtlr     r3
0000127C: 38610010  addi     r3,rsp,16
00001280: 3A801002  li       r20,4098
00001284: 4E800021  blrl
00001288: 00001288  dc.l     0x00001288              ; Invalid opcode '....'
0000128C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="InstructionBreakpoint"  Size=0
00001300: 7E9043A6  mtsprg0  r20
00001304: 7EB143A6  mtsprg1  r21
00001308: 7E800026  mfcr     r20
0000130C: 7C350B78  mr       r21,rsp
00001310: 3AB5FF00  subi     r21,r21,256
00001314: 929500A8  stw      r20,168(r21)
00001318: 92D50068  stw      r22,104(r21)
0000131C: 92F5006C  stw      r23,108(r21)
00001320: 7E9042A6  mfsprg0  r20
00001324: 92950060  stw      r20,96(r21)
00001328: 7ED142A6  mfsprg1  r22
0000132C: 92D50064  stw      r22,100(r21)
00001330: 7E8802A6  mflr     r20
00001334: 929500A0  stw      r20,160(r21)
00001338: 7EC902A6  mfctr    r22
0000133C: 92D5009C  stw      r22,156(r21)
00001340: 7E8102A6  mfxer    r20
00001344: 929500A4  stw      r20,164(r21)
00001348: 7EDA02A6  mfsrr0   r22
0000134C: 7EFB02A6  mfsrr1   r23
00001350: 90150010  stw      r0,16(r21)
00001354: 90350014  stw      rsp,20(r21)
00001358: 90550018  stw      r2,24(r21)
0000135C: 90350000  stw      rsp,0(r21)
00001360: 7EA1AB78  mr       rsp,r21
00001364: 9075001C  stw      r3,28(r21)
00001368: 90950020  stw      r4,32(r21)
0000136C: 90B50024  stw      r5,36(r21)
00001370: 90D50028  stw      r6,40(r21)
00001374: 806E8014  lwz      r3,-32748(r14)
00001378: 7C6803A6  mtlr     r3
0000137C: 38610010  addi     r3,rsp,16
00001380: 3A801002  li       r20,4098
00001384: 4E800021  blrl
00001388: 00001388  dc.l     0x00001388              ; Invalid opcode '....'
0000138C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="SysManageInt"  Size=0
00001400: 7E9043A6  mtsprg0  r20
00001404: 7EB143A6  mtsprg1  r21
00001408: 7E800026  mfcr     r20
0000140C: 7C350B78  mr       r21,rsp
00001410: 3AB5FF00  subi     r21,r21,256
00001414: 929500A8  stw      r20,168(r21)
00001418: 92D50068  stw      r22,104(r21)
0000141C: 92F5006C  stw      r23,108(r21)
00001420: 7E9042A6  mfsprg0  r20
00001424: 92950060  stw      r20,96(r21)
00001428: 7ED142A6  mfsprg1  r22
0000142C: 92D50064  stw      r22,100(r21)
00001430: 7E8802A6  mflr     r20
00001434: 929500A0  stw      r20,160(r21)
00001438: 7EC902A6  mfctr    r22
0000143C: 92D5009C  stw      r22,156(r21)
00001440: 7E8102A6  mfxer    r20
00001444: 929500A4  stw      r20,164(r21)
00001448: 7EDA02A6  mfsrr0   r22
0000144C: 7EFB02A6  mfsrr1   r23
00001450: 90150010  stw      r0,16(r21)
00001454: 90350014  stw      rsp,20(r21)
00001458: 90550018  stw      r2,24(r21)
0000145C: 90350000  stw      rsp,0(r21)
00001460: 7EA1AB78  mr       rsp,r21
00001464: 9075001C  stw      r3,28(r21)
00001468: 90950020  stw      r4,32(r21)
0000146C: 90B50024  stw      r5,36(r21)
00001470: 90D50028  stw      r6,40(r21)
00001474: 806E8014  lwz      r3,-32748(r14)
00001478: 7C6803A6  mtlr     r3
0000147C: 38610010  addi     r3,rsp,16
00001480: 3A801002  li       r20,4098
00001484: 4E800021  blrl
00001488: 00001488  dc.l     0x00001488              ; Invalid opcode '....'
0000148C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Reserved5"  Size=0
00001500: 7E9043A6  mtsprg0  r20
00001504: 7EB143A6  mtsprg1  r21
00001508: 7E800026  mfcr     r20
0000150C: 7C350B78  mr       r21,rsp
00001510: 3AB5FF00  subi     r21,r21,256
00001514: 929500A8  stw      r20,168(r21)
00001518: 92D50068  stw      r22,104(r21)
0000151C: 92F5006C  stw      r23,108(r21)
00001520: 7E9042A6  mfsprg0  r20
00001524: 92950060  stw      r20,96(r21)
00001528: 7ED142A6  mfsprg1  r22
0000152C: 92D50064  stw      r22,100(r21)
00001530: 7E8802A6  mflr     r20
00001534: 929500A0  stw      r20,160(r21)
00001538: 7EC902A6  mfctr    r22
0000153C: 92D5009C  stw      r22,156(r21)
00001540: 7E8102A6  mfxer    r20
00001544: 929500A4  stw      r20,164(r21)
00001548: 7EDA02A6  mfsrr0   r22
0000154C: 7EFB02A6  mfsrr1   r23
00001550: 90150010  stw      r0,16(r21)
00001554: 90350014  stw      rsp,20(r21)
00001558: 90550018  stw      r2,24(r21)
0000155C: 90350000  stw      rsp,0(r21)
00001560: 7EA1AB78  mr       rsp,r21
00001564: 9075001C  stw      r3,28(r21)
00001568: 90950020  stw      r4,32(r21)
0000156C: 90B50024  stw      r5,36(r21)
00001570: 90D50028  stw      r6,40(r21)
00001574: 806E8014  lwz      r3,-32748(r14)
00001578: 7C6803A6  mtlr     r3
0000157C: 38610010  addi     r3,rsp,16
00001580: 3A801002  li       r20,4098
00001584: 4E800021  blrl
00001588: 00001588  dc.l     0x00001588              ; Invalid opcode '....'
0000158C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Reserved6"  Size=0
00001600: 7E9043A6  mtsprg0  r20
00001604: 7EB143A6  mtsprg1  r21
00001608: 7E800026  mfcr     r20
0000160C: 7C350B78  mr       r21,rsp
00001610: 3AB5FF00  subi     r21,r21,256
00001614: 929500A8  stw      r20,168(r21)
00001618: 92D50068  stw      r22,104(r21)
0000161C: 92F5006C  stw      r23,108(r21)
00001620: 7E9042A6  mfsprg0  r20
00001624: 92950060  stw      r20,96(r21)
00001628: 7ED142A6  mfsprg1  r22
0000162C: 92D50064  stw      r22,100(r21)
00001630: 7E8802A6  mflr     r20
00001634: 929500A0  stw      r20,160(r21)
00001638: 7EC902A6  mfctr    r22
0000163C: 92D5009C  stw      r22,156(r21)
00001640: 7E8102A6  mfxer    r20
00001644: 929500A4  stw      r20,164(r21)
00001648: 7EDA02A6  mfsrr0   r22
0000164C: 7EFB02A6  mfsrr1   r23
00001650: 90150010  stw      r0,16(r21)
00001654: 90350014  stw      rsp,20(r21)
00001658: 90550018  stw      r2,24(r21)
0000165C: 90350000  stw      rsp,0(r21)
00001660: 7EA1AB78  mr       rsp,r21
00001664: 9075001C  stw      r3,28(r21)
00001668: 90950020  stw      r4,32(r21)
0000166C: 90B50024  stw      r5,36(r21)
00001670: 90D50028  stw      r6,40(r21)
00001674: 806E8014  lwz      r3,-32748(r14)
00001678: 7C6803A6  mtlr     r3
0000167C: 38610010  addi     r3,rsp,16
00001680: 3A801002  li       r20,4098
00001684: 4E800021  blrl
00001688: 00001688  dc.l     0x00001688              ; Invalid opcode '....'
0000168C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Reserved7"  Size=0
00001700: 7E9043A6  mtsprg0  r20
00001704: 7EB143A6  mtsprg1  r21
00001708: 7E800026  mfcr     r20
0000170C: 7C350B78  mr       r21,rsp
00001710: 3AB5FF00  subi     r21,r21,256
00001714: 929500A8  stw      r20,168(r21)
00001718: 92D50068  stw      r22,104(r21)
0000171C: 92F5006C  stw      r23,108(r21)
00001720: 7E9042A6  mfsprg0  r20
00001724: 92950060  stw      r20,96(r21)
00001728: 7ED142A6  mfsprg1  r22
0000172C: 92D50064  stw      r22,100(r21)
00001730: 7E8802A6  mflr     r20
00001734: 929500A0  stw      r20,160(r21)
00001738: 7EC902A6  mfctr    r22
0000173C: 92D5009C  stw      r22,156(r21)
00001740: 7E8102A6  mfxer    r20
00001744: 929500A4  stw      r20,164(r21)
00001748: 7EDA02A6  mfsrr0   r22
0000174C: 7EFB02A6  mfsrr1   r23
00001750: 90150010  stw      r0,16(r21)
00001754: 90350014  stw      rsp,20(r21)
00001758: 90550018  stw      r2,24(r21)
0000175C: 90350000  stw      rsp,0(r21)
00001760: 7EA1AB78  mr       rsp,r21
00001764: 9075001C  stw      r3,28(r21)
00001768: 90950020  stw      r4,32(r21)
0000176C: 90B50024  stw      r5,36(r21)
00001770: 90D50028  stw      r6,40(r21)
00001774: 806E8014  lwz      r3,-32748(r14)
00001778: 7C6803A6  mtlr     r3
0000177C: 38610010  addi     r3,rsp,16
00001780: 3A801002  li       r20,4098
00001784: 4E800021  blrl
00001788: 00001788  dc.l     0x00001788              ; Invalid opcode '....'
0000178C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Reserved8"  Size=0
00001800: 7E9043A6  mtsprg0  r20
00001804: 7EB143A6  mtsprg1  r21
00001808: 7E800026  mfcr     r20
0000180C: 7C350B78  mr       r21,rsp
00001810: 3AB5FF00  subi     r21,r21,256
00001814: 929500A8  stw      r20,168(r21)
00001818: 92D50068  stw      r22,104(r21)
0000181C: 92F5006C  stw      r23,108(r21)
00001820: 7E9042A6  mfsprg0  r20
00001824: 92950060  stw      r20,96(r21)
00001828: 7ED142A6  mfsprg1  r22
0000182C: 92D50064  stw      r22,100(r21)
00001830: 7E8802A6  mflr     r20
00001834: 929500A0  stw      r20,160(r21)
00001838: 7EC902A6  mfctr    r22
0000183C: 92D5009C  stw      r22,156(r21)
00001840: 7E8102A6  mfxer    r20
00001844: 929500A4  stw      r20,164(r21)
00001848: 7EDA02A6  mfsrr0   r22
0000184C: 7EFB02A6  mfsrr1   r23
00001850: 90150010  stw      r0,16(r21)
00001854: 90350014  stw      rsp,20(r21)
00001858: 90550018  stw      r2,24(r21)
0000185C: 90350000  stw      rsp,0(r21)
00001860: 7EA1AB78  mr       rsp,r21
00001864: 9075001C  stw      r3,28(r21)
00001868: 90950020  stw      r4,32(r21)
0000186C: 90B50024  stw      r5,36(r21)
00001870: 90D50028  stw      r6,40(r21)
00001874: 806E8014  lwz      r3,-32748(r14)
00001878: 7C6803A6  mtlr     r3
0000187C: 38610010  addi     r3,rsp,16
00001880: 3A801002  li       r20,4098
00001884: 4E800021  blrl
00001888: 00001888  dc.l     0x00001888              ; Invalid opcode '....'
0000188C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="Reserved9"  Size=0
00001900: 7E9043A6  mtsprg0  r20
00001904: 7EB143A6  mtsprg1  r21
00001908: 7E800026  mfcr     r20
0000190C: 7C350B78  mr       r21,rsp
00001910: 3AB5FF00  subi     r21,r21,256
00001914: 929500A8  stw      r20,168(r21)
00001918: 92D50068  stw      r22,104(r21)
0000191C: 92F5006C  stw      r23,108(r21)
00001920: 7E9042A6  mfsprg0  r20
00001924: 92950060  stw      r20,96(r21)
00001928: 7ED142A6  mfsprg1  r22
0000192C: 92D50064  stw      r22,100(r21)
00001930: 7E8802A6  mflr     r20
00001934: 929500A0  stw      r20,160(r21)
00001938: 7EC902A6  mfctr    r22
0000193C: 92D5009C  stw      r22,156(r21)
00001940: 7E8102A6  mfxer    r20
00001944: 929500A4  stw      r20,164(r21)
00001948: 7EDA02A6  mfsrr0   r22
0000194C: 7EFB02A6  mfsrr1   r23
00001950: 90150010  stw      r0,16(r21)
00001954: 90350014  stw      rsp,20(r21)
00001958: 90550018  stw      r2,24(r21)
0000195C: 90350000  stw      rsp,0(r21)
00001960: 7EA1AB78  mr       rsp,r21
00001964: 9075001C  stw      r3,28(r21)
00001968: 90950020  stw      r4,32(r21)
0000196C: 90B50024  stw      r5,36(r21)
00001970: 90D50028  stw      r6,40(r21)
00001974: 806E8014  lwz      r3,-32748(r14)
00001978: 7C6803A6  mtlr     r3
0000197C: 38610010  addi     r3,rsp,16
00001980: 3A801002  li       r20,4098
00001984: 4E800021  blrl
00001988: 00001988  dc.l     0x00001988              ; Invalid opcode '....'
0000198C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ReservedA"  Size=0
00001A00: 7E9043A6  mtsprg0  r20
00001A04: 7EB143A6  mtsprg1  r21
00001A08: 7E800026  mfcr     r20
00001A0C: 7C350B78  mr       r21,rsp
00001A10: 3AB5FF00  subi     r21,r21,256
00001A14: 929500A8  stw      r20,168(r21)
00001A18: 92D50068  stw      r22,104(r21)
00001A1C: 92F5006C  stw      r23,108(r21)
00001A20: 7E9042A6  mfsprg0  r20
00001A24: 92950060  stw      r20,96(r21)
00001A28: 7ED142A6  mfsprg1  r22
00001A2C: 92D50064  stw      r22,100(r21)
00001A30: 7E8802A6  mflr     r20
00001A34: 929500A0  stw      r20,160(r21)
00001A38: 7EC902A6  mfctr    r22
00001A3C: 92D5009C  stw      r22,156(r21)
00001A40: 7E8102A6  mfxer    r20
00001A44: 929500A4  stw      r20,164(r21)
00001A48: 7EDA02A6  mfsrr0   r22
00001A4C: 7EFB02A6  mfsrr1   r23
00001A50: 90150010  stw      r0,16(r21)
00001A54: 90350014  stw      rsp,20(r21)
00001A58: 90550018  stw      r2,24(r21)
00001A5C: 90350000  stw      rsp,0(r21)
00001A60: 7EA1AB78  mr       rsp,r21
00001A64: 9075001C  stw      r3,28(r21)
00001A68: 90950020  stw      r4,32(r21)
00001A6C: 90B50024  stw      r5,36(r21)
00001A70: 90D50028  stw      r6,40(r21)
00001A74: 806E8014  lwz      r3,-32748(r14)
00001A78: 7C6803A6  mtlr     r3
00001A7C: 38610010  addi     r3,rsp,16
00001A80: 3A801002  li       r20,4098
00001A84: 4E800021  blrl
00001A88: 00001A88  dc.l     0x00001a88              ; Invalid opcode '....'
00001A8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ReservedB"  Size=0
00001B00: 7E9043A6  mtsprg0  r20
00001B04: 7EB143A6  mtsprg1  r21
00001B08: 7E800026  mfcr     r20
00001B0C: 7C350B78  mr       r21,rsp
00001B10: 3AB5FF00  subi     r21,r21,256
00001B14: 929500A8  stw      r20,168(r21)
00001B18: 92D50068  stw      r22,104(r21)
00001B1C: 92F5006C  stw      r23,108(r21)
00001B20: 7E9042A6  mfsprg0  r20
00001B24: 92950060  stw      r20,96(r21)
00001B28: 7ED142A6  mfsprg1  r22
00001B2C: 92D50064  stw      r22,100(r21)
00001B30: 7E8802A6  mflr     r20
00001B34: 929500A0  stw      r20,160(r21)
00001B38: 7EC902A6  mfctr    r22
00001B3C: 92D5009C  stw      r22,156(r21)
00001B40: 7E8102A6  mfxer    r20
00001B44: 929500A4  stw      r20,164(r21)
00001B48: 7EDA02A6  mfsrr0   r22
00001B4C: 7EFB02A6  mfsrr1   r23
00001B50: 90150010  stw      r0,16(r21)
00001B54: 90350014  stw      rsp,20(r21)
00001B58: 90550018  stw      r2,24(r21)
00001B5C: 90350000  stw      rsp,0(r21)
00001B60: 7EA1AB78  mr       rsp,r21
00001B64: 9075001C  stw      r3,28(r21)
00001B68: 90950020  stw      r4,32(r21)
00001B6C: 90B50024  stw      r5,36(r21)
00001B70: 90D50028  stw      r6,40(r21)
00001B74: 806E8014  lwz      r3,-32748(r14)
00001B78: 7C6803A6  mtlr     r3
00001B7C: 38610010  addi     r3,rsp,16
00001B80: 3A801002  li       r20,4098
00001B84: 4E800021  blrl
00001B88: 00001B88  dc.l     0x00001b88              ; Invalid opcode '....'
00001B8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ReservedC"  Size=0
00001C00: 7E9043A6  mtsprg0  r20
00001C04: 7EB143A6  mtsprg1  r21
00001C08: 7E800026  mfcr     r20
00001C0C: 7C350B78  mr       r21,rsp
00001C10: 3AB5FF00  subi     r21,r21,256
00001C14: 929500A8  stw      r20,168(r21)
00001C18: 92D50068  stw      r22,104(r21)
00001C1C: 92F5006C  stw      r23,108(r21)
00001C20: 7E9042A6  mfsprg0  r20
00001C24: 92950060  stw      r20,96(r21)
00001C28: 7ED142A6  mfsprg1  r22
00001C2C: 92D50064  stw      r22,100(r21)
00001C30: 7E8802A6  mflr     r20
00001C34: 929500A0  stw      r20,160(r21)
00001C38: 7EC902A6  mfctr    r22
00001C3C: 92D5009C  stw      r22,156(r21)
00001C40: 7E8102A6  mfxer    r20
00001C44: 929500A4  stw      r20,164(r21)
00001C48: 7EDA02A6  mfsrr0   r22
00001C4C: 7EFB02A6  mfsrr1   r23
00001C50: 90150010  stw      r0,16(r21)
00001C54: 90350014  stw      rsp,20(r21)
00001C58: 90550018  stw      r2,24(r21)
00001C5C: 90350000  stw      rsp,0(r21)
00001C60: 7EA1AB78  mr       rsp,r21
00001C64: 9075001C  stw      r3,28(r21)
00001C68: 90950020  stw      r4,32(r21)
00001C6C: 90B50024  stw      r5,36(r21)
00001C70: 90D50028  stw      r6,40(r21)
00001C74: 806E8014  lwz      r3,-32748(r14)
00001C78: 7C6803A6  mtlr     r3
00001C7C: 38610010  addi     r3,rsp,16
00001C80: 3A801002  li       r20,4098
00001C84: 4E800021  blrl
00001C88: 00001C88  dc.l     0x00001c88              ; Invalid opcode '....'
00001C8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ReservedD"  Size=0
00001D00: 7E9043A6  mtsprg0  r20
00001D04: 7EB143A6  mtsprg1  r21
00001D08: 7E800026  mfcr     r20
00001D0C: 7C350B78  mr       r21,rsp
00001D10: 3AB5FF00  subi     r21,r21,256
00001D14: 929500A8  stw      r20,168(r21)
00001D18: 92D50068  stw      r22,104(r21)
00001D1C: 92F5006C  stw      r23,108(r21)
00001D20: 7E9042A6  mfsprg0  r20
00001D24: 92950060  stw      r20,96(r21)
00001D28: 7ED142A6  mfsprg1  r22
00001D2C: 92D50064  stw      r22,100(r21)
00001D30: 7E8802A6  mflr     r20
00001D34: 929500A0  stw      r20,160(r21)
00001D38: 7EC902A6  mfctr    r22
00001D3C: 92D5009C  stw      r22,156(r21)
00001D40: 7E8102A6  mfxer    r20
00001D44: 929500A4  stw      r20,164(r21)
00001D48: 7EDA02A6  mfsrr0   r22
00001D4C: 7EFB02A6  mfsrr1   r23
00001D50: 90150010  stw      r0,16(r21)
00001D54: 90350014  stw      rsp,20(r21)
00001D58: 90550018  stw      r2,24(r21)
00001D5C: 90350000  stw      rsp,0(r21)
00001D60: 7EA1AB78  mr       rsp,r21
00001D64: 9075001C  stw      r3,28(r21)
00001D68: 90950020  stw      r4,32(r21)
00001D6C: 90B50024  stw      r5,36(r21)
00001D70: 90D50028  stw      r6,40(r21)
00001D74: 806E8014  lwz      r3,-32748(r14)
00001D78: 7C6803A6  mtlr     r3
00001D7C: 38610010  addi     r3,rsp,16
00001D80: 3A801002  li       r20,4098
00001D84: 4E800021  blrl
00001D88: 00001D88  dc.l     0x00001d88              ; Invalid opcode '....'
00001D8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ReservedE"  Size=0
00001E00: 7E9043A6  mtsprg0  r20
00001E04: 7EB143A6  mtsprg1  r21
00001E08: 7E800026  mfcr     r20
00001E0C: 7C350B78  mr       r21,rsp
00001E10: 3AB5FF00  subi     r21,r21,256
00001E14: 929500A8  stw      r20,168(r21)
00001E18: 92D50068  stw      r22,104(r21)
00001E1C: 92F5006C  stw      r23,108(r21)
00001E20: 7E9042A6  mfsprg0  r20
00001E24: 92950060  stw      r20,96(r21)
00001E28: 7ED142A6  mfsprg1  r22
00001E2C: 92D50064  stw      r22,100(r21)
00001E30: 7E8802A6  mflr     r20
00001E34: 929500A0  stw      r20,160(r21)
00001E38: 7EC902A6  mfctr    r22
00001E3C: 92D5009C  stw      r22,156(r21)
00001E40: 7E8102A6  mfxer    r20
00001E44: 929500A4  stw      r20,164(r21)
00001E48: 7EDA02A6  mfsrr0   r22
00001E4C: 7EFB02A6  mfsrr1   r23
00001E50: 90150010  stw      r0,16(r21)
00001E54: 90350014  stw      rsp,20(r21)
00001E58: 90550018  stw      r2,24(r21)
00001E5C: 90350000  stw      rsp,0(r21)
00001E60: 7EA1AB78  mr       rsp,r21
00001E64: 9075001C  stw      r3,28(r21)
00001E68: 90950020  stw      r4,32(r21)
00001E6C: 90B50024  stw      r5,36(r21)
00001E70: 90D50028  stw      r6,40(r21)
00001E74: 806E8014  lwz      r3,-32748(r14)
00001E78: 7C6803A6  mtlr     r3
00001E7C: 38610010  addi     r3,rsp,16
00001E80: 3A801002  li       r20,4098
00001E84: 4E800021  blrl
00001E88: 00001E88  dc.l     0x00001e88              ; Invalid opcode '....'
00001E8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="ReservedF"  Size=0
00001F00: 7E9043A6  mtsprg0  r20
00001F04: 7EB143A6  mtsprg1  r21
00001F08: 7E800026  mfcr     r20
00001F0C: 7C350B78  mr       r21,rsp
00001F10: 3AB5FF00  subi     r21,r21,256
00001F14: 929500A8  stw      r20,168(r21)
00001F18: 92D50068  stw      r22,104(r21)
00001F1C: 92F5006C  stw      r23,108(r21)
00001F20: 7E9042A6  mfsprg0  r20
00001F24: 92950060  stw      r20,96(r21)
00001F28: 7ED142A6  mfsprg1  r22
00001F2C: 92D50064  stw      r22,100(r21)
00001F30: 7E8802A6  mflr     r20
00001F34: 929500A0  stw      r20,160(r21)
00001F38: 7EC902A6  mfctr    r22
00001F3C: 92D5009C  stw      r22,156(r21)
00001F40: 7E8102A6  mfxer    r20
00001F44: 929500A4  stw      r20,164(r21)
00001F48: 7EDA02A6  mfsrr0   r22
00001F4C: 7EFB02A6  mfsrr1   r23
00001F50: 90150010  stw      r0,16(r21)
00001F54: 90350014  stw      rsp,20(r21)
00001F58: 90550018  stw      r2,24(r21)
00001F5C: 90350000  stw      rsp,0(r21)
00001F60: 7EA1AB78  mr       rsp,r21
00001F64: 9075001C  stw      r3,28(r21)
00001F68: 90950020  stw      r4,32(r21)
00001F6C: 90B50024  stw      r5,36(r21)
00001F70: 90D50028  stw      r6,40(r21)
00001F74: 806E8014  lwz      r3,-32748(r14)
00001F78: 7C6803A6  mtlr     r3
00001F7C: 38610010  addi     r3,rsp,16
00001F80: 3A801002  li       r20,4098
00001F84: 4E800021  blrl
00001F88: 00001F88  dc.l     0x00001f88              ; Invalid opcode '....'
00001F8C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_LOCAL_CODE     Name="RunModeTrace"  Size=0
00002000: 7E9043A6  mtsprg0  r20
00002004: 7EB143A6  mtsprg1  r21
00002008: 7E800026  mfcr     r20
0000200C: 7C350B78  mr       r21,rsp
00002010: 3AB5FF00  subi     r21,r21,256
00002014: 929500A8  stw      r20,168(r21)
00002018: 92D50068  stw      r22,104(r21)
0000201C: 92F5006C  stw      r23,108(r21)
00002020: 7E9042A6  mfsprg0  r20
00002024: 92950060  stw      r20,96(r21)
00002028: 7ED142A6  mfsprg1  r22
0000202C: 92D50064  stw      r22,100(r21)
00002030: 7E8802A6  mflr     r20
00002034: 929500A0  stw      r20,160(r21)
00002038: 7EC902A6  mfctr    r22
0000203C: 92D5009C  stw      r22,156(r21)
00002040: 7E8102A6  mfxer    r20
00002044: 929500A4  stw      r20,164(r21)
00002048: 7EDA02A6  mfsrr0   r22
0000204C: 7EFB02A6  mfsrr1   r23
00002050: 90150010  stw      r0,16(r21)
00002054: 90350014  stw      rsp,20(r21)
00002058: 90550018  stw      r2,24(r21)
0000205C: 90350000  stw      rsp,0(r21)
00002060: 7EA1AB78  mr       rsp,r21
00002064: 9075001C  stw      r3,28(r21)
00002068: 90950020  stw      r4,32(r21)
0000206C: 90B50024  stw      r5,36(r21)
00002070: 90D50028  stw      r6,40(r21)
00002074: 806E8014  lwz      r3,-32748(r14)
00002078: 7C6803A6  mtlr     r3
0000207C: 38610010  addi     r3,rsp,16
00002080: 3A801002  li       r20,4098
00002084: 4E800021  blrl
00002088: 00002088  dc.l     0x00002088              ; Invalid opcode '.. .'
0000208C: 0000309C  dc.l     0x0000309c              ; Invalid opcode '..0.'

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="_end_of_vectors"  Size=0

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="transfer_to_handler"  Size=0
00003000: 92D50090  stw      r22,144(r21)
00003004: 3EC00004  lis      r22,4
00003008: 7EF7B078  andc     r23,r23,r22
0000300C: 92F50094  stw      r23,148(r21)
00003010: 90F5002C  stw      r7,44(r21)
00003014: 91150030  stw      r8,48(r21)
00003018: 91350034  stw      r9,52(r21)
0000301C: 91550038  stw      r10,56(r21)
00003020: 9175003C  stw      r11,60(r21)
00003024: 91950040  stw      r12,64(r21)
00003028: 91B50044  stw      r13,68(r21)
0000302C: 91D50048  stw      r14,72(r21)
00003030: 91F5004C  stw      r15,76(r21)
00003034: 92150050  stw      r16,80(r21)
00003038: 92350054  stw      r17,84(r21)
0000303C: 92550058  stw      r18,88(r21)
00003040: 9275005C  stw      r19,92(r21)
00003044: 93150070  stw      r24,112(r21)
00003048: 93350074  stw      r25,116(r21)
0000304C: 93550078  stw      r26,120(r21)
00003050: 9375007C  stw      r27,124(r21)
00003054: 93950080  stw      r28,128(r21)
00003058: 93B50084  stw      r29,132(r21)
0000305C: 93D50088  stw      r30,136(r21)
00003060: 93F5008C  stw      r31,140(r21)
00003064: 7EE802A6  mflr     r23
00003068: 72F83F00  andi.    r24,r23,0x3f00
0000306C: 931500B0  stw      r24,176(r21)
00003070: 3AC00000  li       r22,0
00003074: 92D500BC  stw      r22,188(r21)
00003078: 7ED243A6  mtsprg2  r22
0000307C: 83170000  lwz      r24,0(r23)
00003080: 82F70004  lwz      r23,4(r23)
00003084: 7F1A03A6  mtsrr0   r24
00003088: 7E9B03A6  mtsrr1   r20
0000308C: 7EE803A6  mtlr     r23
00003090: 7C0004AC  sync
00003094: 4C00012C  isync
00003098: 4C000064  rfi

Hunk:	Kind=HUNK_LOCAL_CODE     Name="int_return"  Size=0
0000309C: 7FA000A6  mfmsr    r29
000030A0: 38800000  li       r4,0
000030A4: 60848000  ori      r4,r4,0x8000
000030A8: 7FBD2078  andc     r29,r29,r4
000030AC: 7C0004AC  sync
000030B0: 4C00012C  isync
000030B4: 7FA00124  mtmsr    r29
000030B8: 7C0004AC  sync
000030BC: 4C00012C  isync
000030C0: 8041009C  lwz      r2,156(rsp)
000030C4: 800100A0  lwz      r0,160(rsp)
000030C8: 7C4903A6  mtctr    r2
000030CC: 7C0803A6  mtlr     r0
000030D0: 804100A4  lwz      r2,164(rsp)
000030D4: 800100A8  lwz      r0,168(rsp)
000030D8: 7C4103A6  mtxer    r2
000030DC: 7C0FF120  mtcrf    0xff,r0
000030E0: 8061001C  lwz      r3,28(rsp)
000030E4: 80810020  lwz      r4,32(rsp)
000030E8: 80A10024  lwz      r5,36(rsp)
000030EC: 80C10028  lwz      r6,40(rsp)
000030F0: 80E1002C  lwz      r7,44(rsp)
000030F4: 81010030  lwz      r8,48(rsp)
000030F8: 81210034  lwz      r9,52(rsp)
000030FC: 81410038  lwz      r10,56(rsp)
00003100: 8161003C  lwz      r11,60(rsp)
00003104: 81810040  lwz      r12,64(rsp)
00003108: 81A10044  lwz      r13,68(rsp)
0000310C: 81C10048  lwz      r14,72(rsp)
00003110: 81E1004C  lwz      r15,76(rsp)
00003114: 82010050  lwz      r16,80(rsp)
00003118: 82210054  lwz      r17,84(rsp)
0000311C: 82410058  lwz      r18,88(rsp)
00003120: 8261005C  lwz      r19,92(rsp)
00003124: 82810060  lwz      r20,96(rsp)
00003128: 82A10064  lwz      r21,100(rsp)
0000312C: 82C10068  lwz      r22,104(rsp)
00003130: 82E1006C  lwz      r23,108(rsp)
00003134: 83010070  lwz      r24,112(rsp)
00003138: 83210074  lwz      r25,116(rsp)
0000313C: 83410078  lwz      r26,120(rsp)
00003140: 8361007C  lwz      r27,124(rsp)
00003144: 83810080  lwz      r28,128(rsp)
00003148: 83A10084  lwz      r29,132(rsp)
0000314C: 83C10088  lwz      r30,136(rsp)
00003150: 83E1008C  lwz      r31,140(rsp)
00003154: 80410090  lwz      r2,144(rsp)
00003158: 80010094  lwz      r0,148(rsp)
0000315C: 7C5A03A6  mtsrr0   r2
00003160: 7C1B03A6  mtsrr1   r0
00003164: 80010010  lwz      r0,16(rsp)
00003168: 80410018  lwz      r2,24(rsp)
0000316C: 80210014  lwz      rsp,20(rsp)
00003170: 7C0004AC  sync
00003174: 4C00012C  isync
00003178: 4C000064  rfi

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="icache_enable"  Size=0
0000317C: 7CB0FAA6  mfdbsr   r5
00003180: 60A58800  ori      r5,r5,0x8800
00003184: 3CC0FFFF  lis      r6,-1
00003188: 60C6F7FF  ori      r6,r6,0xf7ff
0000318C: 7CA63038  and      r6,r5,r6
00003190: 7C0004AC  sync
00003194: 7CB0FBA6  mtdbsr   r5
00003198: 7CD0FBA6  mtdbsr   r6
0000319C: 4C00012C  isync
000031A0: 7C0004AC  sync
000031A4: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="icache_disable"  Size=0
000031A8: 7CB0FAA6  mfdbsr   r5
000031AC: 3CC0FFFF  lis      r6,-1
000031B0: 60C67FFF  ori      r6,r6,0x7fff
000031B4: 7CA53038  and      r5,r5,r6
000031B8: 7C0004AC  sync
000031BC: 7CB0FBA6  mtdbsr   r5
000031C0: 4C00012C  isync
000031C4: 7C0004AC  sync
000031C8: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="icache_status"  Size=0
000031CC: 7C70FAA6  mfdbsr   r3
000031D0: 54638BFE  srwi     r3,r3,15
000031D4: 70630001  andi.    r3,r3,0x0001
000031D8: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="dcache_enable"  Size=0
000031DC: 7CB0FAA6  mfdbsr   r5
000031E0: 60A54400  ori      r5,r5,0x4400
000031E4: 7C9F42A6  mfpvr    r4
000031E8: 7C838670  srawi    r3,r4,16
000031EC: 2C03000C  cmpwi    r3,0x000c
000031F0: 40820008  bne      *+8                     ; 0x000031F8
000031F4: 60A50040  ori      r5,r5,0x0040

Hunk:	Kind=HUNK_LOCAL_CODE     Name="NotMax"  Size=0
000031F8: 3CC0FFFF  lis      r6,-1
000031FC: 60C6FBFF  ori      r6,r6,0xfbff
00003200: 7CA63038  and      r6,r5,r6
00003204: 7C0004AC  sync
00003208: 7CB0FBA6  mtdbsr   r5
0000320C: 7CD0FBA6  mtdbsr   r6
00003210: 4C00012C  isync
00003214: 7C0004AC  sync
00003218: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="dcache_disable"  Size=0
0000321C: 7CB0FAA6  mfdbsr   r5
00003220: 3CC0FFFF  lis      r6,-1
00003224: 60C6BFFF  ori      r6,r6,0xbfff
00003228: 7CA53038  and      r5,r5,r6
0000322C: 7C0004AC  sync
00003230: 7CB0FBA6  mtdbsr   r5
00003234: 4C00012C  isync
00003238: 7C0004AC  sync
0000323C: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="dcache_status"  Size=0
00003240: 7C70FAA6  mfdbsr   r3
00003244: 546393BE  srwi     r3,r3,14
00003248: 70630001  andi.    r3,r3,0x0001
0000324C: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="dc_read"  Size=0
00003250: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="get_pvr"  Size=0
00003254: 7C7F42A6  mfpvr    r3
00003258: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="udelay"  Size=0
0000325C: 1C832710  mulli    r4,r3,10000
00003260: 38A00C35  li       r5,3125
00003264: 7C842BD6  divw     r4,r4,r5
00003268: 7CAD42E6  mftbu    r5
0000326C: 7CCC42E6  mftb     r6
00003270: 7CED42E6  mftbu    r7
00003274: 7C053800  cmpw     r5,r7
00003278: 4082FFF0  bne      *-16                    ; 0x00003268
0000327C: 7D262014  addc     r9,r6,r4
00003280: 7D050194  addze    r8,r5
00003284: 7CAD42E6  mftbu    r5
00003288: 7C054000  cmpw     r5,r8
0000328C: 4180FFF8  blt      *-8                     ; 0x00003284
00003290: 41810010  bgt      *+16                    ; 0x000032A0
00003294: 7CCC42E6  mftb     r6
00003298: 7C064800  cmpw     r6,r9
0000329C: 4180FFE8  blt      *-24                    ; 0x00003284
000032A0: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="relocate_code"  Size=0

Hunk:	Kind=HUNK_LOCAL_CODE     Name="clear_bss"  Size=0
000032A4: 806E801C  lwz      r3,-32740(r14)
000032A8: 808E8018  lwz      r4,-32744(r14)
000032AC: 7C032040  cmplw    r3,r4
000032B0: 41820018  beq      *+24                    ; 0x000032C8
000032B4: 38000000  li       r0,0
000032B8: 90030000  stw      r0,0(r3)
000032BC: 38630004  addi     r3,r3,4
000032C0: 7C032040  cmplw    r3,r4
000032C4: 4082FFF4  bne      *-12                    ; 0x000032B8
000032C8: 7D234B78  mr       r3,r9
000032CC: 7D445378  mr       r4,r10
000032D0: 48000001  bl       *+0                     ; 0x000032D0

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="get_endaddr"  Size=0
000032D4: 806E8018  lwz      r3,-32744(r14)
000032D8: 4E800020  blr

Hunk:	Kind=HUNK_GLOBAL_CODE     Name="trap_init"  Size=0
000032DC: 80EE8008  lwz      r7,-32760(r14)
000032E0: 810E8010  lwz      r8,-32752(r14)
000032E4: 54E904BE  clrlwi   r9,r7,18
000032E8: 7C074040  cmplw    r7,r8
000032EC: 4C800020  bgelr
000032F0: 7C8802A6  mflr     r4
000032F4: 80070000  lwz      r0,0(r7)
000032F8: 90090000  stw      r0,0(r9)
000032FC: 38E70004  addi     r7,r7,4
00003300: 39290004  addi     r9,r9,4
00003304: 7C074040  cmplw    r7,r8
00003308: 4082FFEC  bne      *-20                    ; 0x000032F4
0000330C: 38E00288  li       r7,648
00003310: 39000600  li       r8,1536
00003314: 48000041  bl       *+64                    ; 0x00003354
00003318: 38E70100  addi     r7,r7,256
0000331C: 7C074040  cmplw    r7,r8
00003320: 4180FFF4  blt      *-12                    ; 0x00003314
00003324: 38E0069C  li       r7,1692
00003328: 4800002D  bl       *+44                    ; 0x00003354
0000332C: 38E0078C  li       r7,1932
00003330: 48000025  bl       *+36                    ; 0x00003354
00003334: 38E00888  li       r7,2184
00003338: 39002090  li       r8,8336
0000333C: 48000019  bl       *+24                    ; 0x00003354
00003340: 38E70100  addi     r7,r7,256
00003344: 7C074040  cmplw    r7,r8
00003348: 4180FFF4  blt      *-12                    ; 0x0000333C
0000334C: 7C8803A6  mtlr     r4
00003350: 4E800020  blr

Hunk:	Kind=HUNK_LOCAL_CODE     Name="trap_reloc"  Size=0
00003354: 80070000  lwz      r0,0(r7)
00003358: 7C001A14  add      r0,r0,r3
0000335C: 90070000  stw      r0,0(r7)
00003360: 80070004  lwz      r0,4(r7)
00003364: 7C001A14  add      r0,r0,r3
00003368: 90070004  stw      r0,4(r7)
0000336C: 4E800020  blr
00003370: 0000B21C  dc.l     0x0000b21c              ; Invalid opcode '....'

==> .data

==> .bss

==> .rela.got2
    entry  offset   addend   type name(symbol id)
    [   0] 00000000 00000000    1 _GOT2_TABLE_(74)
    [   1] 00000004 00000000    1 _FIXUP_TABLE_(75)
    [   2] 00000008 00000000    1 _start(76)
    [   3] 0000000c 00000000    1 _start_of_vectors(77)
    [   4] 00000010 00000000    1 _end_of_vectors(78)
    [   5] 00000014 00000000    1 transfer_to_handler(79)
    [   6] 00000018 00000000    1 _end(80)
    [   7] 0000001c 00000000    1 .bss(34)

==> .got2
00000000: 00000000  00000000  00000000  00000000 '................'
00000010: 00000000  00000000  00000000  00000000 '................'
