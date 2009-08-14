DRIVER_TOPDIR = ..

DEBUG = 0
STANDALONE = 0

include $(DRIVER_TOPDIR)/Config.make

obj-m		:= avia_av.o avia_gt.o avia_gt_napi.o avia_gt_fb.o avia_gt_oss.o avia_gt_lirc.o avia_gt_dvr.o
export-objs	:= avia_av_core.o avia_gt_core.o avia_gt_accel.o avia_gt_dmx.o avia_gt_gv.o avia_gt_pcm.o avia_gt_capture.o avia_gt_pig.o avia_gt_ir.o avia_gt_vbi.o fbgen.o

AVIA_AV_MODS	:= avia_av_event.o avia_av_proc.o
AVIA_AV		:= avia_av_core.o 

AVIA_GT_MODS	:= avia_gt_accel.o avia_gt_dmx.o avia_gt_gv.o avia_gt_pcm.o avia_gt_capture.o avia_gt_pig.o avia_gt_ir.o avia_gt_vbi.o
AVIA_GT		:= avia_gt_core.o avia_gt_enx.o avia_gt_gtx.o
AVIA_GT_FB	:= avia_gt_fb_core.o fbgen.o

ifeq ($(DEBUG), 1)
EXTRA_CFLAGS	+= -DDEBUG
endif

ifeq ($(STANDALONE), 1)
obj-m		+= $(AVIA_AV_MODS) $(AVIA_GT_MODS)
EXTRA_CFLAGS	+= -DSTANDALONE
else
AVIA_AV		+= $(AVIA_AV_MODS)
AVIA_GT		+= $(AVIA_GT_MODS)
endif

avia_av.o: $(AVIA_AV)
	$(LD) -r -o $@ $(AVIA_AV)

avia_gt.o: $(AVIA_GT)
	$(LD) -r -o $@ $(AVIA_GT)

avia_gt_fb.o: $(AVIA_GT_FB)
	$(LD) -r -o $@ $(AVIA_GT_FB)

fbgen.c: $(KERNEL_LOCATION)/drivers/video/fbgen.c
	ln -s $(KERNEL_LOCATION)/drivers/video/fbgen.c

include $(DRIVER_TOPDIR)/Rules.make
