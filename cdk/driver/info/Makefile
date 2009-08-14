DRIVER_TOPDIR = ..

include $(DRIVER_TOPDIR)/Config.make

obj-m				:= tuxbox.o
obj-$(CONFIG_HARDWARE_DBOX2)	+= info.o
export-objs			:= info.o

TUXBOX_OBJS			:= tuxbox_core.o

ifdef CONFIG_HARDWARE_DBOX2
TUXBOX_OBJS			+= tuxbox_hardware_dbox2.o
endif
ifdef CONFIG_HARDWARE_DREAMBOX
TUXBOX_OBJS			+= tuxbox_hardware_dreambox.o
endif

tuxbox.o: $(TUXBOX_OBJS)
	$(LD) -r -o $@ $(TUXBOX_OBJS)

include $(DRIVER_TOPDIR)/Rules.make
