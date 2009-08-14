DEPENDS = "enigma"
RDEPENDS = "enigma ipkg"
DESCRIPTION = "Enigma Plugin IPKG"
MAINTAINER = "Felix Domke <tmbinc@elitedvb.net>"
LICENSE = "GPL"

PV = "${DISTRO_VERSION}"
PN = "ipkgpl"
PR = "r0"

inherit svnrev

SRC_URI =  "${PLISVNURL}/${PLISVNBRANCH}/cdk/apps/tuxbox/plugins/enigma;module=${PN};rev=${PLISVNREV};proto=${PLISVNPROTO}"

S = "${WORKDIR}/ipkgpl"

FILES_${PN} =  "/usr/lib/tuxbox/plugins/ipkgpl.so \
		/usr/lib/tuxbox/plugins/ipkgpl.cfg"

inherit autotools pkgconfig

EXTRA_OECONF = "--with-target=native "

do_install() {
	install -d ${D}/usr/lib/tuxbox/plugins
	install ${S}/.libs/ipkgpl.so ${D}/usr/lib/tuxbox/plugins
	install -m 0644 ${S}/ipkgpl.cfg ${D}/usr/lib/tuxbox/plugins
}
