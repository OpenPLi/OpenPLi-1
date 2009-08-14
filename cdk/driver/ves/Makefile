DRIVER_TOPDIR = ..

include $(DRIVER_TOPDIR)/Config.make

obj-m		:= ves1820.o ves1893.o ves1993.o tda8044h.o at76c651.o

EXTRA_CFLAGS	:= -D__DVB_PACK__

include $(DRIVER_TOPDIR)/Rules.make
