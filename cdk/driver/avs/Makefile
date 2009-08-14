DRIVER_TOPDIR = ..

include $(DRIVER_TOPDIR)/Config.make

obj-m		:= avs.o
export-objs	:= avs_core.o

AVS_OBJS	:= stv6412.o cxa2092.o cxa2126.o avs_core.o

avs.o:	$(AVS_OBJS)
	$(LD) -r -o $@ $(AVS_OBJS)

include $(DRIVER_TOPDIR)/Rules.make
