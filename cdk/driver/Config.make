KERNELRELEASE 	:= \
	$(shell \
	for TAG in VERSION PATCHLEVEL SUBLEVEL EXTRAVERSION FLAVOUR ; do \
		eval `sed -ne "/^$$TAG/s/[   ]//gp" $(KERNEL_LOCATION)/Makefile` ; \
	done ; \
	echo $$VERSION.$$PATCHLEVEL.$$SUBLEVEL$$EXTRAVERSION$${FLAVOUR:+-$$FLAVOUR})

ARCH		:= ppc

CONFIG_SHELL 	:= $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
		else if [ -x /bin/bash ]; then echo /bin/bash; \
		else echo sh; fi ; fi)
TOPDIR		:= $(KERNEL_LOCATION)

HPATH		:= $(DRIVER_TOPDIR)/include

CROSS_COMPILE =

AS		= $(CROSS_COMPILE)as
LD		= $(CROSS_COMPILE)ld
CC		= $(CROSS_COMPILE)gcc
CPP		= $(CC) -E
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
STRIP		= $(CROSS_COMPILE)strip
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump

CPPFLAGS	:= -D__KERNEL__ -DMODULE -I$(DRIVER_TOPDIR)/include -I$(KERNEL_LOCATION)/include
CFLAGS		:= $(CPPFLAGS) -Wall -Wstrict-prototypes -Wno-trigraphs -Werror -O2 -fno-strict-aliasing -fno-common -fomit-frame-pointer
AFLAGS		:= -D__ASSEMBLY__ $(CPPFLAGS)

MODLIB		:= $(INSTALL_MOD_PATH)/lib/modules/$(KERNELRELEASE)

ifeq ($(HARDWARE),dbox2)
CONFIG_HARDWARE_DBOX2		:= m
else
ifeq ($(HARDWARE),dreambox)
CONFIG_HARDWARE_DREAMBOX	:= m
else
CONFIG_HARDWARE_DBOX2		:= m
endif
endif

export	TOPDIR HPATH

ifeq (.depend,$(wildcard .depend))
all: modules
else
all: depend modules
endif

include $(KERNEL_LOCATION)/.config

include $(KERNEL_LOCATION)/arch/$(ARCH)/Makefile

SUBDIRS =
