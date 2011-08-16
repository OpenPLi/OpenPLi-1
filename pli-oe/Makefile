#!/usr/bin/make -f

# target platform: dm500plus, dm600pvr, dm7020, dm7025, dm800, dm8000
MACHINE ?= dm8000
BUILDDIR = build-${MACHINE}

# for a list of some other repositories have
# a look at http://git.opendreambox.org/
GIT_URL = git://openpli.git.sourceforge.net/gitroot/openpli/openembedded
GIT_BRANCH = master

# in case you want to send pull requests or generate patches
#GIT_AUTHOR_NAME ?= Your Name
#GIT_AUTHOR_EMAIL ?= you@example.com

# uncomment this if you're upgrading an installation
# from monotone. this changes the directory layout
# to the old version without shared directories.
#UPGRADE_FROM_MONOTONE=1

# you should not need to change anything below
BB_URL = git://git.opendreambox.org/git/bitbake
BB_BRANCH = 1.4.2-git

GIT = git

SED = sed

PWD := $(shell pwd)

OE_BASE = $(PWD)

all: initialize
	@echo 
	@echo "Openembedded for the OpenPLi environment has been initialized"
	@echo "properly. Now you can start building your image, by doing either:"
	@echo
	@echo " make -f Makefile-openembedded-openpli image"
	@echo "    or"
	@echo " cd $(BUILDDIR) ; source env.source ; bitbake dreambox-image"
	@echo
	@echo "and after 'some time' you should find your image (.nfi) in"
	@echo "$(BUILDDIR)/tmp/deploy/images/"
	@echo 

bb: bb/.git

bb/.git:
	$(GIT) clone -n $(BB_URL) bb
	cd bb && ( \
		if [ -n "$(GIT_AUTHOR_EMAIL)" ]; then git config user.email "$(GIT_AUTHOR_EMAIL)"; fi; \
		if [ -n "$(GIT_AUTHOR_NAME)" ]; then git config user.name "$(GIT_AUTHOR_NAME)"; fi; \
		$(GIT) branch --track $(BB_BRANCH) origin/$(BB_BRANCH) || true; \
		$(GIT) checkout -f $(BB_BRANCH) \
	)

.PHONY: image initialize openembedded-update openembedded-update-all

image: initialize openembedded-update
	cd $(OE_BASE)/${BUILDDIR}; . ./env.source; bitbake dreambox-image

initialize: $(OE_BASE)/cache sources $(OE_BASE)/${BUILDDIR} $(OE_BASE)/${BUILDDIR}/conf \
	$(OE_BASE)/${BUILDDIR}/tmp $(OE_BASE)/openembedded $(OE_BASE)/${BUILDDIR}/conf/local.conf \
	$(OE_BASE)/${BUILDDIR}/env.source bb

openembedded-update: initialize
	cd $(OE_BASE)/openembedded && $(GIT) pull origin $(GIT_BRANCH)

$(OE_BASE)/${BUILDDIR} $(OE_BASE)/${BUILDDIR}/conf $(OE_BASE)/${BUILDDIR}/tmp $(OE_BASE)/cache sources:
	mkdir -p $@

$(OE_BASE)/${BUILDDIR}/conf/local.conf:
	echo 'DL_DIR = "$(PWD)/sources"' > $@
	echo 'OE_BASE = "$(OE_BASE)"' >> $@
	echo 'BBFILES = "$${OE_BASE}/openembedded/packages/*/*.bb"' >> $@
	echo 'BBMASK = "(nslu.*|.*-sdk.*)"' >> $@
	echo 'PREFERRED_PROVIDERS += " virtual/$${TARGET_PREFIX}gcc-initial:gcc-cross-initial"' >> $@
	echo 'PREFERRED_PROVIDERS += " virtual/$${TARGET_PREFIX}gcc:gcc-cross"' >> $@
	echo 'PREFERRED_PROVIDERS += " virtual/$${TARGET_PREFIX}g++:gcc-cross"' >> $@
	echo 'MACHINE = "$(MACHINE)"' >> $@
	echo 'TARGET_OS = "linux"' >> $@
	echo 'DISTRO = "opendreambox-openpli"' >> $@
	echo 'CACHE = "$(OE_BASE)/cache/oe-cache.$${MACHINE}.$${USER}"' >> $@

$(OE_BASE)/${BUILDDIR}/env.source:
	echo 'OE_BASE=$(OE_BASE)' > $@
	echo 'export BBPATH="$${OE_BASE}/openembedded/:$(PWD)/bb/:$${OE_BASE}/${BUILDDIR}/"' >> $@
	echo 'PATH=$(PWD)/bb/bin:$${OE_BASE}/${BUILDDIR}/tmp/cross/bin:$${PATH}' >> $@
	echo 'export PATH' >> $@
	echo 'export LD_LIBRARY_PATH=' >> $@
	echo 'export LANG=C' >> $@
	echo 'export CVS_RSH=ssh' >> $@
	echo 'umask 0022' >> $@
	cat $@

$(OE_BASE)/openembedded: $(OE_BASE)/openembedded/.git

$(OE_BASE)/openembedded/.git:
	$(GIT) clone -n $(GIT_URL) $(OE_BASE)/openembedded
	cd $(OE_BASE)/openembedded && ( \
		if [ -n "$(GIT_AUTHOR_EMAIL)" ]; then git config user.email "$(GIT_AUTHOR_EMAIL)"; fi; \
		if [ -n "$(GIT_AUTHOR_NAME)" ]; then git config user.name "$(GIT_AUTHOR_NAME)"; fi; \
		$(GIT) branch --track $(GIT_BRANCH) origin/$(GIT_BRANCH) || true; \
		$(GIT) checkout -f $(GIT_BRANCH) \
	)

