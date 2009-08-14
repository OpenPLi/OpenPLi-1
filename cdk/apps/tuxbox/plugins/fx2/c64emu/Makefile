CC 	  = powerpc-linux-gcc
CXX       = powerpc-linux-g++
CPP       = powerpc-linux-gcc -E
LIBRARIES =
CFLAGS    = -O3 -g3 -Wall -Wall -DREGPARAM="__attribute__((regparm(3)))"  -I./ -DBROKEN_JOYSTICK_H=1 -DKBD_LANG=0
# if you don't have lots of memory/swap and time to spend, maybe better use -O2

.SUFFIXES: .o .cpp .h

## Files
OBJS    = main.o Display.o Prefs.o SID.o REU.o IEC.o 1541fs.o \
          1541d64.o 1541t64.o 1541job.o SAM.o CmdPipe.o vga.o
SLOBJS  = $(OBJS) C64.o CPUC64.o VIC.o CIA.o CPU1541.o

c64emu: $(SLOBJS)
	$(CXX) -o c64emu $(SLOBJS) $(LDFLAGS) $(LIBRARIES)
	powerpc-linux-strip c64emu
	cp c64emu ../c64

all: c64emu

clean:
	@-rm -f $(SLOBJS)
	@-rm -f c64emu

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) -o $@ -c $*.cpp
       
.cpp.s:
	$(CC) $(INCLUDES) $(CFLAGS) $(EXTRAFLAGS) -o $@ -S $*.cpp -g0

C64_PC.o: C64.cpp C64.h C64_x.i CmdPipe.h CPUC64.h CPU1541.h VIC.h SID.h CIA.h REU.h IEC.h 1541job.h Display.h Prefs.h
	$(CC) $(INCLUDES) $(CFLAGS) $(PCFLAGS) -o $@ -c $*.cpp

CPUC64_PC.o: CPUC64.cpp CPUC64.h CPU_emulline.i C64.h VIC.h SID.h CIA.h REU.h IEC.h Display.h Version.h
	$(CC) $(INCLUDES) $(CFLAGS) $(PCFLAGS) -o $@ -c $*.cpp

CPU1541_PC.o: CPU1541.cpp CPU1541.h CPU_emulline.i 1541job.h C64.h CIA.h Display.h
	$(CC) $(INCLUDES) $(CFLAGS) $(PCFLAGS) -o $@ -c $*.cpp

C64_SC.o: C64_SC.cpp C64.h C64_x.i CmdPipe.h CPUC64.h CPU1541.h VIC.h SID.h CIA.h REU.h IEC.h 1541job.h Display.h Prefs.h
	$(CC) $(INCLUDES) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CPUC64_SC.o: CPUC64_SC.cpp CPUC64.h CPU_emulcycle.i CPU_common.h C64.h VIC.h SID.h CIA.h REU.h IEC.h Display.h Version.h
	$(CC) $(INCLUDES) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CPU1541_SC.o: CPU1541_SC.cpp CPU1541.h CPU_emulcycle.i CPU_common.h 1541job.h C64.h CIA.h Display.h
	$(CC) $(INCLUDES) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

VIC_SC.o: VIC_SC.cpp VIC.h C64.h CPUC64.h Display.h Prefs.h
	$(CC) $(INCLUDES) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

CIA_SC.o: CIA_SC.cpp CIA.h CPUC64.h CPU1541.h VIC.h Prefs.h
	$(CC) $(INCLUDES) $(CFLAGS) $(SCFLAGS) -o $@ -c $*.cpp

## Dependencies
main.o: main.cpp main.h main_x.i C64.h Display.h Prefs.h SAM.h Version.h
C64.o: C64.cpp C64.h C64_x.i CmdPipe.h CPUC64.h CPU1541.h VIC.h SID.h CIA.h REU.h IEC.h 1541job.h Display.h Prefs.h
CmdPipe.o: CmdPipe.h
Display.o: Display.cpp Display.h Display_x.i Display_dbox.i main.h Prefs.h keyemul.c
Prefs.o: Prefs.cpp Prefs.h Display.h main.h
CPUC64.o: CPUC64.cpp CPUC64.h CPU_emulline.i C64.h VIC.h SID.h CIA.h REU.h IEC.h Display.h Version.h
CPU1541.o: CPU1541.cpp CPU1541.h CPU_emulline.i 1541job.h C64.h CIA.h Display.h
CPU_common.o: CPU_common.cpp CPU_common.h
VIC.o: VIC.cpp VIC.h C64.h CPUC64.h Display.h Prefs.h
SID.o: SID.cpp SID.h Prefs.h
CIA.o: CIA.cpp CIA.h CPUC64.h CPU1541.h VIC.h Prefs.h
IEC.o: IEC.cpp IEC.h 1541fs.h 1541d64.h 1541t64.h Prefs.h Display.h
1541fs.o: 1541fs.cpp 1541fs.h IEC.h main.h Prefs.h
1541fs.h: IEC.h
1541d64.o: 1541d64.cpp 1541d64.h IEC.h Prefs.h
1541d64.h: IEC.h
1541t64.o: 1541t64.cpp 1541t64.h IEC.h Prefs.h
1541t64.h: IEC.h
1541job.o: 1541job.cpp 1541job.h CPU1541.h Prefs.h
REU.o: REU.cpp REU.h CPUC64.h Prefs.h
SAM.o: SAM.cpp SAM.h C64.h CPUC64.h CPU1541.h VIC.h SID.h CIA.h
