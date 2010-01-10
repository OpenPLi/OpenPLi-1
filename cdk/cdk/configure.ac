AC_INIT(Makefile.am)
AC_PREREQ(2.50)
AM_INIT_AUTOMAKE(tuxbox-cdk, 0.0.1)
AM_MAINTAINER_MODE

AC_PROG_CC

AC_CANONICAL_BUILD
AC_SUBST(build)
if test "$target_alias" = "" ; then
	target_alias="powerpc-tuxbox-linux-gnu"
fi
AC_CANONICAL_TARGET
AC_SUBST(target)

AC_ARG_ENABLE(libcrypto,
	AC_HELP_STRING([--disable-libcrypto], [.libcrypto will not be build by 'make all' (reduces the size of some other libs]),
	,[enable_libcrypto=yes])

AM_CONDITIONAL(WITH_LIBCRYPTO,test "$enable_libcrypto" = "yes")

AC_ARG_ENABLE(flashmodutils,
	AC_HELP_STRING([--disable-flashmodutils], [disable installation of modutils when targetruleset=flash (insmod lsmod rmmod are covered by busybox)]),
	,[enable_flashmodutils=yes])

AM_CONDITIONAL(WITH_FLASHMODUTILS,test "$enable_flashmodutils" = "yes")

AC_ARG_WITH(cpu,
	[  --with-cpu=MODEL        cpu model [[823,405]]],
	[CPU_MODEL="$withval"],[CPU_MODEL="405"])

AC_SUBST(CPU_MODEL)
AM_CONDITIONAL(CPU_MODEL_823,test "$CPU_MODEL" = "823")
AM_CONDITIONAL(CPU_MODEL_405,test "$CPU_MODEL" = "405")


AC_ARG_WITH(hostruleset,
	[  --with-hostruleset=NAME host ruleset [[standard,cygwin]]],
	[HOSTRULESET="$withval"],[HOSTRULESET="NULL"])

if test "$HOSTRULESET" = "NULL"; then
	if test "$host_os" = "cygwin"; then
		HOSTRULESET="cygwin"
	else
		HOSTRULESET="standard"
	fi
fi

if test "$HOSTRULESET" = "standard"; then
	MAKERULESETFILE=""
elif test "$HOSTRULESET" = "cygwin"; then
	MAKERULESETFILE="-cygwin"
else
	AC_MSG_ERROR([invalid hostruleset "$HOSTRULESET", choose on from standard or cygwin]);
fi
AC_ARG_WITH(ccachedir,
	[  --with-ccachedir=DIR directory where ccache is installed ],
	[ccachedir="$with_ccachedir"],[ccachedir="NONE"])
AC_SUBST(ccachedir)


AC_SUBST(HOSTRULESET)
AM_CONDITIONAL(HOSTRULESET_STANDARD,test "$HOSTRULESET" = "standard")
AM_CONDITIONAL(HOSTRULESET_CYGWIN,test "$HOSTRULESET" = "cygwin")

AC_ARG_WITH(boxtype,
	[  --with-boxtype=NAME dreambox type [[dm7000,dm56x0,dm500]]],
	[BOXTYPE="$withval"],[BOXTYPE="dm7000"])

AC_SUBST(BOXTYPE)
AM_CONDITIONAL(BOXTYPE_DM7000,test "$BOXTYPE" = "dm7000")
AM_CONDITIONAL(BOXTYPE_DM56x0,test "$BOXTYPE" = "dm56x0")
AM_CONDITIONAL(BOXTYPE_DM500,test "$BOXTYPE" = "dm500")

AC_ARG_WITH(maxvar,
[  --with-maxvar          create a maxvar image (default off)],
[MAXVAR="$withval"],[MAXVAR="no"])

AM_CONDITIONAL(MAXVAR,test "$MAXVAR" = "yes")

AC_ARG_WITH(libtoolize-version,
	[  --with-libtoolize-version=NAME libtoolize version [[1,2]]],
	[LIBTOOLIZEVERSION="$withval"],[LIBTOOLIZEVERSION="1"])

AC_SUBST(LIBTOOLIZEVERSION)
AM_CONDITIONAL(LIBTOOLIZEVERSION_1,test "$LIBTOOLIZEVERSION" = 1)
AM_CONDITIONAL(LIBTOOLIZEVERSION_2,test "$LIBTOOLIZEVERSION" = 2)

AC_ARG_WITH(webif,
	[  --with-webif=NAME dreambox webif [[standard,expert]]],
	[WEBIF="$withval"],[WEBIF="standard"])

AC_SUBST(WEBIF)
AM_CONDITIONAL(WEBIF_STANDARD,test "$WEBIF" = "standard")
AM_CONDITIONAL(WEBIF_EXPERT,test "$WEBIF" = "expert")

AC_ARG_WITH(epg,
	[  --with-epg=NAME dreambox epg [[standard,private]]],
	[EPG="$withval"],[EPG="standard"])

AC_SUBST(EPG)
AM_CONDITIONAL(EPG_STANDARD,test "$EPG" = "standard")
AM_CONDITIONAL(EPG_PRIVATE,test "$EPG" = "private")

AC_ARG_WITH(busybox,
	[  --with-busybox=NAME dreambox busybox [[1.0,1.2,1.7,1.13]]],
	[BUSYBOX="$withval"],[BUSYBOX="1.0"])
AC_SUBST(BUSYBOX)
AM_CONDITIONAL(BUSYBOX_12,test "$BUSYBOX" = "1.2")
AM_CONDITIONAL(BUSYBOX_17,test "$BUSYBOX" = "1.7")
AM_CONDITIONAL(BUSYBOX_113,test "$BUSYBOX" = "1.13")

AC_ARG_WITH(freetype,
	[  --with-freetype=NAME dreambox freetype [[2.0,2.1]]],
	[FREETYPE="$withval"],[FREETYPE="2.0"])
AC_SUBST(FREETYPE)
AM_CONDITIONAL(FREETYPE_21,test "$FREETYPE" = "2.1")


AC_ARG_WITH(mhw-epg,
	[  --with-mhw-epg          enable capture of mhw epg (default off)],
	[MHW_EPG="$withval"],[MHW_EPG="no"])

AC_SUBST(MHW_EPG)
AM_CONDITIONAL(MHW_EPG_NO,test "$MHW_EPG" = "no")
AM_CONDITIONAL(MHW_EPG_YES,test "$MHW_EPG" = "yes")

AC_ARG_WITH(chinese,
	[  --with-chinese          create a chinese image (default off)],
	[CHINESE="$withval"],[CHINESE="no"])

AC_SUBST(CHINESE)
AM_CONDITIONAL(CHINESE_NO,test "$CHINESE" = "no")
AM_CONDITIONAL(CHINESE_YES,test "$CHINESE" = "yes")


AC_ARG_WITH(dish-epg,
	[  --with-dish-epg          enable capture of dish epg (default off)],
	[DISH_EPG="$withval"],[DISH_EPG="no"])

AC_SUBST(DISH_EPG)
AM_CONDITIONAL(DISH_EPG_NO,test "$DISH_EPG" = "no")
AM_CONDITIONAL(DISH_EPG_YES,test "$DISH_EPG" = "yes")

AC_ARG_WITH(flashtool,
	[  --with-flashtool=NAME dreambox flashtool [[standard,expert]]],
	[FLASHTOOL="$withval"],[FLASHTOOL="standard"])

AC_SUBST(FLASHTOOL)
AM_CONDITIONAL(FLASHTOOL_STANDARD,test "$FLASHTOOL" = "standard")
AM_CONDITIONAL(FLASHTOOL_EXPERT,test "$FLASHTOOL" = "expert")

AC_ARG_WITH(reiserfs,
	[  --with-reiserfs=NAME dreambox reiserfs [[yes,no]]],
	[REISERFS="$withval"],[REISERFS="no"])

AC_SUBST(REISERFS)
AM_CONDITIONAL(REISERFS_NO,test "$REISERFS" = "no")
AM_CONDITIONAL(REISERFS_YES,test "$REISERFS" = "yes")

AC_ARG_WITH(ext-flashtool,
	[  --with-ext-flashtool=NAME dreambox ext-flashtool [[yes,no]]],
	[EXTFLASHTOOL="$withval"],[EXTFLASHTOOL="yes"])

AC_ARG_WITH(enigma-debug,
	[  --with-enigma-debug=NAME dreambox enigma-debug [[yes,no]]],
	[ENIGMA_DEBUG="$withval"],[ENIGMA_DEBUG="yes"])

AC_SUBST(ENIGMA_DEBUG)
AM_CONDITIONAL(ENIGMA_DEBUG_NO,test "$ENIGMA_DEBUG" = "no")
AM_CONDITIONAL(ENIGMA_DEBUG_YES,test "$ENIGMA_DEBUG" = "yes")

AC_SUBST(EXTFLASHTOOL)
AM_CONDITIONAL(EXTFLASHTOOL_NO,test "$EXTFLASHTOOL" = "no")
AM_CONDITIONAL(EXTFLASHTOOL_YES,test "$EXTFLASHTOOL" = "yes")

AC_ARG_WITH(targetruleset,
	[  --with-targetruleset=NAME target ruleset [[standard,flash]]],
	[TARGETRULESET="$withval"],[TARGETRULESET="standard"])

if test "$TARGETRULESET" = "standard"; then
	INSTALLRULESETFILE=""
elif test "$TARGETRULESET" = "flash"; then
	INSTALLRULESETFILE="-flash"
else
	AC_MSG_ERROR([invalid targetruleset "$TARGETRULESET", choose on from standard or flash]);
fi

AC_SUBST(TARGETRULESET)
AM_CONDITIONAL(TARGETRULESET_STANDARD,test "$TARGETRULESET" = "standard")
AM_CONDITIONAL(TARGETRULESET_FLASH,test "$TARGETRULESET" = "flash")

if test -z "$CFLAGS" ; then
	CFLAGS="-O2"
fi

TARGET_CFLAGS="-mcpu=$CPU_MODEL -msoft-float -mmultiple -mstring -meabi -pipe -Os -ggdb3 -DHAVE_DREAMBOX_HARDWARE"
if test "$BOXTYPE" = "dm500" ; then
	TARGET_CFLAGS="$TARGET_CFLAGS -DHAVE_DREAMBOX_DM500"
fi
TARGET_LDFLAGS=""
TARGET_CXXFLAGS="$TARGET_CFLAGS"

AC_SUBST(CFLAGS)
AC_SUBST(TARGET_CFLAGS)
AC_SUBST(TARGET_CXXFLAGS)
AC_SUBST(TARGET_LDFLAGS)

AC_PREFIX_DEFAULT(/dbox2)

if test "$prefix" = "NONE"; then
	prefix=/dbox2
fi

AC_ARG_WITH(targetprefix,
	[  --with-targetprefix=DIR prefix for target files [[PREFIX/cdkroot]]],
	[targetprefix="$with_targetprefix"],[targetprefix="${prefix}/cdkroot"])

AC_ARG_WITH(hostprefix,
	[  --with-hostprefix=DIR   prefix for host files [[PREFIX/cdk]]],
	[hostprefix="$with_hostprefix"],[hostprefix="${prefix}/cdk"])

AC_ARG_WITH(bootprefix,
	[  --with-bootprefix=DIR   prefix for boot files [[PREFIX/tftpboot]]],
	[bootprefix="$with_bootprefix"],[bootprefix="${prefix}/tftpboot"])

AC_ARG_WITH(flashprefix,
	[  --with-flashprefix=DIR  prefix for flash files [[PREFIX/cdkflash]] (only used for flash building)],
	[flashprefix="$with_flashprefix"],[flashprefix="${prefix}/cdkflash"])

AC_ARG_WITH(downloadserverprefix,
	[  --with-downloadserverprefix=DIR prefix for download server files [[PREFIX/downloadserver]]],
	[downloadserverprefix="$with_downloadserverprefix"],[downloadserverprefix="${prefix}/downloadserver"])

buildprefix=`pwd`
AC_SUBST(targetprefix)
AC_SUBST(hostprefix)
AC_SUBST(buildprefix)
AC_SUBST(bootprefix)
AC_SUBST(flashprefix)
AC_SUBST(downloadserverprefix)
flashprefix_ro=${flashprefix}/ro
AC_SUBST(flashprefix_ro)
flashprefix_rw=${flashprefix}/rw
AC_SUBST(flashprefix_rw)

AC_ARG_WITH(cvsdir,
	[  --with-cvsdir=DIR       where to find the cvs],
	[cvsdir="$with_cvsdir"],[cvsdir="NONE"])

AC_ARG_WITH(appsdir,
	[  --with-appsdir=DIR      apps dir from cvs [[[CVS/]apps/]]],
	[appsdir="$with_appsdir"],[appsdir="NONE"])

AC_ARG_WITH(bootdir,
	[  --with-bootdir=DIR      boot dir from cvs [[[CVS/]boot/]]],
	[bootdir="$with_bootdir"],[bootdir="NONE"])

AC_ARG_WITH(driverdir,
	[  --with-driverdir=DIR    driver dir from cvs [[[CVS/]driver]]],
	[driverdir="$with_driverdir"],[driverdir="NONE"])

AC_ARG_WITH(hostapsdir,
	[  --with-hostappsdir=DIR  hostapps dir from cvs [[[CVS/]hostapps]]],
	[hostappsdir="$with_driverdir"],[hostappsdir="NONE"])

AC_ARG_WITH(gnuserver,
	[  --with-gnuserver=ADDRESS   the gnu server for gnu-stuff (without ftp://)],
	[gnuserver="$with_gnuserver"],[gnuserver="NONE"])

if test "$cvsdir" = "NONE"; then
	if test "$appsdir" = "NONE"; then
		appsdir="$buildprefix/apps"
	fi
	if test "$bootdir" = "NONE"; then
		bootdir="$buildprefix/boot"
	fi
	if test "$driverdir" = "NONE"; then
		driverdir="$buildprefix/driver"
	fi
	if test "$hostappsdir" = "NONE"; then
		hostappsdir="$buildprefix/hostapps"
	fi
else
	if test "$appsdir" = "NONE"; then
		appsdir="$cvsdir/apps"
	fi
	if test "$bootdir" = "NONE"; then
		bootdir="$cvsdir/boot"
	fi
	if test "$driverdir" = "NONE"; then
		driverdir="$cvsdir/driver"
	fi
	if test "$hostappsdir" = "NONE"; then
		hostappsdir="$cvsdir/hostapps"
	fi
fi

if test "$gnuserver" = "NONE"; then
	gnuserver="ftp.gnu.org/gnu"
fi

AC_MSG_CHECKING([for apps directory])
if test ! -f "$appsdir/dvb/zapit/configure.ac" ; then
	appsdir=no
	AC_MSG_RESULT([no])
else
	AC_MSG_RESULT([yes])
fi

AC_MSG_CHECKING([for boot directory])
if test ! -f "$bootdir/ppcboot/Makefile" ; then
	bootdir=no
	AC_MSG_RESULT([no])
else
	AC_MSG_RESULT([yes])
fi

AC_MSG_CHECKING([for driver directory])
if test ! -f "$driverdir/Makefile" ; then
	driverdir=no
	AC_MSG_RESULT([no])
else
	AC_MSG_RESULT([yes])
fi

AC_MSG_CHECKING([for hostapps directory])
if test ! -f "$hostappsdir/configure.ac" ; then
	hostappsdir=no
	AC_MSG_RESULT([no])
else
	AC_MSG_RESULT([yes])
fi

AC_SUBST(appsdir)
AC_SUBST(bootdir)
AC_SUBST(driverdir)
AC_SUBST(hostappsdir)
AC_SUBST(gnuserver)

if test "$TARGETRULESET" = "flash"; then
	AC_PATH_PROG(ZIP,zip,no)
	if test "$ZIP" = "no"; then
		AC_MSG_WARN(can't compress with zip)
		ZIP=/bin/false
	fi
	AC_PATH_PROG(PYTHON,python,no)
	if test "$PYTHON" = "no"; then
		AC_MSG_WARN(can't build lib dir)
		PYTHON=/bin/false
	fi
	AC_PATH_PROG(FAKEROOT,fakeroot,no)
	if test "$FAKEROOT" = "no"; then
		AC_MSG_WARN(can't build images with uid 0 as non-root)
		FAKEROOT=
	fi
	AC_PATH_PROG(MKCRAMFS,mkcramfs,no,"$PATH:/usr/sbin")
	if test "$MKCRAMFS" = "no"; then
		AC_MSG_WARN(can't build cramfs)
		MKCRAMFS=/bin/false
	fi
	AC_PATH_PROG(MKJFFS2,mkjffs2,no)
	if test "$MKJFFS2" = "no"; then
		AC_PATH_PROG(MKFSJFFS2,mkfs.jffs2,no)
		if test "$MKFSJFFS2" = "no"; then
			AC_MSG_WARN(can't build jffs2)
			MKJFFS2=/bin/false
		else
			MKJFFS2=$MKFSJFFS2
		fi
	fi
fi

FONTDIR="/share/fonts"
AC_SUBST(FONTDIR)
UCODEDIR="/var/tuxbox/ucodes"
AC_SUBST(UCODEDIR)

CLEANUP="rm -rf"
AC_SUBST(CLEANUP)

#
# core
#
AC_SUBST(KERNELVERSION)
TUXBOX_RULES_MAKE(linux)
TUXBOX_RULES_MAKE(linux_libc_headers)
KERNELVERSION=$VERSION_linux
TUXBOX_RULES_MAKE_EXDIR(binutils)
TUXBOX_RULES_MAKE_EXDIR(bootstrap_gcc)
TUXBOX_RULES_MAKE_EXDIR(glibc)
TUXBOX_RULES_MAKE_EXDIR(gcc)
TUXBOX_RULES_MAKE(uclibc)
TUXBOX_RULES_MAKE(squashfs)

#
# root
#
TUXBOX_RULES_MAKE(busybox)
TUXBOX_RULES_MAKE(busybox12)
TUXBOX_RULES_MAKE(busybox17)
TUXBOX_RULES_MAKE(busybox113)
TUXBOX_RULES_MAKE(ftpd)
TUXBOX_RULES_MAKE(modutils)
TUXBOX_RULES_MAKE(portmap)
TUXBOX_RULES_MAKE(procps)
TUXBOX_RULES_MAKE(procps3)
TUXBOX_RULES_MAKE(watchdog)
TUXBOX_RULES_MAKE(mrouted)

#
# contrib libs
#
TUXBOX_RULES_MAKE(libdirectfb)
TUXBOX_RULES_MAKE(libdirectfbpp)
TUXBOX_RULES_MAKE(libppdirectfb)
TUXBOX_RULES_MAKE(libboost)
TUXBOX_RULES_MAKE(libcommoncplusplus)
TUXBOX_RULES_MAKE(libcrypto)
TUXBOX_RULES_MAKE(libcurl)
TUXBOX_RULES_MAKE(libdb2)
TUXBOX_RULES_MAKE(libdvbpsi)
TUXBOX_RULES_MAKE(libevent)
TUXBOX_RULES_MAKE(libffi)
TUXBOX_RULES_MAKE(libfreetype)
TUXBOX_RULES_MAKE(libfreetype21)
TUXBOX_RULES_MAKE(libfribidi)
TUXBOX_RULES_MAKE(libgmp)
TUXBOX_RULES_MAKE(libmad)
TUXBOX_RULES_MAKE(libid3tag)
TUXBOX_RULES_MAKE(libncurses)
TUXBOX_RULES_MAKE(libnet)
TUXBOX_RULES_MAKE(libnids)
TUXBOX_RULES_MAKE(libjpeg)
TUXBOX_RULES_MAKE(libpcap)
TUXBOX_RULES_MAKE(libpng)
TUXBOX_RULES_MAKE(libpopt)
TUXBOX_RULES_MAKE(libreadline)
TUXBOX_RULES_MAKE(libsdl)
TUXBOX_RULES_MAKE(libsdl_mixer)
TUXBOX_RULES_MAKE(libsdl_sound)
TUXBOX_RULES_MAKE(libsdl_image)
TUXBOX_RULES_MAKE(libsdl_net)
TUXBOX_RULES_MAKE(libsigc)
TUXBOX_RULES_MAKE(libungif)
TUXBOX_RULES_MAKE(libupnp)
TUXBOX_RULES_MAKE(libvorbisidec)
TUXBOX_RULES_MAKE(libflac)
TUXBOX_RULES_MAKE(libxml2)
TUXBOX_RULES_MAKE(libxmlparse)
TUXBOX_RULES_MAKE(libxmlwrapp)
TUXBOX_RULES_MAKE(libz)

#
# contrib apps
#
TUXBOX_RULES_MAKE(console_data)
TUXBOX_RULES_MAKE(console_tools)
TUXBOX_RULES_MAKE(dsniff)
TUXBOX_RULES_MAKE(fbset)
TUXBOX_RULES_MAKE(lirc)
TUXBOX_RULES_MAKE(lsof)
TUXBOX_RULES_MAKE(polipo)
TUXBOX_RULES_MAKE(pump)
TUXBOX_RULES_MAKE(ssh)
TUXBOX_RULES_MAKE(tcpdump)
TUXBOX_RULES_MAKE(tor)
TUXBOX_RULES_MAKE(ushare)
TUXBOX_RULES_MAKE(xrc)
TUXBOX_RULES_MAKE(thttpd)
TUXBOX_RULES_MAKE(e2fsprogs)
TUXBOX_RULES_MAKE(reiserfsprogs)
TUXBOX_RULES_MAKE(utillinux)
TUXBOX_RULES_MAKE(samba)
TUXBOX_RULES_MAKE(samba3)
TUXBOX_RULES_MAKE(etherwake)
TUXBOX_RULES_MAKE(dreamfiles)
TUXBOX_RULES_MAKE(dreamdriver_dm7000)
TUXBOX_RULES_MAKE(dreamdriver_dm56x0)
TUXBOX_RULES_MAKE(dreamdriver_dm500)
TUXBOX_RULES_MAKE(automount)
TUXBOX_RULES_MAKE(nfs_utils)
TUXBOX_RULES_MAKE(less)
TUXBOX_RULES_MAKE(diffutils)
TUXBOX_RULES_MAKE(sqlite)
TUXBOX_RULES_MAKE(dropbear)
TUXBOX_RULES_MAKE(fuse)
TUXBOX_RULES_MAKE(djmount)

#
# additional
#
TUXBOX_RULES_MAKE(gdb)
TUXBOX_RULES_MAKE(insight)
TUXBOX_RULES_MAKE(ltrace)
TUXBOX_RULES_MAKE(strace)
TUXBOX_RULES_MAKE(nano)
TUXBOX_RULES_MAKE(joe)

#
# java
#
TUXBOX_RULES_MAKE(kaffe)
TUXBOX_RULES_MAKE(kaffeh)

#
# fun stuff
#
TUXBOX_RULES_MAKE(gnuboy)
TUXBOX_RULES_MAKE(scummvm)
TUXBOX_RULES_MAKE(sdldoom)
TUXBOX_RULES_MAKE(wxbase)
TUXBOX_RULES_MAKE(amule)
TUXBOX_RULES_MAKE(ctorrent)

#
# dvb apps
#
TUXBOX_RULES_MAKE(dvbdate)
TUXBOX_RULES_MAKE(dvbstream)
TUXBOX_RULES_MAKE(dvbtext)
TUXBOX_RULES_MAKE(dvbtune)
TUXBOX_RULES_MAKE(vls)

#
# internal
#
AC_SUBST_FILE(ARCHIVE)
ARCHIVE=Makefile-archive
AC_CONFIG_COMMANDS_PRE([
AC_MSG_NOTICE([creating Makefile-archive])
${srcdir}/rules-archive.pl ${srcdir}/rules-archive > Makefile-archive
])

#
# summary
#
AC_MSG_RESULT([
summary:
----------------------------------------
target cpu:		$CPU_MODEL
target box:		$BOXTYPE
webif type:		$WEBIF
epg:			$EPG
mhw-epg:		$MHW_EPG
dish-epg:		$DISH_EPG
flashtool:		$FLASHTOOL
external flashtool:	$EXTFLASHTOOL
reiserfs:		$REISERFS
enigma debug:		$ENIGMA_DEBUG
host rule set:		$HOSTRULESET
target rule set:	$TARGETRULESET
target prefix:		$targetprefix
host prefix:		$hostprefix
boot prefix:		$bootprefix
build prefix:		$buildprefix
target apps directory:	$appsdir
boot directory:		$bootdir
driver directory:	$driverdir
host apps directory:	$hostappsdir
GNU server:		$gnuserver
ccache:			$ccachedir
])

AC_OUTPUT([
Makefile
root/Makefile
root/etc/Makefile
root/etc/init.d/Makefile
root/share/Makefile
root/share/tuxbox/Makefile
root/share/udhcpc/Makefile
root/var_init/Makefile
root/var_init/etc/Makefile
root/var_init/tuxbox/Makefile
root/var_init/tuxbox/config/Makefile
root/var_init/tuxbox/config/enigma/Makefile
])
