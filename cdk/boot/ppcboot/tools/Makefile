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

include $(TOPDIR)/config.mk

BINS	= img2srec mkimage

OBJS	= img2srec.o mkimage.o crc32.o

#TOOLSUBDIRS = gdb

#
# Use native tools and options
#
CPPFLAGS   = -Wall -pedantic -O
CFLAGS     = $(CPPFLAGS) -I../include
MAKEDEPEND = makedepend

all:	.depend $(BINS) subdirs

img2srec:	img2srec.o
		$(HOSTCC) $(CFLAGS) -s -o $@ $^

mkimage:	mkimage.o crc32.o
		$(HOSTCC) -g $(CFLAGS) -o $@ $^

crc32.o:	crc32.c
		$(HOSTCC) -g $(CFLAGS) -c $<

mkimage.o:	mkimage.c
		$(HOSTCC) -g $(CFLAGS) -c $<

subdirs:
#		@for dir in $(TOOLSUBDIRS) ; do $(MAKE) -C $$dir || exit 1 ; done

crc32.c:
		ln -s ../ppc/crc32.c crc32.c

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(HOSTCC) -M $(CPPFLAGS) -I../include $(OBJS:.o=.c) > $@

%.o: %.c
		$(HOSTCC) -g $(CFLAGS) -c $<

sinclude .depend

#########################################################################
