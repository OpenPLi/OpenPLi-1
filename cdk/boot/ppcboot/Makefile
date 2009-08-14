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

HOSTARCH := $(shell uname -m | \
	sed -e s/i.86/i386/ -e s/sun4u/sparc64/ -e s/arm.*/arm/ -e s/sa110/arm/)

ifeq ($(HOSTARCH),ppc)
CROSS_COMPILE =
else
CROSS_COMPILE = powerpc-linux-
endif

export	CROSS_COMPILE HOSTARCH

#########################################################################

ifndef CYGWIN
TOPDIR	:= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
else
TOPDIR	:= $(shell pwd)
endif
export	TOPDIR

ifeq (include/config.mk,$(wildcard include/config.mk))
# load ARCH, BOARD, and CPU configuration
include include/config.mk
export	ARCH CPU BOARD
# load other configuration
include $(TOPDIR)/config.mk

ifndef CYGWIN
SUBDIRS	= $(ARCH) $(CPU) $(BOARD) common net disk examples tools
else
SUBDIRS	= $(ARCH) $(CPU) $(BOARD) common net disk
endif

#########################################################################
# ppcboot objects....order is important (i.e. start must be first)

OBJS  =	$(CPU)/start.o		\
	common/libcommon.a

OBJS +=	net/libnet.a disk/libdisk.a

OBJS +=	$(BOARD)/lib$(BOARD).a	\
	$(CPU)/lib$(CPU).a	\
	$(ARCH)/lib$(ARCH).a

ifeq ($(CPU),ppc4xx)
OBJS +=	$(CPU)/resetvec.o
endif

#########################################################################

all:	ppcboot.srec ppcboot.bin

install:	all
		#cp ppcboot.srec /tftpboot/moni.srec
		cp ppcboot.srec /net/gatekeeper/tftpboot/moni.srec

ppcboot.srec:	ppcboot
		$(OBJCOPY) ${OBJCFLAGS} -O srec $< $@

ppcboot.bin:	ppcboot
		$(OBJCOPY) ${OBJCFLAGS} -O binary $< $@

ppcboot:	depend subdirs $(OBJS) $(LDSCRIPT)
		$(LD) $(LDFLAGS) $(OBJS) -Map ppcboot.map -o ppcboot

subdirs:
		@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir || exit 1 ; done

depend dep:
		@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir .depend ; done

#########################################################################
else
all install ppcboot ppcboot.srec depend dep:
	@echo "System not configured - see README" >&2
	@ exit 1
endif

#########################################################################

unconfig:
	rm -f include/config.h include/config.mk

TQM823L_config	\
TQM850L_config	\
FPS850L_config	\
TQM855L_config	\
TQM860L_config	:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = tqm8xx"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

dbox2_config     :       unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = dbox"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

ETX094_config	:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = etx094"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

SPD823TS_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = spd8xx"	>>config.mk ;	\
 	echo "CPU   = mpc8xx"	>>config.mk ;	\
 	echo "#include <config_$(@:_config=).h>" >config.h

IVMS8_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = ivms8"	>>config.mk ;	\
 	echo "CPU   = mpc8xx"	>>config.mk ;	\
 	echo "#include <config_$(@:_config=).h>" >config.h

SM850_config	:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = tqm8xx"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

FADS823_config	\
FADS850SAR_config \
FADS860T_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = fads"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

ADS860_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = fads"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

CPCI405_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = cpci405"	>>config.mk ;	\
	echo "CPU   = ppc4xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

ADCIOP_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = adciop"	>>config.mk ;	\
	echo "CPU   = ppc4xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

cogent_mpc8xx_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = cogent"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

GENIETV_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = genietv"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

MBX_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = mbx8xx"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

MBX860T_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = mbx"	>>config.mk ;	\
	echo "CPU   = mpc8xx"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

Sandpoint8240_config: unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = sandpoint"	>>config.mk ;	\
	echo "CPU   = mpc8240"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

hymod_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = hymod"	>>config.mk ;	\
	echo "CPU   = mpc8260"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

cogent_mpc8260_config:	unconfig
	@echo "Configuring for $(@:_config=) Board..." ; \
	cd include ;				\
	echo "ARCH  = ppc"	> config.mk ;	\
	echo "BOARD = cogent"	>>config.mk ;	\
	echo "CPU   = mpc8260"	>>config.mk ;	\
	echo "#include <config_$(@:_config=).h>" >config.h

#########################################################################

clean:
	rm -f `find . -type f \
		\( -name 'core' -o -name '*.bak' \
		-o -name '*.o'  -o -name '*.a'  \) -print`
	rm -f examples/hello_world examples/timer
	rm -f tools/img2srec tools/mkimage tools/easylogo/easylogo
	rm -f tools/gdb/astest tools/gdb/gdbcont tools/gdb/gdbsend

clobber:	clean
	rm -f `find . -type f \
		\( -name .depend -o -name '*.srec' -o -name '*.bin' \) \
		-print`
	rm -f $(OBJS) *.bak
	rm -f ppcboot ppcboot.bin ppcboot.elf ppcboot.srec ppcboot.map

distclean:	clobber unconfig

backup:
	F=`basename $(TOPDIR)` ; cd .. ; \
	gtar --force-local -zcvf `date "+$$F-%Y-%m-%d-%T.tar.gz"` $$F

#########################################################################
