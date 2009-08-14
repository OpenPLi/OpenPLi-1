AUTOMAKE_OPTIONS = gnu

SUBDIRS = \
	include tuxmail tuxtxt tuxcom tuxcal fx2 vncviewer enigma

if BOXTYPE_DREAMBOX
SUBDIRS += pip mosaic
else
SUBDIRS += pluginx
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	tuxbox-plugins.pc

