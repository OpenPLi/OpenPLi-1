#!/bin/bash
#
# example driver build script
# modify to suit your setup
#
# usage:
#   choose one or more of the following, depending on your intention:
#
#    ./build.sh
#    ./build.sh dep
#    ./build.sh install
#    ./build.sh clean
#    ./build.sh distclean
#
#   view Makefile to see what the different targets do.
#

# TODO: create some kind of tuxboxcdk-config script and use it
CDKROOT=${CDKPREFIX}/cdkroot
PATH=${CDKPREFIX}/cdk/bin:${PATH}

CVSROOT=${CVSROOT:-${HOME}/cvs/tuxbox}
MAKE=/usr/bin/make

${MAKE} ${1} \
	ARCH=ppc \
	CROSS_COMPILE=powerpc-tuxbox-linux-gnu- \
	KERNEL_LOCATION=${CVSROOT}/cdk/linux \
	INSTALL_MOD_PATH=${CDKROOT}

