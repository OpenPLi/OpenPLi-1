#!/usr/bin/make -f

# user config
MACHINE=dm7020
DISTRO=opendreambox-openpli

# for a list of some other repositories have
# a look at http://git.opendreambox.org/
GIT_URL=git://git.openembedded.net/openembedded

# in case you want to send pull requests or generate patches
#GIT_AUTHOR_NAME ?= Your Name
#GIT_AUTHOR_EMAIL ?= you@example.com

BB_VERSION=1.4.2
SVN=svn
SVN_BITBAKE_PARENT="svn://svn.berlios.de/bitbake/"

GIT=git
GIT_BRANCH=org.openembedded.dreambox

PWD := $(shell pwd)

#OE_BASE = $(PWD)/$(MACHINE)
OE_BASE = ${PWD}

all: initialize
	@echo 
	@echo "Openembedded for the Dreambox environment has been initialized"
	@echo "properly. Now you can either:"
	@echo
	@echo "  - make the 'image'-target to build an image, or"
	@echo "  - go into build/, source env.source and start on your own!"
	@echo 

bb:
	${SVN} checkout ${SVN_BITBAKE_PARENT}/tags/bitbake-${BB_VERSION} bb



.PHONY: image initialize openembedded-update openembedded-update-all

image: initialize openembedded-update
	cd ${OE_BASE}/build; . ./env.source; bitbake dreambox-image

initialize: $(OE_BASE)/cache sources $(OE_BASE)/build $(OE_BASE)/build/conf \
	$(OE_BASE)/build/tmp $(OE_BASE)/openembedded $(OE_BASE)/build/conf/local.conf \
	$(OE_BASE)/build/env.source bb

openembedded-update: $(OE_BASE)/openembedded
	cd $(OE_BASE)/openembedded && $(GIT) pull

openembedded-update-all:
	@for dir in dm*/openembedded; do \
		echo "running ${GIT} pull in $$dir"; \
		cd $$dir && ${GIT} pull && cd -; \
	done

$(OE_BASE)/build $(OE_BASE)/build/conf $(OE_BASE)/build/tmp $(OE_BASE)/cache sources:
	mkdir -p $@

$(OE_BASE)/build/conf/local.conf:
	echo 'DL_DIR = "${PWD}/sources"' > $@
	echo 'OE_BASE = "${PWD}"' >> $@
	echo 'BBFILES = "$${OE_BASE}/pli/packages/*/*.bb $${OE_BASE}/openembedded/packages/*/*.bb"' >> $@
	echo 'BBFILE_COLLECTIONS = "upstream pli"' >> $@
	echo 'BBFILE_PATTERN_upstream = "^$${OE_BASE}/openembedded/"' >> $@
	echo 'BBFILE_PATTERN_pli = "^$${OE_BASE}/pli/"' >> $@
	echo 'BBFILE_PRIORITY_upstream = "5"' >> $@
	echo 'BBFILE_PRIORITY_pli = "10"' >> $@
	echo >> $@
	echo 'BBMASK = "(nslu.*|.*-sdk.*)"' >> $@
	echo 'PREFERRED_PROVIDERS += " virtual/$${TARGET_PREFIX}gcc-initial:gcc-cross-initial"' >> $@
	echo 'PREFERRED_PROVIDERS += " virtual/$${TARGET_PREFIX}gcc:gcc-cross"' >> $@
	echo 'PREFERRED_PROVIDERS += " virtual/$${TARGET_PREFIX}g++:gcc-cross"' >> $@
	echo 'MACHINE = "${MACHINE}"' >> $@
	echo 'TARGET_OS = "linux"' >> $@
	echo 'DISTRO = "${DISTRO}"' >> $@
	echo 'CACHE = "${PWD}/cache/oe-cache.$${DISTRO}.$${MACHINE}"' >> $@

$(OE_BASE)/build/env.source:
	echo 'OE_BASE=${PWD}' > $@
	echo 'export BBPATH="$${OE_BASE}/pli/:$${OE_BASE}/openembedded/:$${OE_BASE}/bb/:$${OE_BASE}/build/"' >> $@
	echo 'PATH=$${PATH}:$${OE_BASE}/bb/bin:$${OE_BASE}/build/tmp/cross/bin' >> $@
	echo 'export PATH' >> $@
	echo 'export LD_LIBRARY_PATH=' >> $@
	echo 'export LANG=C' >> $@
	echo 'export CVS_RSH=ssh' >> $@
	echo 'umask 0022' >> $@
	cat $@

$(OE_BASE)/openembedded: $(OE_BASE)/openembedded/.git

$(OE_BASE)/openembedded/.git:
	@if [ -d $(OE_BASE)/openembedded/_MTN ]; then echo "Please remove your old monotone repository from $(OE_BASE)/openembedded!"; exit 1; fi
	$(GIT) clone -n $(GIT_URL) $(OE_BASE)/openembedded
	cd $(OE_BASE)/openembedded && ( \
		if [ -n "$(GIT_AUTHOR_EMAIL)" ]; then git config user.email "$(GIT_AUTHOR_EMAIL)"; fi; \
		if [ -n "$(GIT_AUTHOR_NAME)" ]; then git config user.name "$(GIT_AUTHOR_NAME)"; fi; \
		$(GIT) branch $(GIT_BRANCH) origin/$(GIT_BRANCH) || true; \
		$(GIT) checkout $(GIT_BRANCH) \
	)
