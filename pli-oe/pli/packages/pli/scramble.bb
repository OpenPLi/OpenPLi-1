PV = "${DISTRO_VERSION}"
PN = "scramble"
PR = "r0"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/cdk/apps/tuxbox/tools;module=${PN};rev=${PLISVNREV};proto=${PLISVNPROTO}"

S = "${WORKDIR}/scramble"

inherit autotools pkgconfig

do_install_append() {
	[ -e ${D}/usr/bin/descramble ] && rm ${D}/usr/bin/descramble
}
