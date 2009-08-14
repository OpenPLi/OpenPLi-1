#
# (C) Copyright 2000
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

#########################################################################

PLATFORM_RELFLAGS=
PLATFORM_CPPFLAGS=

ifdef	ARCH
sinclude $(TOPDIR)/$(ARCH)/config.mk	# include architecture dependend rules
endif
ifdef	CPU
sinclude $(TOPDIR)/$(CPU)/config.mk	# include  CPU	specific rules
endif
ifdef	BOARD
sinclude $(TOPDIR)/$(BOARD)/config.mk	# include board specific rules
endif

#########################################################################

CONFIG_SHELL	:= $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
		    else if [ -x /bin/bash ]; then echo /bin/bash; \
		    else echo sh; fi ; fi)

HOSTCC		= gcc
HOSTCFLAGS	= -Wall -Wstrict-prototypes -O2 -fomit-frame-pointer

#########################################################################

#
# Include the make variables (CC, etc...)
#
AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB	= $(CROSS_COMPILE)RANLIB

RELFLAGS= $(PLATFORM_RELFLAGS)
DBGFLAGS= #-g -DDEBUG
OPTFLAGS= -Os -fomit-frame-pointer
#LDSCRIPT := $(BOARD)/ppcboot.lds.debug
LDSCRIPT := $(BOARD)/ppcboot.lds

CPPFLAGS := $(DBGFLAGS) $(OPTFLAGS) $(RELFLAGS)	\
	-D__KERNEL__ -D__powerpc__		\
	-I$(TOPDIR)/include			\
	-fno-builtin				\
	-pipe $(PLATFORM_CPPFLAGS)

CFLAGS := $(CPPFLAGS) -Wall -Wno-uninitialized -Wstrict-prototypes
AFLAGS := -D__ASSEMBLY__ $(CPPFLAGS)

LDFLAGS += -Bstatic -T $(LDSCRIPT) -Ttext $(TEXT_BASE)

#OBJCFLAGS = --remove-section=.stab			\
#	--remove-section=.comment			\
#	--remove-section=.stabstr			\
#	--change-section-lma=.data+$(TEXT_BASE)		\
#	--change-section-lma=__ex_table+$(TEXT_BASE)	\
#	--change-section-lma=.bss+$(TEXT_BASE)		\
#	--set-section-flags=.bss=contents,alloc,load,data

#########################################################################

export	CONFIG_SHELL HPATH HOSTCC HOSTCFLAGS CROSS_COMPILE \
	AS LD CC CPP AR NM STRIP OBJCOPY OBJDUMP \
	MAKE
export	CPPFLAGS CFLAGS AFLAGS

#########################################################################

%.s:	%.S
	$(CPP) $(AFLAGS) -o $@ $<
%.o:	%.S
	$(CC) $(AFLAGS) -c -o $@ $<
%.o:	%.c
	$(CC) $(CFLAGS) -c -o $@ $<

#########################################################################
