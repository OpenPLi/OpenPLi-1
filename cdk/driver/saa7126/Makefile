DRIVER_TOPDIR = ..

include $(DRIVER_TOPDIR)/Config.make

obj-m		:= saa7126.o
export-objs	:= saa7126_core.o

SAA_OBJS	:= saa7126_core.o

saa7126.o: $(SAA_OBJS)
	$(LD) -r -o $@ $(SAA_OBJS)

include $(DRIVER_TOPDIR)/Rules.make