#
# (C) Copyright 2000
# Murray Jensen <Murray.Jensen@cmst.csiro.au>
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

BINS	= gdbsend gdbcont astest

OBJS	= gdbsend.o gdbcont.o astest.o error.o remote.o serial.o

#
# Use native tools and options
#
CPPFLAGS   = -Wall -pedantic -O -I/opt/powerpc/include
CFLAGS     = $(CPPFLAGS)
CC	   = $(HOSTCC)
MAKEDEPEND = makedepend

ifeq ($(OSTYPE),cygwin)
all:
.depend:
else
all:	$(BINS)

gdbsend:	gdbsend.o error.o remote.o serial.o
		$(CC) $(CFLAGS) -o $@ $^

gdbcont:	gdbcont.o error.o remote.o serial.o
		$(CC) $(CFLAGS) -o $@ $^

astest:		astest.o error.o
		$(CC) $(CFLAGS) -o $@ $^ -L/opt/powerpc/lib -lbfd -liberty

clean:
	rm -f $(OBJS)

distclean:	clean
	rm -f $(BINS) core *.bak .depend

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CPPFLAGS) -I../include $(OBJS:.o=.c) > $@

-include .depend

#########################################################################

endif	# cygwin
