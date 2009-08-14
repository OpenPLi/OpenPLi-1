DRIVER_TOPDIR = ..

include $(DRIVER_TOPDIR)/Config.make

obj-m		:= dmxdev.o dvb.o dvb_demux.o dvb_filter.o dvb_frontend.o dvb_net.o dvbdev.o
export-objs	:= dvb.o dvbdev.o

EXTRA_CFLAGS	:= -D__DVB_PACK__

include $(DRIVER_TOPDIR)/Rules.make
