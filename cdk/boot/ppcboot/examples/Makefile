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

LOAD_ADDR = 0x40000

include $(TOPDIR)/config.mk

SREC	= hello_world.srec

# The following example is pretty 8xx specific...
ifeq ($(CPU),mpc8xx)
SREC   += timer.srec
endif

OBJS	= $(SREC:.srec=.o)

CPPFLAGS += -I..

all:	.depend $(SREC)

#########################################################################

%.srec:	%.o
	$(LD) -s -Ttext $(LOAD_ADDR) -o $(<:.o=) -e $(<:.o=) $<
	$(OBJCOPY) -O srec $(<:.o=) $@

#########################################################################

.depend:	Makefile $(OBJS:.o=.c)
		$(CC) -M $(CFLAGS) $(OBJS:.o=.c) > $@

sinclude .depend

#########################################################################
