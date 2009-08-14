DEPENDS = "dreambox-dvbincludes tuxbox-libs"
RDEPENDS = "libtuxbox-mpegtools0"
DESCRIPTION = "tuxbox net streaming tools"
MAINTAINER = "Felix Domke <tmbinc@elitdvb.net>"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/cdk/apps/dvb/tools;module=stream;rev=${PLISVNREV};proto=${PLISVNPROTO} \
						file://acinclude.m4 \
						file://enable_transform.diff;patch=1;pnum=1 \
						file://add_configfiles.diff;patch=1;pnum=1"

S = "${WORKDIR}/stream"
PV = "${DISTRO_VERSION}"
PR = "r3"

inherit autotools pkgconfig

bindir = "/usr/bin"
sbindir = "/usr/sbin"

EXTRA_OECONF = "--with-target=native --with-boxtype=${MACHINE}"

CFLAGS_append = " -DHAVE_DREAMBOX_HARDWARE"

do_configure_prepend() {
	install ${WORKDIR}/acinclude.m4 ${S}/acinclude.m4
}

do_install_append() {
	ln -s streampes ${D}/usr/sbin/streames
}
