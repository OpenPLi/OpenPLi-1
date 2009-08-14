DEPENDS = "sqlite3 tuxbox-libs"

PV = "${DISTRO_VERSION}"
PR = "r0"

inherit svnrev

SRC_URI = "${PLISVNURL}/${PLISVNBRANCH}/cdk/apps/tuxbox/tools;module=db_epg;rev=${PLISVNREV};proto=${PLISVNPROTO}"

S = "${WORKDIR}/db_epg"

inherit autotools pkgconfig
